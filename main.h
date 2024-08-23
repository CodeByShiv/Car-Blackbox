#ifndef MAIN_H
#define MAIN_H

#define DASH_BOARD      0X11
#define PASS_SCRN       0X12
#define MENU_SCRN       0X13
#define MENU_ENT        0X14

#define VIEW_LOG        0X00
#define DOWN_LOG        0X01
#define CLEAR_LOG       0X02
#define CHANGE_PASSWORD 0x03
#define SET_TIME        0x04

#define RESET_PASS      0X07
#define SET_TIME        0X08
#define RESET_LOG       0X09
#define RESET_NOTHING   0X0A


void store_event(void);
void display_dashboard(void);
void printlog(unsigned char i, unsigned char no);
void enter_password(void);
void display_menu(void);
void blink (unsigned char pin);
static void get_time(void);

void view_log(void);
void download_log(void);
void clear_log(void);
void change_password(void);
void set_time(void);

#endif