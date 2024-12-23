/*
 *	uvc_gadget.h  --  USB Video Class Gadget driver
 *
 *	Copyright (C) 2009-2010
 *	    Laurent Pinchart (laurent.pinchart@ideasonboard.com)
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 */

#ifndef _U_CAM_H_
#define _U_CAM_H_

#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/usb/ch9.h>
#include <linux/kconfig.h>
#include <linux/usb/composite.h>
#include <linux/usb/gadget.h>


#define FUNC_MAX_CNT				6

typedef struct _device_info {
	int adb_en;
	int Vid;
	int Pid;
	int version;
	char serial[64];
	char product[64];
	char manufacturer[64];
}device_info_t;

struct ucamera_dev_function {
	/* function name */
	char *name;

	/* dev config */
	void *config;

	/* reserve: config for configfs */
	struct device *dev;
	char *dev_name;
	struct device_attribute **attributes;

	/* for enable functions */
	struct list_head enabled_list;

	/* Optional: initialization during gadget bind */
	int (*init)(struct ucamera_dev_function *, struct usb_composite_dev *);

	/* Optional: deinit during gadget unbind */
	void (*cleanup)(struct ucamera_dev_function *);

	/* Optional: called when the function is added the list of enabled functions */
	void (*enable)(struct ucamera_dev_function *);

	/* Optional: called when it is removed */
	void (*disable)(struct ucamera_dev_function *);

	/* Optional：called when add function config */
	int (*bind_config)(struct ucamera_dev_function *, struct usb_configuration *);

	/* Optional: called when the configuration is removed */
	void (*unbind_config)(struct ucamera_dev_function *, struct usb_configuration *);

	/* Optional: handle ctrl requests before the device is configured */
	int (*ctrlrequest)(struct ucamera_dev_function *, struct usb_composite_dev *,
			   const struct usb_ctrlrequest *);

	long (*cmd_handle)(unsigned int , unsigned long);

	void *def_param;
	void *drv_data;
};

struct ucamera_dev {
        struct ucamera_dev_function **functions;
        struct list_head enabled_functions;
        struct usb_composite_dev *cdev;
        struct device *dev;

        bool enabled;
        int disable_depth;
        struct mutex mutex;
	int supported_functions_num;
};



struct ucamera_dev_function *uvc_function_get_instance(void);
#ifdef CONFIG_DUAL_VIDEO
struct ucamera_dev_function *uvc2_function_get_instance(void);
#endif
struct ucamera_dev_function *uac_function_get_instance(void);
struct ucamera_dev_function *hid_function_get_instance(void);
struct ucamera_dev_function *adb_function_get_instance(void);
struct ucamera_dev_function *acm_function_get_instance(void);
struct ucamera_dev_function *imd_function_get_instance(void);

#endif /* _U_CAM_H_ */

