# DataNuke Roadmap

## Version 1.0 (Current - Released)
- [x] AES-256-CBC encryption
- [x] OpenSSL integration
- [x] 7-pass Gutmann key wiping
- [x] Cross-platform support (Linux, macOS, Windows)
- [x] BSI-compliant secure deletion (Encrypt-then-Delete-Key)
- [x] Memory locking (mlock)
- [x] One-time key display
- [x] CMake build system
- [x] POSIX-style output (no colors, no Unicode)
- [x] Block device encryption support
- [x] Help flags (--help, -h, help)
- [x] Makefile for easy installation

## Version 1.1 (Planned - Next Release)
- [ ] Unit test suite
- [ ] Batch file processing
- [ ] Man page documentation
- [ ] Debian/RPM packaging
- [ ] Improved error handling for edge cases

## Version 2.0 (Future)
- [ ] Progress indicators for large files (optional)
- [ ] Verification mode (check if file was properly encrypted)
- [ ] Configuration file support (optional features only)

## Long-term Considerations

### Core Principles
- **Keep it simple**: POSIX-compliant, minimal dependencies
- **Security first**: Focus on correct implementation of BSI method
- **No bloat**: Reject features that don't serve core mission

### Performance
- Multi-threaded processing for large devices (optional)
- Direct I/O for better device performance

### Platform Extensions
- FreeBSD/OpenBSD support
- Verification of successful encryption

### Advanced Security
- Post-quantum cryptography readiness (when standardized)
- Formal verification of key wiping routines

## Note on Removed Features

Previous roadmap versions mentioned:
- GUI frontend (Qt/GTK)
- Cloud storage integration
- TPM 2.0 support
- Blockchain audit trails
- Common Criteria certification
- Zero-knowledge proofs
- HSM integration
- Enterprise features

These features have been **removed from the roadmap** as they add complexity without clear benefits for the core use case of secure data deletion. DataNuke follows the Unix philosophy: **do one thing and do it well**.

The tool focuses on being a simple, reliable, POSIX-compliant tool for securely deleting data through the BSI-recommended "Encrypt-then-Delete-Key" method.
