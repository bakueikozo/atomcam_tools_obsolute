#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
  IMP_ENC_TYPE_AVC      = 0,
  IMP_ENC_TYPE_HEVC     = 1,
  IMP_ENC_TYPE_JPEG     = 4,
} IMPEncoderEncType;

#define IMP_ENC_AVC_PROFILE_IDC_BASELINE      66
#define IMP_ENC_AVC_PROFILE_IDC_MAIN          77
#define IMP_ENC_AVC_PROFILE_IDC_HIGH          100
#define IMP_ENC_HEVC_PROFILE_IDC_MAIN         1

typedef enum {
  IMP_ENC_PROFILE_AVC_BASELINE  = ((IMP_ENC_TYPE_AVC << 24) | (IMP_ENC_AVC_PROFILE_IDC_BASELINE)),
  IMP_ENC_PROFILE_AVC_MAIN      = ((IMP_ENC_TYPE_AVC << 24) | (IMP_ENC_AVC_PROFILE_IDC_MAIN)),
  IMP_ENC_PROFILE_AVC_HIGH      = ((IMP_ENC_TYPE_AVC << 24) | (IMP_ENC_AVC_PROFILE_IDC_HIGH)),
  IMP_ENC_PROFILE_HEVC_MAIN     = ((IMP_ENC_TYPE_HEVC << 24) | (IMP_ENC_HEVC_PROFILE_IDC_MAIN)),
  IMP_ENC_PROFILE_JPEG          = (IMP_ENC_TYPE_JPEG << 24),
} IMPEncoderProfile;

typedef enum {
  IMP_ENC_PIC_FORMAT_400_8BITS = 0x0088,
  IMP_ENC_PIC_FORMAT_420_8BITS = 0x0188,
  IMP_ENC_PIC_FORMAT_422_8BITS = 0x0288,
} IMPEncoderPicFormat;

typedef struct {
    bool		enable;
    uint32_t	x;
    uint32_t	y;
    uint32_t	w;
    uint32_t	h;
} IMPEncoderCropCfg;

typedef struct {
  IMPEncoderProfile       eProfile;
  uint8_t                 uLevel;
  uint8_t                 uTier;
  uint16_t                uWidth;
  uint16_t                uHeight;
  IMPEncoderPicFormat     ePicFormat;
  uint32_t                eEncOptions;
  uint32_t                eEncTools;
  IMPEncoderCropCfg       crop;
} IMPEncoderEncAttr;

typedef enum {
  IMP_ENC_RC_MODE_FIXQP             = 0x0,
  IMP_ENC_RC_MODE_CBR               = 0x1,
  IMP_ENC_RC_MODE_VBR               = 0x2,
  IMP_ENC_RC_MODE_CAPPED_VBR        = 0x4,
  IMP_ENC_RC_MODE_CAPPED_QUALITY    = 0x8,
  IMP_ENC_RC_MODE_INVALID           = 0xff,
} IMPEncoderRcMode;

typedef struct {
  int16_t       iInitialQP;
} IMPEncoderAttrFixQP;

typedef struct {
  uint32_t              uTargetBitRate;
  int16_t               iInitialQP;
  int16_t               iMinQP;
  int16_t               iMaxQP;
  int16_t               iIPDelta;
  int16_t               iPBDelta;
  uint32_t              eRcOptions;
  uint32_t              uMaxPictureSize;
} IMPEncoderAttrCbr;

typedef struct {
  uint32_t              uTargetBitRate;
  uint32_t              uMaxBitRate;
  int16_t               iInitialQP;
  int16_t               iMinQP;
  int16_t               iMaxQP;
  int16_t               iIPDelta;
  int16_t               iPBDelta;
  uint32_t              eRcOptions;
  uint32_t              uMaxPictureSize;
} IMPEncoderAttrVbr;

typedef struct {
  uint32_t              uTargetBitRate;
  uint32_t              uMaxBitRate;
  int16_t               iInitialQP;
  int16_t               iMinQP;
  int16_t               iMaxQP;
  int16_t               iIPDelta;
  int16_t               iPBDelta;
  uint32_t              eRcOptions;
  uint32_t              uMaxPictureSize;
  uint16_t              uMaxPSNR;
} IMPEncoderAttrCappedVbr;

typedef IMPEncoderAttrCappedVbr IMPEncoderAttrCappedQuality;

typedef struct {
  IMPEncoderRcMode              rcMode;
  union {
    IMPEncoderAttrFixQP         attrFixQp;
    IMPEncoderAttrCbr           attrCbr;
    IMPEncoderAttrVbr           attrVbr;
    IMPEncoderAttrCappedVbr     attrCappedVbr;
    IMPEncoderAttrCappedQuality attrCappedQuality;
  };
} IMPEncoderAttrRcMode;

typedef struct {
	uint32_t	frmRateNum;
	uint32_t	frmRateDen;
} IMPEncoderFrmRate;

typedef struct {
  IMPEncoderAttrRcMode      attrRcMode;
  IMPEncoderFrmRate         outFrmRate;
} IMPEncoderRcAttr;

typedef enum {
  IMP_ENC_GOP_CTRL_MODE_DEFAULT       = 0x02,
  IMP_ENC_GOP_CTRL_MODE_PYRAMIDAL     = 0x04,
  IMP_ENC_GOP_CTRL_MAX_ENUM           = 0xff,
} IMPEncoderGopCtrlMode;

typedef struct {
  IMPEncoderGopCtrlMode   uGopCtrlMode;
  uint16_t                uGopLength;
  uint8_t                 uNumB;
  uint32_t                uMaxSameSenceCnt;
  bool                    bEnableLT;
  uint32_t                uFreqLT;
  bool                    bLTRC;
} IMPEncoderGopAttr;

typedef struct {
	IMPEncoderEncAttr     encAttr;
	IMPEncoderRcAttr      rcAttr;
	IMPEncoderGopAttr     gopAttr;
} IMPEncoderChnAttr;

static int (*original_IMP_Encoder_CreateChn)(int encChn, const IMPEncoderChnAttr *attr);

int IMP_Encoder_CreateChn(int encChn, IMPEncoderChnAttr *attr) {

  fprintf(stderr, "ch%d eProfile:%08x width: %d height: %d pixFormat %04x\n", encChn, attr->encAttr.eProfile, attr->encAttr.uWidth, attr->encAttr.uHeight, attr->encAttr.ePicFormat);
  if(attr->rcAttr.attrRcMode.rcMode == 8) {
    fprintf(stderr, "attrRcMode: %02x targetBitRate %d MaxBitRate %d MaxPicSize %d \n", attr->rcAttr.attrRcMode.rcMode,
      attr->rcAttr.attrRcMode.attrCappedQuality.uTargetBitRate,
      attr->rcAttr.attrRcMode.attrCappedQuality.uMaxBitRate,
      attr->rcAttr.attrRcMode.attrCappedQuality.uMaxPictureSize
    );
  }
  return original_IMP_Encoder_CreateChn(encChn, attr);
}

static void __attribute ((constructor)) encoder_init(void) {

  original_IMP_Encoder_CreateChn = dlsym(dlopen("/system/lib/libimp.so", RTLD_LAZY), "IMP_Encoder_CreateChn");
}
