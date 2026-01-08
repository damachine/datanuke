# DataNuke Developer Guide

Technical documentation for contributors.

## Architecture

```
main.c  → Entry point, CLI handling
crypto.c → AES-256-CBC encryption, key generation, key wiping
platform.c → Memory locking (mlock/VirtualLock)
```

## Project Structure

```
src/
├── main.c       # CLI + workflow
├── crypto.c     # Encryption + key management
└── platform.c   # OS-specific memory operations

include/
└── datanuke.h   # Public API
```

## Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

## Core Functions

### crypto.c

- `crypto_init()` - Initialize context, generate random key/IV
- `crypto_encrypt_file()` - AES-256-CBC encryption (EVP API)
- `crypto_display_key()` - TUI key display with 3-second countdown
- `crypto_secure_wipe_key()` - 7-pass Gutmann key wipe
- `crypto_cleanup()` - Free OpenSSL context

### main.c

- Parse args → encrypt file → display key → wipe key → done

### platform.c

- `platform_lock_memory()` - mlock (Unix) / VirtualLock (Windows)
- `platform_unlock_memory()` - munlock / VirtualUnlock

## Key Security

**Key Lifecycle:**
1. Generated with `RAND_bytes()` (CSPRNG)
2. Locked in RAM with `mlock()` (no swap)
3. Used for encryption
4. Displayed for 3 seconds (TUI)
5. Wiped with 7-pass Gutmann
6. Memory unlocked

**Gutmann Wipe:**
- Pass 1: 0x00
- Pass 2: 0xFF
- Pass 3: random
- Pass 4: 0x00
- Passes 5-7: volatile pointer overwrite

## TUI Implementation

**ANSI Codes:**
- Bold: `\033[1m`
- Cyan: `\033[1;36m`
- Yellow: `\033[1;33m`
- Reverse: `\033[7m`
- Reset: `\033[0m`

**Countdown:**
```c
for (int i = 3; i > 0; i--) {
    printf("\rWiping key in %d...", i);
    fflush(stdout);
    sleep(1);
}
```

## Contributing

1. Fork repo
2. Create feature branch
3. Make changes
4. Test with: `./build/datanuke test.txt`
5. Submit PR

## Testing

```bash
# Manual test
echo "test" > test.txt
sudo ./build/datanuke test.txt
hexdump -C test.txt  # Should be encrypted

# Decrypt with saved key
openssl enc -d -aes-256-cbc -K <key> -iv <iv> -in test.txt -out recovered.txt
```

## Code Style

- C11 standard
- Linux kernel style (indent -linux)
- Doxygen comments for functions
- Max 80 chars per line

## Security Considerations

- Never log keys
- Use volatile pointers for sensitive data
- Always check OpenSSL return values
- Lock keys in memory (mlock)
- Wipe keys before free

## References

- [OpenSSL EVP API](https://www.openssl.org/docs/man3.0/man7/evp.html)
- [BSI CON.6](https://www.bsi.bund.de/)
- [NIST AES](https://csrc.nist.gov/publications/detail/fips/197/final)

---

For issues: [GitHub Issues](https://github.com/damachine/datanuke/issues)
