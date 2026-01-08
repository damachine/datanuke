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
- `crypto_encrypt_file()` - AES-256-CBC file encryption (EVP API)
- `crypto_encrypt_device()` - AES-256-CBC block device encryption
- `crypto_display_key()` - Display key once (POSIX-style plain text)
- `crypto_secure_wipe_key()` - 7-pass Gutmann key wipe
- `crypto_cleanup()` - Free OpenSSL context

### main.c

- Parse args (including --help/-h/help flags)
- Check if target is file or block device
- Encrypt file/device with AES-256-CBC
- Display key once (plain text, no countdown)
- Wipe key from memory (7-pass Gutmann)
- Output: POSIX-compliant, no ANSI colors

### platform.c

- `platform_lock_memory()` - mlock (Unix) / VirtualLock (Windows)
- `platform_unlock_memory()` - munlock / VirtualUnlock

## Key Security

**Key Lifecycle (Encrypt-then-Delete-Key Method):**
1. Generated with `RAND_bytes()` (CSPRNG)
2. Locked in RAM with `mlock()` (no swap)
3. Used for encryption (file or device)
4. Displayed once (plain text, save now or lose forever)
5. 3-second pause for user to save key
6. Wiped with 7-pass Gutmann
7. Memory unlocked

Without the key, encrypted data is permanently irrecoverable (BSI method).

**Gutmann Wipe:**
- Pass 1: 0x00
- Pass 2: 0xFF
- Pass 3: random
- Pass 4: 0x00
- Passes 5-7: volatile pointer overwrite

## POSIX-Style Output

**Design Principles:**
- No ANSI escape codes (no colors)
- No Unicode box-drawing characters
- No emojis or fancy formatting
- Plain text output only
- Compatible with all POSIX terminals

**Key Display:**
```c
printf("---\n");
printf("ENCRYPTION KEY - SAVE NOW OR LOSE FOREVER\n\n");
printf("Key: %s\n", key_hex);
printf("IV:  %s\n\n", iv_hex);
printf("Key is stored in RAM only and will be wiped immediately.\n");
printf("Write it down now if you need to decrypt later.\n");
printf("---\n");
sleep(3);  // Silent pause, no countdown
```

## Device Support

**Block Device Encryption:**
- Detects devices with `platform_is_device()`
- Gets device size with `platform_get_device_size()`
- Requires "YES" confirmation before encryption
- Cannot encrypt mounted devices
- Cannot encrypt device with running OS

**Supported Devices:**
- `/dev/sdb`, `/dev/sdc`, etc. (entire drives)
- `/dev/sdb1`, `/dev/sdb2`, etc. (partitions)
- `/dev/nvme0n1`, `/dev/nvme0n1p1` (NVMe drives)

## Contributing

1. Fork repo
2. Create feature branch
3. Make changes
4. Test with: `./build/datanuke test.txt`
5. Test device mode (requires root): `sudo ./build/datanuke /dev/loop0`
6. Submit PR

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
