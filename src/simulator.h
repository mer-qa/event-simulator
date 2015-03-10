/****************************************************************************
 *
 *  simulator.h - event-simulator, an event simulator for Jolla QA.
 *  Copyright (C) 2015 Jolla Ltd.
 *  Contact: Maciej Blomberg <maciej.blomberg@jolla.com>
 *
 *  This file is part of event-simulator.

 *  event-simulator is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 ****************************************************************************/

#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/input.h>

class Simulator {
public:
	int evdev;
	enum DeviceKind { A, B } kind;

	Simulator();
	~Simulator();
	
	void click(int x, int y, long long int duration);
	void drag(int x1, int y1, int x2, int y2, long long int duration);
	void send_report(int x, int y, int pressure, int tracking_id);

};

#endif
