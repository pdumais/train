#include "usblib.h"
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include "Descriptors.h"
#include <LUFA/Drivers/Board/LEDs.h>
#include <LUFA/Drivers/Peripheral/Serial.h>
#include <LUFA/Drivers/Misc/RingBuffer.h>
#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Platform/Platform.h>

USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface =
{
.Config = {
	.ControlInterfaceNumber = INTERFACE_ID_CDC_CCI,
	.DataINEndpoint = {
		.Address = CDC_TX_EPADDR,
		.Size = CDC_TXRX_EPSIZE,
		.Banks = 1,
	},
	.DataOUTEndpoint = {
		.Address = CDC_RX_EPADDR,
		.Size = CDC_TXRX_EPSIZE,
		.Banks = 1,
	},
	.NotificationEndpoint = {
		.Address = CDC_NOTIFICATION_EPADDR,
		.Size = CDC_NOTIFICATION_EPSIZE,
		.Banks = 1,
	},
},
};

void CDCWork()
{
	CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
	USB_USBTask();
}

uint8_t GetCDCChar(uint8_t* data)
{
    int16_t r = CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
    if (r >= 0)
    {   
        *data = r;
        return 1;
    }
    return 0;
}

void SendCDCChar(uint8_t data)
{
    Endpoint_SelectEndpoint(VirtualSerial_CDC_Interface.Config.DataINEndpoint.Address);
    CDC_Device_SendByte(&VirtualSerial_CDC_Interface, data);
}

void InitCDC() 
{
	MCUSR &= ~(1 << WDRF);
	wdt_disable();
	clock_prescale_set(clock_div_1);
	USB_Init();
}

void EVENT_USB_Device_Connect(void)
{
}

void EVENT_USB_Device_Disconnect(void)
{
}

void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;
	ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);
}

void EVENT_USB_Device_ControlRequest(void)
{
	CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}

void EVENT_CDC_Device_LineEncodingChanged(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo)
{
}

