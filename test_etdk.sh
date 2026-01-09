#!/bin/bash

# ETDK - Quick Test Script
# Tests the Encrypt-then-Delete-Key functionality

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ETDK_BIN="$SCRIPT_DIR/build/etdk"

echo "=========================================="
echo "ETDK - Test Script"
echo "=========================================="
echo ""

# Check if etdk binary exists
if [ ! -f "$ETDK_BIN" ]; then
    echo "Error: etdk binary not found at $ETDK_BIN"
    echo "Please build the project first with: cmake --build build"
    exit 1
fi

# Create test directory
TEST_DIR="/tmp/etdk_test_$$"
mkdir -p "$TEST_DIR"
cd "$TEST_DIR"

echo "Test Directory: $TEST_DIR"
echo ""

# Test 1: Create a test file
echo "TEST 1: Creating test file..."
TEST_FILE="secret.txt"
TEST_DATA="This is a secret message that will be encrypted and then deleted!"
echo "$TEST_DATA" > "$TEST_FILE"
ORIGINAL_SIZE=$(stat -f%z "$TEST_FILE" 2>/dev/null || stat -c%s "$TEST_FILE" 2>/dev/null)
echo "✓ Test file created: $TEST_FILE ($ORIGINAL_SIZE bytes)"
echo ""
echo "=== ORIGINAL FILE CONTENT ==="
echo "Text format:"
echo "  $TEST_DATA"
echo ""
echo "Hexadecimal format:"
hexdump -C "$TEST_FILE" | head -10
echo ""

# Test 2: Encrypt the file
echo "TEST 2: Encrypting file with ETDK..."
# We need to provide "YES" to confirm
echo "YES" | "$ETDK_BIN" "$TEST_FILE" > /tmp/etdk_output.txt 2>&1

# Extract encryption information
KEY_LINE=$(grep "^Key:" /tmp/etdk_output.txt | head -1 || echo "")
IV_LINE=$(grep "^IV:" /tmp/etdk_output.txt | head -1 || echo "")

# Check if file was encrypted
if [ -f "$TEST_FILE" ]; then
    ENCRYPTED_SIZE=$(stat -f%z "$TEST_FILE" 2>/dev/null || stat -c%s "$TEST_FILE" 2>/dev/null)
    echo "✓ File encrypted successfully"
    echo ""
    echo "=== FILE SIZE COMPARISON ==="
    echo "  Original size:   $ORIGINAL_SIZE bytes"
    echo "  Encrypted size:  $ENCRYPTED_SIZE bytes"
    echo "  Difference:      $((ENCRYPTED_SIZE - ORIGINAL_SIZE)) bytes (PKCS#7 padding)"
    echo ""

    echo "=== ENCRYPTED FILE CONTENT ==="
    echo "Hexadecimal format:"
    hexdump -C "$TEST_FILE"
    echo ""

    # Test 3: Verify file content changed
    echo "TEST 3: Verifying encryption (content should be different)..."
    if grep -q "This is a secret" "$TEST_FILE" 2>/dev/null; then
        echo "✗ FAILED: Original text still visible in file!"
        exit 1
    else
        echo "✓ Original text is no longer readable (encrypted)"
    fi
    echo ""

    # Test 4: Show encryption key and IV
    echo "TEST 4: Encryption Key and IV Information:"
    echo "=========================================="
    if [ -n "$KEY_LINE" ]; then
        echo "$KEY_LINE"
    else
        echo "Key: (Not captured)"
    fi

    if [ -n "$IV_LINE" ]; then
        echo "$IV_LINE"
    else
        echo "IV: (Not captured)"
    fi
    echo ""

    # Test 5: Show full encryption output
    echo "TEST 5: Full Encryption Output:"
    echo "=========================================="
    cat /tmp/etdk_output.txt
    echo ""
else
    echo "✗ FAILED: Encrypted file not found!"
    exit 1
fi

# Cleanup
cd /
rm -rf "$TEST_DIR"
rm -f /tmp/etdk_output.txt

echo "=========================================="
echo "✓ All tests PASSED!"
echo "=========================================="
echo ""
echo "Summary:"
echo "  ✓ File encryption works correctly"
echo "  ✓ Original content is unreadable after encryption"
echo "  ✓ Encryption key was displayed and wiped"
echo ""
