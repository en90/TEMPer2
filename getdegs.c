/*
 * Copyright (c) 2009 Robert Kavaler (relavak.com). All rights reserved.
 * Copyright (c) 2012 Sylvain Leroux <sylvain@chicoree.fr>
 * Copyright (c) 2016 Erik Nordstrøm <erik@nordstroem.no>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <usb.h>
#include <errno.h>
#include <unistd.h>

struct TemperData {
	float value;
	enum Unit {
		TEMPER_ABS_TEMP,	/* absolute temperature  (in °C) */
	} unit;
};
typedef struct TemperData TemperData;

struct Temper {
        struct usb_device *device;
        usb_dev_handle *handle;
        int timeout;
};
typedef struct Temper Temper;

#if !defined TEMPER_TIMEOUT
#define TEMPER_TIMEOUT 1000	/* milliseconds */
#endif

Temper * TemperCreate (struct usb_device *device, int timeout)
{
	Temper *t;

	t = calloc(1, sizeof(*t));
	t->device = device;
	t->timeout = timeout;

	t->handle = usb_open(t->device);

	if (!t->handle)
	{
		free(t);
		return NULL;
	}

	if (usb_detach_kernel_driver_np(t->handle, 0))
	{
		if (errno == ENODATA)
		{
			fprintf(stderr, "Device already detached\n");
		}
		else
		{
			fprintf(stderr, "Detach failed: %s[%d]\n",
				strerror(errno), errno);
		}
	}

	if (usb_detach_kernel_driver_np(t->handle, 1))
	{
		if (errno == ENODATA)
		{
			fprintf(stderr, "Device already detached\n");
		}
		else
		{
			fprintf(stderr, "Detach failed: %s[%d]\n",
				strerror(errno), errno);
		}
	}

	if (usb_set_configuration(t->handle, 1) < 0 ||
		usb_claim_interface(t->handle, 0) < 0 ||
		usb_claim_interface(t->handle, 1))
	{
		usb_close(t->handle);
		free(t);

		return NULL;
	}

	return t;
}

Temper * TemperCreateFromDeviceNumber (int deviceNum, int timeout)
{
	struct usb_bus *bus;
	int n = 0;

	for (bus=usb_get_busses(); bus; bus=bus->next)
	{
		struct usb_device *dev;

		for (dev=bus->devices; dev; dev=dev->next)
		{
			if (dev->descriptor.idVendor == 0x0c45 &&
				dev->descriptor.idProduct == 0x7401)
			{
				if (n == deviceNum)
				{
					return TemperCreate(dev, timeout);
				}

				n++;
			}
		}
	}

	return NULL;
}

/*
void TemperFree (Temper *t)
{
	if (t)
	{
		if (t->handle)
		{
			usb_close(t->handle);
		}

		free(t);
	}
}
*/

int main(int argc, char **argv)
{
	Temper *t;

	usb_init();
	usb_find_busses();
	usb_find_devices();

	t = TemperCreateFromDeviceNumber(0, TEMPER_TIMEOUT);

	if (!t)
	{
		perror("TemperCreate");
		return EXIT_FAILURE;
	}

	static unsigned char cmd[8+8*8] =
		{0x01, 0x80, 0x33, 0x01, 0x00, 0x00, 0x00};

	unsigned char buf[8];
	TemperData data[2];
	fprintf(stderr, "Internal [°C]\tExternal [°C]\n");
	for (;;)
	{
		int ret_cmd = usb_control_msg(t->handle, 0x21, 9, 0x200, 0x01,
			(char *) cmd, sizeof(cmd), t->timeout);

		if (ret_cmd != sizeof(cmd))
		{
			perror("usb_control_msg failed");
			return EXIT_FAILURE;
		}

		int ret_buf = usb_interrupt_read(t->handle, 0x82,
			(char*)buf, sizeof(buf), t->timeout);

		if (ret_buf != sizeof(buf))
		{
			perror("usb_interrupt_read failed");
			return EXIT_FAILURE;
		}

		for(int i = 0; i < 2; ++i)
		{
			int16_t word = ((int8_t)buf[2*i+2] << 8) | buf[2*i+3];
			data[i].value = ((float)word) * (125.0 / 32000.0);
			data[i].unit = TEMPER_ABS_TEMP;
		}

		printf("%+13.2f\t%+13.2f\n", data[0].value, data[1].value);
		sleep(1);
	}

	return EXIT_SUCCESS;
}
