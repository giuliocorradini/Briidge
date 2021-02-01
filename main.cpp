#include <iostream>
#include <wiiuse.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreGraphics/CGEvent.h>
#include <CoreGraphics/CGEventTypes.h>
#include <CoreFoundation/CFBase.h>
#include <signal.h>

#define MAX_WIIMOTES    4
#define CLASSIC_CTRL_BUTTONS    15

int classic_controller_buttons[CLASSIC_CTRL_BUTTONS] = {
        CLASSIC_CTRL_BUTTON_A,
        CLASSIC_CTRL_BUTTON_B,
        CLASSIC_CTRL_BUTTON_X,
        CLASSIC_CTRL_BUTTON_Y,
        CLASSIC_CTRL_BUTTON_UP,
        CLASSIC_CTRL_BUTTON_DOWN,
        CLASSIC_CTRL_BUTTON_LEFT,
        CLASSIC_CTRL_BUTTON_RIGHT,
        CLASSIC_CTRL_BUTTON_ZL,
        CLASSIC_CTRL_BUTTON_ZR,
        CLASSIC_CTRL_BUTTON_FULL_L,
        CLASSIC_CTRL_BUTTON_FULL_R,
        CLASSIC_CTRL_BUTTON_PLUS,
        CLASSIC_CTRL_BUTTON_MINUS,
        CLASSIC_CTRL_BUTTON_HOME
};

CGKeyCode cc_mapping[CLASSIC_CTRL_BUTTONS] = {
    0x08,
    0x31,
    0x07,
    0x06,
    0x7e,
    0x7d,
    0x7b,
    0x7c,
    0x09,
    0x38,
    0x38,
    0x38,
    0x38,
    0x38,
    0x35
};

bool stop = false;

typedef void (*sighandler_t)(int);
void catch_sigint_and_cleanup(int signum) {
    stop = true;
}

void wiimote_handle_event(wiimote_t* wm) {
    if(wm->exp.type == EXP_CLASSIC) {
        classic_ctrl_t* cc = &wm->exp.classic;

        for(int i=0; i<CLASSIC_CTRL_BUTTONS; i++) {
            if(IS_PRESSED(cc, classic_controller_buttons[i])) {
                CGEventRef event = CGEventCreateKeyboardEvent(NULL, cc_mapping[i], true);
                CGEventPost(kCGHIDEventTap, event);
                CFRelease(event);
            } else if (IS_RELEASED(cc, classic_controller_buttons[i])) {
                CGEventRef event = CGEventCreateKeyboardEvent(NULL, cc_mapping[i], false);
                CGEventPost(kCGHIDEventTap, event);
                CFRelease(event);
            }
        }

        cc->ljs.ang;
        cc->ljs.mag;

    }
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

int main() {

    signal(SIGINT, catch_sigint_and_cleanup);

    wiimote** wiimotes = wiiuse_init(MAX_WIIMOTES);

    int found, connected;
    do {
        found = wiiuse_find(wiimotes, MAX_WIIMOTES, 5);
        if(found == 0) {
            std::cout << "No Wiimotes found" << std::endl;
        }
    } while (found == 0);

    connected = wiiuse_connect(wiimotes, MAX_WIIMOTES);
    if(connected) {
        std::cout << "Connected to " << connected << " wiimote(s)." << std::endl;
    } else {
        return 0;
    }

    for(int i=0; i<connected; i++) {
        wiiuse_set_leds(wiimotes[i], WIIMOTE_LED_1 << i);
    }

    while(any_wiimote_connected(wiimotes, MAX_WIIMOTES) && !stop) {
        if(wiiuse_poll(wiimotes, MAX_WIIMOTES)) {
            for(int i=0; i<MAX_WIIMOTES; i++) {
                switch(wiimotes[i]->event) {
                    case WIIUSE_EVENT:
                        wiimote_handle_event(wiimotes[i]);
                        break;

                    case WIIUSE_STATUS:
                        break;

                    case WIIUSE_DISCONNECT:
                    case WIIUSE_UNEXPECTED_DISCONNECT:
                        break;

                    case WIIUSE_READ_DATA:
                        break;

                    case WIIUSE_NUNCHUK_INSERTED:
                        /*
                         *	a nunchuk was inserted
                         *	This is a good place to set any nunchuk specific
                         *	threshold values.  By default they are the same
                         *	as the wiimote.
                         */
                        /* wiiuse_set_nunchuk_orient_threshold((struct nunchuk_t*)&wiimotes[i]->exp.nunchuk, 90.0f); */
                        /* wiiuse_set_nunchuk_accel_threshold((struct nunchuk_t*)&wiimotes[i]->exp.nunchuk, 100); */
                        break;

                    case WIIUSE_CLASSIC_CTRL_INSERTED:
                        break;

                    case WIIUSE_NUNCHUK_REMOVED:
                    case WIIUSE_CLASSIC_CTRL_REMOVED:
                    case WIIUSE_GUITAR_HERO_3_CTRL_REMOVED:
                    case WIIUSE_WII_BOARD_CTRL_REMOVED:
                    case WIIUSE_MOTION_PLUS_REMOVED:
                        break;

                    default:
                        break;
                }
            }
        }
    }

    for(int i=0; i<connected; i++) {
        wiiuse_disconnect(wiimotes[i]);
        wiiuse_cleanup(wiimotes, MAX_WIIMOTES);
    }
    return 0;
}
