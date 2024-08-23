#include <xc.h>
#include "clcd.h"
#include "matrix_keypad.h"
#include "eeprom.h"
#include "adc.h"
#include "main.h"
#include "timer0.h"
#include "ds1307.h"
#include "uart.h"
#include "i2c.h"
#include <string.h>

char event[8][3] = {"ON", "GR", "GN", "G1", "G2", "G3", "G4", "-C"};
unsigned short index = 0;
unsigned short lap;
unsigned reset_flag = RESET_NOTHING;
unsigned char screen_flag = DASH_BOARD;

void init_config(void) {
    init_clcd();
    init_adc();
    init_matrix_keypad();
    init_i2c();
    init_ds1307();
    init_uart();

//    GIE = 1;    
//	PEIE = 1;
}

void main()
{
    init_config();
    unsigned char key;
    
    while(1)
    {
        key = read_switches(STATE_CHANGE);
        
        if(key == MK_SW1)
        {
            index = 7;
            store_event();
        }
        else if(index == 7 && (key == MK_SW2 || key == MK_SW3))
        {
            index = 2;
            store_event();
        }
        else if(key == MK_SW2 && index < 6)
        {
            index++;
            store_event();
        }
        else if(key == MK_SW3 && index > 1)
        {
            index--;
            store_event();
        }
        else if(key == MK_SW11 && screen_flag == DASH_BOARD)
        {
            screen_flag = PASS_SCRN;
        }
        
        switch (screen_flag)
        {
            case DASH_BOARD:
                display_dashboard();
                break;
            case PASS_SCRN:
                enter_password();
                break;
            case MENU_SCRN:
                display_menu();
                break;
                
            case VIEW_LOG:
                view_log();
                break;
            case DOWN_LOG:
                download_log();
                screen_flag=MENU_SCRN;
                break;
            case CLEAR_LOG:
                clear_log();
                break;
            case CHANGE_PASSWORD:
                change_password();
                break;
            case SET_TIME:
                set_time();
                break;
        }
    }
}
