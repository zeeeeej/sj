/*************************************************************************
 * Copyright (C) 2005~2022 Ingenic Semiconductor Co.,Ltd
 *
 * File Name    : imp_audio_ctl.c
 * Author       : wxyang
 * Mail         : wxyang@ingenic.com
 * Created Time : 2022-11-24 17:42
 *************************************************************************/
#include <stdio.h>
#include <string.h>
#include <global_config.h>
#include <imp-common.h>
#include <imp/imp_audio.h>

#define MODULE_TAG "IMP_AUDIO"

void sample_audio_amic_init(void)
{
  int ret = -1;
  audio_info_t uac_ctx;
  memcpy(&uac_ctx, &g_func_param.audio_info, sizeof(audio_info_t));

  printf("INFO[%s]: samplerate %d, bitwidth %d, soundmode %d!\n", MODULE_TAG, uac_ctx.amic_info.samplerate, uac_ctx.amic_info.bitwidth, uac_ctx.amic_info.soundmode);
  /* Step 1: set public attribute of AI device. */

  int devID = 1;
  IMPAudioIOAttr attr;
  attr.samplerate = uac_ctx.amic_info.samplerate;// AUDIO_SAMPLE_RATE_16000;
  attr.bitwidth = uac_ctx.amic_info.bitwidth;
  attr.soundmode = uac_ctx.amic_info.soundmode;
  attr.frmNum = 2;
  attr.numPerFrm = (uac_ctx.amic_info.samplerate)/1000*40;
  attr.chnCnt = uac_ctx.amic_info.prev[0];
  ret = IMP_AI_SetPubAttr(devID, &attr);
  if(ret != 0) {
    IMP_LOG_ERR(MODULE_TAG, "set ai %d attr err: %d\n", devID, ret);
    return;
  }

  memset(&attr, 0x0, sizeof(attr));
  ret = IMP_AI_GetPubAttr(devID, &attr);
  if(ret != 0) {
    IMP_LOG_ERR(MODULE_TAG, "get ai %d attr err: %d\n", devID, ret);
    return;
  }

#if 0
  printf("Audio In GetPubAttr samplerate : %d\n", attr.samplerate);
  printf("Audio In GetPubAttr   bitwidth : %d\n", attr.bitwidth);
  printf("Audio In GetPubAttr  soundmode : %d\n", attr.soundmode);
  printf("Audio In GetPubAttr     frmNum : %d\n", attr.frmNum);
  printf("Audio In GetPubAttr  numPerFrm : %d\n", attr.numPerFrm);
  printf("Audio In GetPubAttr     chnCnt : %d\n", attr.chnCnt);
#endif

  /* Step 2: enable AI device. */
  ret = IMP_AI_Enable(devID);
  if(ret != 0) {
    IMP_LOG_ERR(MODULE_TAG, "enable ai %d err\n", devID);
    return;
  }

  IMP_LOG_INFO(MODULE_TAG, "INFO[%s]: AI enable ok!\n", MODULE_TAG);

  /* Step 3: set audio channel attribute of AI device. */
  int chnID = 0;
  IMPAudioIChnParam chnParam;
  chnParam.usrFrmDepth = 2;
  ret = IMP_AI_SetChnParam(devID, chnID, &chnParam);
  if(ret != 0) {
    IMP_LOG_ERR(MODULE_TAG, "set ai %d channel %d attr err: %d\n", devID, chnID, ret);
    return;
  }

  memset(&chnParam, 0x0, sizeof(chnParam));
  ret = IMP_AI_GetChnParam(devID, chnID, &chnParam);
  if(ret != 0) {
    IMP_LOG_ERR(MODULE_TAG, "get ai %d channel %d attr err: %d\n", devID, chnID, ret);
    return;
  }

  IMP_LOG_INFO(MODULE_TAG, "Audio In GetChnParam usrFrmDepth : %d\n", chnParam.usrFrmDepth);

  /* Step 4: enable AI channel. */
  ret = IMP_AI_EnableChn(devID, chnID);
  if(ret != 0) {
    IMP_LOG_ERR(MODULE_TAG, "Audio Record enable channel failed\n");
    return;
  }

  /* Step 5: Set audio channel volume. */
  int chnVol = uac_ctx.amic_info.volume;
  chnVol = 2 * chnVol + 60;
  ret = IMP_AI_SetVol(devID, chnID, chnVol);
  if(ret != 0) {
    IMP_LOG_ERR(MODULE_TAG, "Audio Record set volume failed\n");
    return;
  }

  ret = IMP_AI_GetVol(devID, chnID, &chnVol);
  if(ret != 0) {
    IMP_LOG_ERR(MODULE_TAG, "Audio Record get volume failed\n");
    return;
  }
  IMP_LOG_INFO(MODULE_TAG, "Audio In GetVol    vol : %d\n", chnVol);

  int aigain = 31;
  ret = IMP_AI_SetGain(devID, chnID, aigain);
  if(ret != 0) {
    IMP_LOG_ERR(MODULE_TAG, "Audio Record Set Gain failed\n");
    return;
  }

  ret = IMP_AI_GetGain(devID, chnID, &aigain);
  if(ret != 0) {
    IMP_LOG_ERR(MODULE_TAG, "Audio Record Get Gain failed\n");
    return;
  }
  IMP_LOG_INFO(MODULE_TAG, "Audio In GetGain    gain : %d\n", aigain);

  int audio_ns = uac_ctx.audio_ns;
  if (audio_ns >= 0) {
    if (audio_ns < NS_LOW || audio_ns > NS_VERYHIGH) {
      IMP_LOG_ERR(MODULE_TAG, "Audio set unvilid NS Leave. \n");
      return ;
    }
    ret = IMP_AI_EnableNs(&attr, audio_ns);
    if(ret != 0) {
      printf("enable audio ns error.\n");
      IMP_LOG_INFO(MODULE_TAG, "enable audio ns error.\n");
      return ;
    }

  }

  return;

}

void sample_audio_amic_exit()
{
  int ret = -1;

  /* Step 1: set public attribute of AI device. */
  int devID = 1;
  int chnID = 0;
  /* Step 9: disable the audio channel. */
  ret = IMP_AI_DisableChn(devID, chnID);
  if(ret != 0) {
    IMP_LOG_ERR(MODULE_TAG, "Audio channel disable error\n");
    return;
  }

  /* Step 10: disable the audio devices. */
  ret = IMP_AI_Disable(devID);
  if(ret != 0) {
    IMP_LOG_ERR(MODULE_TAG, "Audio device disable error\n");
    return;
  }
  return;
}

int sample_audio_amic_pcm_get(short *pcm)
{

  /* Step 6: get audio record frame. */
  int ret = 0;
  int devID = 1;
  int chnID = 0;

  ret = IMP_AI_PollingFrame(devID, chnID, 1000);
  if (ret != 0 ) {
    IMP_LOG_ERR(MODULE_TAG, "Audio Polling Frame Data error\n");
  }
  IMPAudioFrame frm;
  ret = IMP_AI_GetFrame(devID, chnID, &frm, BLOCK);
  if(ret != 0) {
    IMP_LOG_ERR(MODULE_TAG, "Audio Get Frame Data error\n");
    return 0;
  }

  /* Step 7: Save the recording data to a file. */
  memcpy((void *)pcm, frm.virAddr, frm.len);

  //struct timeval start;
  //gettimeofday(&start,NULL);
  //unsigned long long start_1 = 1000000 * start.tv_sec  +  start.tv_usec;
  //printf("%s: timestamp = %llu star_1 = %llu.\n",__func__, frm.timeStamp, start_1);
  /* Step 8: release the audio record frame. */
  ret = IMP_AI_ReleaseFrame(devID, chnID, &frm);
  if(ret != 0) {
    IMP_LOG_ERR(MODULE_TAG, "Audio release frame data error\n");
    return 0;
  }
  return frm.len;

}

int sample_speak_init(void)
{
  int ret = -1;
  /* Step 1: set public attribute of AO device. */
  audio_info_t uac_ctx;
  memcpy(&uac_ctx, &g_func_param.audio_info, sizeof(audio_info_t));

  int devID = 0;
  IMPAudioIOAttr attr;

  attr.samplerate = uac_ctx.spk_info.samplerate;
  attr.bitwidth = uac_ctx.spk_info.bitwidth;
  attr.soundmode = uac_ctx.spk_info.soundmode;
  attr.frmNum = 2;
  attr.numPerFrm = (uac_ctx.spk_info.samplerate)/1000*40;
  attr.chnCnt = 1;
  ret = IMP_AO_SetPubAttr(devID, &attr);
  if (ret != 0) {
    IMP_LOG_ERR(MODULE_TAG, "set ao %d attr err: %d\n", devID, ret);
    return -1;
  }

  memset(&attr, 0x0, sizeof(attr));
  ret = IMP_AO_GetPubAttr(devID, &attr);
  if (ret != 0) {
    IMP_LOG_ERR(MODULE_TAG, "get ao %d attr err: %d\n", devID, ret);
    return -1;
  }

  /* IMP_LOG_INFO(MODULE_TAG, "Audio Out GetPubAttr samplerate:%d\n", attr.samplerate); */
  /* IMP_LOG_INFO(MODULE_TAG, "Audio Out GetPubAttr   bitwidth:%d\n", attr.bitwidth); */
  /* IMP_LOG_INFO(MODULE_TAG, "Audio Out GetPubAttr  soundmode:%d\n", attr.soundmode); */
  /* IMP_LOG_INFO(MODULE_TAG, "Audio Out GetPubAttr     frmNum:%d\n", attr.frmNum); */
  /* IMP_LOG_INFO(MODULE_TAG, "Audio Out GetPubAttr  numPerFrm:%d\n", attr.numPerFrm); */
  /* IMP_LOG_INFO(MODULE_TAG, "Audio Out GetPubAttr     chnCnt:%d\n", attr.chnCnt); */

  /* Step 2: enable AO device. */
  ret = IMP_AO_Enable(devID);
  if (ret != 0) {
    IMP_LOG_ERR(MODULE_TAG, "enable ao %d err\n", devID);
    return -1;
  }

  /* Step 3: enable AI channel. */
  int chnID = 0;
  ret = IMP_AO_EnableChn(devID, chnID);
  if (ret != 0) {
    IMP_LOG_ERR(MODULE_TAG, "Audio play enable channel failed\n");
    return -1;
  }

  /* Step 4: Set audio channel volume. */
  int chnVol = uac_ctx.spk_info.volume;
  chnVol = chnVol * 2 + 60;
  ret = IMP_AO_SetVol(devID, chnID, chnVol);
  if (ret != 0) {
    IMP_LOG_ERR(MODULE_TAG, "Audio Play set volume failed\n");
    return -1;
  }

  ret = IMP_AO_GetVol(devID, chnID, &chnVol);
  if (ret != 0) {
    IMP_LOG_ERR(MODULE_TAG, "Audio Play get volume failed\n");
    return -1;
  }
  /* IMP_LOG_INFO(MODULE_TAG, "Audio Out GetVol    vol:%d\n", chnVol); */

  int aogain = 28;
  ret = IMP_AO_SetGain(devID, chnID, aogain);
  if (ret != 0) {
    IMP_LOG_ERR(MODULE_TAG, "Audio Record Set Gain failed\n");
    return -1;
  }

  ret = IMP_AO_GetGain(devID, chnID, &aogain);
  if (ret != 0) {
    IMP_LOG_ERR(MODULE_TAG, "Audio Record Get Gain failed\n");
    return -1;
  }
  /* IMP_LOG_INFO(MODULE_TAG, "Audio Out GetGain    gain : %d\n", aogain); */

  return 0;
}

void sample_speak_exit(void)
{
  int ret = -1;
  int devID = 0;
  int chnID = 0;

  ret = IMP_AO_FlushChnBuf(devID, chnID);
  if (ret != 0) {
    IMP_LOG_ERR(MODULE_TAG, "IMP_AO_FlushChnBuf error\n");
    return;
  }
  /* Step 6: disable the audio channel. */
  ret = IMP_AO_DisableChn(devID, chnID);
  if (ret != 0) {
    IMP_LOG_ERR(MODULE_TAG, "Audio channel disable error\n");
    return;
  }

  /* Step 7: disable the audio devices. */
  ret = IMP_AO_Disable(devID);
  if (ret != 0) {
    IMP_LOG_ERR(MODULE_TAG, "Audio device disable error\n");
    return ;
  }

}

/* ----------------- USB Audio Class 1.0 process -----------------*/
/* ----------------- Feature Unit ----------------- */

static int g_vol = 0;
/*
 * @func: uac_fu_mic_mute_set
 * @brief: MUTE_CONTROL
 * @param:
 *   mute: mute value

 * @return:
 *   -1 error, 0 succeed
 */
int uac_fu_mic_mute_set(int mute)
{
  int ret = -1;
  printf("set mic volume mute:%d\n", mute);

  audio_info_t uac_ctx;
  memcpy(&uac_ctx, &g_func_param.audio_info, sizeof(audio_info_t));

  ret = IMP_AI_SetVolMute(1, 0, mute);

  if(ret != 0) {
    IMP_LOG_ERR(MODULE_TAG, "Audio Record set volume failed\n");
    return -1;
  }

  return 0;
}

/*
 * @func: uac_fu_mic_volume_set
 * @brief: VOLMUE_CONTROL
 * @param:
 *   db: volume

 * @return:
 *   -1 error, 0 succeed
 */
int uac_fu_mic_volume_set(int16_t db)
{
  int ret, vol;
  vol = db / 128 + 60;
  g_vol = vol;

  printf("set mic volume db:0x%x percent:%d\n", db, vol);

  ret = IMP_AI_SetVol(1, 0, vol);

  if(ret != 0) {
    IMP_LOG_ERR(MODULE_TAG, "Audio Record set volume failed\n");
    return -1;
  }

  return 0;
}

/*
 * @func: uac_fu_spk_mute_set
 * @brief: MUTE_CONTROL
 * @param:
 *   mute: mute value

 * @return:
 *   -1 error, 0 succeed
 */
int uac_fu_spk_mute_set(int mute)
{
  printf("set spk volume mute:%d\n", mute);

  int ret = IMP_AO_SetVolMute(0, 0, mute);
  if (ret != 0) {
    IMP_LOG_ERR(MODULE_TAG, "Audio Play set volume failed\n");
    return -1;
  }

  return 0;
}

/*
 * @func: uac_fu_spk_volume_set
 * @brief: VOLMUE_CONTROL
 * @param:
 *   db: volume

 * @return:
 *   -1 error, 0 succeed
 */
int uac_fu_spk_volume_set(int16_t db)
{
  int ret, vol;
  vol = db / 128 + 60;
  printf("set spk volume db:0x%x percent:%d\n", db, vol);

  ret = IMP_AO_SetVol(0, 0, vol);
  if (ret != 0) {
    IMP_LOG_ERR(MODULE_TAG, "Audio Play set volume failed\n");
    return -1;
  }

  return 0;
}
