#include <fudge.h>
#include <abi.h>

static void loaddriver(char *path)
{

    unsigned int channel = fsp_spawn("initrd:bin/elfload");

    if (channel)
    {

        channel_send(channel, EVENT_MAIN);
        channel_send_fmt1(channel, EVENT_PATH, "%s\\0", path);
        channel_send(channel, EVENT_END);
        channel_wait(EVENT_TERMRESPONSE);

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int channel;

    loaddriver("initrd:kernel/base.ko");
    loaddriver("initrd:kernel/system.ko");
    loaddriver("initrd:kernel/log.ko");
    loaddriver("initrd:kernel/block.ko");
    loaddriver("initrd:kernel/clock.ko");
    loaddriver("initrd:kernel/console.ko");
    loaddriver("initrd:kernel/keyboard.ko");
    loaddriver("initrd:kernel/mouse.ko");
    loaddriver("initrd:kernel/timer.ko");
    loaddriver("initrd:kernel/audio.ko");
    loaddriver("initrd:kernel/video.ko");
    loaddriver("initrd:kernel/ethernet.ko");
    loaddriver("initrd:kernel/info.ko");
    loaddriver("initrd:kernel/io.ko");
    loaddriver("initrd:kernel/cpuid.ko");
    loaddriver("initrd:kernel/msr.ko");
    loaddriver("initrd:kernel/pat.ko");
    loaddriver("initrd:kernel/acpi.ko");
    loaddriver("initrd:kernel/pic.ko");
    loaddriver("initrd:kernel/apic.ko");
    loaddriver("initrd:kernel/platform.ko");
    loaddriver("initrd:kernel/pci.ko");
    loaddriver("initrd:kernel/pit.ko");
    loaddriver("initrd:kernel/rtc.ko");
    loaddriver("initrd:kernel/ps2.ko");
    loaddriver("initrd:kernel/ps2-keyboard.ko");
    loaddriver("initrd:kernel/ps2-mouse.ko");
    loaddriver("initrd:kernel/bga.ko");
    loaddriver("initrd:kernel/uart.ko");
    loaddriver("initrd:kernel/vga.ko");
    loaddriver("initrd:kernel/rtl8139.ko");
    loaddriver("initrd:kernel/ide.ko");
    loaddriver("initrd:kernel/ahci.ko");
    loaddriver("initrd:kernel/nvme.ko");
    loaddriver("initrd:kernel/ata.ko");
    loaddriver("initrd:kernel/virtio-network.ko");

    channel = fsp_spawn("initrd:bin/shell");
    channel_send_fmt0(channel, EVENT_OPTION, "pwd\\0initrd:\\0");
    channel_send_fmt0(channel, EVENT_OPTION, "tty-service\\0100\\0");
    channel_send(channel, EVENT_MAIN);

    channel = fsp_spawn("initrd:bin/wm");
    channel_send_fmt0(channel, EVENT_OPTION, "pwd\\0initrd:\\0");
    channel_send_fmt0(channel, EVENT_OPTION, "listen\\012345\\0");
    channel_send_fmt0(channel, EVENT_OPTION, "keyboard-service\\0110\\0");
    channel_send_fmt0(channel, EVENT_OPTION, "mouse-service\\0124\\0");
    channel_send_fmt0(channel, EVENT_OPTION, "video-service\\0400\\0");
    channel_send(channel, EVENT_MAIN);

    /*
    loaddriver("initrd:kernel/smp.ko");
    */

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);

}

