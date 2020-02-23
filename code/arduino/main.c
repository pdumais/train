#include "usblib.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define MAXCMD 10

uint16_t relayAutoTurnOff[26];

struct Command 
{
    char    cmd;
    uint8_t number;
};

struct PinInfo
{
    uint8_t *port;
    uint8_t *ddr;
    uint8_t pin;
};

struct PinInfo pins[] = {
    {&PORTD, &DDRD, 1}, 
    {&PORTD, &DDRD, 4}, 
    {&PORTE, &DDRE, 6}, 
    {&PORTB, &DDRB, 4}, 
    {&PORTD, &DDRD, 6}, 
    {&PORTF, &DDRF, 7},
    {&PORTF, &DDRF, 6},
    {&PORTF, &DDRF, 5},
    {&PORTF, &DDRF, 4},
    {&PORTF, &DDRF, 1},
    {&PORTF, &DDRF, 0},
    #define FIRST_PWM 11
    // PWMs
    {&PORTB, &DDRB, 7,},  // l
    {&PORTD, &DDRD, 0,},  // m
    {&PORTB, &DDRB, 5,},  // n
    {&PORTB, &DDRB, 6,},  // o
    {&PORTC, &DDRC, 6,},  // p
    {&PORTD, &DDRD, 7,},  // q
    {&PORTC, &DDRC, 7,}   // r
    #define LAST_PWM 17
}; 


void sendString(char* str)
{
    while (*str != 0)
    {
        SendCDCChar(*str);
        str++;
    }
}

void sendNumber(uint8_t n)
{
    char tmp[4];
    tmp[3] = 0;

    tmp[0] = (n/100);
    n -= tmp[0]*100;
    tmp[1] = (n/10);
    n -= tmp[1]*10;
    tmp[2] = n;

    for (int i = 0; i < 3; i++) tmp[i]+=48;
    sendString((char*)&tmp[0]);
}

struct Command readCommand(char *buf, uint8_t i)
{
    struct Command cmd;

    cmd.cmd=0;
    if (buf[0] < 'a' || buf[0] > 'z') return cmd;
    cmd.cmd = buf[0];


    // We will ignore numbers longer than 3 digits
    if (i > 5) i = 5;

    if (buf[i] == '\n') i--;
    uint8_t mul = 1;
    uint8_t number = 0;

    while (i > 0)
    {
        if (buf[i] < '0' || buf[i] > '9')
        {
            number = 0;
            break;
        }
        number += (buf[i]-48)*mul;
        mul = mul*10;
        i--;
    }
    cmd.number = number;


    return cmd;

}

void setRelay(uint8_t dev, uint8_t val)
{
    uint8_t *port = pins[dev].port;
    uint8_t pin = pins[dev].pin;

    if (val)
    {
        if (val == 2)
        {
            // This will turn on the relay for about 250ms
            relayAutoTurnOff[dev] = 10000;
        }
        *port &= ~(1<<pin); 
    }
    else
    {
        *port |= (1<<pin); 
    }
}

void setPWM(uint8_t dev, uint8_t val)
{
    uint8_t *port = pins[dev].port;
    uint8_t pin = pins[dev].pin;

    ////////// DEBUG 
    char tmp[2];
    tmp[0] = (dev-FIRST_PWM)+48;
    tmp[1] = 0;
    sendString("===> [");
    sendString((char*)&tmp[0]);
    sendString("] = [");
    sendNumber(val);
    sendString("]\r\n");

    val = 255-val;
    if (val == 0) 
    {
        *port |= (1<<pin); 
        enable_pwm(dev-FIRST_PWM, 0);
    } 
    else if (val == 255)
    {
        *port &= ~(1<<pin); 
        enable_pwm(dev-FIRST_PWM, 0);
    }
    else
    {
        enable_pwm(dev-FIRST_PWM, 1); 
        set_duty(dev-FIRST_PWM, val);
    }
}

void process_command(uint8_t cmd, uint8_t value)
{

    cmd -= 'a';
    if (cmd < FIRST_PWM) 
    {
        setRelay(cmd, value);
    }
    else if (cmd <= (LAST_PWM)) 
    {
        setPWM(cmd, value);
    }

}

int main(void)
{
    uint8_t pb,pd,pe;
    uint8_t newData = 0;
    uint8_t lastData = 0;
    char stabilizerCount = -1;

    for (int i = 0; i < 26; i++) relayAutoTurnOff[i]=0;
    InitCDC();
    sei();

    // Init outputs
    for (int i = 0; i < FIRST_PWM; i++)
    {
        uint8_t *port = pins[i].port;
        uint8_t *ddr = pins[i].ddr;
        uint8_t pin = pins[i].pin;

        *ddr |= (1<<pin);
        *port |= (1<<pin); // initially off (high = off)
    }

    for (int i = FIRST_PWM; i <= LAST_PWM; i++)
    {
        uint8_t *port = pins[i].port;
        uint8_t *ddr = pins[i].ddr;
        uint8_t pin = pins[i].pin;

        *ddr |= (1<<pin);
        *port &= ~(1<<pin); // initially off 
    }

    setup_timers();

    char buf[MAXCMD];
    char tmp;
    uint8_t i = 0;
    while (1)
    {
        for (int i = 0; i < 26; i++) 
        {
            if (!relayAutoTurnOff[i]) continue;
            relayAutoTurnOff[i]--;
            if (relayAutoTurnOff[i] == 1)
            {
                setRelay(i,0);
            }
        }
        CDCWork();

        if (GetCDCChar(&tmp))
        {
            buf[i] = tmp;
            if ((i == (MAXCMD-1)) || tmp == '\n')
            {
                struct Command cmd = readCommand(buf, i);
                process_command(cmd.cmd, cmd.number);
                
                i = 0;
            }
            else
            {
                i++;
            }

        }

    }
}



