#include <fudge.h>
#include <kernel.h>
#include <modules/base/base.h>
#include <modules/system/system.h>
#include <modules/video/video.h>
#include <modules/arch/x86/pic/pic.h>
#include <modules/arch/x86/pci/pci.h>
#include "i915.h"

static struct base_driver driver;
static struct video_interface videointerface;

static unsigned int read(unsigned int reg)
{

    volatile unsigned int *value = (volatile unsigned int *)(unsigned long)reg;

    return *value;

}

static void write(unsigned int reg, unsigned int val)
{

    volatile unsigned int *value = (volatile unsigned int *)(unsigned long)reg;

    *value = val;

}

static void wait(unsigned int num)
{

    unsigned int i = 0xcfffffff;
    unsigned int j = 0xcfffffff;

    while (j--)
    {

        while (i--);

    }

}

static void waitvblank()
{

    write(I915_PIPEB_STATUS, I915_PIPE_STATUS_VBLANK);

    while (!(read(I915_PIPEB_STATUS) & I915_PIPE_STATUS_VBLANK));

}

static void disablevga()
{

    write(I915_VGA_CONTROL, I915_VGA_CONTROL_DISABLE);

}

static void enabledpll()
{

    unsigned int pllb = read(I915_DPLLB_CONTROL);

    if (!(pllb & I915_DPLL_CONTROL_ENABLE_VCO))
    {

        write(I915_DPLLB_CONTROL, pllb);
        read(I915_DPLLB_CONTROL);
        wait(150);
        write(I915_DPLLB_CONTROL, pllb | I915_DPLL_CONTROL_ENABLE_VCO);
        read(I915_DPLLB_CONTROL);
        wait(150);
        write(I915_DPLLB_CONTROL, pllb | I915_DPLL_CONTROL_ENABLE_VCO);
        read(I915_DPLLB_CONTROL);
        wait(150);

    }

}

static void enablepipe()
{

    write(I915_PIPEB_CONFIG, read(I915_PIPEB_CONFIG) & I915_PIPE_CONFIG_ENABLE);

}

static void enableplane()
{

    write(I915_DISPLAYB_CONTROL, read(I915_DISPLAYB_CONTROL) & I915_DISPLAY_CONTROL_ENPLANE);

}

static void setpipemode(unsigned int width, unsigned int height)
{

    unsigned int htotal = (read(I915_DISPLAYB_HTOTAL) >> 16) + 1;
    unsigned int hdisplay = (read(I915_DISPLAYB_HTOTAL) & 0xFFFF) + 1;
    unsigned int hsyncs = (read(I915_DISPLAYB_HSYNC) >> 16) + 1;
    unsigned int hsynce = (read(I915_DISPLAYB_HSYNC) & 0xFFFF) + 1;
    unsigned int vtotal = (read(I915_DISPLAYB_VTOTAL) >> 16) + 1;
    unsigned int vdisplay = (read(I915_DISPLAYB_VTOTAL) & 0xFFFF) + 1;
    unsigned int vsyncs = (read(I915_DISPLAYB_VSYNC) >> 16) + 1;
    unsigned int vsynce = (read(I915_DISPLAYB_VSYNC) & 0xFFFF) + 1;

    write(I915_DISPLAYB_HTOTAL, ((htotal - 1) << 16) | ((hdisplay - 1)));
    write(I915_DISPLAYB_HBLANK, ((htotal - 1) << 16) | ((hdisplay - 1)));
    write(I915_DISPLAYB_HSYNC, ((hsyncs - 1) << 16) | ((hsynce - 1)));
    write(I915_DISPLAYB_VTOTAL, ((vtotal - 1) << 16) | ((vdisplay - 1)));
    write(I915_DISPLAYB_VBLANK, ((vtotal - 1) << 16) | ((vdisplay - 1)));
    write(I915_DISPLAYB_VSYNC, ((vsyncs - 1) << 16) | ((vsynce - 1)));

}

static void handleirq(unsigned int irq)
{

}

static void videointerface_setmode(struct ctrl_videosettings *settings)
{

    ctrl_setvideosettings(&videointerface.settings, settings->w, settings->h, settings->bpp);

}

static unsigned int videointerface_rdata(unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int videointerface_wdata(unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    return 0;

}

static void driver_init()
{

    video_initinterface(&videointerface, videointerface_setmode, videointerface_rdata, videointerface_wdata, 0, 0);

}

static unsigned int driver_match(unsigned int id)
{

    return pci_inw(id, PCI_CONFIG_VENDOR) == 0x8086 && pci_inw(id, PCI_CONFIG_DEVICE) == 0x27AE;

}

static void driver_attach(unsigned int id)
{

    enabledpll();
    enablepipe();
    enableplane();
    waitvblank();
    disablevga();
    setpipemode(640, 480);
    video_registerinterface(&videointerface, id);
    pic_setroutine(pci_getirq(id), handleirq);

}

static void driver_detach(unsigned int id)
{

    video_unregisterinterface(&videointerface);
    pic_unsetroutine(pci_getirq(id));

}

void module_init()
{

    base_initdriver(&driver, "i915", driver_init, driver_match, driver_attach, driver_detach);

}

void module_register()
{

    base_registerdriver(&driver, PCI_BUS);

}

void module_unregister()
{

    base_unregisterdriver(&driver, PCI_BUS);

}

