/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Quantis hardware diagnostic utility
 * Maintainer: Seric <seric@seric.com>
 * Developed in 2026.
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "quantis_pci.h"

int main(void)
{
    unsigned int version = 0;
    unsigned int cards = 0;
    unsigned int board = 0;
    unsigned int mask = 0;
    unsigned int status = 0;
    unsigned int ais31 = 0;
    int failed = 0;

    int fd = open("/dev/qrandom0", O_RDONLY | O_CLOEXEC);

    if (fd < 0) {
        perror("Unable to open Quantis device");
        return 1;
    }

#define GET(cmd, var) \
    do { \
        if (ioctl(fd, cmd, &var) < 0) { \
            perror(#cmd); \
            failed = 1; \
        } \
    } while (0)

    GET(QUANTIS_IOCTL_GET_DRIVER_VERSION, version);
    GET(QUANTIS_IOCTL_GET_CARD_COUNT, cards);
    GET(QUANTIS_IOCTL_GET_BOARD_VERSION, board);
    GET(QUANTIS_IOCTL_GET_MODULES_MASK, mask);
    GET(QUANTIS_IOCTL_GET_MODULES_STATUS, status);
    GET(QUANTIS_IOCTL_GET_AIS31_STARTUP_TESTS_REQUEST_FLAG, ais31);

    close(fd);

    if (failed)
        return 1;

    printf("Driver version:       %u\n", version);
    printf("Detected cards:       %u\n", cards);
    printf("Board core version:   0x%08x\n", board);
    printf("Modules present:      0x%02x\n", mask);
    printf("Module status mask:   0x%02x\n", status);
    printf("AIS31 request flag:   %u\n", ais31);

    int healthy = cards > 0 && mask != 0 &&
                  (status & mask) == mask;

    printf("Reported module health: %s\n",
           healthy ? "PASS" : "FAIL");

    return healthy ? 0 : 2;
}
