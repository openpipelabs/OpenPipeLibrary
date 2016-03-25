/**
 * Copyright (c) 2016 OpenPipe Labs. <openpipelabs@gmail.com>. All rights reserved.
 *
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OPENPIPE_H
#define OPENPIPE_H

#include "fingerings.h"

class OpenPipeClass
{
public:
	unsigned char note;
	unsigned char drone_note;
	OpenPipeClass();
	void config();
	void power(char vcc_pin, char gnd_pin);
	void setFingering(int fing);
	int readFingers();
	void printFingers();
	char isON(void);

private:
	int fingering;
	unsigned long * fingering_table;
	unsigned int fingers;
	unsigned int control;
	void mpr121QuickConfig(void);
	char mpr121Read(unsigned char address);
	void mpr121Write(unsigned char address, unsigned char data);
	unsigned char fingersToNote(void);
};

extern OpenPipeClass OpenPipe;

#endif
