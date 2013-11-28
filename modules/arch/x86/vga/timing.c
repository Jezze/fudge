#include <fudge/module.h>
#include <base/base.h>
#include <base/terminal.h>
#include <base/video.h>
#include <arch/x86/io/io.h>
#include "vga.h"
#include "registers.h"
#include "timing.h"

#define VGA_CRTC_COUNT                  24
#define VGA_ATC_COUNT                   21
#define VGA_GRAPHICS_COUNT              9
#define VGA_SEQUENCER_COUNT             5
#define VGA_MISC_COUNT

#define VGA_CRTC_OFFSET                 0
#define VGA_ATC_OFFSET                  24
#define VGA_GRAPHICS_OFFSET             45
#define VGA_SEQUENCER_OFFSET            54
#define VGA_MISCOUTPUT                  59
#define VGA_TOTAL_REGS                  60

#define VGAREG_CR(i)                    (i)
#define VGAREG_AR(i)                    (i + VGA_ATC_OFFSET)
#define VGAREG_GR(i)                    (i + VGA_GRAPHICS_OFFSET)
#define VGAREG_SR(i)                    (i + VGA_SEQUENCER_OFFSET)

#define VGA_CR0                         VGAREG_CR(0x00)
#define VGA_CR1                         VGAREG_CR(0x01)
#define VGA_CR2                         VGAREG_CR(0x02)
#define VGA_CR3                         VGAREG_CR(0x03)
#define VGA_CR4                         VGAREG_CR(0x04)
#define VGA_CR5                         VGAREG_CR(0x05)
#define VGA_CR6                         VGAREG_CR(0x06)
#define VGA_CR7                         VGAREG_CR(0x07)
#define VGA_CR8                         VGAREG_CR(0x08)
#define VGA_CR9                         VGAREG_CR(0x09)
#define VGA_CRA                         VGAREG_CR(0x0A)
#define VGA_CRB                         VGAREG_CR(0x0B)
#define VGA_CRC                         VGAREG_CR(0x0C)
#define VGA_CRD                         VGAREG_CR(0x0D)
#define VGA_CRE                         VGAREG_CR(0x0E)
#define VGA_CRF                         VGAREG_CR(0x0F)
#define VGA_CR10                        VGAREG_CR(0x10)
#define VGA_CR11                        VGAREG_CR(0x11)
#define VGA_CR12                        VGAREG_CR(0x12)
#define VGA_CR13                        VGAREG_CR(0x13)
#define VGA_SCANLINEOFFSET              VGAREG_CR(0x13)
#define VGA_CR14                        VGAREG_CR(0x14)
#define VGA_CR15                        VGAREG_CR(0x15)
#define VGA_CR16                        VGAREG_CR(0x16)
#define VGA_CR17                        VGAREG_CR(0x17)
#define VGA_CR18                        VGAREG_CR(0x18)
#define VGA_AR0                         VGAREG_AR(0x00)
#define VGA_AR10                        VGAREG_AR(0x10)
#define VGA_AR11                        VGAREG_AR(0x11)
#define VGA_AR12                        VGAREG_AR(0x12)
#define VGA_AR13                        VGAREG_AR(0x13)
#define VGA_AR14                        VGAREG_AR(0x14)
#define VGA_AR0                         VGAREG_AR(0x00)
#define VGA_AR10                        VGAREG_AR(0x10)
#define VGA_AR11                        VGAREG_AR(0x11)
#define VGA_AR12                        VGAREG_AR(0x12)
#define VGA_AR13                        VGAREG_AR(0x13)
#define VGA_AR14                        VGAREG_AR(0x14)
#define VGA_GR0                         VGAREG_GR(0x00)
#define VGA_GR1                         VGAREG_GR(0x01)
#define VGA_GR2                         VGAREG_GR(0x02)
#define VGA_GR3                         VGAREG_GR(0x03)
#define VGA_GR4                         VGAREG_GR(0x04)
#define VGA_GR5                         VGAREG_GR(0x05)
#define VGA_GR6                         VGAREG_GR(0x06)
#define VGA_GR7                         VGAREG_GR(0x07)
#define VGA_GR8                         VGAREG_GR(0x08)
#define VGA_SR0                         VGAREG_SR(0x00)
#define VGA_SR1                         VGAREG_SR(0x01)
#define VGA_SR2                         VGAREG_SR(0x02)
#define VGA_SR3                         VGAREG_SR(0x03)
#define VGA_SR4                         VGAREG_SR(0x04)

#define CLOCK_ALLOWANCE                 10
#define PROGRAMMABLE_CLOCK_MAGIC_NUMBER 0x1234

static struct vga_info infos[] = {
    {80, 25, VGA_COLOR4, 160, 0},
    {320, 200, VGA_COLOR4, 40, 0},
    {640, 200, VGA_COLOR4, 80, 0},
    {640, 350, VGA_COLOR4, 80, 0},
    {640, 480, VGA_COLOR4, 80, 0},
    {320, 200, VGA_COLOR8, 320, 1},
    {320, 240, VGA_COLOR8, 80, 0},
    {320, 400, VGA_COLOR8, 80, 0},
    {360, 480, VGA_COLOR8, 90, 0},
    {640, 480, VGA_COLOR1, 80, 0},
    {640, 480, VGA_COLOR8, 640, 1},
    {800, 600, VGA_COLOR8, 800, 1},
    {1024, 768, VGA_COLOR8, 1024, 1},
    {1280, 1024, VGA_COLOR8, 1280, 1},
    {320, 200, VGA_COLOR15, 640, 2},
    {320, 200, VGA_COLOR16, 640, 2},
    {320, 200, VGA_COLOR24, 320 * 3, 3},
    {640, 480, VGA_COLOR15, 640 * 2, 2},
    {640, 480, VGA_COLOR16, 640 * 2, 2},
    {640, 480, VGA_COLOR24, 640 * 3, 3},
    {800, 600, VGA_COLOR15, 800 * 2, 2},
    {800, 600, VGA_COLOR16, 800 * 2, 2},
    {800, 600, VGA_COLOR24, 800 * 3, 3},
    {1024, 768, VGA_COLOR15, 1024 * 2, 2},
    {1024, 768, VGA_COLOR16, 1024 * 2, 2},
    {1024, 768, VGA_COLOR24, 1024 * 3, 3},
    {1280, 1024, VGA_COLOR15, 1280 * 2, 2},
    {1280, 1024, VGA_COLOR16, 1280 * 2, 2},
    {1280, 1024, VGA_COLOR24, 1280 * 3, 3},
    {800, 600, VGA_COLOR4, 100, 0},
    {1024, 768, VGA_COLOR4, 128, 0},
    {1280, 1024, VGA_COLOR4, 160, 0},
    {720, 348, VGA_COLOR1, 90, 0},
    {320, 200, VGA_COLOR24, 320 * 4, 4},
    {640, 480, VGA_COLOR24, 640 * 4, 4},
    {800, 600, VGA_COLOR24, 800 * 4, 4},
    {1024, 768, VGA_COLOR24, 1024 * 4, 4},
    {1280, 1024, VGA_COLOR24, 1280 * 4, 4},
    {1152, 864, VGA_COLOR4, 144, 0},
    {1152, 864, VGA_COLOR8, 1152, 1},
    {1152, 864, VGA_COLOR15, 1152 * 2, 2},
    {1152, 864, VGA_COLOR16, 1152 * 2, 2},
    {1152, 864, VGA_COLOR24, 1152 * 3, 3},
    {1152, 864, VGA_COLOR24, 1152 * 4, 4},
    {1600, 1200, VGA_COLOR4, 200, 0},
    {1600, 1200, VGA_COLOR8, 1600, 1},
    {1600, 1200, VGA_COLOR15, 1600 * 2, 2},
    {1600, 1200, VGA_COLOR16, 1600 * 2, 2},
    {1600, 1200, VGA_COLOR24, 1600 * 3, 3},
    {1600, 1200, VGA_COLOR24, 1600 * 4, 4},
    {320, 240, VGA_COLOR8, 320, 1},
    {320, 240, VGA_COLOR15, 320 * 2, 2},
    {320, 240, VGA_COLOR16, 320 * 2, 2},
    {320, 240, VGA_COLOR24, 320 * 3, 3},
    {320, 240, VGA_COLOR24, 320 * 4, 4},
    {400, 300, VGA_COLOR8, 400, 1},
    {400, 300, VGA_COLOR15, 400 * 2, 2},
    {400, 300, VGA_COLOR16, 400 * 2, 2},
    {400, 300, VGA_COLOR24, 400 * 3, 3},
    {400, 300, VGA_COLOR24, 400 * 4, 4},
    {512, 384, VGA_COLOR8, 512, 1}, 
    {512, 384, VGA_COLOR15, 512 * 2, 2},
    {512, 384, VGA_COLOR16, 512 * 2, 2},
    {512, 384, VGA_COLOR24, 512 * 3, 3},
    {512, 384, VGA_COLOR24, 512 * 4, 4},
    {960, 720, VGA_COLOR8, 960, 1},
    {960, 720, VGA_COLOR15, 960 * 2, 2},
    {960, 720, VGA_COLOR16, 960 * 2, 2},
    {960, 720, VGA_COLOR24, 960 * 3, 3},
    {960, 720, VGA_COLOR24, 960 * 4, 4},
    {1920, 1440, VGA_COLOR8, 1920, 1},
    {1920, 1440, VGA_COLOR15, 1920 * 2, 2},
    {1920, 1440, VGA_COLOR16, 1920 * 2, 2},
    {1920, 1440, VGA_COLOR24, 1920 * 3, 3},
    {1920, 1440, VGA_COLOR24, 1920 * 4, 4},
    {320, 400, VGA_COLOR8, 320, 1},
    {320, 400, VGA_COLOR15, 320 * 2, 2},
    {320, 400, VGA_COLOR16, 320 * 2, 2},
    {320, 400, VGA_COLOR24, 320 * 3, 3},
    {320, 400, VGA_COLOR24, 320 * 4, 4},
    {640, 400, VGA_COLOR8, 640, 1},
    {640, 400, VGA_COLOR15, 640 * 2, 2},
    {640, 400, VGA_COLOR16, 640 * 2, 2},
    {640, 400, VGA_COLOR24, 640 * 3, 3},
    {640, 400, VGA_COLOR24, 640 * 4, 4},
    {320, 480, VGA_COLOR8, 320, 1},
    {320, 480, VGA_COLOR15, 320 * 2, 2},
    {320, 480, VGA_COLOR16, 320 * 2, 2},
    {320, 480, VGA_COLOR24, 320 * 3, 3},
    {320, 480, VGA_COLOR24, 320 * 4, 4},
    {720, 540, VGA_COLOR8, 720, 1},
    {720, 540, VGA_COLOR15, 720 * 2, 2},
    {720, 540, VGA_COLOR16, 720 * 2, 2},
    {720, 540, VGA_COLOR24, 720 * 3, 3},
    {720, 540, VGA_COLOR24, 720 * 4, 4},
    {848, 480, VGA_COLOR8, 848, 1},
    {848, 480, VGA_COLOR15, 848 * 2, 2},
    {848, 480, VGA_COLOR16, 848 * 2, 2},
    {848, 480, VGA_COLOR24, 848 * 3, 3},
    {848, 480, VGA_COLOR24, 848 * 4, 4},
    {1072, 600, VGA_COLOR8, 1072, 1},
    {1072, 600, VGA_COLOR15, 1072 * 2, 2},
    {1072, 600, VGA_COLOR16, 1072 * 2, 2},
    {1072, 600, VGA_COLOR24, 1072 * 3, 3},
    {1072, 600, VGA_COLOR24, 1072 * 4, 4},
    {1280, 720, VGA_COLOR8, 1280, 1},
    {1280, 720, VGA_COLOR15, 1280 * 2, 2},
    {1280, 720, VGA_COLOR16, 1280 * 2, 2},
    {1280, 720, VGA_COLOR24, 1280 * 3, 3},
    {1280, 720, VGA_COLOR24, 1280 * 4, 4},
    {1360, 768, VGA_COLOR8, 1360, 1},
    {1360, 768, VGA_COLOR15, 1360 * 2, 2},
    {1360, 768, VGA_COLOR16, 1360 * 2, 2},
    {1360, 768, VGA_COLOR24, 1360 * 3, 3},
    {1360, 768, VGA_COLOR24, 1360 * 4, 4},
    {1800, 1012, VGA_COLOR8, 1800, 1},
    {1800, 1012, VGA_COLOR15, 1800 * 2, 2},
    {1800, 1012, VGA_COLOR16, 1800 * 2, 2},
    {1800, 1012, VGA_COLOR24, 1800 * 3, 3},
    {1800, 1012, VGA_COLOR24, 1800 * 4, 4},
    {1920, 1080, VGA_COLOR8, 1920, 1},
    {1920, 1080, VGA_COLOR15, 1920 * 2, 2},
    {1920, 1080, VGA_COLOR16, 1920 * 2, 2},
    {1920, 1080, VGA_COLOR24, 1920 * 3, 3},
    {1920, 1080, VGA_COLOR24, 1920 * 4, 4},
    {2048, 1152, VGA_COLOR8, 2048, 1},
    {2048, 1152, VGA_COLOR15, 2048 * 2, 2},
    {2048, 1152, VGA_COLOR16, 2048 * 2, 2},
    {2048, 1152, VGA_COLOR24, 2048 * 3, 3},
    {2048, 1152, VGA_COLOR24, 2048 * 4, 4},
    {2048, 1536, VGA_COLOR8, 2048, 1},
    {2048, 1536, VGA_COLOR15, 2048 * 2, 2},
    {2048, 1536, VGA_COLOR16, 2048 * 2, 2},
    {2048, 1536, VGA_COLOR24, 2048 * 3, 3},
    {2048, 1536, VGA_COLOR24, 2048 * 4, 4},
    {512, 480, VGA_COLOR8, 512, 1},
    {512, 480, VGA_COLOR15, 512 * 2, 2},
    {512, 480, VGA_COLOR16, 512 * 2, 2},
    {512, 480, VGA_COLOR24, 512 * 3, 3},
    {512, 480, VGA_COLOR24, 512 * 4, 4},
    {400, 600, VGA_COLOR8, 400, 1},
    {400, 600, VGA_COLOR15, 400 * 2, 2},
    {400, 600, VGA_COLOR16, 400 * 2, 2},
    {400, 600, VGA_COLOR24, 400 * 3, 3},
    {400, 600, VGA_COLOR24, 400 * 4, 4},
    {400, 300, VGA_COLOR8, 100, 0},
    {320, 200, VGA_COLOR8, 320, 1},

};

static struct vga_monitormodetiming timings[] = {
    /* 320x200 @ 70 Hz, 31.5 kHz hsync */
    {12588, 320, 336, 384, 400, 200, 204, 206, 225, DOUBLESCAN},
    /* 320x200 @ 83 Hz, 37.5 kHz hsync */
    {13333, 320, 336, 384, 400, 200, 204, 206, 225, DOUBLESCAN},
    /* 320x240 @ 60 Hz, 31.5 kHz hsync */
    {12588, 320, 336, 384, 400, 240, 245, 247, 263, DOUBLESCAN},
    /* 320x240 @ 72Hz, 38.5 kHz hsync */
    {15000, 320, 336, 384, 400, 240, 244, 246, 261, DOUBLESCAN},
    /* 320x400 @ 70 Hz, 31.5 kHz hsync */
    {12588, 320, 336, 384, 400, 400, 408, 412, 450, 0},
    /* 320x400 @ 83 Hz, 37.5 kHz hsync */
    {13333, 320, 336, 384, 400, 400, 408, 412, 450, 0},
    /* 320x480 @ 60 Hz, 31.5 kHz hsync */
    {12588, 320, 336, 384, 400, 480, 490, 494, 526, 0},
    /* 320x480 @ 72Hz, 38.5 kHz hsync */
    {15000, 320, 336, 384, 400, 480, 488, 492, 522, 0},
    /* 400x300 @ 56 Hz, 35.2 kHz hsync, 4:3 aspect ratio */
    {18000, 400, 416, 448, 512, 300, 301, 302, 312, DOUBLESCAN},
    /* 400x300 @ 60 Hz, 37.8 kHz hsync */
    {20000, 400, 416, 480, 528, 300, 301, 303, 314, DOUBLESCAN},
    /* 400x300 @ 72 Hz, 48.0 kHz hsync*/
    {25000, 400, 424, 488, 520, 300, 319, 322, 333, DOUBLESCAN},
    /* 400x600 @ 56 Hz, 35.2 kHz hsync, 4:3 aspect ratio */
    {18000, 400, 416, 448, 512, 600, 602, 604, 624, 0},
    /* 400x600 @ 60 Hz, 37.8 kHz hsync */
    {20000, 400, 416, 480, 528, 600, 602, 606, 628, 0},
    /* 400x600 @ 72 Hz, 48.0 kHz hsync*/
    {25000, 400, 424, 488, 520, 600, 639, 644, 666, 0},
    /* 512x384 @ 67Hz */
    {19600, 512, 522, 598, 646, 384, 418, 426, 454, 0 },
    /* 512x384 @ 86Hz */
    {25175, 512, 522, 598, 646, 384, 418, 426, 454,0},
    /* 512x480 @ 55Hz */
    {19600, 512, 522, 598, 646, 480, 500, 510, 550, 0},
    /* 512x480 @ 71Hz */
    {25175, 512, 522, 598, 646, 480, 500, 510, 550,0},
    {25175, 640, 664, 760, 800, 400, 409, 411, 450, 0},
    /* 640x480 at 60 Hz, 31.5 kHz hsync */
    {25175, 640, 664, 760, 800, 480, 491, 493, 525, 0},
    /* 640x480 at 72 Hz, 36.5 kHz hsync */
    {31500, 640, 664, 704, 832, 480, 489, 491, 520, 0},
    /* 800x600 at 56 Hz, 35.15 kHz hsync */
    {36000, 800, 824, 896, 1024, 600, 601, 603, 625, 0},
    /* 800x600 at 60 Hz, 37.8 kHz hsync */
    {40000, 800, 840, 968, 1056, 600, 601, 605, 628, PHSYNC | PVSYNC},
    /* 800x600 at 72 Hz, 48.0 kHz hsync */
    {50000, 800, 856, 976, 1040, 600, 637, 643, 666, PHSYNC | PVSYNC},
    /* 960x720 @ 70Hz */
    {66000, 960, 984, 1112, 1248, 720, 723, 729, 756, NHSYNC | NVSYNC},
    /* 960x720* interlaced, 35.5 kHz hsync */
    {40000, 960, 984, 1192, 1216, 720, 728, 784, 817, INTERLACED},
    /* 1024x768 at 87 Hz interlaced, 35.5 kHz hsync */
    {44900, 1024, 1048, 1208, 1264, 768, 776, 784, 817, INTERLACED},
    /* 1024x768 at 100 Hz, 40.9 kHz hsync */
    {55000, 1024, 1048, 1208, 1264, 768, 776, 784, 817, INTERLACED},
    /* 1024x768 at 60 Hz, 48.4 kHz hsync */
    {65000, 1024, 1032, 1176, 1344, 768, 771, 777, 806, NHSYNC | NVSYNC},
    /* 1024x768 at 70 Hz, 56.6 kHz hsync */
    {75000, 1024, 1048, 1184, 1328, 768, 771, 777, 806, NHSYNC | NVSYNC},
    /* 1152x864 at 59.3Hz */
    {85000, 1152, 1214, 1326, 1600, 864, 870, 885, 895, 0},
    /* 1280x1024 at 87 Hz interlaced, 51 kHz hsync */
    {80000, 1280, 1296, 1512, 1568, 1024, 1025, 1037, 1165, INTERLACED},
    /* 1024x768 at 76 Hz, 62.5 kHz hsync */
    {85000, 1024, 1032, 1152, 1360, 768, 784, 787, 823, 0},
    /* 1280x1024 at 60 Hz, 64.3 kHz hsync */
    {110000, 1280, 1328, 1512, 1712, 1024, 1025, 1028, 1054, 0},
    /* 1280x1024 at 74 Hz, 78.9 kHz hsync */
    {135000, 1280, 1312, 1456, 1712, 1024, 1027, 1030, 1064, 0},
    /* 1600x1200 at 60Hz */
    {162000, 1600, 1668, 1860, 2168, 1200, 1201, 1204, 1250, 0},
    /* 1600x1200 at 68Hz */
    {188500, 1600, 1792, 1856, 2208, 1200, 1202, 1205, 1256, 0},
    /* 1600x1200 at 75 Hz */
    {198000, 1600, 1616, 1776, 2112, 1200, 1201, 1204, 1250, 0},
    /* 720x540 at 56 Hz, 35.15 kHz hsync */
    {32400, 720, 744, 808, 920, 540, 541, 543, 563, 0},
    /* 720x540 at 60 Hz, 37.8 kHz hsync */
    {36000, 720, 760, 872, 952, 540, 541, 545, 565, 0},
    /* 720x540 at 72 Hz, 48.0 kHz hsync */
    {45000, 720, 768, 880, 936, 540, 552, 558, 599, 0},
    /* 1072x600 at 57 Hz interlaced, 35.5 kHz hsync */
    {44900, 1072, 1096, 1208, 1264, 600, 602, 604, 625, 0},
    /* 1072x600 at 65 Hz, 40.9 kHz hsync */
    {55000, 1072, 1096, 1208, 1264, 600, 602, 604, 625, 0},
    /* 1072x600 at 78 Hz, 48.4 kHz hsync */
    {65000, 1072, 1088, 1184, 1344, 600, 603, 607, 625, NHSYNC | NVSYNC},
    /* 1072x600 at 90 Hz, 56.6 kHz hsync */
    {75000, 1072, 1096, 1200, 1328, 768, 603, 607, 625, NHSYNC | NVSYNC},
    /* 1072x600 at 100 Hz, 62.5 kHz hsync */
    {85000, 1072, 1088, 1160, 1360, 768, 603, 607, 625, 0},
};

void vga_setregisters(unsigned char *registers, unsigned int ega)
{

    unsigned int i;

    if (ega)
    {

    }

    io_outb(VGA_REGISTER_MISCWRITE, registers[VGA_MISCOUTPUT]);

    outsr(0x00, 0x01);
    outsr(0x01, registers[VGA_SEQUENCER_OFFSET + 1] | 0x20);

    for (i = 2; i < VGA_SEQUENCER_COUNT; i++)
        outsr(i, registers[VGA_SEQUENCER_OFFSET + i]);

    outsr(0x00, 0x03);

    if (ega)
        outcrt1(0x11, incrt1(0x11) & 0x7f);

    for (i = 0; i < VGA_CRTC_COUNT; i++)
        outcrt1(i, registers[VGA_CRTC_OFFSET + i]);

    for (i = 0; i < VGA_GRAPHICS_COUNT; i++)
        outgr(i, registers[VGA_GRAPHICS_OFFSET + i]);

    for (i = 0; i < VGA_ATC_COUNT; i++)
        outar(i, registers[VGA_ATC_OFFSET + i]);

}

void vga_initregisters(unsigned char *registers, struct vga_modetiming *modetiming, struct vga_modeinfo *modeinfo)
{

    int i;

    if ((modetiming->flags & (PHSYNC | NHSYNC)) && (modetiming->flags & (PVSYNC | NVSYNC)))
    {

        registers[VGA_MISCOUTPUT] = 0x23;

        if (modetiming->flags & NHSYNC)
            registers[VGA_MISCOUTPUT] |= 0x40;

        if (modetiming->flags & NVSYNC)
            registers[VGA_MISCOUTPUT] |= 0x80;

    }

    else
    {

        if (modetiming->VDisplay < 400)
            registers[VGA_MISCOUTPUT] = 0xA3;
        else if (modetiming->VDisplay < 480)
            registers[VGA_MISCOUTPUT] = 0x63;
        else if (modetiming->VDisplay < 768)
            registers[VGA_MISCOUTPUT] = 0xE3;
        else
            registers[VGA_MISCOUTPUT] = 0x23;

    }

    registers[VGA_SR0] = 0x00;

    if (modeinfo->bpp == VGA_BPP4)
        registers[VGA_SR0] = 0x02;

    registers[VGA_SR1] = 0x01;
    registers[VGA_SR2] = 0x0F;
    registers[VGA_SR3] = 0x00;
    registers[VGA_SR4] = 0x0E;

    if (modeinfo->bpp == VGA_BPP4)
        registers[VGA_SR4] = 0x06;

    registers[VGA_CR0] = (modetiming->CrtcHTotal / 8) - 5;
    registers[VGA_CR1] = (modetiming->CrtcHDisplay / 8) - 1;
    registers[VGA_CR2] = (modetiming->CrtcHSyncStart / 8) - 1;
    registers[VGA_CR3] = ((modetiming->CrtcHSyncEnd / 8) & 0x1F) | 0x80;
    registers[VGA_CR4] = (modetiming->CrtcHSyncStart / 8);
    registers[VGA_CR5] = (((modetiming->CrtcHSyncEnd / 8) & 0x20) << 2) | ((modetiming->CrtcHSyncEnd / 8) & 0x1F);
    registers[VGA_CR6] = (modetiming->CrtcVTotal - 2) & 0xFF;
    registers[VGA_CR7] = (((modetiming->CrtcVTotal - 2) & 0x100) >> 8) | (((modetiming->CrtcVDisplay - 1) & 0x100) >> 7) | ((modetiming->CrtcVSyncStart & 0x100) >> 6) | (((modetiming->CrtcVSyncStart) & 0x100) >> 5) | 0x10 | (((modetiming->CrtcVTotal - 2) & 0x200) >> 4) | (((modetiming->CrtcVDisplay - 1) & 0x200) >> 3) | ((modetiming->CrtcVSyncStart & 0x200) >> 2);
    registers[VGA_CR8] = 0x00;
    registers[VGA_CR9] = ((modetiming->CrtcVSyncStart & 0x200) >> 4) | 0x40;

    if (modetiming->flags & DOUBLESCAN)
        registers[VGA_CR9] |= 0x80;

    registers[VGA_CRA] = 0x00;
    registers[VGA_CRB] = 0x00;
    registers[VGA_CRC] = 0x00;
    registers[VGA_CRD] = 0x00;
    registers[VGA_CRE] = 0x00;
    registers[VGA_CRF] = 0x00;
    registers[VGA_CR10] = modetiming->CrtcVSyncStart & 0xFF;
    registers[VGA_CR11] = (modetiming->CrtcVSyncEnd & 0x0F) | 0x20;
    registers[VGA_CR12] = (modetiming->CrtcVDisplay - 1) & 0xFF;
    registers[VGA_CR13] = modeinfo->lineWidth >> 4;
    registers[VGA_CR14] = 0x00;
    registers[VGA_CR15] = modetiming->CrtcVSyncStart & 0xFF;
    registers[VGA_CR16] = (modetiming->CrtcVSyncStart + 1) & 0xFF;
    registers[VGA_CR17] = 0xC3;

    if (modeinfo->bpp == VGA_BPP4)
        registers[VGA_CR17] = 0xE3;

    registers[VGA_CR18] = 0xFF;
    registers[VGA_GR0] = 0x00;
    registers[VGA_GR1] = 0x00;
    registers[VGA_GR2] = 0x00;
    registers[VGA_GR3] = 0x00;
    registers[VGA_GR4] = 0x00;
    registers[VGA_GR5] = 0x40;

    if (modeinfo->bpp == VGA_BPP4)
        registers[VGA_GR5] = 0x02;

    registers[VGA_GR6] = 0x05;
    registers[VGA_GR7] = 0x0F;
    registers[VGA_GR8] = 0xFF;

    for (i = 0; i < 16; i++)
        registers[VGA_AR0 + i] = i;

    registers[VGA_AR10] = 0x41;

    if (modeinfo->bpp == VGA_BPP4)
        registers[VGA_AR10] = 0x01;

    registers[VGA_AR11] = 0x00;
    registers[VGA_AR12] = 0x0F;
    registers[VGA_AR13] = 0x00;
    registers[VGA_AR14] = 0x00;

}

static int findclock(int clock, struct vga_cardspecs *cardspecs)
{

    int i;

    for (i = 0; i < cardspecs->nClocks; i++)
    {

        int diff = cardspecs->clocks[i] - clock;

        if (diff < 0)
            diff = -diff;

        if (diff * 1000 / clock < CLOCK_ALLOWANCE)
            return i;

    }

    if (cardspecs->flags & CLOCK_PROGRAMMABLE)
    {

        int diff = cardspecs->matchProgrammableClock(clock) - clock;

        if (diff < 0)
            diff = -diff;

        if (diff * 1000 / clock < CLOCK_ALLOWANCE)
            return PROGRAMMABLE_CLOCK_MAGIC_NUMBER;

    }

    return -1;

}

static unsigned int get_maxclock(unsigned int bpp, struct vga_cardspecs *cardspecs)
{

    if (bpp == VGA_BPP4)
        return cardspecs->maxPixelClock4bpp;
    else if (bpp == VGA_BPP8)
        return cardspecs->maxPixelClock8bpp;
    else if (bpp == VGA_BPP16)
        return cardspecs->maxPixelClock16bpp;
    else if (bpp == VGA_BPP24)
        return cardspecs->maxPixelClock24bpp;
    else if (bpp == VGA_BPP32)
        return cardspecs->maxPixelClock32bpp;

    return 0;

}

static struct vga_monitormodetiming *find_timing(struct vga_modeinfo *modeinfo, struct vga_cardspecs *cardspecs)
{

    int maxclock = get_maxclock(modeinfo->bpp, cardspecs);
    int clock = 0;
    struct vga_monitormodetiming *timing = 0;
    struct vga_monitormodetiming *current;

    for (current = timings; current; current = current->next)
    {

        if (current->HDisplay != modeinfo->width)
            continue;

        if (current->VDisplay != modeinfo->height)
            continue;

        if (current->pixelClock > maxclock)
            continue;

        if (current->pixelClock <= clock)
            continue;

        if (((!(current->flags & INTERLACED)) || (!(cardspecs->flags & NO_INTERLACE))) && cardspecs->mapHorizontalCrtc(modeinfo->bpp, current->pixelClock, current->HTotal) <= cardspecs->maxHorizontalCrtc && findclock(cardspecs->mapClock(modeinfo->bpp, current->pixelClock), cardspecs) != -1)
        {

            clock = current->pixelClock;
            timing = current;

        }

    }

    return timing;

}

void vga_initmodetiming(struct vga_modetiming *modetiming, struct vga_modeinfo *modeinfo, struct vga_cardspecs *cardspecs)
{

    struct vga_monitormodetiming *timing = find_timing(modeinfo, cardspecs);
    int desiredclock;

    if (!timing)
        return;

    modetiming->flags = timing->flags;
    modetiming->pixelClock = timing->pixelClock;
    desiredclock = cardspecs->mapClock(modeinfo->bpp, timing->pixelClock);
    modetiming->selectedClockNo = findclock(desiredclock, cardspecs);

    if (modetiming->selectedClockNo == PROGRAMMABLE_CLOCK_MAGIC_NUMBER)
    {

        modetiming->programmedClock = cardspecs->matchProgrammableClock(desiredclock);
        modetiming->flags |= USEPROGRCLOCK;

    }

    else
    {

        modetiming->programmedClock = cardspecs->clocks[modetiming->selectedClockNo];

    }

    modetiming->HDisplay = timing->HDisplay;
    modetiming->HSyncStart = timing->HSyncStart;
    modetiming->HSyncEnd = timing->HSyncEnd;
    modetiming->HTotal = timing->HTotal;

    if (cardspecs->mapHorizontalCrtc(modeinfo->bpp, modetiming->programmedClock, timing->HTotal) != timing->HTotal)
    {

        modetiming->CrtcHDisplay = cardspecs->mapHorizontalCrtc(modeinfo->bpp, modetiming->programmedClock, timing->HDisplay);
        modetiming->CrtcHSyncStart = cardspecs->mapHorizontalCrtc(modeinfo->bpp, modetiming->programmedClock, timing->HSyncStart);
        modetiming->CrtcHSyncEnd = cardspecs->mapHorizontalCrtc(modeinfo->bpp, modetiming->programmedClock, timing->HSyncEnd);
        modetiming->CrtcHTotal = cardspecs->mapHorizontalCrtc(modeinfo->bpp, modetiming->programmedClock, timing->HTotal);
        modetiming->flags |= HADJUSTED;

    }

    else
    {

        modetiming->CrtcHDisplay = timing->HDisplay;
        modetiming->CrtcHSyncStart = timing->HSyncStart;
        modetiming->CrtcHSyncEnd = timing->HSyncEnd;
        modetiming->CrtcHTotal = timing->HTotal;

    }

    modetiming->VDisplay = timing->VDisplay;
    modetiming->VSyncStart = timing->VSyncStart;
    modetiming->VSyncEnd = timing->VSyncEnd;
    modetiming->VTotal = timing->VTotal;

    if (modetiming->flags & DOUBLESCAN)
    {

        modetiming->VDisplay <<= 1;
        modetiming->VSyncStart <<= 1;
        modetiming->VSyncEnd <<= 1;
        modetiming->VTotal <<= 1;

    }

    modetiming->CrtcVDisplay = modetiming->VDisplay;
    modetiming->CrtcVSyncStart = modetiming->VSyncStart;
    modetiming->CrtcVSyncEnd = modetiming->VSyncEnd;
    modetiming->CrtcVTotal = modetiming->VTotal;

    if (((modetiming->flags & INTERLACED) && (cardspecs->flags & INTERLACE_DIVIDE_VERT)) || (modetiming->VTotal >= 1024 && (cardspecs->flags & GREATER_1024_DIVIDE_VERT)))
    {

        modetiming->CrtcVDisplay /= 2;
        modetiming->CrtcVSyncStart /= 2;
        modetiming->CrtcVSyncEnd /= 2;
        modetiming->CrtcVTotal /= 2;
        modetiming->flags |= VADJUSTED;

    }

}

void vga_initmodeinfo(struct vga_modeinfo *modeinfo, int mode)
{

    modeinfo->width = infos[mode].xdim;
    modeinfo->height = infos[mode].ydim;
    modeinfo->bytespp = infos[mode].bytespp;

    switch (infos[mode].colors)
    {

    case VGA_COLOR4:
        modeinfo->colorBits = VGA_BPP4;

        break;

    case VGA_COLOR8:
        modeinfo->colorBits = VGA_BPP8;

        break;

    case VGA_COLOR15:
        modeinfo->colorBits = VGA_BPP15;
        modeinfo->blueOffset = 0;
        modeinfo->greenOffset = 5;
        modeinfo->redOffset = 10;
        modeinfo->blueWeight = 5;
        modeinfo->greenWeight = 5;
        modeinfo->redWeight = 5;

        break;

    case VGA_COLOR16:
        modeinfo->colorBits = VGA_BPP16;
        modeinfo->blueOffset = 0;
        modeinfo->greenOffset = 5;
        modeinfo->redOffset = 11;
        modeinfo->blueWeight = 5;
        modeinfo->greenWeight = 6;
        modeinfo->redWeight = 5;

        break;

    case VGA_COLOR24:
        modeinfo->colorBits = VGA_BPP24;
        modeinfo->blueOffset = 0;
        modeinfo->greenOffset = 8;
        modeinfo->redOffset = 16;
        modeinfo->blueWeight = 8;
        modeinfo->greenWeight = 8;
        modeinfo->redWeight = 8;

        break;

    }

    modeinfo->bpp = modeinfo->bytespp * 8;

    if (infos[mode].colors == VGA_COLOR4)
        modeinfo->bpp = VGA_BPP4;

    modeinfo->lineWidth = infos[mode].xbytes;

}

