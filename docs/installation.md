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

Use a conventional system installation umask for DKMS operations:

    sudo sh -c 'umask 022; exec dkms "$@"' sh \
        add -m quantis-pci -v 18.03.08-linux61.3

    sudo sh -c 'umask 022; exec dkms "$@"' sh \
        build -m quantis-pci -v 18.03.08-linux61.3

    sudo sh -c 'umask 022; exec dkms "$@"' sh \
        install -m quantis-pci -v 18.03.08-linux61.3

A restrictive inherited umask, such as 077, can cause depmod
to generate root-only module dependency and alias indexes.
The module may then be discoverable by root but not by
unprivileged users or services.

If this occurs, regenerate the indexes using:

    sudo sh -c 'umask 022; exec depmod -a "$1"' sh "$(uname -r)"

Verify module discovery without elevated privileges:

    /usr/sbin/modinfo -n quantis_pci

Inspect the generated module indexes:

    ls -l /lib/modules/$(uname -r)/modules.dep*
    ls -l /lib/modules/$(uname -r)/modules.alias*

On the tested Debian system, these indexes are normally
owned by root and have mode 0644.

This installation-specific umask does not require relaxing
the permissions of private backups, logs or signing keys.

Load the installed driver:

    sudo modprobe quantis_pci

Verify detection:

    ls -l /dev/qrandom*
    sudo dmesg | grep -i quantis
    sudo dkms status

## Automatic loading

Automatic PCI device discovery depends on the system's
module-loading configuration.

On the tested Debian installation, PCI modalias resolution
automatically loaded the installed module after both a normal
reboot and a cold power cycle. No explicit module-loading
configuration was required.

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

    sudo sh -c 'umask 022; exec dkms "$@"' sh \
        remove -m quantis-pci \
        -v 18.03.08-linux61.3 --all

Review and remove any project-specific module-loading
configuration, udev rules, and DKMS source files.

## Production upgrades

Test builds against the target kernel before upgrading
security-sensitive systems.

Keep a known-good driver version and a recovery path.

Before replacing an existing installation:

- Record the running kernel and current DKMS inventory.
- Preserve the previous source and kernel-specific module.
- Verify the integrity of the recovery artifacts.
- Stop applications before unloading an active module.
- Review existing module-signing and trust requirements.

A successful DKMS build does not establish that the module
has loaded or that the hardware is operational.

Verify the installed module, loaded driver identity,
device permissions, hardware diagnostics and acquisition
behavior after deployment.

For an additional installed kernel, use its explicit kernel
release with the DKMS build and install commands:

    sudo sh -c 'umask 022; exec dkms "$@"' sh \
        build -m quantis-pci -v 18.03.08-linux61.3 \
        -k TARGET_KERNEL

    sudo sh -c 'umask 022; exec dkms "$@"' sh \
        install -m quantis-pci -v 18.03.08-linux61.3 \
        -k TARGET_KERNEL

Replace TARGET_KERNEL with the actual target kernel release.
Do not load a module compiled for a different running kernel.

Verify module discovery for that kernel:

    /usr/sbin/modinfo -k TARGET_KERNEL -n quantis_pci

Once the replacement installation and independent recovery
artifacts have been verified, review and retire obsolete DKMS
registrations that could compete during future kernel updates.

Retain a recovery copy independently of the active DKMS tree.

After reboot, verify automatic module loading, hardware health,
permissions and dependent application startup.

The package enables DKMS AUTOINSTALL. Successful automatic
rebuilding during a future kernel upgrade remains dependent on
compatible kernel interfaces, headers and system configuration.
