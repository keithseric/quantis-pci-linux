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

## Acquisition and synchronization in linux61.3

The Linux character-device interface bounds individual read
requests to 64 KiB.

Each device maintains an admission semaphore allowing up to eight
private acquisition buffers to be active at a time. Blocking
read buffers use memory-accounted kernel allocation. Nonblocking
reads use non-sleeping allocation and try-lock acquisition.

The hardware semaphore serializes register access and acquisition.
Userspace buffer copies occur after releasing this semaphore.
Private acquisition buffers are explicitly erased before release.

The maximum admitted acquisition payload is 512 KiB per device
instance, excluding allocator overhead. This is not a global
memory limit across all live and retired device instances.

### Acquisition deadlines

FIFO polling uses monotonic deadlines and sleepable waits.

- Individual FIFO wait deadline: up to 100 milliseconds.
- Overall hardware acquisition deadline: 500 milliseconds.
- FIFO polling sleep range: 50 to 100 microseconds.

The overall acquisition deadline begins after hardware semaphore
acquisition. Admission waits, hardware semaphore waits and
userspace memory copies are outside this deadline.

The deadline is not a hard real-time guarantee.

Blocking acquisition can return ETIMEDOUT, EIO or an interrupted
system-call error when acquisition cannot complete.

Nonblocking acquisition can return EAGAIN for temporary resource
or data unavailability. A successful nonblocking read is bounded
to one available FIFO batch, up to 4096 bytes.

A hardware acquisition failure does not trigger software random
number generation.

### Administrative IOCTL authorization

Administrative operations require both a writable file descriptor
and CAP_SYS_RAWIO.

These operations include board reset, module enable, module disable
and modification of the AIS31 startup-test request flag.

Diagnostic commands require a readable file descriptor.

Module enable and disable operations reject zero masks and masks
containing bits outside the detected module population.

Unknown IOCTL commands return ENOTTY.

The existing fixed-width IOCTL argument representation permits
compat_ptr_ioctl dispatch on supported Linux compatibility kernels.

### Device lifetime and removal

Publication state and hardware access have separate synchronization.

Device removal disconnects the existing device instance and
unpublishes its character device. Outstanding descriptors retain
their associated device lifetime reference until closed.

Nonzero reads and supported diagnostic IOCTLs on disconnected
descriptors fail with ENODEV rather than accessing released MMIO
resources, subject to earlier argument and authorization checks.

Zero-length reads may return zero without accessing hardware.
Unsupported or unauthorized IOCTLs can return their corresponding
command or authorization errors.

An acquisition already in progress can complete before removal
finishes, and previously acquired bytes can be copied to userspace
after disconnection.

Minor numbers are not reused until module unload. Rebinding can
therefore register a different character-device name while old
descriptors remain isolated.

Suspend and resume are not qualified.
