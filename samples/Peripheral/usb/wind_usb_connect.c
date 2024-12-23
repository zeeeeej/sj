
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/prctl.h>

#include <imp-common.h>
#include <global_config.h>
#include <module_config.h>
#include <module_ucamera_control.h>


#define MOTAG_TAG            "sample_ucamera"

#include "wind_usb_connect.h"


int start_usb_connect()
{
	int ret = -1;
	printf("startup [%s:%s]\n", __DATE__, __TIME__);
	IMP_LOG_DBG("main", "program startup\n");

	/*=============================
	 * Step 1 :config init
	 *============================*/
	ret = module_config_init( UVC_CONFIG_FILE_PATH );
	if ( ret < 0 ) {
		printf( "ERROR(%s): module_config_init failed!\n", MOTAG_TAG );
		return -1;
	}
	system_config_init( &g_func_param );
	IMP_LOG_DBG("main", "imp system go to init\n");

	/*==============================
	 * Step 2: imp system init
	 *==============================*/
	sample_system_init( &g_func_param.imp_attr );

	/*==============================
	 * Step 3: ucamera_int
	 *==============================*/
	ret = module_ucamera_init( &g_func_param );
	if ( ret < 0 ) {
		printf( "ERROR(%s): module_ucamera_int failed!\n", MOTAG_TAG );
		return -1;
	}
	printf(" INFO[%s]: module ucamera init ok!\n", MOTAG_TAG );
    return 0;
}

int stop_usb_connect()
{
    module_ucamera_deinit();
    sample_system_exit();
    module_config_deinit();
    return 0;
}