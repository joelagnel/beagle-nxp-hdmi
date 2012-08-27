/**
 * Copyright (C) 2009 NXP N.V., All Rights Reserved.
 * This source code and any compilation or derivative thereof is the proprietary
 * information of NXP N.V. and is confidential in nature. Under no circumstances
 * is this software to be  exposed to or placed under an Open Source License of
 * any type without the expressed written permission of NXP N.V.
 *
 */

#include <linux/module.h>

#include "tmbslHdmiTx_types.h"
#include "tmbslTDA9989_Functions.h"
#include "tmbslTDA9989_local.h"

typedef struct {
   tmErrorCode_t
   (*tmbslTDA9989HdcpCheck)
   (
    tmUnitSelect_t          txUnit,
    UInt16                  uTimeSinceLastCallMs,
    tmbslHdmiTxHdcpCheck_t  *pResult
    );
   tmErrorCode_t
   (*tmbslTDA9989HdcpConfigure)
   (
    tmUnitSelect_t           txUnit,
    UInt8                    slaveAddress,
    tmbslHdmiTxHdcpTxMode_t  txMode,
    tmbslHdmiTxHdcpOptions_t options,
    UInt16                   uCheckIntervalMs,
    UInt8                    uChecksToDo
    );
   tmErrorCode_t
   (*tmbslTDA9989HdcpDownloadKeys)
   (
    tmUnitSelect_t          txUnit,
    UInt16                  seed,
    tmbslHdmiTxDecrypt_t    keyDecryption
    );
   tmErrorCode_t
   (*tmbslTDA9989HdcpEncryptionOn)
   (
    tmUnitSelect_t  txUnit,
    Bool            bOn
    );
   tmErrorCode_t
   (*tmbslTDA9989HdcpGetOtp)
   (
    tmUnitSelect_t          txUnit,
    UInt8                   otpAddress,
    UInt8                   *pOtpData
    );
   tmErrorCode_t
   (*tmbslTDA9989HdcpGetT0FailState)
   (
    tmUnitSelect_t  txUnit,
    UInt8           *pFailState
    );
   tmErrorCode_t
   (*tmbslTDA9989HdcpHandleBCAPS)
   (
    tmUnitSelect_t  txUnit
    );
   tmErrorCode_t
   (*tmbslTDA9989HdcpHandleBKSV)
   (
    tmUnitSelect_t  txUnit,
    UInt8           *pBksv,
    Bool            *pbCheckRequired  /* May be null, but only for testing */
    );
   tmErrorCode_t
   (*tmbslTDA9989HdcpHandleBKSVResult)
   (
    tmUnitSelect_t  txUnit,
    Bool            bSecure
    );
   tmErrorCode_t
   (*tmbslTDA9989HdcpHandleBSTATUS)
   (
    tmUnitSelect_t  txUnit,
    UInt16          *pBstatus   /* May be null */
    );
   tmErrorCode_t
   (*tmbslTDA9989HdcpHandleENCRYPT)
   (
    tmUnitSelect_t  txUnit
    );
   tmErrorCode_t
   (*tmbslTDA9989HdcpHandlePJ)
   (
    tmUnitSelect_t  txUnit
    );
   tmErrorCode_t
   (*tmbslTDA9989HdcpHandleSHA_1)
   (
    tmUnitSelect_t  txUnit,
    UInt8           maxKsvDevices,
    UInt8           *pKsvList,          /* May be null if maxKsvDevices is 0 */
    UInt8           *pnKsvDevices,      /* May be null if maxKsvDevices is 0 */
    UInt8           *pDepth             /* Connection tree depth returned with KSV list */
    );
   tmErrorCode_t
   (*tmbslTDA9989HdcpHandleSHA_1Result)
   (
    tmUnitSelect_t  txUnit,
    Bool            bSecure
    );
   tmErrorCode_t
   (*tmbslTDA9989HdcpHandleT0)
   (
    tmUnitSelect_t  txUnit
    );
   tmErrorCode_t
   (*tmbslTDA9989HdcpInit)
   (
    tmUnitSelect_t      txUnit,
    tmbslHdmiTxVidFmt_t voutFmt,
    tmbslHdmiTxVfreq_t  voutFreq
    );
   tmErrorCode_t
   (*tmbslTDA9989HdcpRun)
   (
    tmUnitSelect_t  txUnit
    );
   tmErrorCode_t
   (*tmbslTDA9989HdcpStop)
   (
    tmUnitSelect_t  txUnit
    );
   tmErrorCode_t
   (*tmbslTDA9989HdcpGetSinkCategory)
   (
    tmUnitSelect_t txUnit,
    tmbslHdmiTxSinkCategory_t *category
    );
   tmErrorCode_t
   (*tmbslTDA9989handleBKSVResultSecure)
   (
    tmUnitSelect_t txUnit
    );
   tmErrorCode_t (*f1)(tmHdmiTxobject_t *pDis);
   int (*f2)(tmHdmiTxobject_t *pDis);
} hdcp_private_t;

extern void register_hdcp_private(hdcp_private_t *hdcp);
extern void reset_hdmi(int hdcp_module);
extern tmErrorCode_t tmbslTDA9989handleBKSVResultSecure(tmUnitSelect_t txUnit);

hdcp_private_t h;

tmErrorCode_t f1(tmHdmiTxobject_t *pDis) {
   tmErrorCode_t err;
   err = setHwRegisterField(pDis, E_REG_P12_TX0_RW,
                            E_MASKREG_P12_TX0_rpt_sr, 1);
   RETIF_REG_FAIL(err);
   
   err = setHwRegisterField(pDis, E_REG_P12_TX0_RW,
                            E_MASKREG_P12_TX0_rpt_sr, 0);
   return err;
}

int f2(tmHdmiTxobject_t *pDis) {
   /* Enable EESS if HDCP 1.1 receiver and if 1.1 not disabled */
   if (((pDis->HdcpBcaps & E_MASKREG_P12_BCAPS_RX_1_1) > 0)
       && ((pDis->HdcpOptions & HDMITX_HDCP_OPTION_FORCE_NO_1_1) == 0))
      {
         return 1;
      }
   else
      {
         return 0;
      }
}

/*
 *  Module :: start up
 */
static int __init hdcp_init(void)
{
   h.tmbslTDA9989HdcpCheck = tmbslTDA9989HdcpCheck;
   h.tmbslTDA9989HdcpConfigure = tmbslTDA9989HdcpConfigure;
   h.tmbslTDA9989HdcpDownloadKeys = tmbslTDA9989HdcpDownloadKeys;
   h.tmbslTDA9989HdcpEncryptionOn = tmbslTDA9989HdcpEncryptionOn;
   h.tmbslTDA9989HdcpGetOtp = tmbslTDA9989HdcpGetOtp;
   h.tmbslTDA9989HdcpGetT0FailState = tmbslTDA9989HdcpGetT0FailState;
   h.tmbslTDA9989HdcpHandleBCAPS = tmbslTDA9989HdcpHandleBCAPS;
   h.tmbslTDA9989HdcpHandleBKSV = tmbslTDA9989HdcpHandleBKSV;
   h.tmbslTDA9989HdcpHandleBKSVResult = tmbslTDA9989HdcpHandleBKSVResult;
   h.tmbslTDA9989HdcpHandleBSTATUS = tmbslTDA9989HdcpHandleBSTATUS;
   h.tmbslTDA9989HdcpHandleENCRYPT = tmbslTDA9989HdcpHandleENCRYPT;
   h.tmbslTDA9989HdcpHandlePJ = tmbslTDA9989HdcpHandlePJ;
   h.tmbslTDA9989HdcpHandleSHA_1 = tmbslTDA9989HdcpHandleSHA_1;
   h.tmbslTDA9989HdcpHandleSHA_1Result = tmbslTDA9989HdcpHandleSHA_1Result;
   h.tmbslTDA9989HdcpHandleT0 = tmbslTDA9989HdcpHandleT0;
   h.tmbslTDA9989HdcpInit = tmbslTDA9989HdcpInit;
   h.tmbslTDA9989HdcpRun = tmbslTDA9989HdcpRun;
   h.tmbslTDA9989HdcpStop = tmbslTDA9989HdcpStop;
   h.tmbslTDA9989HdcpGetSinkCategory = tmbslTDA9989HdcpGetSinkCategory;
   h.tmbslTDA9989handleBKSVResultSecure = tmbslTDA9989handleBKSVResultSecure;
   h.f1 = f1;
   h.f2 = f2;

   register_hdcp_private(&h);
   reset_hdmi(1);
   return 0;
}

/*
 *  Module :: shut down
 */
static void __exit hdcp_exit(void)
{

   /* PATCH because of SetPowerState that calls SetHdcp that has just been removed by nwolc :( */
   reset_hdmi(2);

   h.tmbslTDA9989HdcpCheck = NULL;
   h.tmbslTDA9989HdcpConfigure = NULL;
   h.tmbslTDA9989HdcpDownloadKeys = NULL;
   h.tmbslTDA9989HdcpEncryptionOn = NULL;
   h.tmbslTDA9989HdcpGetOtp = NULL;
   h.tmbslTDA9989HdcpGetT0FailState = NULL;
   h.tmbslTDA9989HdcpHandleBCAPS = NULL;
   h.tmbslTDA9989HdcpHandleBKSV = NULL;
   h.tmbslTDA9989HdcpHandleBKSVResult = NULL;
   h.tmbslTDA9989HdcpHandleBSTATUS = NULL;
   h.tmbslTDA9989HdcpHandleENCRYPT = NULL;
   h.tmbslTDA9989HdcpHandlePJ = NULL;
   h.tmbslTDA9989HdcpHandleSHA_1 = NULL;
   h.tmbslTDA9989HdcpHandleSHA_1Result = NULL;
   h.tmbslTDA9989HdcpHandleT0 = NULL;
   h.tmbslTDA9989HdcpInit = NULL;
   h.tmbslTDA9989HdcpRun = NULL;
   h.tmbslTDA9989HdcpStop = NULL;
   h.tmbslTDA9989HdcpGetSinkCategory = NULL;
   h.tmbslTDA9989handleBKSVResultSecure = NULL;

   register_hdcp_private(NULL);
   reset_hdmi(0);
}


/*
 *  Module
 *  ------
 */
module_init(hdcp_init);
module_exit(hdcp_exit);

/*
 *  Disclamer
 *  ---------
 */
MODULE_LICENSE("PROPRIETARY");
MODULE_AUTHOR("NXP");
