/*
 * Port replacement for WICED SDK's wwd_network.c (lwIP network glue)
 *
 * The SDK version uses WICED-custom lwIP extensions and a bundled lwIP 2.0.3.
 * This implementation works with standard lwIP 2.1.x as used by nanoFramework/ChibiOS.
 *
 * Provides:
 *  - host_network_process_ethernet_data()  — RX path: WICED core → lwIP
 *  - ethernetif_init()                      — lwIP netif init callback
 *  - low_level_output()                     — TX path: lwIP → WICED core
 *  - host_network_set_ethertype_filter()    — ethertype filter registration
 */

#include "wwd_network.h"
#include "wwd_buffer_interface.h"
#include "wwd_network_interface.h"
#include "wwd_network_constants.h"
#include "wwd_wifi.h"
#include "wwd_constants.h"
#include "wwd_assert.h"
#include "wwd_management.h"

#include "lwip/tcpip.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"
#include "lwip/etharp.h"
#include "lwip/stats.h"
#include "netif/ethernet.h"

#if LWIP_IPV6
#include "lwip/ethip6.h"
#endif

#include <string.h>

/* EAPOL ethertype */
#define ETHTYPE_EAPOL  0x888E

/* Maximum number of ethertype filters per interface */
#define MAX_ETHERTYPE_FILTERS  4

/* Per-interface ethertype filter entry */
typedef struct
{
    uint16_t ethertype;
    wwd_network_filter_ethernet_packet_t callback;
    void *userdata;
} ethertype_filter_t;

static ethertype_filter_t ethertype_filters[WWD_INTERFACE_MAX][MAX_ETHERTYPE_FILTERS];

/* Forward declarations */
static err_t low_level_output(struct netif *netif, struct pbuf *p);

/*
 * ethernetif_init — lwIP netif init callback for WICED WiFi interfaces.
 *
 * Called by lwIP's netif_add(). Sets up the interface name, MAC, MTU,
 * flags, and output hooks.
 */
err_t ethernetif_init(struct netif *netif)
{
    wiced_mac_t mac;
    wwd_interface_t interface;

    /* netif->state carries the wwd_interface_t (set by caller of netif_add) */
    interface = (wwd_interface_t)(intptr_t)netif->state;

    /* Set the interface name for lwIP (wi0, wi1, ...) */
    netif->name[0] = 'w';
    netif->name[1] = 'i';

    /* Set output hooks */
    netif->output = etharp_output;
#if LWIP_IPV6
    netif->output_ip6 = ethip6_output;
#endif
    netif->linkoutput = low_level_output;

    /* Get MAC address from the WiFi chip */
    if (wwd_wifi_get_mac_address(&mac, interface) != WWD_SUCCESS)
    {
        /* If we can't get the MAC, use a placeholder — this shouldn't happen */
        memset(netif->hwaddr, 0, 6);
    }
    else
    {
        memcpy(netif->hwaddr, &mac, 6);
    }
    netif->hwaddr_len = 6;

    /* Set MTU and flags */
    netif->mtu = (u16_t)WICED_PAYLOAD_MTU;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP |
                   NETIF_FLAG_IGMP;
#if LWIP_IPV6 && LWIP_IPV6_MLD
    netif->flags |= NETIF_FLAG_MLD6;
#endif

    return ERR_OK;
}

/*
 * low_level_output — lwIP TX callback.
 *
 * Called by lwIP when it has a packet to send. Bumps the reference count
 * on the pbuf (WICED core frees it after transmission), then hands it
 * to the WICED WiFi driver.
 */
static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
    wwd_interface_t interface;

    if (p == NULL)
    {
        return ERR_ARG;
    }

    interface = (wwd_interface_t)(intptr_t)netif->state;

    /* Check if WiFi is ready */
    if (wwd_wifi_is_ready_to_transceive(interface) != WWD_SUCCESS)
    {
        return ERR_CONN;
    }

    /* WICED core will free the pbuf after TX, so bump ref count */
    pbuf_ref(p);

    LINK_STATS_INC(link.xmit);

    wwd_network_send_ethernet_data(p, interface);

    return ERR_OK;
}

/*
 * host_network_process_ethernet_data — RX path from WICED core.
 *
 * Called by the WICED WiFi driver when an Ethernet frame is received
 * from the WiFi chip. Dispatches to the appropriate lwIP netif via
 * tcpip_input(), or to EAPOL handler, or to registered ethertype filters.
 */
void host_network_process_ethernet_data(wiced_buffer_t buffer, wwd_interface_t interface)
{
    struct netif *tmp_netif;
    uint16_t ethertype;
    struct eth_hdr *ethhdr;
    int i;

    if (buffer == NULL)
    {
        return;
    }

    /* Extract ethertype from the Ethernet header */
    ethhdr = (struct eth_hdr *)host_buffer_get_current_piece_data_pointer(buffer);
    ethertype = lwip_htons(ethhdr->type);

    /* Handle 802.1Q VLAN tag — skip 4 bytes to get real ethertype */
    if (ethertype == 0x8100) /* ETHTYPE_VLAN */
    {
        uint8_t *payload = (uint8_t *)ethhdr;
        ethertype = (uint16_t)((payload[16] << 8) | payload[17]);
    }

    /* Check ethertype filters first */
    if (interface < WWD_INTERFACE_MAX)
    {
        for (i = 0; i < MAX_ETHERTYPE_FILTERS; i++)
        {
            if (ethertype_filters[interface][i].ethertype == ethertype &&
                ethertype_filters[interface][i].callback != NULL)
            {
                ethertype_filters[interface][i].callback(buffer,
                    ethertype_filters[interface][i].userdata);
                return;
            }
        }
    }

    /* Handle EAPOL packets (802.1X authentication) */
    if (ethertype == ETHTYPE_EAPOL)
    {
        extern void wwd_eapol_receive_eapol_packet(wiced_buffer_t buffer,
                                                   wwd_interface_t interface);
        wwd_eapol_receive_eapol_packet(buffer, interface);
        return;
    }

    /* Find the lwIP netif for this WICED interface */
    for (tmp_netif = netif_list; tmp_netif != NULL; tmp_netif = tmp_netif->next)
    {
        if ((wwd_interface_t)(intptr_t)tmp_netif->state == interface)
        {
            /* Pass to lwIP's TCP/IP thread */
            if (tcpip_input(buffer, tmp_netif) != ERR_OK)
            {
                host_buffer_release(buffer, WWD_NETWORK_RX);
            }
            return;
        }
    }

    /* No matching interface found — drop the packet */
    host_buffer_release(buffer, WWD_NETWORK_RX);
}

/*
 * host_network_set_ethertype_filter — register a filter for a specific ethertype.
 *
 * Used by the WICED SDK for special protocol handling (e.g., custom management frames).
 * Pass callback=NULL to remove a filter.
 */
void host_network_set_ethertype_filter(uint16_t ethertype, wwd_interface_t interface,
                                       wwd_network_filter_ethernet_packet_t callback,
                                       void *userdata)
{
    int i;

    if (interface >= WWD_INTERFACE_MAX)
    {
        return;
    }

    /* If removing, find and clear */
    if (callback == NULL)
    {
        for (i = 0; i < MAX_ETHERTYPE_FILTERS; i++)
        {
            if (ethertype_filters[interface][i].ethertype == ethertype)
            {
                ethertype_filters[interface][i].ethertype = 0;
                ethertype_filters[interface][i].callback = NULL;
                ethertype_filters[interface][i].userdata = NULL;
                return;
            }
        }
        return;
    }

    /* Find empty slot */
    for (i = 0; i < MAX_ETHERTYPE_FILTERS; i++)
    {
        if (ethertype_filters[interface][i].callback == NULL)
        {
            ethertype_filters[interface][i].ethertype = ethertype;
            ethertype_filters[interface][i].callback = callback;
            ethertype_filters[interface][i].userdata = userdata;
            return;
        }
    }
}
