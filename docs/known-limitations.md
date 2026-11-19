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

Runtime tests have validated up to 16 concurrent reader
processes and software PCI unbinding and rebinding with
outstanding file descriptors.

Removal raced against active MMIO acquisition, surprise physical
removal, prolonged stress testing and fault injection remain
unqualified.

Minor numbers are reserved until module unload. Repeated PCI
rebind operations consume the available device slots. Applications
must not assume that a rebound device retains its previous name.

Suspend and resume behavior is not qualified.

Do not remove PCI hardware physically from an energized
system unless the platform explicitly supports hot-plug.

## Security-sensitive deployments

Review the source, hardware documentation, operating
environment and application requirements before using
the driver for security-critical workloads.

## Acquisition timing and resource limits

Hardware acquisition deadlines do not include time spent waiting
for admission, waiting for the hardware semaphore or copying
acquired bytes into userspace memory.

The driver does not provide a hard real-time read deadline.

Private acquisition buffers prevent userspace memory faults from
holding the hardware semaphore. Processes stalled in userspace
memory copies can nevertheless retain admission slots and delay
additional readers.

## Hardware health limitations

Health checks use sampled hardware register state. They cannot
establish that every transient hardware failure is observed.

The diagnostic IOCTLs expose hardware-reported status and should
not be interpreted as independent hardware attestation.

Diagnostic register validity checks are not equivalent to the
stricter checks performed during acquisition. Some invalid
register states can produce misleading diagnostic results.

FIFO error-bit semantics remain unresolved. Additional fault
injection and recovery testing is required.

## Performance

Acquisition remains polling-based and uses individual MMIO FIFO
register reads. Sustained acquisition can consume a substantial
fraction of one CPU.

The observed linux61.3 benchmark maintained rated throughput
while reducing CPU utilization from approximately 99.1 percent
to 90.3 percent of one logical CPU.

The original engineering target of 85 percent was not achieved.

Small read requests incur additional overhead from the stronger
health checks and private buffer management.

## Validation scope

Runtime qualification of linux61.3 has focused on one Quantis
PCI 16 Mbps card under Linux 6.1.0-54-amd64.

Results do not establish compatibility with every Quantis model,
kernel series, architecture or operating environment.
