# ETDK Developer Guide

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
└── etdk.h   # Public API
```

## Build

### Quick Build (Debug with full symbols)
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . -j4
```

### Build with GDB Debugging
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS="-g -O0 -Wall -Wextra -Wpedantic"
cmake --build . -j4
```

### Run with GDB
```bash
gdb ./etdk
(gdb) run test.txt
(gdb) break crypto_encrypt_file
(gdb) continue
```

### Build with Valgrind Memory Analysis
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . -j4
valgrind --leak-check=full --show-leak-kinds=all ./etdk test.txt
```

### Build with Address Sanitizer (Memory errors)
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS="-g -O1 -fsanitize=address -fno-omit-frame-pointer"
cmake --build . -j4
./etdk test.txt  # Will report memory issues
```

### Build with Code Coverage
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS="-g -O0 --coverage"
cmake --build . -j4
./etdk test.txt
gcov src/crypto.c src/platform.c src/main.c
```

### Compile Commands for IDE (VS Code, CLion, etc.)
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
# Creates compile_commands.json for IDE code analysis
```

### Testing
```bash
# Run automated test script
bash ../test_etdk.sh

# Manual encryption test
echo "test secret data" > test.txt
./etdk test.txt
hexdump -C test.txt  # Verify it's encrypted
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
4. Test with: `bash test_etdk.sh`
5. Run Codacy analysis (see below)
6. Test device mode (requires root): `sudo ./build/etdk /dev/loop0`
7. Submit PR

## Pre-Commit Checks

Before pushing your changes, run these quality checks:

### Automated Test Script
```bash
bash test_etdk.sh
# Runs encryption test with hexdump comparison
```

### Code Quality Analysis (Codacy)
```bash
# Install Codacy CLI (optional but recommended)
npm install -g @codacy/codacy-cli

# Run local analysis
codacy-cli analyze --tool eslint
```

### Memory Safety (Valgrind)
```bash
cd build
valgrind --leak-check=full --show-leak-kinds=all ./etdk ../test_file.txt
# Check for memory leaks before pushing
```

### Compiler Warnings
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS="-Wall -Wextra -Wpedantic -Werror"
cmake --build .
# All warnings must be fixed (treated as errors)
```

## Performance Profiling

### Profile with Perf (Linux)
```bash
cd build
perf record -g ./etdk large_file.iso
perf report
# Analyze performance hotspots
```

### Benchmark File Encryption
```bash
cd build
# Create test file
dd if=/dev/urandom of=test_1gb.bin bs=1M count=1024

# Time the encryption
time ./etdk test_1gb.bin
hexdump -C test_1gb.bin | head  # Verify encrypted
```

### Check Memory Footprint
```bash
/usr/bin/time -v ./etdk large_file.bin
# Shows max RSS, page faults, etc.
```

## Environment Setup

### macOS
```bash
# Install dependencies
brew install cmake openssl

# Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug \
  -DOPENSSL_DIR=$(brew --prefix openssl@3)
cmake --build . -j4
```

### Linux (Ubuntu/Debian)
```bash
# Install dependencies
sudo apt-get install cmake libssl-dev build-essential

# Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . -j4
```

### Windows (MSVC)
```bash
# Install OpenSSL from https://slproweb.com/products/Win32OpenSSL.html
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -G "Visual Studio 16 2019"
cmake --build . --config Debug
```

## Testing

```bash
# Manual test
echo "test" > test.txt
sudo ./build/etdk test.txt
hexdump -C test.txt  # Should be encrypted

# Decrypt with saved key
openssl enc -d -aes-256-cbc -K <key> -iv <iv> -in test.txt -out recovered.txt
```

## Code Style

- C11 standard
- Linux kernel style (indent -linux)
- Doxygen comments for functions
- Max 80 chars per line

## Common Issues & Troubleshooting

### Build Fails: "Cannot find OpenSSL"
```bash
# macOS
cmake .. -DOPENSSL_DIR=$(brew --prefix openssl@3)

# Linux - install libssl-dev
sudo apt-get install libssl-dev

# Windows - download from https://slproweb.com/products/Win32OpenSSL.html
```

### Build Fails: "CMakeCache.txt conflict"
```bash
# Clean build directory
rm -rf build build-release
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . -j4
```

### Encryption Too Slow (Large Files)
```bash
# Use Release build instead of Debug
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j4
# Release build is 5-10x faster due to -O3 optimization
```

### Memory Errors with Valgrind
```bash
# If suppressions needed, create valgrind.supp:
valgrind --leak-check=full --suppressions=valgrind.supp ./etdk test.txt
```

### Device Not Found
```bash
# Verify device exists and permissions
ls -la /dev/sdb*
sudo ./etdk /dev/sdb  # May require root

# Use loop device for testing (no data loss)
sudo losetup /dev/loop0 test_file.iso
sudo ./etdk /dev/loop0
sudo losetup -d /dev/loop0
```

### GDB Debugging Tips
```bash
# Start GDB
gdb ./etdk

# Set breakpoint
(gdb) break crypto_encrypt_file
(gdb) break main

# Run with arguments
(gdb) run test.txt

# Print variables
(gdb) print ctx->key[0]

# Continue execution
(gdb) continue

# Step through code
(gdb) step
(gdb) next

# Print stack trace
(gdb) backtrace
```

## Security Testing Checklist

Before any release, verify:

- [ ] No keys printed to logs
- [ ] No keys in error messages
- [ ] Memory properly locked with mlock
- [ ] Secure wipe uses volatile pointers
- [ ] OpenSSL errors checked everywhere
- [ ] No buffer overflows (use fixed sizes)
- [ ] Input validation on paths
- [ ] No hardcoded test keys/IVs in code
- [ ] Valgrind shows no leaks
- [ ] Address Sanitizer shows no errors

## Documentation

### Building API Docs (Doxygen)
```bash
# Install Doxygen
sudo apt-get install doxygen graphviz  # Linux
brew install doxygen graphviz          # macOS

# Generate docs
doxygen Doxyfile
# Output in docs/html/index.html
```

### Update README
- Keep installation instructions current
- Document all command-line options
- Add usage examples
- Update supported platforms

### Update DEVELOPER_GUIDE
- Add new build options
- Document new functions
- Add troubleshooting for new issues
- Keep references current

## Release Process

```bash
# 1. Ensure all tests pass
bash test_etdk.sh

# 2. Update version in include/etdk.h
#define ETDK_VERSION "1.0.1"

# 3. Update CHANGELOG
# Add new features, bugfixes, security updates

# 4. Commit and tag
git add -A
git commit -m "Release v1.0.1"
git tag -a v1.0.1 -m "Release version 1.0.1"
git push origin master --tags

# 5. Build Release binary
rm -rf build-release
mkdir build-release && cd build-release
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j4
strip etdk  # Remove debug symbols

# 6. Create GitHub Release with binary
```

## Performance Optimization

### Profile-Guided Optimization
```bash
# Build with PGO
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS="-fprofile-generate"
cmake --build . -j4

# Run representative workload
./etdk large_test_file.bin

# Rebuild with profile data
cmake .. -DCMAKE_C_FLAGS="-fprofile-use -fprofile-correction"
cmake --build . -j4
```

### Compiler Optimizations
- `-O3` - Aggressive optimization (Release build)
- `-O2` - Standard optimization
- `-O1` - Light optimization (with Address Sanitizer)
- `-march=native` - CPU-specific optimizations (performance)

## References

- [OpenSSL EVP API](https://www.openssl.org/docs/man3.0/man7/evp.html)
- [BSI CON.6](https://www.bsi.bund.de/)
- [NIST AES](https://csrc.nist.gov/publications/detail/fips/197/final)

---

For issues: [GitHub Issues](https://github.com/damachine/etdk/issues)
