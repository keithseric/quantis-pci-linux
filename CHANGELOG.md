# Modernization changelog

## 18.03.08-linux61.2 - Initial publication candidate

Based on ID Quantique Quantis v18.03.08.

### Driver changes

- Ported Linux kernel interfaces to Linux 6.1.
- Updated PCI resource and MMIO handling.
- Modernized device lifecycle and slot management.
- Updated character device management.
- Added stricter RNG module health enforcement.

### Packaging and utilities

- Added DKMS configuration.
- Assigned a distinct publication DKMS package version.
- Passed isolated compilation against Linux 6.1.
- Validated the diagnostic utility with physical hardware.
- Added read-only hardware diagnostic utility.
- Removed generated build artifacts.
- Excluded obsolete Big Kernel Lock source.
- Limited the publication repository to Linux support.

### Documentation

- Added installation instructions.
- Added hardware compatibility information.
- Documented hardware validation and limitations.
- Preserved upstream attribution and source provenance.
- Licensed project modifications under GPL-2.0-only.

The original upstream changelog is preserved in
Changelog.txt.
