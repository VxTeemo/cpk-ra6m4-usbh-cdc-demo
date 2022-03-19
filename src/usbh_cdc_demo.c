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

static struct rt_thread usbh_cdc_demo;
static rt_uint8_t usbh_cdc_demo_stack[5120];
uint8_t device_address;
int loop_count = 0;

void usb_basic_example (void* parameter);

int thread_sample_init()
{
    rt_err_t result;
    /* 初始化线程 1 */
    /* 线程的入口是 thread1_entry，参数是 RT_NULL
     * 线程栈是 thread1_stack
     * 优先级是 200，时间片是 10 个 OS Tick
     */
    result = rt_thread_init(&usbh_cdc_demo,
                            "thread1",
                            usb_basic_example, RT_NULL,
                            &usbh_cdc_demo_stack[0], sizeof(usbh_cdc_demo_stack),
                            20, 10);
    /* 启动线程 */
    if (result == RT_EOK) rt_thread_startup(&usbh_cdc_demo);

    return 0;
}

void send_msg(int argc, char**argv)
{
    rt_kprintf("%4d send_msg %d %s %s\n", loop_count++, argc, argv[0], argv[1]);
    strcpy((char*)g_snd_buf, argv[1]);
    memset(g_rcv_buf, 0, 100);
    g_usb_on_usb.write(&g_basic0_ctrl, g_snd_buf, strlen(argv[1]), device_address);
}
MSH_CMD_EXPORT(send_msg, send_msg);
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

void usb_basic_example (void* parameter)
{
    usb_status_t     event, last_event = 0;
    usb_event_info_t event_info;
    g_usb_on_usb.open(&g_basic0_ctrl, &g_basic0_cfg);

    while (1)
    {
        /* Get USB event data */
        g_usb_on_usb.eventGet(&event_info, &event);
        if (last_event != event) {
            rt_kprintf("%4d UsbEvent %s\n", loop_count++, e_usb_status_msg[event]);
            last_event = event;
        }

        /* Handle the received event (if any) */
        switch (event)
        {
            case USB_STATUS_CONFIGURED:
                /* Configure virtual UART settings */
                set_line_coding(&g_basic0_ctrl, event_info.device_address); /* CDC Class request "SetLineCoding" */
                device_address = event_info.device_address;
                break;
            case USB_STATUS_READ_COMPLETE:
                if (USB_CLASS_HCDC == event_info.type)
                {
                    if (event_info.data_size > 0)
                    {
                        /* Send the received data back to the connected peripheral */
                        //g_usb_on_usb.write(&g_basic0_ctrl, g_snd_buf, event_info.data_size, event_info.device_address);
                        rt_kprintf("%4d usb_cdc rcvget:%s\n", loop_count++, g_rcv_buf);
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
                rt_kprintf("%4d usb_cdc sent done, start read\n", loop_count++);
                break;
            case USB_STATUS_REQUEST_COMPLETE:
                rt_kprintf("%4d request_type %x\n", loop_count++, (event_info.setup.request_type & USB_BREQUEST));
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
