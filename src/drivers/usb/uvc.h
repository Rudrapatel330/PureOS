#ifndef UVC_H
#define UVC_H

#include "usb.h"

// UVC Class Codes
#define UVC_CC_VIDEO 0x0E
#define UVC_SC_VIDEOCONTROL 0x01
#define UVC_SC_VIDEOSTREAMING 0x02

// VideoControl Class-Specific Request Codes
#define UVC_RC_UNDEFINED 0x00
#define UVC_SET_CUR 0x01
#define UVC_GET_CUR 0x81
#define UVC_GET_MIN 0x82
#define UVC_GET_MAX 0x83
#define UVC_GET_RES 0x84
#define UVC_GET_LEN 0x85
#define UVC_GET_INFO 0x86
#define UVC_GET_DEF 0x87

// VS Interface Control Selectors
#define UVC_VS_CONTROL_UNDEFINED 0x00
#define UVC_VS_PROBE_CONTROL 0x01
#define UVC_VS_COMMIT_CONTROL 0x02

// Video Probe and Commit Control Structure (UVC 1.1)
typedef struct {
  uint16_t bmHint;
  uint8_t bFormatIndex;
  uint8_t bFrameIndex;
  uint32_t dwFrameInterval;
  uint16_t wKeyFrameRate;
  uint16_t wPFrameRate;
  uint16_t wCompQuality;
  uint16_t wCompWindowSize;
  uint16_t wDelay;
  uint32_t dwMaxVideoFrameSize;
  uint32_t dwMaxPayloadTransferSize;
} __attribute__((packed)) uvc_probe_commit_t;

void uvc_init_device(usb_device_t *dev);
void uvc_update(void);

#endif
