# Security policy

## Supported versions

CPPToolkit is under active development. Security fixes target the latest
published release or prerelease and the `development` branch. Older versions
do not receive guaranteed backports; upgrade to the latest version containing
the fix.

Prereleases are intended for evaluation and do not imply production readiness.
If no release is currently available, use the latest `development` revision.

## Reporting a vulnerability

**Do not disclose vulnerabilities, exploit details, credentials or private
data in public issues, pull requests or discussions.**

Use GitHub's private vulnerability reporting for this repository:

https://github.com/f-barbato/CPPToolkit/security/advisories/new

This channel requires the repository maintainer to enable private vulnerability
reporting in GitHub's security settings. If it is unavailable, open an issue
requesting a private reporting channel **without including vulnerability
details**, and wait for the maintainer to arrange confidential communication.

Include the following in your private report:

- Affected release/tag or commit, modules and vcpkg features.
- Operating system, architecture, compiler and static/shared build configuration.
- Description of the issue, expected behavior and potential impact.
- Minimal reproduction steps or a proof of concept, using synthetic data.
- Relevant logs, stack traces or code locations, with secrets removed.
- Any known workaround and whether a third-party dependency is involved.

Only test systems and data you own or have explicit permission to assess.
Keep reproduction steps limited to what is necessary to demonstrate the issue.

## Handling and disclosure

The maintainer will assess reports based on impact, reproducibility and affected
versions, and may request additional information through the private channel.
Response and remediation times depend on maintainer availability; this project
does not provide a security response SLA.

Confirmed reports should be handled privately while a fix or mitigation is
prepared. Coordinate public disclosure with the maintainer so users can obtain
the fix before technical details are published. Reporter credit will be
included only with the reporter's consent.

Where appropriate, the maintainer will publish a GitHub Security Advisory with
affected versions, fixed versions and mitigations, and request a CVE through
GitHub. Release notes belong in the current `CHANGELOG.md` and their matching
entry in `HISTORY.md`, without prematurely disclosing sensitive details.

## Third-party dependencies

CPPToolkit uses dependencies such as raylib, Dear ImGui, ImPlot, rlImGui and,
when BLE is enabled, SimpleBLE. For vulnerabilities originating in those
projects, follow their security policies and notify CPPToolkit privately when
its builds or distributed archives are affected.

Dependency updates and rebuilt binary archives may be necessary even when
CPPToolkit source code is unchanged. Pinning a dependency version or verifying
an archive checksum does not guarantee that the dependency is vulnerability-free.

## Maintainer checklist

- Enable **Private vulnerability reporting** in the repository's GitHub security
  settings; adding this file does not enable that feature automatically.
- Triage privately and identify affected modules, versions and distributed
  OS/architecture packages.
- Add a regression test and verify the fix in relevant build configurations.
- Review dependency updates and rebuild affected release artifacts.
- Publish coordinated advisory/release notes and notify the reporter through
  the private channel.
