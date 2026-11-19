# Installation

## Requirements

- Supported Quantis PCI/PCIe hardware
- A compatible PCI or PCIe connection
- Linux kernel headers matching the target kernel
- GCC and GNU Make
- DKMS for automatic module rebuilds

The tested operating system is Debian 12 with Linux 6.1.

## Verify hardware detection

    lspci -nn -d 179a:

Confirm that the Quantis device is detected before
attempting to load the driver.

## Install dependencies

    sudo apt update
    sudo apt install build-essential dkms \
        linux-headers-$(uname -r) pciutils

## Compile without installing

From the repository root:

    make -C linux modules

The resulting module should appear at:

    linux/quantis_pci.ko

Inspect its metadata:

    /usr/sbin/modinfo ./linux/quantis_pci.ko

The modified module declares GPL v2 licensing.

## DKMS installation

The included DKMS configuration uses:

- Package: quantis-pci
- Version: 18.03.08-linux61.3
- Module: quantis_pci

Check for an existing installation:

    sudo dkms status

Do not overwrite an existing DKMS source directory
without first reviewing and backing up that installation.

For a new installation, run from a clean source checkout.
If the mkdir command fails because the directory already
exists, stop and review the existing installation before
continuing.

Do not copy compiled build artifacts into the DKMS source
directory.

Run from the repository root:

    sudo mkdir /usr/src/quantis-pci-18.03.08-linux61.3

    git archive HEAD common linux dkms.conf | \
        sudo tar -xf - -C /usr/src/quantis-pci-18.03.08-linux61.3

    sudo dkms add -m quantis-pci -v 18.03.08-linux61.3

    sudo dkms build -m quantis-pci -v 18.03.08-linux61.3

    sudo dkms install -m quantis-pci -v 18.03.08-linux61.3

Load the installed driver:

    sudo modprobe quantis_pci

Verify detection:

    ls -l /dev/qrandom*
    sudo dmesg | grep -i quantis
    sudo dkms status

## Automatic loading

Automatic PCI device discovery depends on the system's
module-loading configuration.

If explicit boot-time loading is required:

    echo quantis_pci | sudo tee \
        /etc/modules-load.d/quantis-pci.conf

## Build the diagnostic utility

    gcc -O2 -Wall -Wextra \
        -Icommon tools/quantis-status.c \
        -o tools/quantis-status

Run the utility:

    sudo ./tools/quantis-status

Exit codes:

| Code | Meaning |
|---|---|
| 0 | All reported present modules report healthy |
| 1 | Device open or diagnostic IOCTL failure |
| 2 | Reported hardware module health failure |

The diagnostic utility currently targets /dev/qrandom0.

## Device permissions

Restrict access to the hardware RNG device.

For example, create a dedicated group:

    sudo groupadd --system quantis

Create /etc/udev/rules.d/99-quantis.rules containing:

    KERNEL=="qrandom[0-9]*", OWNER="root", GROUP="quantis", MODE="0640"

Reload and apply the rules:

    sudo udevadm control --reload-rules
    sudo udevadm trigger --sysname-match='qrandom*'

Verify the resulting permissions with:

    ls -l /dev/qrandom*

A reboot may be required if the device rules are not
applied as expected.

Add authorized service accounts to the group as appropriate.

Do not grant unrestricted device access unnecessarily.
Read-only open permissions do not inherently restrict
all possible IOCTL operations.

## Secure Boot and module signing

DKMS may sign the resulting module using a locally
generated signing key.

Secure Boot or kernel module signature enforcement may
require additional trust enrollment.

Consult your operating system documentation rather than
disabling security controls solely to load this module.

Never distribute private module signing keys.

## Removal

Stop all applications using the Quantis device first.

    sudo modprobe -r quantis_pci

    sudo dkms remove \
        -m quantis-pci \
        -v 18.03.08-linux61.3 \
        --all

Review and remove any project-specific module-loading
configuration, udev rules, and DKMS source files.

## Production upgrades

Test builds against the target kernel before upgrading
security-sensitive systems.

Keep a known-good driver version and a recovery path.
