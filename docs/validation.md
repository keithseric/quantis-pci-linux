# Hardware validation

## Completed operational validation

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

## Validation scope

The production development source and publication
source have been compared, with subsequent differences
in licensing declarations and publication metadata.

The publication source passed an isolated compilation
test on 2026-11-17.

## Publication compilation validation

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

## Outstanding validation

- Broader kernel compatibility
- Additional Quantis hardware models
- Multiple-card operation
- Comprehensive concurrency stress testing
- Hardware removal and lifecycle testing
- Fault injection
- Independent source review
- Formal statistical and entropy evaluation
