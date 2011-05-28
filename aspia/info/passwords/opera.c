/*
 * PROJECT:         Aspia
 * FILE:            aspia/info/passwords/opera.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "../../main.h"

#include "openssl/md5.h"
#include "openssl/des.h"

typedef VOID (WINAPI *PDES_EDE3_CBC_ENCRYPT)(const UCHAR*,UCHAR *,
              LONG, DES_key_schedule*,DES_key_schedule*, DES_key_schedule*,DES_cblock*,INT);

typedef VOID (WINAPI *PDES_SET_KEY_UNCHECHED)(const_DES_cblock*,DES_key_schedule*);

typedef UCHAR* (WINAPI *PMD5)(const UCHAR*, size_t, UCHAR*);

const UCHAR opera_salt[11] =
{
    0x83, 0x7D, 0xFC, 0x0F, 0x8E, 0xB3, 0xE8, 0x69, 0x73, 0xAF, 0xFF
};

HINSTANCE hOpenSSLDLL;

PDES_EDE3_CBC_ENCRYPT pDES_ede3_cbc_encrypt = NULL;
PDES_SET_KEY_UNCHECHED pDES_set_key_unchecked = NULL;
PMD5 pMD5 = NULL;


BOOL
LoadSSLFunctions(VOID)
{
    hOpenSSLDLL = LoadLibrary(L"libeay32.dll");

    if (!hOpenSSLDLL) return FALSE;

    pDES_ede3_cbc_encrypt = (PDES_EDE3_CBC_ENCRYPT)GetProcAddress(hOpenSSLDLL, "des_ede3_cbc_encrypt");
    if (!pDES_ede3_cbc_encrypt)
    {
        FreeLibrary(hOpenSSLDLL);
        return FALSE;
    }

    pDES_set_key_unchecked = (PDES_EDE3_CBC_ENCRYPT)GetProcAddress(hOpenSSLDLL, "des_set_key_unchecked");
    if (!pDES_set_key_unchecked)
    {
        FreeLibrary(hOpenSSLDLL);
        return FALSE;
    }

    pMD5 = (PMD5)GetProcAddress(hOpenSSLDLL, "MD5");
    if (!pMD5)
    {
        FreeLibrary(hOpenSSLDLL);
        return FALSE;
    }

    return TRUE;
}


VOID
SOFTWARE_PasswdOperaInfo(VOID)
{
    FILE *fdWand = fopen("c:\\wand.dat", "rb");
    ULONG fileSize, wandOffset = 0, blockLength, dataLength;
    UCHAR *wandData, *blockLengthPtr, *dataLengthPtr;
    UCHAR hashSignature1[MD5_DIGEST_LENGTH], hashSignature2[MD5_DIGEST_LENGTH], tmpBuffer[256];
    UCHAR *cryptoData;
    DES_key_schedule key_schedule1, key_schedule2, key_schedule3;
    DES_cblock iVector;

    if (!LoadSSLFunctions()) return;

    if (!fdWand) return;

    fseek(fdWand, 0, SEEK_END);

    fileSize = ftell(fdWand);

    wandData = (UCHAR*)malloc(fileSize);
    if (!wandData)
    {
        fclose(fdWand);
        return;
    }

    MessageBox(0, L"1", 0, 0);

    rewind(fdWand);
    fread(wandData, fileSize, 1, fdWand);
    fclose(fdWand);

    IoAddIcon(IDI_OPERA);

    while (wandOffset < fileSize)
    {
        UCHAR *wandKey = (UCHAR*)
            memchr(wandData + wandOffset, DES_KEY_SZ, fileSize - wandOffset);

        if (!wandKey) break;

        wandOffset = ++wandKey - wandData;

        blockLengthPtr = wandKey - 8;
        dataLengthPtr = wandKey + DES_KEY_SZ;

        if (blockLengthPtr < wandData || dataLengthPtr > wandData + fileSize)
            continue;

        blockLength  = *blockLengthPtr++ << 24;
        blockLength |= *blockLengthPtr++ << 16;
        blockLength |= *blockLengthPtr++ <<  8;
        blockLength |= *blockLengthPtr;

        dataLength  = *dataLengthPtr++ << 24;
        dataLength |= *dataLengthPtr++ << 16;
        dataLength |= *dataLengthPtr++ <<  8;
        dataLength |= *dataLengthPtr;

        if(blockLength != dataLength + DES_KEY_SZ + 4 + 4)
            continue;

        if(dataLength > fileSize - (wandOffset + DES_KEY_SZ + 4)
           || dataLength < 8 || dataLength % 8 != 0)
        {
            continue;
        }

        memcpy(tmpBuffer, opera_salt, sizeof(opera_salt));
        memcpy(tmpBuffer + sizeof(opera_salt), wandKey, DES_KEY_SZ);

        pMD5(tmpBuffer, sizeof(opera_salt) + DES_KEY_SZ, hashSignature1);

        memcpy(tmpBuffer, hashSignature1, sizeof(hashSignature1));
        memcpy(tmpBuffer + sizeof(hashSignature1),
            opera_salt, sizeof(opera_salt));

        memcpy(tmpBuffer + sizeof(hashSignature1) + 
            sizeof(opera_salt), wandKey, DES_KEY_SZ);

        pMD5(tmpBuffer, sizeof(hashSignature1) +
            sizeof(opera_salt) + DES_KEY_SZ, hashSignature2);

        pDES_set_key_unchecked((const_DES_cblock *)&hashSignature1[0],
            &key_schedule1);

        pDES_set_key_unchecked((const_DES_cblock *)&hashSignature1[8],
            &key_schedule2);

        pDES_set_key_unchecked((const_DES_cblock *)&hashSignature2[0],
            &key_schedule3);

        memcpy(iVector, &hashSignature2[8], sizeof(DES_cblock));

        cryptoData = wandKey + DES_KEY_SZ + 4;

        pDES_ede3_cbc_encrypt(cryptoData, cryptoData, dataLength,
            &key_schedule1, &key_schedule2, &key_schedule3, &iVector, 0);

        if (0x00 == *cryptoData || 0x08 == *cryptoData)
        {
            //std::wcout << L"<null>" << std::endl;
        }
        else
        {
            UCHAR *padding = cryptoData + dataLength - 1;
            memset(padding - (*padding - 1), 0x00, *padding);

            //std::wcout << (wchar_t *)cryptoData << std::endl;
            MessageBox(0, (wchar_t *)cryptoData, 0, 0);
        }

        wandOffset = wandOffset + DES_KEY_SZ + 4 + dataLength;
    }

    free(wandData);

    FreeLibrary(hOpenSSLDLL);
}
