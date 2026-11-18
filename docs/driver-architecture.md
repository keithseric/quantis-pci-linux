# Driver architecture

The driver is derived from the Linux implementation
included in ID Quantique Quantis v18.03.08.

## Source layout

- common/quantis_pci.h: Shared interface definitions
- common/quantis_pci_common.h: Shared hardware operations
- linux/quantis_pci_linux.c: Linux kernel implementation
- linux/Kbuild: Kernel module build configuration
- linux/Makefile: Build targets
- dkms.conf: DKMS integration
- tools/quantis-status.c: Userspace diagnostic utility

## Data path

    Quantis hardware
          |
    PCI / PCIe interface
          |
    Kernel driver
          |
    /dev/qrandomN
          |
    Userspace application

Applications obtain bytes through the character device.

The driver does not substitute a software-generated
random stream when hardware reads fail.

## Modernization

The modernization addresses:

- Contemporary Linux kernel interfaces
- PCI resource acquisition and release
- PCI device lifecycle management
- Memory-mapped I/O handling
- Character device management
- Device lifetime and slot handling
- Hardware module health validation

## Hardware health enforcement

The modified common hardware implementation checks
module test state, enabled module state, module presence,
and reported health before servicing hardware reads.

Reads are rejected when the relevant module state
conditions are not satisfied.

This does not require that every physically installed
module be enabled.

Hardware state can change between observations.
A successful read is not a guarantee of future health.

## Diagnostic interface

The diagnostic utility uses the existing Quantis IOCTL
interface to query:

- Driver version
- Detected card count
- Board core version
- Module presence mask
- Module health mask
- AIS31 startup test request flag

The utility does not intentionally change hardware
configuration or consume random bytes.

The AIS31 request flag is not an independent indication
that a test suite has passed.
