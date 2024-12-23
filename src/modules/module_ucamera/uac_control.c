#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/prctl.h>

#include <imp/imp_audio.h>
#include <imp-common.h>

#include <usbcamera.h>
#include <global_config.h>

#ifndef T23
#include <imp/imp_dmic.h>
#endif

#define MODULE_TAG                     "uac_control"

struct Ucamera_Audio_CB_Func a_func;

static audio_info_t uac_ctx;

static int sample_set_mic_volume(int16_t db)
{
	return uac_fu_mic_volume_set(db);
}

static int sample_set_mic_mute(int mute)
{
	return uac_fu_mic_mute_set(mute);
}

static int sample_set_spk_volume(int16_t db)
{
	return uac_fu_spk_volume_set(db);
}

static int sample_set_spk_mute(int mute)
{
	return uac_fu_spk_mute_set(mute);
}

static int sample_get_record_off()
{
	printf("INFO(%s): ------RECORD  OFF------\n", MODULE_TAG);
	return 0;
}

static int sample_get_record_on()
{
	printf("INFO(%s): ------RECORD  ON------\n", MODULE_TAG);
	return 0;
}

static int sample_get_speak_off()
{
	printf("INFO(%s): ------SPEAK  OFF------\n", MODULE_TAG);
	return 0;
}

static int sample_get_speak_on()
{
	printf("INFO(%s): ------SPEAK  ON------\n", MODULE_TAG);
	return 0;
}

static pthread_t play_thread_id;
static int ao_play_stop = 0;

static void *_ao_test_play_thread(void *argv)
{
	/*int size = 0;*/
	int ret = -1;
	int devID = 0;
	int chnID = 0;

	prctl(PR_SET_NAME, "audio_speak");

	struct Ucamera_Audio_Frame u_frame;
	/*struct timeval time_cur;*/
	/*uint32_t time_recv;*/

	while (!ao_play_stop) {
		ret = IF_Ucam_Audio_Get_Frame(&u_frame);
		if (ret || u_frame.len <= 0)
		{
			usleep(10000);
			continue;
		}

		/* Step 5: send frame data. */
		IMPAudioFrame frm;
		frm.virAddr = (uint32_t *)u_frame.data;
		frm.len = u_frame.len;
		ret = IMP_AO_SendFrame(devID, chnID, &frm, BLOCK);
		if (ret != 0) {
			IMP_LOG_ERR(MODULE_TAG, "send Frame Data error\n");
			usleep(10000);
			continue;
		}

		IF_Ucam_Audio_Release_Frame(&u_frame);

	}

	/* pthread_exit(0); */
	return NULL;
}

static int sample_audio_play_start(void)
{
	int ret = -1;

	ret = pthread_create(&play_thread_id, NULL, _ao_test_play_thread, NULL);
	if (ret != 0) {
		IMP_LOG_ERR(MODULE_TAG, "[ERROR] %s: pthread_create Audio Record failed\n", __func__);
		return -1;
	}
	return ret;
}

static void sample_audio_play_stop(void)
{
	ao_play_stop = 1;
	pthread_join(play_thread_id, NULL);
}

static void imp_audio_init(void)
{
	printf("INFO[%s]: dmic_en %d, amic_en %d, spk_en %d!\n", MODULE_TAG, uac_ctx.dmic_en, uac_ctx.amic_en, uac_ctx.spk_en);
#ifdef T23
	printf("INFO[%s]: dmic_en %d, amic_en %d, spk_en %d!\n", MODULE_TAG, uac_ctx.dmic_en, uac_ctx.amic_en, uac_ctx.spk_en);
	sample_audio_amic_init();
#else
	if (uac_ctx.dmic_en) {
		sample_audio_dmic_init();
	} else if(uac_ctx.amic_en) {
		sample_audio_amic_init();
	}
#endif
	if(uac_ctx.spk_en) {
		sample_speak_init();
	}
}

static void imp_audio_deinit()
{
#ifdef T23
	sample_audio_amic_exit();
#else
	if (uac_ctx.dmic_en)
		sample_audio_dmic_exit();
	else if (uac_ctx.amic_en)
		sample_audio_amic_exit();
#endif
	if(uac_ctx.spk_en) {
		sample_speak_exit();
	}
}

static void register_audio_func(void)
{
#ifdef T23
	a_func.get_AudioPcm = sample_audio_amic_pcm_get;
#else
	if (uac_ctx.dmic_en)
		a_func.get_AudioPcm = sample_audio_dmic_pcm_get;
	else
		a_func.get_AudioPcm = sample_audio_amic_pcm_get;
#endif
	a_func.set_Mic_Volume = sample_set_mic_volume;
	a_func.set_Spk_Volume = sample_set_spk_volume;
	a_func.set_Mic_Mute = sample_set_mic_mute;
	a_func.set_Spk_Mute = sample_set_spk_mute;
	a_func.get_record_on = sample_get_record_on;
	a_func.get_record_off = sample_get_record_off;
	a_func.get_speak_on = sample_get_speak_on;
	a_func.get_speak_off = sample_get_speak_off;

	IF_Ucam_Audio_Func(&a_func);
}

int uac_control_init(void *param)
{
	int ret = 0;
	config_func_param_t *cfg_param = (config_func_param_t*)param;

	/* uac context initialize */
	memcpy(&uac_ctx, &cfg_param->audio_info, sizeof(audio_info_t));
	if(uac_ctx.amic_en || uac_ctx.dmic_en || uac_ctx.spk_en){
		imp_audio_init();

		ret = IF_Ucam_Audio_Init(AUDIO_DEVICE_NAME, &cfg_param->audio_info);
		if (ret < 0) {
			printf("ERROR(%s): IF_UCAM_AUDIO_Init failed!\n", MODULE_TAG);
			return -1;
		}

		register_audio_func();

		ret = IF_Ucam_Audio_Start(&cfg_param->audio_info);
		if (ret < 0) {
			printf("ERROR(%s): IF_UCAM_AUDIO_Start failed!\n", MODULE_TAG);
			return -1;
		}
	}

	/* speak play */
	if(uac_ctx.spk_en) {
		sample_audio_play_start();
		printf("INFO[%s]: audio play start...!\n", MODULE_TAG);
	}

	return 0;
}

void uac_control_deinit()
{
	if(!(uac_ctx.amic_en | uac_ctx.dmic_en | uac_ctx.spk_en))
		return;
	if(uac_ctx.spk_en) {
		sample_audio_play_stop();
	}

	IF_Ucam_Audio_Stop(&uac_ctx);
	IF_Ucam_Audio_DeInit();

	imp_audio_deinit();
}
