#include <avr/io.h>

struct PWM
{
    uint8_t* port;
    uint8_t  flags;
    uint8_t* ocr;
};


struct PWM pwms[] = {
    {&TCCR0A, (1<<COM0A0)|(1<<COM0A1), &OCR0A},
    {&TCCR0A, (1<<COM0B0)|(1<<COM0B1), &OCR0B},
    {&TCCR1A, (1<<COM1A0)|(1<<COM1A1), &OCR1A},
    {&TCCR1A, (1<<COM1B0)|(1<<COM1B1), &OCR1B},
    {&TCCR3A, (1<<COM3A0)|(1<<COM3A1), &OCR3A},
    {&TCCR4C, (1<<COM4D0)|(1<<COM4D1)|(1<<PWM4D), &OCR4D},
    {&TCCR4A, (1<<COM4A0)|(1<<COM4A1)|(1<<PWM4A), &OCR4A}
};

void enable_pwm(uint8_t p, uint8_t enabled)
{
    uint8_t* port = pwms[p].port;

    if (enabled)
    {
        *port |= pwms[p].flags;
    }
    else
    {
        *port &= ~(pwms[p].flags);
    }
}

void set_duty(uint8_t p, uint8_t duty)
{
    uint8_t* port = pwms[p].ocr;
    *port = duty;
    
}

void setup_timers()
{
    // Set prescales to /1024 on all 4 timers
    TCCR0A = 0;
    TCCR0B = 0;
    TCCR1A = 0;
    TCCR1B = 0;
    TCCR3A = 0;
    TCCR3B = 0;
    TCCR4A = 0;
    TCCR4B = 0;
    TCCR4C = 0;
    TCCR4D = 0;

    TCCR0B |= (1<<CS00)|(1<<CS02);
    TCCR1B |= (1<<CS10)|(1<<CS12);
    TCCR3B |= (1<<CS30)|(1<<CS32);
    TCCR4B |= (1<<CS43)|(1<<CS41)|(1<<CS40);

    TCCR0A |= (1 << WGM00) | ( 1 << WGM01);
    TCCR1A |= (1 << WGM10);
    TCCR1B |= (1 << WGM12);
    TCCR3A |= (1 << WGM30);
    TCCR3B |= (1 << WGM32);
    OCR4C = 0xFF;

    for (int i = 0; i < 7; i++)
    {
        //enable_pwm(i, 1);

    }
}
