#
# Copyright (c) .NET Foundation and Contributors
# See LICENSE file in the project root for full license information.
#

include(FetchContent)
FetchContent_GetProperties(chibios)

# from os/common/startup/ARMCMx/compilers/GCC/mk/startup_stm32h5xx.mk
# STM32H5xx has a Cortex-M33 core, so the ChibiOS RT port for ARMv8-M mainline is used

################################################################################################
# WHEN ADDING A NEW BOARD add the source code file for the hal/ports/STM32/STM32H5xx/platform.mk
################################################################################################

set(CHIBIOS_PORT_SRCS

    # RT
    chcore.c
    chcoreasm.S
)

foreach(SRC_FILE ${CHIBIOS_PORT_SRCS})

    set(CHIBIOS_H5_SRC_FILE SRC_FILE-NOTFOUND)

    find_file(CHIBIOS_H5_SRC_FILE ${SRC_FILE}
        PATHS

            ${chibios_SOURCE_DIR}/os/common/ports/ARMv8-M-ML-ALT/compilers/GCC
            ${chibios_SOURCE_DIR}/os/common/ports/ARMv8-M-ML-ALT

        CMAKE_FIND_ROOT_PATH_BOTH
    )

    if (BUILD_VERBOSE)
        message("${SRC_FILE} >> ${CHIBIOS_H5_SRC_FILE}")
    endif()

    list(APPEND CHIBIOS_SOURCES ${CHIBIOS_H5_SRC_FILE})

endforeach()

list(APPEND CHIBIOS_INCLUDE_DIRS ${chibios_SOURCE_DIR}/os/common/ports/ARMv8-M-ML-ALT)
