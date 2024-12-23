/*
 *	ucam_driver.c -- USB camera gadget driver
 *
 *	Copyright (C) 2009-2010
 *	    Laurent Pinchart (laurent.pinchart@ideasonboard.com)
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <ucam.h>

extern int ucamera_init(int(*ucam_add_func)(int ,struct ucamera_dev*));
extern void ucamera_exit(void);

/*
 * add fucntion , 默认 所有function 都使能
 */
static int ucamera_add_functions(int func_num, struct ucamera_dev *dev)
{
	int ret = -1;
	int cnt = 0;
	struct ucamera_dev_function *f = NULL;
	struct ucamera_dev_function **supported_functions;
	supported_functions = kmalloc(func_num * sizeof(struct ucamera_dev_function *), GFP_KERNEL);
	if (!supported_functions) {
		ret = -ENOMEM;
		printk("ucamera add function failed, alloc mem error! \n");
		return ret;
	}

	/*Step1: add uvc function */
	f = uvc_function_get_instance();
	if (!f) {
		printk("uvc_function_get_instance error ! \n");
		goto uvc_get_error;
	}
	list_add_tail(&f->enabled_list, &dev->enabled_functions);

	supported_functions[cnt] = f;
	cnt++;

#ifdef CONFIG_DUAL_VIDEO
	f = uvc2_function_get_instance();
	if (!f) {
		printk("uvc_function_get_instance error ! \n");
		goto uvc_get_error;
	}
	list_add_tail(&f->enabled_list, &dev->enabled_functions);

	supported_functions[cnt] = f;
	cnt++;
#endif

	/*Step2: add uac function */
	f = uac_function_get_instance();
	if (!f) {
		printk("uac_function_get_instance error ! \n");
		goto uac_get_error;
	}

	supported_functions[cnt] = f;
	cnt++;

	/*Step3: add adb function */
	// f = adb_function_get_instance();
	// if (!f) {
	// 	goto adb_get_error;
	// }

	f = imd_function_get_instance();
	if(!f)
	{
		return -1;
	}
	printk("imd scuess ! \n");
	list_add_tail(&f->enabled_list, &dev->enabled_functions);
	supported_functions[cnt] = f;
	cnt++;

	dev->functions = supported_functions;
	return cnt;

adb_get_error:
uac_get_error:
uvc_get_error:
	return -1;
}

static int __init ucamera_module_init(void)
{
	printk("module init start \n");
	return ucamera_init(ucamera_add_functions);
}

static void __exit ucamera_module_exit(void)
{
	return ucamera_exit();
}

module_init(ucamera_module_init);
module_exit(ucamera_module_exit);

MODULE_AUTHOR("Laurent Pinchart");
MODULE_DESCRIPTION("USB Camera Driver");
MODULE_LICENSE("GPL");

