/* penxortouch

Copyright (c) 2015 Jakob Kaivo <jakob@kaivo.net>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>
#include <X11/Xatom.h>

#define PENDEVICE	"Wacom ISDv4 EC Pen stylus"
#define TOUCHDEVICE	"ELAN Touchscreen"

#define DEVPROP		"Device Enabled"

XDevice *
open_dev(Display *dpy, const char *devname)
{
	XDeviceInfo *devs = NULL;
	XDeviceInfo *devinfo = NULL;
	XDevice *dev = NULL;
	int i, ndevs;

	devs = XListInputDevices(dpy, &ndevs);
	for (i = 0; i < ndevs; i++) {
		if (!strcmp(devname, devs[i].name)) {
			devinfo = &devs[i];
		}
	}

	if (devinfo) {
		dev = XOpenDevice(dpy, devinfo->id);
	}
	if (!dev) {
		fprintf(stderr, "Couldn't open device %s.\n", devname);
	}
	return dev;
}

Display *
open_display(void)
{
	int ev, error, xi;
	Display *dpy = XOpenDisplay(NULL);
	if (!dpy) {
		fprintf(stderr, "Couldn't open display.\n");
	} else if (!XQueryExtension(dpy, "XInputExtension", &xi, &ev, &error)) {
		fprintf(stderr, "X Input extension not supported.\n");
		XCloseDisplay(dpy);
		dpy = NULL;
	}
	return dpy;
}

int
enable(Display *dpy, XDevice *dev, int en)
{
	unsigned char data = (en ? 1 : 0);
	Atom prop = XInternAtom(dpy, DEVPROP, False);
	XChangeDeviceProperty(dpy, dev, prop, XA_INTEGER, 8, PropModeReplace, (char*)&data, 1);
	return 0;
}

void
watch(Display *dpy, XDevice *pen, XDevice *touch)
{
	XEventClass evlist[2];
	int i, proxin = -1, proxout = -1;
	XEvent ev;

	for (i = 0; i < pen->num_classes; i++) {
		if (pen->classes[i].input_class == ValuatorClass) {
			ProximityIn(pen, proxin, evlist[0]);
			ProximityOut(pen, proxout, evlist[1]);
		}
	}
	if(XSelectExtensionEvent(dpy, RootWindow(dpy, DefaultScreen(dpy)), evlist, 2) != 0) {
		fprintf(stderr, "Couldn't select events.\n");
		return;
	}

	while (1) {
		XNextEvent(dpy, &ev);
		if (ev.type == proxin) {
			enable(dpy, touch, 0);
		} else if (ev.type == proxout) {
			enable(dpy, touch, 1);
		}
	}
}

int
main(int argc, char **argv)
{
	XDevice *pen = NULL, *touch = NULL;
	Display *dpy = NULL;

	if ((dpy = open_display()) == NULL) goto error;
	if ((pen = open_dev(dpy, PENDEVICE)) == NULL) goto error;
	if ((touch = open_dev(dpy, TOUCHDEVICE)) == NULL) goto error;
	watch(dpy, pen, touch);

	/* Only get here on error conditions */
error:
	if (dpy) XCloseDisplay(dpy);
	return 1;
}
