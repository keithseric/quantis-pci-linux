# Hardware validation

## Historical operational validation (before linux61.3)

The development driver has undergone the following
tests on physical hardware:

- Kernel module compilation
- DKMS installation
- PCI device detection
- Character device creation
- Device permission configuration
- Hardware byte acquisition
- Hardware diagnostic IOCTL queries
- Reboot and automatic service recovery
- Hardware throughput measurement
- Integration with a userspace random-generation service

## Example throughput measurement

An observed one-MiB hardware read completed in
approximately 0.523 seconds.

Measured throughput was approximately 16.04 Mbps,
consistent with the rated 16 Mbps hardware throughput.

Timing includes userspace acquisition overhead and
should not be interpreted as a precise hardware benchmark.

## Example statistical measurements

A one-MiB hardware sample produced:

| Measurement | Result |
|---|---|
| Sample bytes | 1048576 |
| One-bit percentage | 49.993026 |
| Zero-bit percentage | 50.006974 |
| Empirical Shannon entropy | 7.99983182 bits/byte |
| Byte chi-square | 244.489746 |
| Chi-square degrees of freedom | 255 |
| Distinct byte values | 256 |
| Longest identical bit run | 23 |
| Byte serial correlation | -0.00221458 |

These measurements were collected using a custom
userspace statistical screening implementation.

They are observational results, not a cryptographic
certification or an independent entropy assessment.

The statistical implementation still requires comparison
against reference statistical testing software.

## Initial publication validation scope (linux61.2)

The production development source and publication
source have been compared, with subsequent differences
in licensing declarations and publication metadata.

The publication source passed an isolated compilation
test on 2026-11-17.

## Initial publication compilation validation (linux61.2)

| Property | Result |
|---|---|
| Kernel | 6.1.0-54-amd64 |
| Architecture | x86-64 |
| GCC | 12.2.0 |
| GNU Make | 4.3 |
| Kernel module compilation | Pass |
| Module license declaration | GPL v2 |
| PCI device alias | 179a:0001 |
| Diagnostic utility compilation | Pass with -Werror |
| Diagnostic utility hardware queries | Pass |
| Reported healthy modules | 4 of 4 |

The diagnostic utility compiled from the publication
repository successfully queried the installed production
driver and physical Quantis hardware.

The publication kernel module itself was compiled but
was not installed or loaded during this test.

The production development driver and all dependent
services remained operational.

The publication package version is distinct from the
locally deployed development DKMS package.

Installation and automatic DKMS rebuilds of the
publication package remain to be validated separately.

## Outstanding validation after initial publication

- Broader kernel compatibility
- Additional Quantis hardware models
- Multiple-card operation
- Comprehensive concurrency stress testing
- Hardware removal and lifecycle testing
- Fault injection
- Independent source review
- Formal statistical and entropy evaluation

## linux61.3 runtime qualification

Qualification was performed on 2026-11-19 using a physical
ID Quantique Quantis PCI 16 Mbps device.

### Test environment

| Property | Value |
|---|---|
| Distribution | Debian 12 |
| Kernel | 6.1.0-54-amd64 |
| Architecture | x86-64 |
| Logical CPUs | 2 |
| PCI vendor/device | 179a:0001 |
| PCI revision | 0x50 |
| Board core version | 0x040a1201 |
| RNG modules present | 4 of 4 |
| RNG modules reported healthy | 4 of 4 |
| Qualified driver source commit | 516616feb0992a952a60e8f29968353c6a7a9e12 |
| Driver ABI version | 29 |

### Performance comparison

The earlier production Linux modernization package
(18.03.08-linux61.1) was compared against the hardened
linux61.3 candidate.

| Metric | Earlier driver | Hardened candidate |
|---|---:|---:|
| Sustained throughput | 16.0016 Mbps | 16.0002 Mbps |
| Sustained CPU utilization | 99.078% | 90.301% |
| Concurrent thread throughput | 15.764 Mbps | 15.978 Mbps |
| 1-byte median acquisition latency | 7.014 us | 14.967 us |
| 32-byte median acquisition latency | 19.315 us | 27.286 us |
| 1 KiB median acquisition latency | 511.265 us | 465.507 us |
| 4 KiB median acquisition latency | 2047.423 us | 2060.802 us |
| 64 KiB median acquisition latency | 32.766 ms | 32.752 ms |
| Unexpected acquisition errors | 0 | 0 |
| Unexpected short reads | 0 | 0 |

CPU utilization is expressed relative to one logical CPU.

The measured sustained CPU reduction was approximately 8.86
percent. The original engineering target of no more than 85
percent of one logical CPU was not achieved.

The earlier driver was measured with application services
running. The candidate was measured with those services stopped.
These differing background workloads limit the strength of
comparative performance conclusions.

### Independent reader processes

Each process acquired 1 MiB using sixteen 64 KiB reads.

| Processes | Aggregate Mbps | Aggregate CPU | Representative median latency | Maximum latency |
|---:|---:|---:|---:|---:|
| 1 | 15.9300 | 90.00% | 32.75 ms | 32.82 ms |
| 4 | 15.9607 | 90.83% | 131.03 ms | 132.61 ms |
| 8 | 15.9708 | 93.25% | 262.14 ms | 265.95 ms |
| 12 | 15.9901 | 92.09% | 393.22 ms | 395.20 ms |
| 16 | 15.9894 | 91.80% | 524.28 ms | 527.22 ms |

All processes completed their acquisitions. No unexpected short
reads, acquisition errors or worker timeouts were observed.

The combined process workload acquired 41 MiB through 656
blocking read calls.

An additional 1,000 open/read/close cycles completed successfully.
The final module reference count was zero.

### Native IOCTL security

| Test | Expected behavior | Result |
|---|---|---|
| Administrative request on read-only descriptor | EBADF | Pass |
| Administrative request on unprivileged writable descriptor | EPERM | Pass |
| Invalid module-selection mask | EINVAL | Pass |
| Invalid userspace input pointer | EFAULT | Pass |
| Invalid userspace output pointer | EFAULT | Pass |
| Unknown command | ENOTTY | Pass |
| Unprivileged diagnostic access | Success | Pass |
| Oversized blocking acquisition | 64 KiB maximum | Pass |
| Zero-length read | Zero bytes | Pass |

Administrative authorization was tested for reset, module enable,
module disable and AIS31 request-flag modification.

The unprivileged writable-descriptor test used an inherited
descriptor without changing device-node permissions.

### Nonblocking acquisition

A sequence of 1,000 nonblocking read attempts produced:

- 836 successful acquisitions.
- 164 EAGAIN responses.
- Zero unexpected errors.

Successful acquisitions did not exceed one 4096-byte FIFO batch.

### Native 32-bit compatibility

A statically linked ELF32 i386 executable invoked Linux system
calls directly, without a 32-bit C library.

All ten IOCTL assertions passed:

- Seven diagnostic queries.
- Invalid output pointer handling.
- Unknown command handling.
- Read-only administrative authorization.

### PCI device lifecycle

Software PCI unbinding was performed while five descriptors
remained open, including one nonblocking descriptor.

The original character device was removed successfully.
All old descriptors returned ENODEV for subsequent reads and
diagnostic queries.

PCI rebinding registered a new device instance with a new minor
number. Old descriptors remained disconnected.

The new device passed diagnostic queries, blocking reads,
device permission checks and unprivileged acquisition tests.

All outstanding descriptors were released, with the final
module reference count returning to zero.

No kernel faults were observed in the collected qualification
kernel logs.

### Production restoration

The earlier production driver was restored following each
controlled qualification session.

Five production restoration sessions completed successfully
during controlled candidate qualification.

The installed legacy production DKMS module remained unchanged
during those sessions, before permanent deployment of linux61.3.

### Permanent DKMS deployment

The hardened source implementation was packaged as
18.03.08-linux61.3 and built using DKMS 3.0.10.

Both Linux 6.1.0-54-amd64 and Linux 6.1.0-53-amd64 builds
completed successfully.

The runtime-qualified candidate and the production DKMS build
for Linux 6.1.0-54-amd64 were compared before deployment.

The comparison found matching contents in all 22 compared
allocated ELF sections, excluding the build-ID note.
All 401 compared relocation records matched.

The comparison supports equivalence of the examined binary
components. It is not a formal proof of behavioral equivalence
or complete binary identity.

An initial deployment encountered module discovery failures
after restrictive inherited umask settings caused depmod to
generate root-only module indexes.

The legacy production module was recovered using an independently
verified recovery artifact. Module indexes were regenerated with
umask 022, restoring normal module discovery.

The corrected deployment completed successfully.

| Validation | Result |
|---|---|
| Production DKMS installation | Pass |
| Installed module checksum | Pass |
| Loaded module identity | Pass |
| Unprivileged module discovery | Pass |
| PCI modalias resolution | Pass |
| Fallback kernel installation | Pass |
| Legacy DKMS registration retirement | Pass |

The independently verified legacy recovery module and archived
source were retained outside the active DKMS registration.

### Installed production security regression

The native security qualification was repeated against the
permanently installed hardened module.

Administrative authorization, invalid mask handling, invalid
userspace pointers, unsupported IOCTL handling and bounded
acquisition behavior passed the regression checks.

An additional 1,000 nonblocking acquisition attempts produced:

- 835 successful acquisitions.
- 165 EAGAIN responses.
- Zero unexpected errors.

Hardware health remained operational and the final module
reference count was zero.

### Production application integration

Dependent userspace services successfully operated with the
permanently installed driver.

Integration testing included hardware diagnostics, random
byte acquisition, application input validation and periodic
hardware health and statistical screening.

All 46 functional and input-validation assertions passed
after correction of a test-harness response-field error.
Additional emoji acquisition tests also passed.

Network-facing service integration was verified separately
from the driver. The driver itself does not provide network
services.

Private application implementation and infrastructure
configuration are outside the scope of this repository.

### Normal reboot and cold boot

On 2026-11-19 the production installation completed both a
normal reboot and a cold power cycle.

The cold power cycle was performed by the maintainer.
The subsequent operating state was verified through a recorded
qualification procedure.

| Validation | Normal reboot | Cold boot |
|---|---|---|
| Correct production kernel | Pass | Pass |
| Installed module integrity | Pass | Pass |
| Automatically loaded module identity | Pass | Pass |
| PCI driver binding | Pass | Pass |
| Character device permissions | Pass | Pass |
| Reported healthy modules | 4 of 4 | 4 of 4 |
| Automatic service startup | Pass | Pass |
| Fresh statistical assurance | Pass | Pass |
| Failed systemd units | 0 | 0 |
| Matching kernel fault indicators | None | None |

No manual driver loading or dependent service startup was
required during either boot qualification.

The expected out-of-tree and module-signature trust warnings
were present. Secure Boot was disabled on the tested system.

### Statistical screening after deployment

The installed production driver supplied three separate
one-MiB samples for periodic statistical screening.

| Measurement | Deployment | Normal reboot | Cold boot |
|---|---:|---:|---:|
| Acquisition throughput, Mbps | 16.0384 | 16.0381 | 16.0260 |
| Empirical Shannon entropy, bits/byte | 7.99980171 | 7.99982972 | 7.99983213 |
| Monobit p-value | 0.136907 | 0.653044 | 0.749706 |
| Byte chi-square p-value | 0.073383 | 0.617482 | 0.676758 |
| Runs p-value | 0.628392 | 0.244635 | 0.477335 |

All three samples passed the configured statistical screening
threshold of 0.001.

These observations are not independent entropy evaluation,
cryptographic certification or proof of randomness.

### Remaining qualification

- Runtime testing of the installed fallback kernel.
- Automatic DKMS rebuilding across kernel updates.
- Enforced module-signature verification and Secure Boot.
- Broader kernel and hardware compatibility.
- Multiple-card operation.
- Removal raced against active acquisition.
- Physical surprise removal.
- Hardware fault injection.
- Long-duration resource and memory testing.
- Independent security and entropy assessment.

Acquisition success, reported health and statistical measurements
do not establish cryptographic certification or guaranteed entropy.
