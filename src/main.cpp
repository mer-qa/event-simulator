/****************************************************************************
 *
 *  main.cpp - event-simulator, an event simulator for Jolla QA.
 *  Copyright (C) 2015 Jolla Ltd.
 *  Contact: Maciej Blomberg <maciej.blomberg@jolla.com>
 *
 *  This file is part of event-simulator.
 *
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
 ****************************************************************************/

#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>


#include "simulator.h"

int main(int argc, char *argv[])
{
	Simulator simulator;
	if (simulator.evdev < 0) return 1;

	if ((argc < 4) || (argc > 5)) {
		fprintf(stderr,"simulator click|drag x,y duration [x2,y2]\n");
		return 2;
	}

	if (strcmp(argv[1],"click") == 0) {
		int x,y;
		long long int duration;
		if (sscanf(argv[2],"%d,%d",&x,&y) != 2) { fprintf(stderr,"wtf? coords, gif proper coords nao\n"); return 3; }
		if (sscanf(argv[3],"%Ld",&duration) != 1) { fprintf(stderr,"wtf? duration is a must\n"); return 4; }
		simulator.click(x,y,duration);
	} else if (strcmp(argv[1],"drag") == 0) {
		int x1,y1,x2,y2;
		long long int duration;
		if (sscanf(argv[2],"%d,%d",&x1,&y1) != 2) { fprintf(stderr,"wtf? coords, gif proper coords nao\n"); return 3; }
		if (sscanf(argv[3],"%Ld",&duration) != 1) { fprintf(stderr,"wtf? duration is a must\n"); return 4; }
		if (sscanf(argv[4],"%d,%d",&x2,&y2) != 2) { fprintf(stderr,"wtf? coords, gif proper coords nao\n"); return 3; }
		simulator.drag(x1,y1,x2,y2,duration);
	} else {
		fprintf(stderr,"click or drag, really, is it *that* hard?\n");
		return 5;
	}

	return 0;
}

