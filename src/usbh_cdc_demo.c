#include <rtthread.h>
#include "hal_data.h"
#include <rtdevice.h>

#define SET_LINE_CODING           (USB_CDC_SET_LINE_CODING | USB_HOST_TO_DEV | USB_CLASS | USB_INTERFACE)
#define GET_LINE_CODING           (USB_CDC_GET_LINE_CODING | USB_DEV_TO_HOST | USB_CLASS | USB_INTERFACE)
#define SET_CONTROL_LINE_STATE    (USB_CDC_SET_CONTROL_LINE_STATE | USB_HOST_TO_DEV | USB_CLASS | USB_INTERFACE)
#define COM_SPEED                 (9600U)
#define COM_DATA_BIT              (8U)
#define COM_STOP_BIT              (0)
#define COM_PARITY_BIT            (0)
#define LINE_CODING_LENGTH        (7)

void set_control_line_state (usb_instance_ctrl_t * p_ctrl, uint8_t device_address);
void set_line_coding (usb_instance_ctrl_t * p_ctrl, uint8_t device_address);
void get_line_coding (usb_instance_ctrl_t * p_ctrl, uint8_t device_address);

uint8_t g_snd_buf[4096];
uint8_t g_rcv_buf[4096];
uint8_t g_usb_dummy[1024];
usb_hcdc_linecoding_t g_com_parm;
uint8_t g_serial_state[512];
#define CDC_DATA_LEN 1024

void usb_basic_example (void)
{
    usb_status_t     event;
    usb_event_info_t event_info;
    g_usb_on_usb.open(&g_basic0_ctrl, &g_basic0_cfg);
    while (1)
    {
        /* Get USB event data */
        g_usb_on_usb.eventGet(&event_info, &event);
        /* Handle the received event (if any) */
        switch (event)
        {
            case USB_STATUS_CONFIGURED:
                /* Configure virtual UART settings */
                set_line_coding(&g_basic0_ctrl, event_info.device_address); /* CDC Class request "SetLineCoding" */
                break;
            case USB_STATUS_READ_COMPLETE:
                if (USB_CLASS_HCDC == event_info.type)
                {
                    if (event_info.data_size > 0)
                    {
                        /* Send the received data back to the connected peripheral */
                        g_usb_on_usb.write(&g_basic0_ctrl, g_snd_buf, event_info.data_size, event_info.device_address);
                    }
                    else
                    {
                        /* Send the data reception request when the zero-length packet is received. */
                        g_usb_on_usb.read(&g_basic0_ctrl, g_rcv_buf, CDC_DATA_LEN, event_info.device_address);
                    }
                }
                else                   /* USB_HCDCC */
                {
                    /* Control Class notification "SerialState" receive start */
                    g_usb_on_usb.read(&g_basic0_ctrl,
                                      (uint8_t *) &g_serial_state,
                                      USB_HCDC_SERIAL_STATE_MSG_LEN,
                                      event_info.device_address);
                }
                break;
            case USB_STATUS_WRITE_COMPLETE:
                /* Start receive operation */
                g_usb_on_usb.read(&g_basic0_ctrl, g_rcv_buf, CDC_DATA_LEN, event_info.device_address);
                break;
            case USB_STATUS_REQUEST_COMPLETE:
                if (USB_CDC_SET_LINE_CODING == (event_info.setup.request_type & USB_BREQUEST))
                {
                    /* Set virtual RTS/DTR signal state */
                    set_control_line_state(&g_basic0_ctrl, event_info.device_address); /* CDC Class request "SetControlLineState" */
                }
                /* Check Complete request "SetControlLineState" */
                else if (USB_CDC_SET_CONTROL_LINE_STATE == (event_info.setup.request_type & USB_BREQUEST))
                {
                    /* Read back virtual UART settings */
                    get_line_coding(&g_basic0_ctrl, event_info.device_address); /* CDC Class request "SetLineCoding" */
                }
                else if (USB_CDC_GET_LINE_CODING == (event_info.setup.request_type & USB_BREQUEST))
                {
                    /* Now that setup is complete, start loopback operation */
                    g_usb_on_usb.read(&g_basic0_ctrl, g_snd_buf, CDC_DATA_LEN, event_info.device_address);
                }
                else
                {
                    /* Unsupported request */
                }
                break;
            default:
                /* Other event */
                break;
        }
    }
}
void set_control_line_state (usb_instance_ctrl_t * p_ctrl, uint8_t device_address)
{
    usb_setup_t setup;
    setup.request_type   = SET_CONTROL_LINE_STATE; /* bRequestCode:SET_CONTROL_LINE_STATE, bmRequestType */
    setup.request_value  = 0x0000;                 /* wValue:Zero */
    setup.request_index  = 0x0000;                 /* wIndex:Interface */
    setup.request_length = 0x0000;                 /* wLength:Zero */
    g_usb_on_usb.hostControlTransfer(p_ctrl, &setup, (uint8_t *) &g_usb_dummy, device_address);
}
void set_line_coding (usb_instance_ctrl_t * p_ctrl, uint8_t device_address)
{
    usb_setup_t setup;
    g_com_parm.dwdte_rate   = (usb_hcdc_line_speed_t) COM_SPEED;
    g_com_parm.bchar_format = (usb_hcdc_stop_bit_t) COM_STOP_BIT;
    g_com_parm.bparity_type = (usb_hcdc_parity_bit_t) COM_PARITY_BIT;
    g_com_parm.bdata_bits   = (usb_hcdc_data_bit_t) COM_DATA_BIT;
    setup.request_type   = SET_LINE_CODING;    /* bRequestCode:SET_LINE_CODING, bmRequestType */
    setup.request_value  = 0x0000;             /* wValue:Zero */
    setup.request_index  = 0x0000;             /* wIndex:Interface */
    setup.request_length = LINE_CODING_LENGTH; /* Data:Line Coding Structure */
    /* Request Control transfer */
    g_usb_on_usb.hostControlTransfer(p_ctrl, &setup, (uint8_t *) &g_com_parm, device_address);
}
void get_line_coding (usb_instance_ctrl_t * p_ctrl, uint8_t device_address)
{
    usb_setup_t setup;
    setup.request_type   = GET_LINE_CODING;    /* bRequestCode:GET_LINE_CODING, bmRequestType */
    setup.request_value  = 0x0000;             /* wValue:Zero */
    setup.request_index  = 0x0000;             /* wIndex:Interface */
    setup.request_length = LINE_CODING_LENGTH; /* Data:Line Coding Structure */
    /* Request Control transfer */
    g_usb_on_usb.hostControlTransfer(p_ctrl, &setup, (uint8_t *) &g_com_parm, device_address);
}
