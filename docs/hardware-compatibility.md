# Hardware compatibility

## Verified hardware

| Property | Value |
|---|---|
| Manufacturer | ID Quantique |
| Product | Quantis PCI 16 Mbps |
| PCI vendor ID | 179a |
| PCI device ID | 0001 |
| PCI revision | 50 |
| Board core version | 0x040a1201 |
| Reported RNG modules | 4 |
| Presence mask | 0x0f |
| Healthy mask | 0x0f |
| Rated throughput | 16 Mbps |

The tested card uses a PCI-to-PCIe bridge.

## Verified host environment

- Intel x86-64 platform
- Debian GNU/Linux 12
- Linux 6.1.0-54-amd64
- GCC 12 series
- DKMS installation

## Other hardware

Compatibility with other Quantis variants, board revisions,
PCIe implementations, multiple-card installations and
other architectures has not been established.

Do not interpret the presence of a supported PCI ID as
evidence of complete hardware compatibility.

## Reporting compatibility

Useful compatibility reports include:

    uname -r
    uname -m
    lspci -nn -d 179a:
    dkms status
    sudo ./tools/quantis-status

Remove private hostnames, usernames, filesystem paths,
network addresses and unrelated device information
before posting diagnostic output.

## linux61.3 runtime qualification

| Property | Tested configuration |
|---|---|
| Hardware | ID Quantique Quantis PCI 16 Mbps |
| PCI vendor/device ID | 179a:0001 |
| PCI revision | 0x50 |
| Board core version | 0x040a1201 |
| Detected RNG modules | 4 |
| Host architecture | x86-64 |
| Kernel | Linux 6.1.0-54-amd64 |
| Distribution | Debian 12 |
| Native userspace | 64-bit |
| Compatible userspace | 32-bit i386 |
| IA32 compatibility IOCTL tests | Pass |
| Software PCI unbind/rebind | Pass |

This table records a tested configuration, not a comprehensive
hardware compatibility matrix.

## Kernel qualification matrix

| Kernel | DKMS build | Installation | Runtime | Normal reboot | Cold boot |
|---|---|---|---|---|---|
| 6.1.0-54-amd64 | Pass | Pass | Pass | Pass | Pass |
| 6.1.0-53-amd64 | Pass | Pass | Not tested | Not tested | Not tested |

The fallback kernel module passed checksum, module metadata,
dependency index and PCI modalias verification. Installation
qualification must not be interpreted as runtime qualification.

Other Quantis models and multiple-card operation require
additional hardware validation.
