#include <xc.h>
#include "clcd.h"
#include "matrix_keypad.h"
#include "eeprom.h"
#include "adc.h"
#include "main.h"
#include "timer0.h"
#include "ds1307.h"
#include "i2c.h"
#include "uart.h"
#include <string.h>

unsigned char clock_reg[3];
unsigned char speed[3];
extern char event[8][3];
unsigned char time[9];
extern unsigned short index;
extern unsigned short lap;
unsigned char overflow_flag=0;
extern unsigned char screen_flag;
char menu[][17] = {"View Log       ", "Download Log   ", "Clear Log      ", "Change Password", "Set Time         "};
unsigned char star = 0,m_index = 0,menu_clear=1;

void display_dashboard(void) 
{
    clcd_print("TIME     EV   SP", LINE1(0));
    unsigned short spd = read_adc(CHANNEL4)/10.33;
    speed[1] = ((spd%10) + 48);
    speed[0] = ((spd/10) + 48);
    speed[2] = '\0';
    get_time();
    clcd_print(time, LINE2(0));
    clcd_print(event[index], LINE2(9));
    clcd_print(speed, LINE2(14));
}

static void get_time(void)
{
	clock_reg[0] = read_ds1307(HOUR_ADDR);
	clock_reg[1] = read_ds1307(MIN_ADDR);
	clock_reg[2] = read_ds1307(SEC_ADDR);

	if (clock_reg[0] & 0x40)
	{
		time[0] = '0' + ((clock_reg[0] >> 4) & 0x01);
		time[1] = '0' + (clock_reg[0] & 0x0F);
	}
	else
	{
		time[0] = '0' + ((clock_reg[0] >> 4) & 0x03);
		time[1] = '0' + (clock_reg[0] & 0x0F);
	}
	time[2] = ':';
	time[3] = '0' + ((clock_reg[1] >> 4) & 0x0F);
	time[4] = '0' + (clock_reg[1] & 0x0F);
	time[5] = ':';
	time[6] = '0' + ((clock_reg[2] >> 4) & 0x0F);
	time[7] = '0' + (clock_reg[2] & 0x0F);
	time[8] = '\0';
}

void enter_password(void) 
{
    //declare arrays to store the password from internal eeprom and the password entered by the user
    CLEAR_DISP_SCREEN;
    unsigned static char my_pass[5], user_pass[5] ;
    unsigned char pin = 0, no_of_attempts = 2, key = 0;
    unsigned int sec = 0, delay = 0;
    for (char i = 0; i < 4; i++) 
    {
        write_internal_eeprom(50 + i, '0');
        my_pass[i] = read_internal_eeprom(50 + i);
    }
    while (1) 
    {
        if (sec++ == 5000) 
        {
            screen_flag = DASH_BOARD;
            sec = 0;
            CLEAR_DISP_SCREEN;
            return;
        }
        key = read_switches(STATE_CHANGE);
        if (pin < 4) 
        {
            clcd_print(" ENTER PASSWORD ", LINE1(0));
            blink(pin);
            
            if (key == MK_SW11) 
            {
                user_pass[pin] = '1';
                clcd_putch('*', LINE2(6 + pin++));
                sec = 0;
            } else if (key == MK_SW12) 
            {
                user_pass[pin] = '0';
                clcd_putch('*', LINE2(6 + pin++));
                sec = 0;
            }
            if (pin == 4) 
            {
                for (unsigned int k = 750000; k--;);
                CLEAR_DISP_SCREEN;
                user_pass[4] = '\0';
            }
        } 
        else 
        {
            if (!strcmp(my_pass, user_pass)) 
            {
                clcd_print("PASSWORD CRACKED", LINE1(0));
                clcd_print("   SUCCESSFULL   ", LINE2(0));
                if(delay++ == 1000)
                    {
                        delay = 0;
                        no_of_attempts = 2;
                        pin = 0;
                        CLEAR_DISP_SCREEN;
                        screen_flag = MENU_SCRN;
                        return;
                    }
            } 
            else 
            {
                
                if (no_of_attempts > 0) 
                {
                    clcd_print(" WRONG PASSWORD ", LINE1(0));
                    clcd_putch(no_of_attempts + 48, LINE2(0));
                    clcd_print(" Attempts Left", LINE2(2));
                    if(delay++ == 1000)
                    {
                        delay = 0;
                        no_of_attempts--;
                        pin = 0;
                        CLEAR_DISP_SCREEN;
                    }
                } 
                else 
                {
                    clcd_print("YOU ARE BLOCKED", LINE1(0));
                    clcd_print("Sec Left", LINE2(4));
                    for (unsigned int j = 6; j--;)
                    for (unsigned int k = 2000; k--;) 
                    {
                        clcd_putch(j / 100 + '0', LINE2(0));
                        clcd_putch((j / 10) % 10 + '0', LINE2(1));
                        clcd_putch(j % 10 + '0', LINE2(2));
                    }
                    no_of_attempts = 2;
                    pin = 0;
                    CLEAR_DISP_SCREEN;
                }
            }
        }
    }
}

void display_menu()
{
    unsigned char key2=read_switches(LEVEL_CHANGE);
    static unsigned short wait = 0,backup;
    if(menu_clear)
    {
        CLEAR_DISP_SCREEN;
        menu_clear=0;
    }
    if(star)
    {
        clcd_putch(' ', LINE1(0));
        clcd_putch('*', LINE2(0));
    }
    else
    {
        clcd_putch(' ', LINE2(0));
        clcd_putch('*', LINE1(0));
    }
    
    clcd_print(menu[m_index], LINE1(1));
    clcd_print(menu[m_index+1], LINE2(1));
    if(key2 != ALL_RELEASED)
    {
        backup=key2;
        wait++;
        
        if( wait>500)    //Level change
        {
            wait=0;
            if(backup == MK_SW12)
            {
                CLEAR_DISP_SCREEN;
                screen_flag = star+m_index;
                menu_clear=1;
            }
            if(backup == MK_SW11)
            {
                CLEAR_DISP_SCREEN;
                m_index=0;
                star=0;
                screen_flag=DASH_BOARD;
                menu_clear=1;
            }
        }
    }
    
    else if( wait>0 && wait < 500)  //State change
    {
        wait=0;
        if(backup == MK_SW11 )
        {
            if(star == 0 && m_index !=0 )
                m_index--;
        
            if(star == 1)
                star--;
            CLEAR_DISP_SCREEN;
        }
        if(backup == MK_SW12)
        {
            if(star == 1 && m_index !=3)
                m_index++;
        
            if(star == 0)
                star++;
            CLEAR_DISP_SCREEN;
        }
    }
    else if( wait>500)    //Level change
    {
        wait=0;
        if(backup == MK_SW12)
        {
            CLEAR_DISP_SCREEN;
            screen_flag = star+m_index;
            menu_clear=1;
        }
        if(backup == MK_SW11)
        {
            CLEAR_DISP_SCREEN;
            m_index=0;
            star=0;
            screen_flag=DASH_BOARD;
            menu_clear=1;
        }
    }
}

void view_log(void)
{
    clcd_print("LOG:-", LINE1(0));
    static unsigned char lap_check, i,no=0;
    if(overflow_flag == 0)
    {
        lap_check = lap - 1;
    }
    else
        lap_check=9;
    
    static unsigned int kcount = 0, hcount = 0;
    if (overflow_flag) {
        i = lap;
    } else {
        i = 0;
    }
    unsigned char key=read_switches(LEVEL_CHANGE);
    printlog(i, no);
    if (key == MK_SW12) 
    {
        kcount++;
    } 
    else if (key == MK_SW11) 
    {
        hcount++;
        if (hcount == 900) 
        {
            hcount = 0;
            CLEAR_DISP_SCREEN;
            screen_flag=MENU_SCRN;
        }
    }
    else 
    {
        if (kcount > 0 && kcount < 2000) 
        {
            if ( no < lap_check) 
            {
                no++;
            }
        } 
        else if (hcount > 0 && hcount < 900) 
        {
            hcount = 0;
            if (no > 0) 
            {
                no--;
            }
        }
        kcount = 0;
    }
}
void printlog(unsigned char i, unsigned char no) {
    char data[17] = "    :  :        ";
    data[0] = no + 48;
    i=i+no %10;
    data[2]=(read_internal_eeprom(i*5+0)/10)+48;
    data[3]=(read_internal_eeprom(i*5+0)%10)+48;
    
    data[5]=(read_internal_eeprom(i*5+1)/10)+48;
    data[6]=(read_internal_eeprom(i*5+1)%10)+48;
    
    data[8]=(read_internal_eeprom(i*5+2)/10)+48;
    data[9]=(read_internal_eeprom(i*5+2)/10)+48;
    
    data[11]=event[read_internal_eeprom(i*5+3)][0];
    data[12]=event[read_internal_eeprom(i*5+3)][1];
    
    data[14]=(read_internal_eeprom(i*5+4)/10)+48;
    data[15]=(read_internal_eeprom(i*5+4)%10)+48;
    
    clcd_print(data, LINE2(0));
}               

void store_event(void) 
{
    char vlog[5];
    //store time 
    vlog[0] = (time[0] - 48)*10 + (time[1] - 48);
    vlog[1] = (time[3] - 48)*10 + (time[4] - 48);
    vlog[2] = (time[6] - 48)*10 + (time[7] - 48);
    //store event
    vlog[3] = index;
    //store speed
    vlog[4] = (speed[0] - 48)*10 + (speed[1] - 48);
    for (unsigned int j = 0; j < 5; j++) {
        write_internal_eeprom(lap * 5 + j, vlog[j]);
    }
    
    lap++;
    if (lap == 10) 
    {
        overflow_flag = 1;
        lap = 0;
    }
}

void download_log(void)
{
    puts("Sl  Time   EV  SP\n\r");
    static unsigned char lap_check, i,no=0;
    if(overflow_flag == 0)
    {
        lap_check = lap;
    }
    else
        lap_check=10;
    
    if (overflow_flag) {
        i = lap;
    } else {
        i = 0;
    }
    for(int no=0;no<lap_check;no++)
    {
        char data[18] = "     :  :        ";
        int k=no+i;
    data[0] = no + 48;
    i=i+no %10;
    data[3]=(read_internal_eeprom(k*5+0)/10)+48;
    data[4]=(read_internal_eeprom(k*5+0)%10)+48;
    
    data[6]=(read_internal_eeprom(k*5+1)/10)+48;
    data[7]=(read_internal_eeprom(k*5+1)%10)+48;
    
    data[9]=(read_internal_eeprom(k*5+2)/10)+48;
    data[10]=(read_internal_eeprom(k*5+2)/10)+48;
    
    data[12]=event[read_internal_eeprom(k*5+3)][0];
    data[13]=event[read_internal_eeprom(k*5+3)][1];
    
    data[15]=(read_internal_eeprom(k*5+4)/10)+48;
    data[16]=(read_internal_eeprom(k*5+4)%10)+48;
    
    puts(data);
    puts("\n\r");
    }
        
}
void clear_log(void)
{
    CLEAR_DISP_SCREEN;
    clcd_print("clear",LINE2(3));
}
void change_password(void)
{
    CLEAR_DISP_SCREEN;
    clcd_print("change pswrd",LINE2(3));
}
void set_time(void)
{
    CLEAR_DISP_SCREEN;
    clcd_print("set time",LINE2(3));
}

/* function to blink the cursor */
void blink (unsigned char pin)
{
    static unsigned long int i = 0;
    if (i++ <= 1000)
        clcd_putch('_', LINE2(6+pin));    
    else if (i++ <= 2000)
        clcd_putch(' ', LINE2(6+pin));
    else
        i = 0;
}