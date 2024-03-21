#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XInput.h>
#include <X11/keysym.h>

#define KEY_SHIFT		XK_Shift_L
#define KEY_ALT			XK_Alt_L

#define MAX_INPUT_CLASSES 512

typedef struct {
	XEventClass classes[MAX_INPUT_CLASSES];
	int classes_n;
	int press_event_id;
	int release_event_id;
} KeyboardEvents;

void switch_input_source(int dir) {
	if (dir < 0) {
		system("qdbus org.kde.keyboard /Layouts org.kde.KeyboardLayouts.switchToPreviousLayout");
	} else {
		system("qdbus org.kde.keyboard /Layouts org.kde.KeyboardLayouts.switchToNextLayout");
	}
}

void attach_input_devices(Display *display, KeyboardEvents *events) {
	memset(events, 0, sizeof(KeyboardEvents));
	
	int num_devices = 0;
	XDeviceInfo *devices = XListInputDevices(display, &num_devices);
	Atom type_keyboard = XInternAtom(display, XI_KEYBOARD, 0);
	
	// Search for keyboard devices
	for (int i = 0; i < num_devices; ++i) {
		if (devices[i].type == type_keyboard) {
			XDevice *device = XOpenDevice(display, devices[i].id);
			if (device) {
				for (int j = 0; j < device->num_classes; ++j) {
					// Create key events
					if (device->classes[j].input_class == KeyClass) {
						
						if (events->classes_n + 1 < MAX_INPUT_CLASSES) {
							DeviceKeyPress(device, events->press_event_id, events->classes[events->classes_n]);
							events->classes_n++;
							
							DeviceKeyRelease(device, events->release_event_id, events->classes[events->classes_n]);
							events->classes_n++;
							
							fprintf(stderr, "OK: %s\n", devices[i].name);
						} else {
							fprintf(stderr, "ERROR: %s (Too much input devices!)\n", devices[i].name);
						}
					}
				}
				XCloseDevice(display, device);
			} else {
				fprintf(stderr, "Can't open device: %s\n", devices[i].name);
			}
		}
	}
	
	// Attach key events
	XSelectExtensionEvent(display, DefaultRootWindow(display), events->classes, events->classes_n);
	
	if (devices)
		XFreeDeviceList(devices);
}

int main() {
	Display *display;
	XEvent event;
	KeyboardEvents events;
	
	display = XOpenDisplay(NULL);
	if (!display) {
		fprintf(stderr, "Can't open X11 display!\n");
		return 1;
	}
	
	XEventClass class_presence;
	int xi_presence;
	
	// Notify when input device add or remove
	DevicePresence(display, xi_presence, class_presence);
	XSelectExtensionEvent(display, DefaultRootWindow(display), &class_presence, 1);
	
	// Find all keyboard devices and select events
	attach_input_devices(display, &events);
	
	int alt_pressed = 0;
	int shift_pressed = 0;
	
	while (1) {
		XNextEvent(display, &event);
		if (event.type == xi_presence) {
			// Reset and find all keyboard devices and select events
			attach_input_devices(display, &events);
			alt_pressed = shift_pressed = 0;
		} else {
			if (event.type == events.press_event_id) {
				XDeviceKeyEvent *key = (XDeviceKeyEvent *) &event;
				KeySym ks = XkbKeycodeToKeysym(display, key->keycode, 0, 0);
				
				int dir = 0;
				
				if (ks == KEY_SHIFT) {
					if (alt_pressed)
						dir = 1;
					shift_pressed = 1;
				}
				
				if (ks == KEY_ALT) {
					if (shift_pressed)
						dir = -1;
					alt_pressed = 1;
				}
				
				if (dir) {
					switch_input_source(dir);
				}
			} else if (event.type == events.release_event_id) {
				XDeviceKeyEvent *key = (XDeviceKeyEvent *) &event;
				KeySym ks = XkbKeycodeToKeysym(display, key->keycode, 0, 0);
				
				if (ks == KEY_SHIFT)
					shift_pressed = 0;
				
				if (ks == KEY_ALT)
					alt_pressed = 0;
			}
		}
	}
	
	return 0;
}
