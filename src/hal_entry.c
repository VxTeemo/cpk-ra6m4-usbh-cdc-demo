/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2021-10-10     Sherman       first version
 * 2021-11-03     Sherman       Add icu_sample
 */

#include <rtthread.h>
#include "hal_data.h"
#include <rtdevice.h>

#define LED3_PIN    BSP_IO_PORT_01_PIN_06
#define USER_INPUT  "P105"

int thread_sample_init();
void send_msg(int argc, char**argv);
void icu_sample(void);
extern int usbh_initialize(void);
extern int cdc_acm_test(void);

void hal_entry(void)
{
    rt_kprintf("\nHello RT-Thread!\n");
    usbh_initialize();
    rt_kprintf("usbh_initialize\n");
    cdc_acm_test();
    rt_kprintf("cdc_acm_test\n");

    thread_sample_init();
    icu_sample();

    while (1)
    {
        rt_pin_write(LED3_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED3_PIN, PIN_LOW);
        rt_thread_mdelay(500);
    }
}
#include <stdio.h>
void irq_callback_test(void *args)
{
    char *cmdd[2];
    char **cmd = cmdd;
    static int send_count = 0;
    char cmd_name[] = "send_msg";
    static char send[30] = "hello usb cdc";

    cmdd[0] = cmd_name;
    cmdd[1] = send;

    sprintf(send,  "hello usb cdc %4d", send_count++);

    send_msg(2, cmd);
}

void icu_sample(void)
{
    /* init */
    rt_uint32_t pin = rt_pin_get(USER_INPUT);
    rt_kprintf("\n pin number : 0x%04X \n", pin);
    rt_err_t err = rt_pin_attach_irq(pin, PIN_IRQ_MODE_RISING, irq_callback_test, RT_NULL);
    if(RT_EOK != err)
    {
        rt_kprintf("\n attach irq failed. \n");
    }
    err = rt_pin_irq_enable(pin, PIN_IRQ_ENABLE);
    if(RT_EOK != err)
    {
        rt_kprintf("\n enable irq failed. \n");
    }
}
MSH_CMD_EXPORT(icu_sample, icu sample);
