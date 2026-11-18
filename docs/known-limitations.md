# Known limitations

## Experimental driver

This is a community-maintained modernization of a
legacy hardware driver.

It has not undergone a formal security audit.

## Kernel compatibility

Testing has focused on Linux 6.1.

Compatibility with newer or older kernel series must
be established through compilation and hardware testing.

## Hardware health

Hardware health information originates from the
device's own reported status.

The driver does not independently verify the physical
quantum process or establish an entropy guarantee.

The diagnostic utility considers detected modules
healthy when their reported status bits are set.
It does not enforce a model-specific expected module count.

The AIS31 startup test request flag does not establish
successful completion or certification.

## FIFO error interpretation

Additional FIFO error enforcement was investigated
during development but produced unexpected read failures
with otherwise functioning hardware.

The relevant register semantics require further investigation.

## Entropy integration

This driver exposes a character device.

It does not currently register as a Linux hwrng provider
or automatically feed the kernel entropy pool.

Any entropy-pool integration requires separate,
carefully evaluated configuration or implementation.

## Statistical assurance

Statistical tests can detect some anomalous output
patterns but cannot prove randomness, independence,
cryptographic unpredictability, or quantum origin.

## Device lifecycle

Additional concurrent access, hot-removal and fault
injection testing would improve confidence in lifecycle
and failure-handling behavior.

Do not remove PCI hardware physically from an energized
system unless the platform explicitly supports hot-plug.

## Security-sensitive deployments

Review the source, hardware documentation, operating
environment and application requirements before using
the driver for security-critical workloads.
