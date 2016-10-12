/****************************************************************************
 *
 *  main.cpp - event-simulator, an event simulator for Jolla QA.
 *  Copyright (C) 2015 Jolla Ltd.
 *  Contact: Maciej Blomberg <maciej.blomberg@jolla.com>
 *
 *  This file is part of event-simulator.

 *  event-simulator is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *
 *  This file incorporates work covered by the following copyright and
 *  permission notice:
 *
 ****************************************************************************/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <glob.h>
#include "simulator.h"


#define DEBUG 0

#define test_bit(a, b) (a[b/8] & (1<<(b%8)))
#define reject(comment...) { if (DEBUG) fprintf(stderr,comment); close(dev); return -1; }

Simulator::Simulator() : evdev(-1)
{
	int tracking_id_fd = open("/tmp/event-simulator-tracking-id",O_RDONLY);
	if (tracking_id_fd < 0) {
		tracking_id = 1;
	} else {
		char buffer[13];
		memset(buffer, 0, sizeof(buffer));
		read(tracking_id_fd,buffer,12);
		if (sscanf(buffer,"%d",&tracking_id) != 1) tracking_id = 1;
		if (DEBUG) fprintf(stderr,"tracking_id from file: %i\n",tracking_id);
		close(tracking_id_fd);
	}

	int evdev_path_cache_fd = open("/tmp/event-simulator-evdev-path",O_RDONLY);
	if (evdev_path_cache_fd < 0) {
		glob_t event_files;
		if (glob("/dev/input/event*",0,0,&event_files)) throw "shit";

		unsigned int i;
		for (i = 0; (i < event_files.gl_pathc) && (evdev==-1); i++) {
			evdev = open_evdev(event_files.gl_pathv[i]);
		}

		if (evdev >= 0) {
			evdev_path_cache_fd = open("/tmp/event-simulator-evdev-path", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
			if (evdev_path_cache_fd >= 0) {
				write(evdev_path_cache_fd,event_files.gl_pathv[i],strlen(event_files.gl_pathv[i]));
				close(evdev_path_cache_fd);
			}
		}
	} else {
		char buffer[33];
		memset(buffer, 0, sizeof(buffer));
		read(evdev_path_cache_fd,buffer,32);
		close(tracking_id_fd);
		evdev = open_evdev(buffer);
	}
}


Simulator::~Simulator()
{
	if (evdev >= 0) close(evdev);

	int tracking_id_fd = open("/tmp/event-simulator-tracking-id", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (tracking_id_fd >= 0) {
		char buffer[13];
		memset(buffer, 0, sizeof(buffer));
		write(tracking_id_fd,buffer,snprintf(buffer,12,"%d",tracking_id)+1);
		close(tracking_id_fd);
	}
}


int Simulator::open_evdev(char *path)
{
	int dev = open(path, O_RDWR);
	if (DEBUG) fprintf(stderr,"scanning %s\n",path);
	if (dev < 0) return -1;
	if (DEBUG) fprintf(stderr,"\topened\n");

	unsigned char abscaps[(ABS_MAX / 8) + 1];
	if (ioctl(dev, EVIOCGBIT(EV_ABS, sizeof (abscaps)), abscaps) < 0) reject("\tfailed to ioctl EV_ABS\n");
	if (!(test_bit(abscaps, ABS_MT_POSITION_X) && test_bit(abscaps, ABS_MT_POSITION_Y))) reject("\tno X(%i) or Y(%i)\n",test_bit(abscaps, ABS_MT_POSITION_X), test_bit(abscaps, ABS_MT_POSITION_Y));
	has_abs_mt_pressure = test_bit(abscaps, ABS_MT_PRESSURE);
	has_abs_mt_touch_major = test_bit(abscaps, ABS_MT_TOUCH_MAJOR);
	has_abs_mt_width_major = test_bit(abscaps, ABS_MT_WIDTH_MAJOR);
	has_abs_mt_tracking_id = test_bit(abscaps, ABS_MT_TRACKING_ID);

	unsigned char keycaps[(KEY_MAX / 8) + 1];
	if (ioctl(dev, EVIOCGBIT(EV_KEY, sizeof (keycaps)), keycaps) < 0) reject("\tfailed to ioctl EV_KEY\n");
	has_btn_touch = test_bit(keycaps, BTN_TOUCH);

	kind = ((test_bit(abscaps, ABS_MT_SLOT) && test_bit(abscaps, ABS_MT_TRACKING_ID)) ? B : A);

	if (DEBUG) fprintf(stderr,"\taccepted, type %c\n",kind+'A');

	return dev;
}


void Simulator::click(int x, int y, long long int duration)
{
	send_report(x,y,68,tracking_id);
	usleep(duration);
	if (kind == A) send_report(x,y,0,tracking_id);
	send_report(x,y,-1,-1);
	tracking_id++;
}


void Simulator::drag(int x1, int y1, int x2, int y2, long long int duration)
{
	int steps = duration / 10000;
	for (int i=0; i<steps; i++) {
		send_report(x1+((x2-x1)*i/steps),y1+((y2-y1)*i/steps),68,tracking_id);
		usleep(duration/steps);
	}
	if (kind == A) send_report(x2,y2,0,tracking_id);
	send_report(x2,y2,-1,-1);
	tracking_id++;
}


void Simulator::send_report(int x, int y, int pressure, int tracking_id)
{
	struct input_event events[9];
	int events_count = 0;

	auto enqueue_event = [&](int event_type, int event_code, int value) {
		events[events_count].type=event_type;
		events[events_count].code=event_code;
		events[events_count++].value=value;
	};

	if (kind == A) {
		x <<= 1;
		y <<= 1;
	}

	if (kind == B) enqueue_event(EV_ABS, ABS_MT_SLOT, 0);

	if (has_abs_mt_tracking_id) enqueue_event(EV_ABS, ABS_MT_TRACKING_ID, tracking_id);

	if (pressure >= 0) {
		if (has_btn_touch) enqueue_event(EV_KEY, BTN_TOUCH, (pressure>0)?1:0);
		enqueue_event(EV_ABS, ABS_MT_POSITION_X, x);
		enqueue_event(EV_ABS, ABS_MT_POSITION_Y, y);
		if (has_abs_mt_touch_major) enqueue_event(EV_ABS, ABS_MT_TOUCH_MAJOR, (pressure>0)?((kind==A)?0x32:0x18):0);
		if (has_abs_mt_width_major) enqueue_event(EV_ABS, ABS_MT_WIDTH_MAJOR, (pressure>0)?((kind==A)?0x32:0x18):0);
		if (has_abs_mt_pressure) enqueue_event(EV_ABS, ABS_MT_PRESSURE, pressure);
	}

	if (kind == A) enqueue_event(EV_SYN, SYN_MT_REPORT, 0);

	enqueue_event(EV_SYN, SYN_REPORT, 0);

	write(evdev,events,sizeof(struct input_event)*events_count);
}
