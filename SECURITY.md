# Security policy

## Supported versions

This project is experimental.

Security maintenance is focused on the current main
development branch. No guaranteed support period or
response time is offered.

## Reporting vulnerabilities

For potentially sensitive security vulnerabilities,
contact the maintainer privately:

    seric@seric.com

Use a subject identifying the Quantis PCI Linux driver.

Please provide:

- Affected source revision
- Kernel version and architecture
- Relevant hardware identifiers
- Description of the issue
- Reproduction steps
- Potential security impact

Do not include private signing keys, credentials,
sensitive random output or unrelated system information.

Please avoid publishing exploit details before the
maintainer has had a reasonable opportunity to review
the report.

## Security considerations

This driver executes in kernel space and should be
treated as privileged software.

Restrict access to its character devices.

Review IOCTL access when deploying the driver in
multi-user or publicly accessible environments.

Hardware diagnostics do not establish cryptographic
certification or guarantee entropy quality.

Applications should handle hardware access failures
explicitly rather than silently substituting a different
randomness source.

## Security controls introduced in linux61.3

Administrative IOCTL operations require a writable descriptor
and CAP_SYS_RAWIO. Diagnostic IOCTL operations require a readable
descriptor.

File ownership and permissions remain an important security
boundary. Administrative authorization is additionally enforced
when a writable descriptor is inherited or transferred to an
unprivileged process.

Acquisition uses bounded private buffers and separates
userspace memory access from serialized hardware access.
Acquisition buffers are explicitly erased before release.

The driver performs repeated hardware health checks and does not
substitute software-generated random data on acquisition failure.

These controls have undergone targeted runtime qualification.
They do not constitute a formal security audit, independent
entropy evaluation or AIS31 certification.

See [known limitations](docs/known-limitations.md) for the
distinction between diagnostic health reporting and acquisition-time
health enforcement.

## Module integrity and trust

DKMS signing and kernel acceptance are separate properties.

A signed module is not necessarily trusted by the running kernel.
Verify the signing and trust configuration required by the
deployment environment.

Do not distribute private module-signing keys or disable
signature enforcement merely to load this driver.

Retain independently verified recovery artifacts when replacing
an installed driver. Restrict access to deployment logs, private
configuration and backup archives.
