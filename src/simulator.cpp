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

#include <glob.h>
#include "simulator.h"

#define test_bit(a, b) (a[b/8] & (1<<(b%8)))

Simulator::Simulator() : evdev(-1)
{
	glob_t event_files;
	if (glob("/dev/input/event*",0,0,&event_files)) throw "shit";

	for (unsigned int i = 0; (i < event_files.gl_pathc) && (evdev==-1); i++) {
		int dev = open(event_files.gl_pathv[i], O_RDWR);
		if (!dev) continue;

		unsigned char abscaps[(ABS_MAX / 8) + 1];
		if (ioctl(dev, EVIOCGBIT(EV_ABS, sizeof (abscaps)), abscaps) < 0) { close(dev); continue; }
		if (!(test_bit(abscaps, ABS_MT_POSITION_X) && test_bit(abscaps, ABS_MT_POSITION_Y) && test_bit(abscaps, ABS_MT_PRESSURE))) { close(dev); continue; }

		kind = ((test_bit(abscaps, ABS_MT_SLOT) && test_bit(abscaps, ABS_MT_TRACKING_ID)) ? B : A);
		evdev = dev;
	}
}


Simulator::~Simulator()
{
	if (evdev >= 0) close(evdev);
}


void Simulator::click(int x, int y, long long int duration)
{
	send_report(x,y,68,1);
	usleep(duration);
	send_report(x,y,0,1);
	send_report(x,y,-1,-1);
}


void Simulator::drag(int x1, int y1, int x2, int y2, long long int duration)
{
	int steps = duration / 10000;
	for (int i=0; i<steps; i++) {
		send_report(x1+((x2-x1)*i/steps),y1+((y2-y1)*i/steps),68,1);
		usleep(duration/steps);
	}
	send_report(x2,y2,0,1);
	send_report(x2,y2,-1,-1);
}


void Simulator::send_report(int x, int y, int pressure, int tracking_id)
{
	struct input_event events[8];
	int events_count = 0;

	if (kind == A) {
		x <<= 1;
		y <<= 1;
	}

	if (kind == B) {
		events[events_count].type=EV_ABS; events[events_count].code=ABS_MT_SLOT;        events[events_count++].value=0;
		events[events_count].type=EV_ABS; events[events_count].code=ABS_MT_TRACKING_ID; events[events_count++].value=tracking_id;
	}

	if (pressure >= 0) {
		events[events_count].type=EV_ABS; events[events_count].code=ABS_MT_POSITION_X;  events[events_count++].value=x;
		events[events_count].type=EV_ABS; events[events_count].code=ABS_MT_POSITION_Y;  events[events_count++].value=y;
		events[events_count].type=EV_ABS; events[events_count].code=ABS_MT_PRESSURE;    events[events_count++].value=pressure;
		events[events_count].type=EV_ABS; events[events_count].code=ABS_MT_TOUCH_MAJOR; events[events_count++].value=6;
	}

	if (kind == A) {
		events[events_count].type=EV_SYN; events[events_count].code=SYN_MT_REPORT;  events[events_count++].value=0;
	}

	events[events_count].type=EV_SYN; events[events_count].code=SYN_REPORT;  events[events_count++].value=x;

	write(evdev,events,sizeof(struct input_event)*events_count);
}
	
