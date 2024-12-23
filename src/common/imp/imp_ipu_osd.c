/*********************************************************
 * File Name   : imp_ipu_osd.c
 * Author      : Ipseven Chen
 * Mail        : Ipseven Chen@ingenic.com
 * Created Time: 2023-02-08 09:29
 ********************************************************/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <semaphore.h>
#include <sys/time.h>
#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_isp.h>
#include <imp/imp_osd.h>

#include <imp_ipu_osd.h>

#define MODULE_TAG "IPU_OSD"
static ipu_osd_param_t g_param;

void sample_ipu_osd_getparam(ipu_osd_param_t **param) {

  *param = &g_param;
  return;
}

int sample_ipu_osd_setrgnsattr(ipu_osd_param_t *param)
{
  int ret;
  ipu_rgn_param_t *region = NULL;
  IMPOSDRgnAttr rAttr;
  memset(&rAttr, 0, sizeof(IMPOSDRgnAttr));

  for(int i = 0; i < param->handle_num; i++) {

    if(i < COVER_NUM) {
      region = param->cover;
      rAttr.type = OSD_REG_COVER;
      rAttr.fmt = PIX_FMT_BGRA;
      rAttr.data.coverData.color = OSD_IPU_BLACK;  /*默认cover黑色*/

    } else if (i < COVER_NUM + PIC_NUM) {
      region = param->pic;
      rAttr.type = OSD_REG_PIC;
      rAttr.fmt = PIX_FMT_NV12;

    } else if (i < COVER_NUM + PIC_NUM + BITMAP_NUM) {

      region = param->bitmap;
      rAttr.type = OSD_REG_BITMAP;
      rAttr.fmt = PIX_FMT_RGB555LE;
    }

    /*
     * mode = 0, 上面已经自动填充相关属性
     * mode = 1, 使用传入参数填充相关属性
     */
    if(region[i].cfg.mode) {

      rAttr.type = region[i].cfg.type;
      rAttr.fmt = region[i].cfg.format;
      if(rAttr.type == OSD_REG_COVER)
        rAttr.data.coverData.color = region[i].cfg.data.color;
    }

    rAttr.rect.p0.x = region[i].x0;
    rAttr.rect.p0.y = region[i].y0;
    rAttr.rect.p1.x = region[i].x1;
    rAttr.rect.p1.y = region[i].y1;

    ret = IMP_OSD_SetRgnAttr(region[i].handle, &rAttr);
    if(ret < 0){
      IMP_LOG_ERR(MODULE_TAG, "IMP_OSD_SetRgnAttr error !\n");
      return -1;
    }

    region[i].cfg.type = rAttr.type;
    region[i].cfg.format = rAttr.fmt;

  }
  return 0;
}

/* 只针对OSD_REG_BITMAP和OSD_REG_PIC的区域类型 */
int sample_ipu_osd_updatergndata_all(int rgn_num, ipu_rgn_param_t *rgn_param)
{
  int ret;
  IMPOSDRgnAttrData rAttrLogoData;
  memset(&rAttrLogoData, 0, sizeof(rAttrLogoData));

  for(int i = 0; i < rgn_num; i++) {

    if(rgn_param[i].cfg.type == OSD_REG_PIC || rgn_param[i].cfg.type == OSD_REG_PIC_RMEM)
      rAttrLogoData.picData.pData = rgn_param[i].cfg.data.data;
    else if (rgn_param[i].cfg.type == OSD_REG_BITMAP)
      rAttrLogoData.bitmapData = rgn_param[i].cfg.data.data;
    else
      printf("ERROR[%s]: (%s)updatergndata type(%d) error\n", MODULE_TAG, __func__,rgn_param[i].cfg.type);

    ret = IMP_OSD_UpdateRgnAttrData(rgn_param[i].handle, &rAttrLogoData);
    if (ret < 0) {
      IMP_LOG_ERR(MODULE_TAG, "IMP_OSD_UpdateRgnAttrData error\n");
      return -1;
    }
  }

  return 0;
}

int sample_ipu_osd_updatergndata_single(int handle, int type, void *data)
{
  int ret;
  IMPOSDRgnAttrData rAttrLogoData;
  memset(&rAttrLogoData, 0, sizeof(rAttrLogoData));

  if(type == OSD_REG_PIC || type == OSD_REG_PIC_RMEM)
    rAttrLogoData.picData.pData = data;
  else if (type == OSD_REG_BITMAP)
    rAttrLogoData.bitmapData = data;
  else
    printf("ERROR[%s]: (%s)updatergndata type error\n", MODULE_TAG, __func__);

  ret = IMP_OSD_UpdateRgnAttrData(handle, &rAttrLogoData);
  if (ret < 0) {
    IMP_LOG_ERR(MODULE_TAG, "IMP_OSD_UpdateRgnAttrData(%d) error\n", handle);
    return -1;
  }

  return 0;
}

int sample_ipu_osd_show_all(int rgn_num, ipu_rgn_param_t *param)
{
  int ret;

  for(int i = 0; i < rgn_num; i++) {

    ret = IMP_OSD_ShowRgn(param[i].handle, param[i].group, param[i].en);
    if (ret != 0) {
      IMP_LOG_ERR(MODULE_TAG, "IMP_OSD_ShowRgn(%d) Logo error\n", param[i].handle);
      return -1;
    }

  }

  return 0;
}

int sample_ipu_osd_show_single(int handle, int group, int enable)
{
  int ret;

  ret = IMP_OSD_ShowRgn(handle, group, enable);
  if (ret != 0) {
    IMP_LOG_ERR(MODULE_TAG, "IMP_OSD_ShowRgn(%d) Logo error\n", handle);
    return -1;
  }
  return 0;
}

int sample_ipu_osd_show_single_timeout(int handle, int group, uint64_t timeout)
{
  int ret;

  uint64_t interval;
  struct timeval start, end;

  gettimeofday(&start, NULL);
  ret = IMP_OSD_ShowRgn(handle, group, 1);
  if (ret != 0) {
    IMP_LOG_ERR(MODULE_TAG, "IMP_OSD_ShowRgn(%d) Logo error\n", handle);
    return -1;
  }
  gettimeofday(&end, NULL);

  do {
    gettimeofday(&end, NULL);
    usleep(timeout / 1000 + 100);
    interval = 1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
  } while(interval > timeout);


  ret = IMP_OSD_ShowRgn(handle, group, 0);
  if (ret != 0) {
    IMP_LOG_ERR(MODULE_TAG, "IMP_OSD_ShowRgn(%d) Logo error\n", handle);
    return -1;
  }

  return 0;
}

int sample_ipu_osd_init(int group, IMPCell *fscell, IMPCell *eccell)
{
  int ret = 0;
  g_param.group = group;
  g_param.handle_num = COVER_NUM+PIC_NUM+BITMAP_NUM;
  IMPRgnHandle handle = -1;
  IMPOSDGrpRgnAttr grpRgnAttr;

  if (IMP_OSD_CreateGroup(group) < 0) {
    IMP_LOG_ERR(MODULE_TAG, "IMP_OSD_CreateGroup(%d) error !\n", group);
    return -1;
  }

  for(int i = 0; i < g_param.handle_num; i++) {

    handle = IMP_OSD_CreateRgn(NULL);
    if (handle == INVHANDLE) {
      IMP_LOG_ERR(MODULE_TAG, "IMP_OSD_CreateRgn error !\n");
      return -1;
    }

    ret = IMP_OSD_RegisterRgn(handle, group, NULL);
    if (ret < 0) {
      IMP_LOG_ERR(MODULE_TAG, "IMP_OSD_RegisterRgn failed\n");
      return -1;
    }

    if (IMP_OSD_GetGrpRgnAttr(handle, group, &grpRgnAttr) < 0) {
      IMP_LOG_ERR(MODULE_TAG, "IMP_OSD_GetGrpRgnAttr Cover error !\n");
      return -1;
    }

    memset(&grpRgnAttr, 0, sizeof(IMPOSDGrpRgnAttr));
    grpRgnAttr.show = 0;
    grpRgnAttr.gAlphaEn = 1;
    grpRgnAttr.fgAlhpa = 0xff;
    grpRgnAttr.layer = 2;

    if (IMP_OSD_SetGrpRgnAttr(handle, group, &grpRgnAttr) < 0) {
      IMP_LOG_ERR(MODULE_TAG, "IMP_OSD_SetGrpRgnAttr Cover error !\n");
      return -1;
    }

    if(i >= COVER_NUM + PIC_NUM) {
      g_param.bitmap[i-COVER_NUM-PIC_NUM].handle = handle;
      continue;
    }
    if(i >= COVER_NUM) {
      g_param.pic[i-COVER_NUM].handle = handle;
      continue;
    }
    g_param.cover[i].handle = handle;
  }

  ret = IMP_OSD_Start(group);
  if (ret < 0) {
    IMP_LOG_ERR(MODULE_TAG, "IMP_OSD_Start TimeStamp or [Logo] or [Cover] or Rect error !\n");
    return -1;
  }

  IMPCell osdcell = {DEV_ID_OSD, group, 0};
  ret = IMP_System_Bind(fscell, &osdcell);
  if (ret < 0) {
    IMP_LOG_ERR(MODULE_TAG, "Bind FrameSource channel%d and OSD failed\n");
    return -1;
  }

  ret = IMP_System_Bind(&osdcell, eccell);
  if (ret < 0) {
    IMP_LOG_ERR(MODULE_TAG, "Bind OSD group(%d) and Encoder failed\n",group);
    return -1;
  }

  return 0;
}

int sample_ipu_osd_exit(IMPCell *fscell, IMPCell *eccell)
{
  int ret;
  int group = g_param.group;
  IMPRgnHandle handle = -1;
  IMPCell osdcell = {DEV_ID_OSD, group, 0};

  ret = IMP_System_UnBind(&osdcell, eccell);
  if (ret < 0) {
    IMP_LOG_ERR(MODULE_TAG, "UnBind OSD channel(%d) and Encoder failed\n",group);
    return -1;
  }

  ret = IMP_System_UnBind(fscell, &osdcell);
  if (ret < 0) {
    IMP_LOG_ERR(MODULE_TAG, "UnBind FrameSource channel and OSD(%d) failed\n",group);
    return -1;
  }

  ret = IMP_OSD_Stop(group);
  if (ret < 0) {
    IMP_LOG_ERR(MODULE_TAG, "IMP_OSD_Stop Group(%d) error !\n", group);
    return -1;
  }

  for(int i = 0; i < g_param.handle_num; i++) {

    if(i < COVER_NUM)
      handle = g_param.cover[i].handle;
    else if(i < COVER_NUM + PIC_NUM)
      handle = g_param.pic[i-COVER_NUM].handle;
    else if(i < COVER_NUM + PIC_NUM + BITMAP_NUM)
      handle = g_param.bitmap[i-COVER_NUM-PIC_NUM].handle;

    ret = IMP_OSD_ShowRgn(handle, group, 0);
    if (ret < 0) {
      IMP_LOG_ERR(MODULE_TAG, "IMP_OSD_ShowRgn close error\n");
    }

    ret = IMP_OSD_UnRegisterRgn(handle, group);
    if (ret < 0) {
      IMP_LOG_ERR(MODULE_TAG, "IMP_OSD_UnRegisterRgn error\n");
    }

    IMP_OSD_DestroyRgn(handle);

    ret = IMP_OSD_DestroyGroup(group);
    if (ret < 0) {
      IMP_LOG_ERR(MODULE_TAG, "IMP_OSD_DestroyGroup(%d) error\n",group);
      return -1;
    }
  }

  return 0;
}

