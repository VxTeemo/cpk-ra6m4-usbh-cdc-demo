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

uint8_t g_buf[4096];
uint8_t g_line_coding[4096];
//uint8_t g_usb_dummy[1024];
//usb_pcdc_linecoding_t g_com_parm;
//uint8_t g_serial_state[512];
#define DATA_LEN 1024

uint8_t device_address;
int loop_count = 0;

static struct rt_thread usbd_cdc_demo;
static rt_uint8_t usbd_cdc_demo_stack[512];

void usb_basic_example (void* parameter);

int thread_sample_init()
{
    rt_err_t result;
    result = rt_thread_init(&usbd_cdc_demo,
                            "usbd_cdc_demo",
                            usb_basic_example, RT_NULL,
                            &usbd_cdc_demo_stack[0], sizeof(usbd_cdc_demo_stack),
                            20, 10);
    /* Æô¶¯Ïß³Ì */
    if (result == RT_EOK) rt_thread_startup(&usbd_cdc_demo);

    return 0;
}

/** USB driver status */
char* e_usb_status_msg[30] =
{
    "USB_STATUS_POWERED",                ///< Powered State
    "USB_STATUS_DEFAULT",                ///< Default State
    "USB_STATUS_ADDRESS",                ///< Address State
    "USB_STATUS_CONFIGURED",             ///< Configured State
    "USB_STATUS_SUSPEND",                ///< Suspend State
    "USB_STATUS_RESUME",                 ///< Resume State
    "USB_STATUS_DETACH",                 ///< Detach State
    "USB_STATUS_REQUEST",                ///< Request State
    "USB_STATUS_REQUEST_COMPLETE",       ///< Request Complete State
    "USB_STATUS_READ_COMPLETE",          ///< Read Complete State
    "USB_STATUS_WRITE_COMPLETE",         ///< Write Complete State
    "USB_STATUS_BC",                     ///< battery Charge State
    "USB_STATUS_OVERCURRENT",            ///< Over Current state
    "USB_STATUS_NOT_SUPPORT",            ///< Device Not Support
    "USB_STATUS_NONE",                   ///< None Status
    "USB_STATUS_MSC_CMD_COMPLETE"       ///< MSC_CMD Complete
};

void usb_basic_example (void* parament)
{
    usb_event_info_t event_info;
    usb_status_t     event, last_event = 0;
    g_usb_on_usb.open(&g_basic0_ctrl, &g_basic0_cfg);
    while (1)
    {
        /* Get USB event data */
        g_usb_on_usb.eventGet(&event_info, &event);
        /* Handle the received event (if any) */
        if (last_event != event) {
            rt_kprintf("%4d UsbEvent %s\n", loop_count++, e_usb_status_msg[event]);
            last_event = event;
        }

        switch (event)
        {
            case USB_STATUS_CONFIGURED:
            case USB_STATUS_WRITE_COMPLETE:
                /* Initialization complete; get data from host */
                g_usb_on_usb.read(&g_basic0_ctrl, g_buf, DATA_LEN, USB_CLASS_PCDC);
                break;
            case USB_STATUS_READ_COMPLETE:
                /* Loop back received data to host */
                g_usb_on_usb.write(&g_basic0_ctrl, g_buf, event_info.data_size, USB_CLASS_PCDC);
                rt_kprintf("%4d usb_cdc rcvget:%s send back\n", loop_count++, g_buf);
                break;
            case USB_STATUS_REQUEST:   /* Receive Class Request */
                rt_kprintf("%4d request_type %x\n", loop_count++, (event_info.setup.request_type & USB_BREQUEST));
                if (USB_PCDC_SET_LINE_CODING == (event_info.setup.request_type & USB_BREQUEST))
                {
                    /* Configure virtual UART settings */
                    g_usb_on_usb.periControlDataGet(&g_basic0_ctrl, (uint8_t *) &g_line_coding, LINE_CODING_LENGTH);
                }
                else if (USB_PCDC_GET_LINE_CODING == (event_info.setup.request_type & USB_BREQUEST))
                {
                    /* Send virtual UART settings back to host */
                    g_usb_on_usb.periControlDataSet(&g_basic0_ctrl, (uint8_t *) &g_line_coding, LINE_CODING_LENGTH);
                }
                else
                {
                    /* ACK all other status requests */
                    g_usb_on_usb.periControlStatusSet(&g_basic0_ctrl, USB_SETUP_STATUS_ACK);
                }
                break;
            case USB_STATUS_SUSPEND:
            case USB_STATUS_DETACH:
                break;
            default:
                break;
        }
    }
}
