//
// Created by v on 24-10-6.
//

#include "wdt.h"

/*
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/watchdog.h>

static int fd;

int wdt_keep_alive(void)
{
	int ret = -1;
    int dummy;
    ret = ioctl(fd, WDIOC_KEEPALIVE, &dummy);
	if (0 != ret) {
		printf("err(%s,%d): %s\n", __func__, __LINE__, strerror(errno));
		return -1;
	}
	return 0;
}

int wdt_enable()
{
    if (fd <= 0)
    {
        fd = open("/dev/watchdog", O_WRONLY);
        if (-1 == fd) {
            printf("err(%s,%d): %s\n", __func__, __LINE__, strerror(errno));
            return -2;
        }
    }
	int ret = -1;
	int flags = 0;
	flags = WDIOS_ENABLECARD;
	ret = ioctl(fd, WDIOC_SETOPTIONS, &flags);
	if (0 != ret) {
		printf("err(%s,%d): %s\n", __func__, __LINE__, strerror(errno));
		return -1;
	}
	return 0;
}

int wdt_disable()
{
    if (fd <= 0)
    {
        return 0;
    }
	int ret = -1;
	int flags = 0;
	flags = WDIOS_DISABLECARD;
	ret = ioctl(fd, WDIOC_SETOPTIONS, &flags);
	if (0 != ret) {
		printf("err(%s,%d): %s\n", __func__, __LINE__, strerror(errno));
		return -1;
	}
    close(fd);
	return 0;

}

int wdt_set_timeout(int to)
{
	int ret = -1;
    int timeout = to;
    ret = ioctl(fd, WDIOC_SETTIMEOUT, &timeout);
	if (0 != ret) {
		printf("err(%s,%d): %s\n", __func__, __LINE__, strerror(errno));
		return -1;
	}
	return 0;

}

int wdt_get_timeout()
{
	int ret = -1;
    int timeout = 0;
    ret = ioctl(fd, WDIOC_GETTIMEOUT, &timeout);
	if (0 != ret) {
		printf("err(%s,%d): %s\n", __func__, __LINE__, strerror(errno));
		return -1;
	}
	return timeout;

}
