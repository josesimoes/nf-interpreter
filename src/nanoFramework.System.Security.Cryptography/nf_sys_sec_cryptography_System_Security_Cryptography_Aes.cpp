//
// Copyright (c) .NET Foundation and Contributors
// See LICENSE file in the project root for full license information.
//

#include "nf_sys_sec_cryptography.h"

HRESULT Library_nf_sys_sec_cryptography_System_Security_Cryptography_Aes::EncryptAesEcb___SZARRAY_U1__SZARRAY_U1(
    CLR_RT_StackFrame &stack)
{
    NANOCLR_HEADER();

    mbedtls_aes_context encodeContext;
    uint16_t passCount = 0;

    CLR_RT_HeapBlock_Array *keyArray;
    CLR_RT_HeapBlock_Array *plainTextArray;
    CLR_RT_HeapBlock_Array *cipherTextArray;
    CLR_RT_HeapBlock *pThis;

    pThis = (CLR_RT_HeapBlock *)stack.This();
    FAULT_ON_NULL(pThis);

    // grab key and check for NULL
    keyArray = pThis[FIELD___key].DereferenceArray();
    if (keyArray == NULL)
    {
        NANOCLR_SET_AND_LEAVE(CLR_E_INVALID_OPERATION);
    }

    // grab plain text and check for NULL
    plainTextArray = stack.Arg1().DereferenceArray();
    FAULT_ON_NULL_ARG(plainTextArray);

    // data has to be multiple of 16
    if (plainTextArray->m_numOfElements % 16 != 0)
    {
        NANOCLR_SET_AND_LEAVE(CLR_E_INVALID_PARAMETER);
    }

    // create the return array (same length as the input)
    stack.PushValueAndClear();
    NANOCLR_CHECK_HRESULT(CLR_RT_HeapBlock_Array::CreateInstance(
        stack.TopValue(),
        plainTextArray->m_numOfElements,
        g_CLR_RT_WellKnownTypes.m_UInt8));
    cipherTextArray = stack.TopValue().DereferenceArray();

    // init mbedtls context
    mbedtls_aes_init(&encodeContext);

    if (mbedtls_aes_setkey_enc(&encodeContext, keyArray->GetFirstElement(), keyArray->m_numOfElements * 8) ==
        MBEDTLS_ERR_AES_INVALID_KEY_LENGTH)
    {
        NANOCLR_SET_AND_LEAVE(CLR_E_INVALID_PARAMETER);
    }

    // itereate through the plain text array, encrypting 16 bytes at a time
    while (passCount < plainTextArray->m_numOfElements / 16)
    {
        if (mbedtls_aes_crypt_ecb(
                &encodeContext,
                MBEDTLS_AES_ENCRYPT,
                plainTextArray->GetElement(passCount * 16),
                cipherTextArray->GetElement(passCount * 16)) != 0)
        {
            NANOCLR_SET_AND_LEAVE(CLR_E_FAIL);
        }

        passCount++;
    }

    NANOCLR_CLEANUP();

    // make sure nothing is left in memory
    mbedtls_aes_free(&encodeContext);

    NANOCLR_CLEANUP_END();
}

HRESULT Library_nf_sys_sec_cryptography_System_Security_Cryptography_Aes::DecryptAesEcb___SZARRAY_U1__SZARRAY_U1(
    CLR_RT_StackFrame &stack)
{
    NANOCLR_HEADER();

    mbedtls_aes_context decodeContext;
    uint16_t passCount = 0;

    CLR_RT_HeapBlock_Array *keyArray;
    CLR_RT_HeapBlock_Array *plainTextArray;
    CLR_RT_HeapBlock_Array *cipherTextArray;
    CLR_RT_HeapBlock *pThis;

    pThis = (CLR_RT_HeapBlock *)stack.This();
    FAULT_ON_NULL(pThis);

    // grab key and check for NULL
    keyArray = pThis[FIELD___key].DereferenceArray();
    if (keyArray == NULL)
    {
        NANOCLR_SET_AND_LEAVE(CLR_E_INVALID_OPERATION);
    }

    // grab cipher text and check for NULL
    cipherTextArray = stack.Arg1().DereferenceArray();
    FAULT_ON_NULL_ARG(cipherTextArray);

    // data has to be multiple of 16
    if (cipherTextArray->m_numOfElements % 16 != 0)
    {
        NANOCLR_SET_AND_LEAVE(CLR_E_INVALID_PARAMETER);
    }

    // create the return array (same length as the input)
    stack.PushValueAndClear();
    NANOCLR_CHECK_HRESULT(CLR_RT_HeapBlock_Array::CreateInstance(
        stack.TopValue(),
        cipherTextArray->m_numOfElements,
        g_CLR_RT_WellKnownTypes.m_UInt8));
    plainTextArray = stack.TopValue().DereferenceArray();

    // init mbedtls context
    mbedtls_aes_init(&decodeContext);

    if (mbedtls_aes_setkey_dec(&decodeContext, keyArray->GetFirstElement(), keyArray->m_numOfElements * 8) ==
        MBEDTLS_ERR_AES_INVALID_KEY_LENGTH)
    {
        NANOCLR_SET_AND_LEAVE(CLR_E_INVALID_PARAMETER);
    }

    // itereate through the cipher text array, decrypting 16 bytes at a time
    while (passCount < cipherTextArray->m_numOfElements / 16)
    {
        if (mbedtls_aes_crypt_ecb(
                &decodeContext,
                MBEDTLS_AES_DECRYPT,
                cipherTextArray->GetElement(passCount * 16),
                plainTextArray->GetElement(passCount * 16)) != 0)
        {
            NANOCLR_SET_AND_LEAVE(CLR_E_FAIL);
        }

        passCount++;
    }

    NANOCLR_CLEANUP();

    // make sure nothing is left in memory
    mbedtls_aes_free(&decodeContext);

    NANOCLR_CLEANUP_END();
}

HRESULT Library_nf_sys_sec_cryptography_System_Security_Cryptography_Aes::EncryptAesCbc___SZARRAY_U1__SZARRAY_U1(
    CLR_RT_StackFrame &stack)
{
    NANOCLR_HEADER();

    mbedtls_cipher_context_t ctx;
    mbedtls_cipher_info_t const *cipherInfo;
    uint8_t *ivCopy = NULL;
    uint8_t *workBuffer = NULL;
    size_t olen;

    CLR_RT_HeapBlock_Array *keyArray;
    CLR_RT_HeapBlock_Array *ivArray;
    CLR_RT_HeapBlock_Array *plainTextArray;
    CLR_RT_HeapBlock_Array *cipherTextArray;
    CLR_RT_HeapBlock *pThis;

    pThis = (CLR_RT_HeapBlock *)stack.This();
    FAULT_ON_NULL(pThis);

    // grab key and check for NULL
    keyArray = pThis[FIELD___key].DereferenceArray();
    if (keyArray == NULL)
    {
        NANOCLR_SET_AND_LEAVE(CLR_E_INVALID_OPERATION);
    }

    // grab IV
    // (expecting this to be filled with the IV from managed code)
    ivArray = pThis[FIELD___iv].DereferenceArray();

    // need a copy of the IV because it will be modified by mbedtls
    ivCopy = (uint8_t *)platform_malloc(ivArray->m_numOfElements);

    if (ivCopy == NULL)
    {
        NANOCLR_SET_AND_LEAVE(CLR_E_OUT_OF_MEMORY);
    }

    memcpy(ivCopy, ivArray->GetFirstElement(), ivArray->m_numOfElements);

    // grab plain text and check for NULL
    plainTextArray = stack.Arg1().DereferenceArray();
    FAULT_ON_NULL_ARG(plainTextArray);

    // data has to be multiple of 16
    if (plainTextArray->m_numOfElements % 16 != 0)
    {
        NANOCLR_SET_AND_LEAVE(CLR_E_INVALID_PARAMETER);
    }

    // create the return array (same length as the input)
    stack.PushValueAndClear();
    NANOCLR_CHECK_HRESULT(CLR_RT_HeapBlock_Array::CreateInstance(
        stack.TopValue(),
        plainTextArray->m_numOfElements,
        g_CLR_RT_WellKnownTypes.m_UInt8));
    cipherTextArray = stack.TopValue().DereferenceArray();

    // init mbedtls context
    mbedtls_cipher_init(&ctx);

    // set up the cipher
    cipherInfo = mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_CBC);
    mbedtls_cipher_setup(&ctx, cipherInfo);

    // need a work buffer to hold the encrypted data
    workBuffer = (uint8_t *)platform_malloc(plainTextArray->m_numOfElements + cipherInfo->private_block_size);

    if (workBuffer == NULL)
    {
        NANOCLR_SET_AND_LEAVE(CLR_E_OUT_OF_MEMORY);
    }

    // set the padding mode to none
    mbedtls_cipher_set_padding_mode(&ctx, MBEDTLS_PADDING_NONE);

    mbedtls_cipher_setkey(&ctx, keyArray->GetFirstElement(), keyArray->m_numOfElements * 8, MBEDTLS_ENCRYPT);

    // encrypt the data
    mbedtls_cipher_crypt(
        &ctx,
        ivCopy,
        ivArray->m_numOfElements,
        plainTextArray->GetFirstElement(),
        plainTextArray->m_numOfElements,
        workBuffer,
        &olen);

    // sanity check
    if (olen != plainTextArray->m_numOfElements)
    {
        NANOCLR_SET_AND_LEAVE(CLR_E_FAIL);
    }

    // make sure nothing is left in memory
    mbedtls_cipher_free(&ctx);

    // copy back to the return array
    memcpy(cipherTextArray->GetFirstElement(), workBuffer, plainTextArray->m_numOfElements);

    NANOCLR_CLEANUP();

    if (ivCopy != NULL)
    {
        platform_free(ivCopy);
    }

    if (workBuffer != NULL)
    {
        platform_free(workBuffer);
    }

    NANOCLR_CLEANUP_END();
}

HRESULT Library_nf_sys_sec_cryptography_System_Security_Cryptography_Aes::DecryptAesCbc___SZARRAY_U1__SZARRAY_U1(
    CLR_RT_StackFrame &stack)
{
    NANOCLR_HEADER();

    mbedtls_aes_context decodeContext;
    uint8_t *ivCopy = NULL;

    CLR_RT_HeapBlock_Array *keyArray;
    CLR_RT_HeapBlock_Array *ivArray;
    CLR_RT_HeapBlock_Array *plainTextArray;
    CLR_RT_HeapBlock_Array *cipherTextArray;
    CLR_RT_HeapBlock *pThis;

    pThis = (CLR_RT_HeapBlock *)stack.This();
    FAULT_ON_NULL(pThis);

    // grab key and check for NULL
    keyArray = pThis[FIELD___key].DereferenceArray();
    if (keyArray == NULL)
    {
        NANOCLR_SET_AND_LEAVE(CLR_E_INVALID_OPERATION);
    }

    // grab cipher text and check for NULL
    cipherTextArray = stack.Arg1().DereferenceArray();
    FAULT_ON_NULL_ARG(cipherTextArray);

    // data has to be multiple of 16
    if (cipherTextArray->m_numOfElements % 16 != 0)
    {
        NANOCLR_SET_AND_LEAVE(CLR_E_INVALID_PARAMETER);
    }

    // grab IV
    // (expecting this to be filled with the IV from managed code)
    ivArray = pThis[FIELD___iv].DereferenceArray();

    // need a copy of the IV because it will be modified by mbedtls
    ivCopy = (uint8_t *)platform_malloc(ivArray->m_numOfElements);

    if (ivCopy == NULL)
    {
        NANOCLR_SET_AND_LEAVE(CLR_E_OUT_OF_MEMORY);
    }

    memcpy(ivCopy, ivArray->GetFirstElement(), ivArray->m_numOfElements);

    // create the return array (same length as the input)
    stack.PushValueAndClear();
    NANOCLR_CHECK_HRESULT(CLR_RT_HeapBlock_Array::CreateInstance(
        stack.TopValue(),
        cipherTextArray->m_numOfElements,
        g_CLR_RT_WellKnownTypes.m_UInt8));
    plainTextArray = stack.TopValue().DereferenceArray();

    // init mbedtls context
    mbedtls_aes_init(&decodeContext);

    if (mbedtls_aes_setkey_dec(&decodeContext, keyArray->GetFirstElement(), keyArray->m_numOfElements * 8) ==
        MBEDTLS_ERR_AES_INVALID_KEY_LENGTH)
    {
        NANOCLR_SET_AND_LEAVE(CLR_E_INVALID_PARAMETER);
    }

    if (mbedtls_aes_crypt_cbc(
            &decodeContext,
            MBEDTLS_AES_DECRYPT,
            plainTextArray->m_numOfElements,
            ivCopy,
            cipherTextArray->GetFirstElement(),
            plainTextArray->GetFirstElement()) != 0)
    {
        NANOCLR_SET_AND_LEAVE(CLR_E_FAIL);
    }

    NANOCLR_CLEANUP();

    // make sure nothing is left in memory
    mbedtls_aes_free(&decodeContext);
    if (ivCopy != NULL)
    {
        platform_free(ivCopy);
    }

    NANOCLR_CLEANUP_END();
}
