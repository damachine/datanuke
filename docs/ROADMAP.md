# DataNuke Roadmap

## Version 1.0 (Current - Released)
- [x] AES-256-CBC encryption
- [x] OpenSSL integration
- [x] 7-pass Gutmann key wiping
- [x] Cross-platform support (Linux, macOS, Windows)
- [x] BSI-compliant secure deletion
- [x] Memory locking (mlock)
- [x] One-time key display
- [x] CMake build system

## Version 1.1 (Planned - Next Release)
- [ ] Unit test suite
- [ ] Batch file processing
- [ ] Improved error messages
- [ ] Logging to file option
- [ ] Man page documentation
- [ ] Debian/RPM packaging

## Version 2.0 (Future)
- [ ] GUI frontend (Qt or GTK)
- [ ] Configuration file support
- [ ] Progress indicators for large files
- [ ] Verification mode (check if file was properly encrypted)
- [ ] Resume capability for interrupted operations

## Long-term Considerations

### Performance
- SIMD optimizations for key wiping
- Multi-threaded processing for large files
- GPU acceleration for encryption (if beneficial)

### Platform Extensions
- Android/iOS support (mobile platforms)
- Container/VM secure deletion
- Cloud storage integration (AWS S3, Azure Blob, etc.)

### Advanced Security
- Post-quantum cryptography readiness
- Hardware security module (HSM) integration
- Formal verification of key wiping routines

### Enterprise Features
- Audit logging and compliance reports
- FIPS 140-2 compliance mode
- Integration with enterprise key management systems

## Note on Removed Features

Previous roadmap versions mentioned:
- TPM 2.0 support
- Blockchain audit trails
- Common Criteria certification
- Zero-knowledge proofs

These features have been **removed from the roadmap** as they add complexity without clear benefits for the core use case of secure data deletion. DataNuke focuses on being a simple, reliable, and BSI-compliant tool for securely deleting data through encryption.
