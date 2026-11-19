# Modernization changelog

## 18.03.08-linux61.3 - Security and runtime hardening

### Security

- Require writable file descriptors and CAP_SYS_RAWIO for
  administrative IOCTL operations.
- Require readable descriptors for diagnostic IOCTL operations.
- Reject invalid module-selection masks and unknown IOCTLs.
- Move userspace memory access outside the hardware semaphore.
- Use bounded, memory-accounted private acquisition buffers.
- Explicitly erase acquisition buffers before releasing them.
- Strengthen hardware health checks throughout data acquisition.
- Separate device publication and hardware removal locking.

### Driver behavior

- Add native 32-bit IOCTL compatibility on supported kernels.
- Replace legacy busy polling with bounded, sleepable FIFO polling.
- Propagate meaningful acquisition errors to userspace.
- Implement nonblocking acquisition behavior.
- Bound individual read requests to 64 KiB.
- Bound nonblocking acquisitions to one FIFO batch.
- Preserve the historical PCI bus-number IOCTL semantics.
- Improve synchronization between acquisition and device removal.

### Runtime qualification

- Maintain approximately 16 Mbps sustained hardware throughput.
- Reduce sustained acquisition CPU consumption by approximately
  8.9 percent in the observed comparative benchmark.
- Validate up to 16 independent concurrent reader processes.
- Complete native and 32-bit IOCTL security tests.
- Complete 1,000 repeated open/read/close cycles.
- Validate PCI unbinding and rebinding with outstanding descriptors.
- Confirm disconnected descriptors cannot access a rebound device.
- Complete repeated restoration of the previous production driver.

### Production deployment

- Compare the qualified candidate with the packaged DKMS build.
- Permanently install and validate the hardened production module.
- Repeat native security regression tests after installation.
- Build and install the package for two Linux 6.1 kernel versions.
- Retire the obsolete development DKMS registration.
- Validate automatic driver loading after normal reboot.
- Validate hardware initialization after a cold power cycle.
- Confirm automatic recovery of dependent userspace services.

### Packaging

- Increment the DKMS package version to 18.03.08-linux61.3.
- Update installation, architecture, security and validation
  documentation.
- Preserve upstream attribution and licensing.

The driver ABI version remains 2.9 (integer 29).
The package version identifies this Linux modernization release.

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
