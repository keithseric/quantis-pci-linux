# Quantis PCI/PCIe Linux Driver

An independent modernization of the ID Quantique Quantis
PCI/PCIe quantum random number generator driver for Linux 6.1.

This project originates from the Quantis v18.03.08 driver
and updates its Linux implementation for contemporary
kernel interfaces.

## Project status

Experimental, community-maintained driver.

The driver has been tested with physical Quantis hardware
on Debian GNU/Linux 12 (Bookworm).

| Component | Tested configuration |
|---|---|
| Operating system | Debian 12 |
| Kernel | Linux 6.1.0-54-amd64 |
| Architecture | x86-64 |
| Hardware | Quantis PCI 16 Mbps |
| PCI vendor/device ID | 179a:0001 |
| PCI revision | 50 |
| Board core version | 0x040a1201 |
| RNG modules | 4 |
| Observed throughput | Approximately 16 Mbps |

Other hardware and kernel versions may work but have not
been validated.

The publication source has passed isolated compilation
against the tested kernel. Its diagnostic utility has
also been compiled and tested against physical hardware
using the installed development driver.

See the validation document for the distinction between
production hardware testing and publication build testing.

## Features

- Linux 6.1 kernel compatibility
- Modern PCI device lifecycle management
- Updated character device handling
- Memory-mapped I/O resource handling
- Hardware module health enforcement
- DKMS support
- Hardware diagnostic utility
- Direct hardware random data access
- No software random-number fallback in the driver

## Quick start

Install the required build dependencies:

    sudo apt update
    sudo apt install build-essential dkms linux-headers-$(uname -r)

Build the kernel module:

    make -C linux modules

Build the diagnostic utility:

    gcc -O2 -Wall -Wextra \
        -Icommon \
        tools/quantis-status.c \
        -o tools/quantis-status

For installation and DKMS registration, see
[Installation](docs/installation.md).

## Usage

The first detected Quantis device is normally exposed as:

    /dev/qrandom0

Read hardware-generated random bytes:

    sudo dd if=/dev/qrandom0 bs=32 count=1 status=none | xxd

Inspect hardware diagnostics:

    sudo ./tools/quantis-status

Example diagnostic output from the tested hardware:

    Driver version:       29
    Detected cards:       1
    Board core version:   0x040a1201
    Modules present:      0x0f
    Module status mask:   0x0f
    AIS31 request flag:   1
    Reported module health: PASS

## Documentation

- [Installation and DKMS](docs/installation.md)
- [Hardware compatibility](docs/hardware-compatibility.md)
- [Driver architecture](docs/driver-architecture.md)
- [Hardware validation](docs/validation.md)
- [Known limitations](docs/known-limitations.md)
- [Security policy](SECURITY.md)
- [Licensing](LICENSING.md)
- [Source provenance and attribution](NOTICE)

## Licensing

Project modifications and newly developed supporting code
are distributed under GPL-2.0-only.

Original ID Quantique copyright notices and upstream
licensing grants are preserved.

See LICENSE, LICENSING.md and NOTICE for details.

## Independence

This is an independent project.

It is not affiliated with, endorsed by, or supported by
ID Quantique SA.

## Development disclosure

Development included AI-assisted programming and analysis
using OpenAI ChatGPT.

Physical hardware testing, deployment, and operational
validation were performed by the project maintainer.

## Assurance limitations

Hardware health reporting and successful statistical tests
do not establish cryptographic certification or prove
the entropy quality of every generated value.

Review the known limitations before deploying this driver
in security-critical environments.

## Security and runtime hardening release

Version 18.03.08-linux61.3 introduces administrative IOCTL
authorization, native 32-bit compatibility, bounded hardware
acquisition deadlines, nonblocking reads, private acquisition
buffers and stronger device lifecycle synchronization.

Runtime qualification on an ID Quantique Quantis PCI 16 Mbps
card under Linux 6.1 demonstrated:

- Approximately 16 Mbps sustained acquisition throughput.
- Approximately 8.9 percent lower sustained acquisition CPU
  consumption in the observed comparative benchmark.
- Successful acquisition with up to 16 independent reader processes.
- Native and 32-bit IOCTL authorization and error handling.
- Correct descriptor isolation following PCI unbinding and rebinding.

See [validation](docs/validation.md) for measured results and
[known limitations](docs/known-limitations.md) for deployment
considerations.

The driver remains experimental and has not undergone a formal
independent security or entropy assessment.
