// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2025 Kevin Schaerer <kevin.schaerer@nettimelogic.com>
 */

#include <debug_uart.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <log.h>
#include <watchdog.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <serial.h>
#include <linux/err.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_EV_RX (1 << 1)

struct uart_liteuart
{
    u32 rxtx;
    u32 txfull;
    u32 rxempty;
    u32 evstatus;
    u32 evpending;
    u32 evenable;
};

struct liteuart_uart_plat
{
    struct uart_liteuart *regs;
};

static int _liteuart_serial_putc(struct uart_liteuart *regs, const char c)
{
    if (readb(&regs->txfull))
        return -EAGAIN;

    writeb(c, &regs->rxtx);

    return 0;
}

static int _liteuart_serial_getc(struct uart_liteuart *regs)
{
    int c;

    if (readb(&regs->rxempty))
        return -EAGAIN;

    c = readb(&regs->rxtx);

    /* refresh UART_RXEMPTY by writing UART_EV_RX to UART_EV_PENDING */
    writeb(UART_EV_RX, &regs->evpending);

    return c;
}

static int liteuart_serial_setbrg(struct udevice *dev, int baudrate)
{
    // not supported
    return 0;
}

static int liteuart_serial_probe(struct udevice *dev)
{
    struct liteuart_uart_plat *plat = dev_get_plat(dev);

    /* No need to reinitialize the UART after relocation */
    if (gd->flags & GD_FLG_RELOC)
        return 0;

    return 0;
}

static int liteuart_serial_getc(struct udevice *dev)
{
    int c;
    struct liteuart_uart_plat *plat = dev_get_plat(dev);
    struct uart_liteuart *regs = plat->regs;

    while ((c = _liteuart_serial_getc(regs)) == -EAGAIN)
        ;

    return c;
}

static int liteuart_serial_putc(struct udevice *dev, const char ch)
{
    int rc;
    struct liteuart_uart_plat *plat = dev_get_plat(dev);

    while ((rc = _liteuart_serial_putc(plat->regs, ch)) == -EAGAIN)
        ;

    return rc;
}

static int liteuart_serial_pending(struct udevice *dev, bool input)
{
    struct liteuart_uart_plat *plat = dev_get_plat(dev);
    struct uart_liteuart *regs = plat->regs;

    if (input)
        return (readl(&regs->ip) & UART_IP_RXWM);
    else
        return !!(readl(&regs->txfifo) & UART_TXFIFO_FULL);
}

static int liteuart_serial_of_to_plat(struct udevice *dev)
{
    struct liteuart_uart_plat *plat = dev_get_plat(dev);

    plat->regs = (struct uart_liteuart *)(uintptr_t)dev_read_addr(dev);
    if (IS_ERR(plat->regs))
        return PTR_ERR(plat->regs);

    return 0;
}

static const struct dm_serial_ops liteuart_serial_ops = {
    .putc = liteuart_serial_putc,
    .getc = liteuart_serial_getc,
    .pending = liteuart_serial_pending,
    .setbrg = liteuart_serial_setbrg,
};

static const struct udevice_id liteuart_serial_ids[] = {
    {.compatible = "litex,liteuart"},
    {/* sentinel */}};

U_BOOT_DRIVER(serial_liteuart) = {
    .name = "serial_liteuart",
    .id = UCLASS_SERIAL,
    .of_match = liteuart_serial_ids,
    .of_to_plat = liteuart_serial_of_to_plat,
    .plat_auto = sizeof(struct liteuart_uart_plat),
    .probe = liteuart_serial_probe,
    .ops = &liteuart_serial_ops,
};
