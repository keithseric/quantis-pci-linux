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
