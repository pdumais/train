#ifndef _USBLIB_
#define _USBLIB_

typedef unsigned char uint8_t;

void InitCDC(); 
uint8_t GetCDCChar(uint8_t* data);
void SendCDCChar(uint8_t data);
void CDCWork();

#endif




