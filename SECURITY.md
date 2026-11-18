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
