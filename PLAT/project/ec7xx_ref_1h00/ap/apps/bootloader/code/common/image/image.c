#include "error.h"
#include "common.h"
#include "ec7xx.h"
#include "image.h"
#include "boot.h"
#include "string.h"
#include "flash_rt.h"
#include "sha256.h"
#include "stdio.h"
#include "cp_flash.h"
#include "mw_aal_hash.h"

#define VERIFY_STATISTIC
#define OUTDATA_BYTE nSHA224_OUTDATA_BYTE

extern uint8_t *GetVerifyPubKey(void);
extern uint8_t FLASH_XIPRead(uint8_t* pData, uint32_t ReadAddr, uint32_t Size);
extern uint32_t VerifySignature(uint8_t is224, uint8_t *hash, uint8_t *ecsda, uint8_t *pubKey);


PLAT_BL_SCT_ZI static uint8_t Output[OUTDATA_BYTE];


ImageHead GImageHead;

uint32_t VerifyImageHead(void)
{
    uint32_t TotalLen;
    uint32_t RetValue = NoError;
    memset(Output, 0, OUTDATA_BYTE);
    ImageHead *pImgHd = (ImageHead *)IMG_HEAD_LOAD_ADDR;
    MwAalSha256Ctx_t sha256Cxt;
#ifdef VERIFY_STATISTIC
    uint32_t StartTime, EndTime, DiffTime;
    StartTime = SysTick_Get_CurrentVal();
#endif
    //skip verify when security is not enabled and type is not match
    if((GetBRInfo()->SecureBootEnabled == 0)&&((GImageHead.Type&0xff)==SHA256_NONE))
    {
        //to be fixed
#ifdef VERIFY_STATISTIC
        BL_TRACE("VerifyImageHead skip\r\n");
#endif
        return RetValue;
    }
    memset(&pImgHd->HashIH[0], 0, sizeof(pImgHd->HashIH));
    TotalLen = sizeof(ImageHead);

    mwAalInitSha256(&sha256Cxt, 1);

    //TotalLen < max_sha_segment_len (64k -1 )
    RetValue = mwAalUpdateSha256(&sha256Cxt, (uint8_t*)pImgHd, Output, TotalLen, 1);
    if (RetValue !=0)
    {
        return RetValue;
    }
    #if 0
    BL_TRACE("VerifyImageHead Output:\n");
    for(int i=0; i<OUTDATA_BYTE; i++)
    {
        BL_TRACE("0x%x,",Output[i]);
    }
    BL_TRACE("\n");
    #endif
    if (memcmp(&GImageHead.HashIH[0], &Output[0], sizeof(GImageHead.HashIH))!= 0)
    {
        BL_TRACE("GImageHead.HashIH not match\r\n");
        RetValue = InvalidHeadKeyHashError;
    }
    #ifdef VERIFY_STATISTIC
    EndTime = SysTick_Get_CurrentVal();
    DiffTime = StartTime - EndTime;
    DiffTime = DiffTime*1000/32768;
    BL_TRACE("sha256_data(len=%d), Time(ms)->%d.\r\n", (int)TotalLen, (int)DiffTime);
    #endif
    return RetValue;
}

uint32_t LoadVerifyImageHead(void)
{
    uint32_t RetValue;
#ifdef VERIFY_STATISTIC
    uint32_t StartTime, EndTime, DiffTime;
    StartTime = SysTick_Get_CurrentVal();
#endif
    //load system img head
    RetValue = FLASH_XIPRead((uint8_t*)IMG_HEAD_LOAD_ADDR,  AP_HEAD_FLASH_OFFSET_ADDR, sizeof(GImageHead));
    if (RetValue != QSPI_OK)
    {
        BL_TRACE("LoadVerifyImageHead FLASH_XIPRead return %d.\r\n", RetValue);
        RetValue = SpiReadErr;
        return RetValue;
    }
    memcpy((uint8_t*)&GImageHead, (uint8_t*)IMG_HEAD_LOAD_ADDR, sizeof(GImageHead));

#ifdef VERIFY_STATISTIC
    EndTime = SysTick_Get_CurrentVal();
    DiffTime = StartTime - EndTime;
    DiffTime = DiffTime*1000/32768;
    BL_TRACE("LoadVerifyImageHead read(len=%d), Time(ms)->%d.\r\n", (int)(sizeof(GImageHead) + GImageHead.RsvArea.ReservedSize), (int)DiffTime);
#endif

    #if 0
    BL_TRACE("GImageHead->HashIH:\n");
    for(int i=0; i<OUTDATA_BYTE; i++)
    {
        BL_TRACE("0x%x,",GImageHead.HashIH[i]);
    }
    BL_TRACE("\n");

    uint8_t* head2 = (uint8_t*)IMG_HEAD_LOAD_ADDR;
    BL_TRACE("system image head(0x%x):\n", AP_HEAD_FLASH_OFFSET_ADDR);
    for(int j=0; j<17; j++){
        for(int i=0; i<16; i++)
        {
            BL_TRACE("0x%x,",head2[i+16*j]);
        }
        BL_TRACE("\n");
    }
    #endif
    RetValue = VerifyImageHead();
    if (RetValue!=NoError)
    {
        BL_TRACE("verify image head failed!!!");
    }
    else
    {
        BL_TRACE("verify image head success\n");
    }

    return RetValue;
}

uint32_t VerifyImageBodySignature(uint8_t is224, uint8_t *hash, uint8_t *ecsda)
{
    return VerifySignature(is224, hash, ecsda, GetVerifyPubKey());
}

uint32_t VerifyImageBody(void)
{
    uint32_t RetValue = NoError;
#ifdef VERIFY_STATISTIC
    uint32_t StartTime, EndTime, DiffTime;
    StartTime = SysTick_Get_CurrentVal();
#endif

    //uint8_t signkey[56] = {0xfc,0x81,0x2d,0x68,0x16,0xd8,0xeb,0x9c,0xb2,0x47,0x39,0xf1,0xa1,0xa3,0xc9,0x56,0xe8,0xb4,0x76,0x34,0xe6,0xd6,0x7c,0xc6,0xd4,0x81,0xb8,
    //                       0x26,0xea,0x6a,0x36,0x3e,0xbe,0x3b,0xb8,0x44,0x78,0x4f,0x29,0x7f,0x27,0xb8,0xcb,0xf,0x8c,0x9f,0xa0,0x8,0xe7,0xda,0xc6,0x7a,0x86,0x74,0xe6,0x4c};
    //memcpy(GImageHead.ImgBodyInfo.ECDSASign, signkey, 56);

    if(NoError != (RetValue = VerifyImageBodySignature(1, GImageHead.ImgBodyInfo.Hash, GImageHead.ImgBodyInfo.ECDSASign))) return RetValue;

#ifdef VERIFY_STATISTIC
    EndTime = SysTick_Get_CurrentVal();
    DiffTime = StartTime - EndTime;
    DiffTime = DiffTime*1000/32768;
    BL_TRACE("uECC_verify Time(ms)->%d.\n", (int)DiffTime);
#endif

    //BL_TRACE("VerifyImageBody succ.\n");

    return RetValue;
}

uint32_t LoadVerifyImageBody(void)
{
    uint32_t RetValue = NoError;
    uint32_t Offset = 0;
    uint32_t RemainLen;
    uint32_t leftLen = 0, fillLen = 0;
    memset(Output, 0, OUTDATA_BYTE);
#ifdef VERIFY_STATISTIC
    uint32_t StartTime, EndTime, DiffTime;
    StartTime = SysTick_Get_CurrentVal();
#endif
    MwAalSha256Ctx_t sha256Cxt;

    #define SHA_BLOCK_SIZE 0x0c000 // 48k < max_sha_segment_len (64k -1 )
    //skip verify when security is not enabled
    if((GetBRInfo()->SecureBootEnabled == 0)&&((GImageHead.Type&0xff)==SHA256_NONE))
    {
        //to be fixed
        BL_TRACE("VerifyImageBody sha256 skip\n");
        return RetValue;
    }
    if (GImageHead.ImgBodyInfo.ApImageSze > AP_BODY_MAX_SIZE)
    {
        return ImageToBigError;
    }

    mwAalInitSha256(&sha256Cxt, 1);

    //GImageHead.ImgBodyInfo.ApImageSze = 2121932;//for test
    BL_TRACE("LoadVerifyImageBody ApImageSze=%d\n", GImageHead.ImgBodyInfo.ApImageSze);
    RemainLen = GImageHead.ImgBodyInfo.ApImageSze;

    for (Offset = 0;  Offset < GImageHead.ImgBodyInfo.ApImageSze; Offset+=SHA_BLOCK_SIZE)
    {
        if (RemainLen > SHA_BLOCK_SIZE)
        {
            RemainLen -= SHA_BLOCK_SIZE;
            RetValue = FLASH_XIPRead((uint8_t*)IMG_BODY_LOAD_ADDR , AP_BODY_FLASH_OFFSET_ADDR+Offset,  SHA_BLOCK_SIZE);
            if (RetValue != QSPI_OK)
            {
                return RetValue;
            }

            RetValue = mwAalUpdateSha256(&sha256Cxt, (uint8_t*)IMG_BODY_LOAD_ADDR, Output, SHA_BLOCK_SIZE, 0);
            if (RetValue !=0)
            {
                return InvalidBodyKeyHashError;
            }
        }
        else
        {
            BL_TRACE("LoadVerifyImageBody::RemainLen=0x%x\r\n", RemainLen);
            leftLen = RemainLen & 0x3F;
            if(leftLen != 0){
                fillLen = 64 - leftLen;
                RetValue = FLASH_XIPRead((uint8_t*)IMG_BODY_LOAD_ADDR , AP_BODY_FLASH_OFFSET_ADDR+Offset,  RemainLen-leftLen);//calc ap image last alignment block
                if (RetValue != QSPI_OK)
                {
                    return RetValue;
                }
                RetValue = mwAalUpdateSha256(&sha256Cxt, (uint8_t*)IMG_BODY_LOAD_ADDR, Output, RemainLen-leftLen, 0);
                if (RetValue != NoError)
                {
                    return RetValue;
                }
            }else{
                BL_TRACE("LoadVerifyImageBody::RemainLen 64 bytes alignment \r\n");
                RetValue = FLASH_XIPRead((uint8_t*)IMG_BODY_LOAD_ADDR , AP_BODY_FLASH_OFFSET_ADDR+Offset,  RemainLen);//ap image 64 alignment
                if (RetValue != QSPI_OK)
                {
                    return RetValue;
                }
                RetValue = mwAalUpdateSha256(&sha256Cxt, (uint8_t*)IMG_BODY_LOAD_ADDR, Output, RemainLen, 0);
                if (RetValue != NoError)
                {
                    return RetValue;
                }
            }
            break;
        }
    }

    if(leftLen != 0){
        RetValue = FLASH_XIPRead((uint8_t*)IMG_BODY_LOAD_ADDR , AP_BODY_FLASH_OFFSET_ADDR+Offset+RemainLen-leftLen,  leftLen);//read ap image leftlen to buffer
        if (RetValue != QSPI_OK)
        {
            return RetValue;
        }
    }
    //GImageHead.ImgBodyInfo.CpImageSze = 449184;//for test
    BL_TRACE("LoadVerifyImageBody CpImageSze=%d\n", GImageHead.ImgBodyInfo.CpImageSze);
    RemainLen = GImageHead.ImgBodyInfo.CpImageSze;

    #if (defined CHIP_EC618)
    CPXIP_Enable();
    #elif (defined TYPE_EC718H)
    CPFLASH_xipInit();
    #endif

    if(leftLen != 0){
        RemainLen -= fillLen;
#ifdef TYPE_EC718H
        RetValue = CPFLASH_read((uint8_t*)IMG_BODY_LOAD_ADDR+leftLen , CP_BODY_FLASH_OFFSET_ADDR,  fillLen); //read cp image filllen to buffer
#else//share ap flash
        RetValue = FLASH_XIPRead((uint8_t*)IMG_BODY_LOAD_ADDR+leftLen , CP_BODY_FLASH_OFFSET_ADDR,  fillLen); //read cp image filllen to buffer
#endif
        if (RetValue != QSPI_OK)
        {
            return RetValue;
        }

        RetValue = mwAalUpdateSha256(&sha256Cxt, (uint8_t*)IMG_BODY_LOAD_ADDR, Output, 64, 0);
        if (RetValue !=0)
        {
            return InvalidBodyKeyHashError;
        }
    }
    for (Offset = fillLen;  Offset < GImageHead.ImgBodyInfo.CpImageSze; Offset+=SHA_BLOCK_SIZE)
    {
        if (RemainLen > SHA_BLOCK_SIZE)
        {
            RemainLen -= SHA_BLOCK_SIZE;
#ifdef TYPE_EC718H
            RetValue = CPFLASH_read((uint8_t*)IMG_BODY_LOAD_ADDR , CP_BODY_FLASH_OFFSET_ADDR+Offset,  SHA_BLOCK_SIZE);
#else//share ap flash
            RetValue = FLASH_XIPRead((uint8_t*)IMG_BODY_LOAD_ADDR , CP_BODY_FLASH_OFFSET_ADDR+Offset,  SHA_BLOCK_SIZE);
#endif
            if (RetValue != QSPI_OK)
            {
                return RetValue;
            }

            RetValue = mwAalUpdateSha256(&sha256Cxt, (uint8_t*)IMG_BODY_LOAD_ADDR, Output, SHA_BLOCK_SIZE, 0);
            if (RetValue !=0)
            {
                return InvalidBodyKeyHashError;
            }
        }
        else
        {

#ifdef TYPE_EC718H
            RetValue = CPFLASH_read((uint8_t*)IMG_BODY_LOAD_ADDR , CP_BODY_FLASH_OFFSET_ADDR+Offset,  RemainLen);
#else//share ap flash
            RetValue = FLASH_XIPRead((uint8_t*)IMG_BODY_LOAD_ADDR , CP_BODY_FLASH_OFFSET_ADDR+Offset,  RemainLen);
#endif
            if (RetValue != QSPI_OK)
            {
                return RetValue;
            }

            RetValue = mwAalUpdateSha256(&sha256Cxt, (uint8_t*)IMG_BODY_LOAD_ADDR, Output, RemainLen, 1);
            if (RetValue != NoError)
            {
                return RetValue;
            }
            break;
        }
    }
    mwAalDeinitSha256(&sha256Cxt);
    //uint8_t hash[28] = {0x51,0x3f,0x53,0x24,0x43,0x7c,0xd,0xa1,0x4c,0x7b,0x9b,0xe4,0x52,0x83,0x8d,0x9b,0xcf,0x8e,0xdd,0xf7,0x23,0x35,0x74,0x5e,0x9b,0xa4,0xae,0xcc};//for test
    //memcpy(GImageHead.ImgBodyInfo.Hash, hash, 28);//for test
    //#if 0
    BL_TRACE("ImgBodyInfo.Hash:\n");
    for(int i=0; i<OUTDATA_BYTE; i++)
    {
        BL_TRACE("0x%x,",GImageHead.ImgBodyInfo.Hash[i]);
    }
    BL_TRACE("\n");

    BL_TRACE("LoadVerifyImageBody Output:\n");
    for(int i=0; i<OUTDATA_BYTE; i++)
    {
        BL_TRACE("0x%x,",Output[i]);
    }
    BL_TRACE("\n");
    //#endif
    if (memcmp(&GImageHead.ImgBodyInfo.Hash[0], &Output[0], sizeof(GImageHead.ImgBodyInfo.Hash)) != 0)
    {
          BL_TRACE("Verify hash mismatch!!!!!!!!!!\n");
          return InvalidBodyKeyHashError;
    }
    #ifdef VERIFY_STATISTIC
    EndTime = SysTick_Get_CurrentVal();
    DiffTime = StartTime - EndTime;
    DiffTime = DiffTime*1000/32768;
    BL_TRACE("sha224_data(aplen=%d,cplen=%d), Time(ms)->%d.\r\n", (int)(GImageHead.ImgBodyInfo.ApImageSze), (int)(GImageHead.ImgBodyInfo.CpImageSze),(int)DiffTime);
    #endif

    if(GetBRInfo()->SecureBootEnabled)
    {
        BL_TRACE("secure bit enable to verify image\n");
        RetValue = VerifyImageBody();
        if (RetValue !=0)
        {
            return RetValue;
        }
    }

    return RetValue;
}
