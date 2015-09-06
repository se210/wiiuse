/*
 *	wiiuse
 *
 *	Written By:
 *		Michael Laforest	< para >
 *		Email: < thepara (--AT--) g m a i l [--DOT--] com >
 *
 *	Copyright 2006-2007
 *
 *	This file is part of wiiuse.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *	$Header$
 *
 */

/**
 *	@file
 *
 *	@brief Example using the wiiuse API.
 *
 *	This file is an example of how to use the wiiuse library.
 */

#include <stdio.h>                      /* for printf */
#include <stdbool.h>

#include "wiiuse.h"                     /* for wiimote_t, classic_ctrl_t, etc */

#ifndef WIIUSE_WIN32
#include <unistd.h>                     /* for usleep */
#endif

#define MAX_WIIMOTES				4

typedef void(*DebugLogFptr)(const char *);
DebugLogFptr UnityDebugLog;

wiimote** wiimotes;

float pitch = 0.0f;
float roll = 0.0f;
float yaw = 0.0f;
bool acc_ready = false;

void DebugLog(const char * fmt, ...)
{
    char buf[1024];

    va_list argptr;
    va_start(argptr, fmt);
    vsprintf_s(buf, sizeof(buf), fmt, argptr);
    va_end(argptr);

    UnityDebugLog(buf);
}

/**
 *	@brief Callback that handles an event.
 *
 *	@param wm		Pointer to a wiimote_t structure.
 *
 *	This function is called automatically by the wiiuse library when an
 *	event occurs on the specified wiimote.
 */
void handle_event(struct wiimote_t* wm) {
	DebugLog("\n\n--- EVENT [id %i] ---\n", wm->unid);

	/* if a button is pressed, report it */
	if (IS_PRESSED(wm, WIIMOTE_BUTTON_A)) {
        DebugLog("A pressed\n");
	}
	if (IS_PRESSED(wm, WIIMOTE_BUTTON_B)) {
        DebugLog("B pressed\n");
	}
	if (IS_PRESSED(wm, WIIMOTE_BUTTON_UP)) {
		DebugLog("UP pressed\n");
	}
	if (IS_PRESSED(wm, WIIMOTE_BUTTON_DOWN))	{
		DebugLog("DOWN pressed\n");
	}
	if (IS_PRESSED(wm, WIIMOTE_BUTTON_LEFT))	{
		DebugLog("LEFT pressed\n");
	}
	if (IS_PRESSED(wm, WIIMOTE_BUTTON_RIGHT))	{
		DebugLog("RIGHT pressed\n");
	}
	if (IS_PRESSED(wm, WIIMOTE_BUTTON_MINUS))	{
		DebugLog("MINUS pressed\n");
	}
	if (IS_PRESSED(wm, WIIMOTE_BUTTON_PLUS))	{
		DebugLog("PLUS pressed\n");
	}
	if (IS_PRESSED(wm, WIIMOTE_BUTTON_ONE)) {
		DebugLog("ONE pressed\n");
	}
	if (IS_PRESSED(wm, WIIMOTE_BUTTON_TWO)) {
		DebugLog("TWO pressed\n");
	}
	if (IS_PRESSED(wm, WIIMOTE_BUTTON_HOME))	{
		DebugLog("HOME pressed\n");
	}

	/*
	 *	Pressing minus will tell the wiimote we are no longer interested in movement.
	 *	This is useful because it saves battery power.
	 */
	if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_MINUS)) {
		wiiuse_motion_sensing(wm, 0);
	}

	/*
	 *	Pressing plus will tell the wiimote we are interested in movement.
	 */
	if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_PLUS)) {
		wiiuse_motion_sensing(wm, 1);
	}

	/*
	 *	Pressing B will toggle the rumble
	 *
	 *	if B is pressed but is not held, toggle the rumble
	 */
	if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_B)) {
		wiiuse_toggle_rumble(wm);
	}

	if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_UP)) {
		wiiuse_set_ir(wm, 1);
	}
	if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_DOWN)) {
		wiiuse_set_ir(wm, 0);
	}

	/*
	 * Motion+ support
	 */
	if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_ONE)) {
		if (WIIUSE_USING_EXP(wm)) {
			wiiuse_set_motion_plus(wm, 2);    // nunchuck pass-through
		} else {
			wiiuse_set_motion_plus(wm, 1);    // standalone
		}
	}

	if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_TWO)) {
		wiiuse_set_motion_plus(wm, 0); // off
	}

	/* if the accelerometer is turned on then print angles */
	//if (WIIUSE_USING_ACC(wm)) {
		DebugLog("wiimote roll  = %f [%f]\n", wm->orient.roll, wm->orient.a_roll);
		DebugLog("wiimote pitch = %f [%f]\n", wm->orient.pitch, wm->orient.a_pitch);
		DebugLog("wiimote yaw   = %f\n", wm->orient.yaw);
        roll = wm->orient.roll;
        pitch = wm->orient.pitch;
        yaw = wm->orient.yaw;
        acc_ready = true;
	//}

	/*
	 *	If IR tracking is enabled then print the coordinates
	 *	on the virtual screen that the wiimote is pointing to.
	 *
	 *	Also make sure that we see at least 1 dot.
	 */
	if (WIIUSE_USING_IR(wm)) {
		int i = 0;

		/* go through each of the 4 possible IR sources */
		for (; i < 4; ++i) {
			/* check if the source is visible */
			if (wm->ir.dot[i].visible) {
				DebugLog("IR source %i: (%u, %u)\n", i, wm->ir.dot[i].x, wm->ir.dot[i].y);
			}
		}

		DebugLog("IR cursor: (%u, %u)\n", wm->ir.x, wm->ir.y);
		DebugLog("IR z distance: %f\n", wm->ir.z);
	}

	/* show events specific to supported expansions */
	if (wm->exp.type == EXP_NUNCHUK || wm->exp.type == EXP_MOTION_PLUS_NUNCHUK) {
		/* nunchuk */
		struct nunchuk_t* nc = (nunchuk_t*)&wm->exp.nunchuk;

		if (IS_PRESSED(nc, NUNCHUK_BUTTON_C)) {
			DebugLog("Nunchuk: C pressed\n");
		}
		if (IS_PRESSED(nc, NUNCHUK_BUTTON_Z)) {
			DebugLog("Nunchuk: Z pressed\n");
		}

		DebugLog("nunchuk roll  = %f\n", nc->orient.roll);
		DebugLog("nunchuk pitch = %f\n", nc->orient.pitch);
		DebugLog("nunchuk yaw   = %f\n", nc->orient.yaw);

		DebugLog("nunchuk joystick angle:     %f\n", nc->js.ang);
		DebugLog("nunchuk joystick magnitude: %f\n", nc->js.mag);

		DebugLog("nunchuk joystick vals:      %f, %f\n", nc->js.x, nc->js.y);
		DebugLog("nunchuk joystick calibration (min, center, max): x: %i, %i, %i  y: %i, %i, %i\n",
		    nc->js.min.x,
		    nc->js.center.x,
		    nc->js.max.x,
		    nc->js.min.y,
		    nc->js.center.y,
		    nc->js.max.y);
	}

	if (wm->exp.type == EXP_MOTION_PLUS ||
        wm->exp.type == EXP_MOTION_PLUS_NUNCHUK) {
		DebugLog("Motion+ angular rates (deg/sec): pitch:%03.2f roll:%03.2f yaw:%03.2f\n",
            wm->exp.mp.angle_rate_gyro.pitch,
            wm->exp.mp.angle_rate_gyro.roll,
            wm->exp.mp.angle_rate_gyro.yaw);
	}
}

/**
 *	@brief Callback that handles a read event.
 *
 *	@param wm		Pointer to a wiimote_t structure.
 *	@param data		Pointer to the filled data block.
 *	@param len		Length in bytes of the data block.
 *
 *	This function is called automatically by the wiiuse library when
 *	the wiimote has returned the full data requested by a previous
 *	call to wiiuse_read_data().
 *
 *	You can read data on the wiimote, such as Mii data, if
 *	you know the offset address and the length.
 *
 *	The \a data pointer was specified on the call to wiiuse_read_data().
 *	At the time of this function being called, it is not safe to deallocate
 *	this buffer.
 */
void handle_read(struct wiimote_t* wm, byte* data, unsigned short len) {
	int i = 0;

	DebugLog("\n\n--- DATA READ [wiimote id %i] ---\n", wm->unid);
	DebugLog("finished read of size %i\n", len);
	for (; i < len; ++i) {
		if (!(i % 16)) {
			DebugLog("\n");
		}
		DebugLog("%x ", data[i]);
	}
	DebugLog("\n\n");
}


/**
 *	@brief Callback that handles a controller status event.
 *
 *	@param wm				Pointer to a wiimote_t structure.
 *	@param attachment		Is there an attachment? (1 for yes, 0 for no)
 *	@param speaker			Is the speaker enabled? (1 for yes, 0 for no)
 *	@param ir				Is the IR support enabled? (1 for yes, 0 for no)
 *	@param led				What LEDs are lit.
 *	@param battery_level	Battery level, between 0.0 (0%) and 1.0 (100%).
 *
 *	This occurs when either the controller status changed
 *	or the controller status was requested explicitly by
 *	wiiuse_status().
 *
 *	One reason the status can change is if the nunchuk was
 *	inserted or removed from the expansion port.
 */
void handle_ctrl_status(struct wiimote_t* wm) {
	DebugLog("\n\n--- CONTROLLER STATUS [wiimote id %i] ---\n", wm->unid);

	DebugLog("attachment:      %i\n", wm->exp.type);
	DebugLog("speaker:         %i\n", WIIUSE_USING_SPEAKER(wm));
	DebugLog("ir:              %i\n", WIIUSE_USING_IR(wm));
	DebugLog("leds:            %i %i %i %i\n", WIIUSE_IS_LED_SET(wm, 1), WIIUSE_IS_LED_SET(wm, 2), WIIUSE_IS_LED_SET(wm, 3), WIIUSE_IS_LED_SET(wm, 4));
	DebugLog("battery:         %f %%\n", wm->battery_level);
}


/**
 *	@brief Callback that handles a disconnection event.
 *
 *	@param wm				Pointer to a wiimote_t structure.
 *
 *	This can happen if the POWER button is pressed, or
 *	if the connection is interrupted.
 */
void handle_disconnect(wiimote* wm) {
	DebugLog("\n\n--- DISCONNECTED [wiimote id %i] ---\n", wm->unid);
}


void test(struct wiimote_t* wm, byte* data, unsigned short len) {
	DebugLog("test: %i [%x %x %x %x]\n", len, data[0], data[1], data[2], data[3]);
}

short any_wiimote_connected(wiimote** wm, int wiimotes) {
	int i;
	if (!wm) {
		return 0;
	}

	for (i = 0; i < wiimotes; i++) {
		if (wm[i] && WIIMOTE_IS_CONNECTED(wm[i])) {
			return 1;
		}
	}

	return 0;
}


/**
 *	@brief main()
 *
 *	Connect to up to two wiimotes and print any events
 *	that occur on either device.
 */
//int main(int argc, char** argv) {
//	wiimote** wiimotes;
//	int found, connected;
//
//	/*
//	 *	Initialize an array of wiimote objects.
//	 *
//	 *	The parameter is the number of wiimotes I want to create.
//	 */
//	wiimotes =  wiiuse_init(MAX_WIIMOTES);
//
//	/*
//	 *	Find wiimote devices
//	 *
//	 *	Now we need to find some wiimotes.
//	 *	Give the function the wiimote array we created, and tell it there
//	 *	are MAX_WIIMOTES wiimotes we are interested in.
//	 *
//	 *	Set the timeout to be 5 seconds.
//	 *
//	 *	This will return the number of actual wiimotes that are in discovery mode.
//	 */
//	found = wiiuse_find(wiimotes, MAX_WIIMOTES, 5);
//	if (!found) {
//		DebugLog("No wiimotes found.\n");
//		return 0;
//	}
//
//	/*
//	 *	Connect to the wiimotes
//	 *
//	 *	Now that we found some wiimotes, connect to them.
//	 *	Give the function the wiimote array and the number
//	 *	of wiimote devices we found.
//	 *
//	 *	This will return the number of established connections to the found wiimotes.
//	 */
//	connected = wiiuse_connect(wiimotes, MAX_WIIMOTES);
//	if (connected) {
//		DebugLog("Connected to %i wiimotes (of %i found).\n", connected, found);
//	} else {
//		DebugLog("Failed to connect to any wiimote.\n");
//		return 0;
//	}
//
//	/*
//	 *	Now set the LEDs and rumble for a second so it's easy
//	 *	to tell which wiimotes are connected (just like the wii does).
//	 */
//	wiiuse_set_leds(wiimotes[0], WIIMOTE_LED_1);
//	wiiuse_set_leds(wiimotes[1], WIIMOTE_LED_2);
//	wiiuse_set_leds(wiimotes[2], WIIMOTE_LED_3);
//	wiiuse_set_leds(wiimotes[3], WIIMOTE_LED_4);
//	wiiuse_rumble(wiimotes[0], 1);
//	wiiuse_rumble(wiimotes[1], 1);
//
//#ifndef WIIUSE_WIN32
//	usleep(200000);
//#else
//	Sleep(200);
//#endif
//
//	wiiuse_rumble(wiimotes[0], 0);
//	wiiuse_rumble(wiimotes[1], 0);
//
//	DebugLog("\nControls:\n");
//	DebugLog("\tB toggles rumble.\n");
//	DebugLog("\t+ to start Wiimote accelerometer reporting, - to stop\n");
//	DebugLog("\tUP to start IR camera (sensor bar mode), DOWN to stop.\n");
//	DebugLog("\t1 to start Motion+ reporting, 2 to stop.\n");
//	DebugLog("\n\n");
//
//	/*
//	 *	Maybe I'm interested in the battery power of the 0th
//	 *	wiimote.  This should be WIIMOTE_ID_1 but to be sure
//	 *	you can get the wiimote associated with WIIMOTE_ID_1
//	 *	using the wiiuse_get_by_id() function.
//	 *
//	 *	A status request will return other things too, like
//	 *	if any expansions are plugged into the wiimote or
//	 *	what LEDs are lit.
//	 */
//	/* wiiuse_status(wiimotes[0]); */
//
//	/*
//	 *	This is the main loop
//	 *
//	 *	wiiuse_poll() needs to be called with the wiimote array
//	 *	and the number of wiimote structures in that array
//	 *	(it doesn't matter if some of those wiimotes are not used
//	 *	or are not connected).
//	 *
//	 *	This function will set the event flag for each wiimote
//	 *	when the wiimote has things to report.
//	 */
//	while (any_wiimote_connected(wiimotes, MAX_WIIMOTES)) {
//		if (wiiuse_poll(wiimotes, MAX_WIIMOTES)) {
//			/*
//			 *	This happens if something happened on any wiimote.
//			 *	So go through each one and check if anything happened.
//			 */
//			int i = 0;
//			for (; i < MAX_WIIMOTES; ++i) {
//				switch (wiimotes[i]->event) {
//					case WIIUSE_EVENT:
//						/* a generic event occurred */
//						handle_event(wiimotes[i]);
//						break;
//
//					case WIIUSE_STATUS:
//						/* a status event occurred */
//						handle_ctrl_status(wiimotes[i]);
//						break;
//
//					case WIIUSE_DISCONNECT:
//					case WIIUSE_UNEXPECTED_DISCONNECT:
//						/* the wiimote disconnected */
//						handle_disconnect(wiimotes[i]);
//						break;
//
//					case WIIUSE_READ_DATA:
//						/*
//						 *	Data we requested to read was returned.
//						 *	Take a look at wiimotes[i]->read_req
//						 *	for the data.
//						 */
//						break;
//
//					case WIIUSE_NUNCHUK_INSERTED:
//						/*
//						 *	a nunchuk was inserted
//						 *	This is a good place to set any nunchuk specific
//						 *	threshold values.  By default they are the same
//						 *	as the wiimote.
//						 */
//						/* wiiuse_set_nunchuk_orient_threshold((struct nunchuk_t*)&wiimotes[i]->exp.nunchuk, 90.0f); */
//						/* wiiuse_set_nunchuk_accel_threshold((struct nunchuk_t*)&wiimotes[i]->exp.nunchuk, 100); */
//						DebugLog("Nunchuk inserted.\n");
//						break;
//
//					case WIIUSE_CLASSIC_CTRL_INSERTED:
//						DebugLog("Classic controller inserted.\n");
//						break;
//
//					case WIIUSE_WII_BOARD_CTRL_INSERTED:
//						DebugLog("Balance board controller inserted.\n");
//						break;
//
//					case WIIUSE_GUITAR_HERO_3_CTRL_INSERTED:
//						/* some expansion was inserted */
//						handle_ctrl_status(wiimotes[i]);
//						DebugLog("Guitar Hero 3 controller inserted.\n");
//						break;
//
//					case WIIUSE_MOTION_PLUS_ACTIVATED:
//						DebugLog("Motion+ was activated\n");
//						break;
//
//					case WIIUSE_NUNCHUK_REMOVED:
//					case WIIUSE_CLASSIC_CTRL_REMOVED:
//					case WIIUSE_GUITAR_HERO_3_CTRL_REMOVED:
//					case WIIUSE_WII_BOARD_CTRL_REMOVED:
//					case WIIUSE_MOTION_PLUS_REMOVED:
//						/* some expansion was removed */
//						handle_ctrl_status(wiimotes[i]);
//						DebugLog("An expansion was removed.\n");
//						break;
//
//					default:
//						break;
//				}
//			}
//		}
//	}
//
//	/*
//	 *	Disconnect the wiimotes
//	 */
//	wiiuse_cleanup(wiimotes, MAX_WIIMOTES);
//
//	return 0;
//}

__declspec(dllexport) void SetDebugLogFptr(DebugLogFptr fp)
{
    UnityDebugLog = fp;
}

__declspec(dllexport) int WiimoteInit()
{
    wiimotes = wiiuse_init(1);
    int found = wiiuse_find(wiimotes, 1, 5);
    if (!found) {
        DebugLog("Failed to find any wiimote.\n");
        return 0;
    }
    int connected = wiiuse_connect(wiimotes, 1);
    if (connected) {
        DebugLog("Connected to %i wiimotes (of %i found).\n", connected, found);
    }
    else {
        DebugLog("Failed to connect to any wiimote.\n");
        return 0;
    }
    wiiuse_rumble(wiimotes[0], 1);
    Sleep(200);
    wiiuse_rumble(wiimotes[0], 0);
    wiiuse_set_leds(wiimotes[0], WIIMOTE_LED_1);
    wiiuse_set_ir(wiimotes[0], 1);

    wiiuse_motion_sensing(wiimotes[0], 1);

    return 1;
}

__declspec(dllexport) void handleEvent()
{
    if (wiiuse_poll(wiimotes, 1)) {
        switch (wiimotes[0]->event) {
        case WIIUSE_EVENT:
            handle_event(wiimotes[0]);
            break;
        case WIIUSE_MOTION_PLUS_ACTIVATED:
            DebugLog("Motion+ was activated\n");
            break;
        default:
            break;
        }
    }
}

__declspec(dllexport) float* Wiimote6DOF()
{
    if (!acc_ready)
        return NULL;

    float* retval = malloc(sizeof(float) * 3);
    retval[0] = pitch;
    retval[1] = roll;
    retval[2] = yaw;

    return retval;
}

__declspec(dllexport) void FreeMemory(float* pMem)
{
    free(pMem);
}