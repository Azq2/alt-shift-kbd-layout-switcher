#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XInput.h>
#include <X11/keysym.h>

#define KEY_SHIFT		XK_Shift_L
#define KEY_ALT			XK_Alt_L

#define CMD_GET_SOURCES			"gsettings get org.gnome.desktop.input-sources sources"
#define CMD_GET_CURRENT_SOURCE	"gsettings get org.gnome.desktop.input-sources current"
#define CMD_SET_CURRENT_SOURCE	"gsettings set org.gnome.desktop.input-sources current %d"

#define MAX_INPUT_CLASSES 512

static int threads_cnt = 0;
pthread_t threads_pool[64];

typedef struct {
	XEventClass classes[MAX_INPUT_CLASSES];
	int classes_n;
	int press_event_id;
	int release_event_id;
} KeyboardEvents;

int get_input_sources_count() {
	char buffer[256] = {0};
	FILE *file = popen(CMD_GET_SOURCES, "r");
	if (file) {
		fgets(buffer, sizeof(buffer) - 1, file);
		fclose(file);
	}
	
	int sources = 0;
	int array_level = 0;
	int list_level = 0;
	
	int i = 0;
	while (buffer[i]) {
		if (buffer[i] == '[') {
			++array_level;
		} else if (buffer[i] == ']') {
			--array_level;
		} else if (buffer[i] == '(') {
			++list_level;
		} else if (buffer[i] == ')') {
			--list_level;
			if (list_level == 0 && array_level == 1)
				++sources;
		}
		++i;
	}
	
	return sources;
}

int get_current_input_source() {
	char buffer[256] = {0};
	FILE *file = popen(CMD_GET_CURRENT_SOURCE, "r");
	if (file) {
		fgets(buffer, sizeof(buffer) - 1, file);
		fclose(file);
	}
	
	int num = 0;
	sscanf(buffer, "uint32 %d", &num);
	return num;
}

void set_current_input_source(int source) {
	char buffer[256];
	sprintf(buffer, CMD_SET_CURRENT_SOURCE, source);
	system(buffer);
}

void switch_input_source(int dir) {
	int current = get_current_input_source();
	int total = get_input_sources_count();
	
	current += dir;
	
	if (current < 0)
		current = total - 1;
	if (current >= total)
		current = 0;
	
	set_current_input_source(current);
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

void *switch_input_source_thread(void *x) {
	int dir = *((int *) x);
	switch_input_source(dir);
	--threads_cnt;
	return NULL;
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
					if (threads_cnt < (sizeof(threads_pool) / sizeof(threads_pool[0]))) {
						pthread_create(&threads_pool[threads_cnt], NULL, switch_input_source_thread, &dir);
						++threads_cnt;
					}
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
