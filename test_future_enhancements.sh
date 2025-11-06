#!/bin/bash
# Test script for OpenCog future enhancements

set -e

echo "Testing OpenCog Future Enhancements..."
echo "======================================="

# Test 1: Build the demo
echo ""
echo "[TEST 1] Building opencog_future_demo..."
cd /home/runner/work/rsyncog/rsyncog
make opencog-future-demo > /dev/null 2>&1
if [ -x ./opencog_future_demo ]; then
    echo "✓ Build successful"
else
    echo "✗ Build failed"
    exit 1
fi

# Test 2: Run the demo
echo ""
echo "[TEST 2] Running opencog_future_demo..."
./opencog_future_demo > /tmp/demo_output.txt 2>&1
if [ $? -eq 0 ]; then
    echo "✓ Demo executed successfully"
else
    echo "✗ Demo execution failed"
    exit 1
fi

# Test 3: Check for key output
echo ""
echo "[TEST 3] Verifying demo output..."
if grep -q "PLN Probabilistic Logic Networks" /tmp/demo_output.txt; then
    echo "✓ PLN inference demonstrated"
else
    echo "✗ PLN inference missing"
    exit 1
fi

if grep -q "Experience-Based Learning" /tmp/demo_output.txt; then
    echo "✓ Learning module demonstrated"
else
    echo "✗ Learning module missing"
    exit 1
fi

if grep -q "Dynamic Reconfiguration" /tmp/demo_output.txt; then
    echo "✓ Dynamic reconfiguration demonstrated"
else
    echo "✗ Dynamic reconfiguration missing"
    exit 1
fi

if grep -q "Distributed AtomSpace" /tmp/demo_output.txt; then
    echo "✓ Distributed AtomSpace demonstrated"
else
    echo "✗ Distributed AtomSpace missing"
    exit 1
fi

# Test 4: Check generated config file
echo ""
echo "[TEST 4] Verifying generated configuration..."
if [ -f ./rsyncd.conf.enhanced ]; then
    echo "✓ Configuration file generated"
else
    echo "✗ Configuration file not found"
    exit 1
fi

if grep -q "AtomSpace Handle:" ./rsyncd.conf.enhanced; then
    echo "✓ AtomSpace metadata embedded"
else
    echo "✗ AtomSpace metadata missing"
    exit 1
fi

if grep -q "Truth Value:" ./rsyncd.conf.enhanced; then
    echo "✓ Truth values embedded"
else
    echo "✗ Truth values missing"
    exit 1
fi

if grep -q "Attention:" ./rsyncd.conf.enhanced; then
    echo "✓ Attention values embedded"
else
    echo "✗ Attention values missing"
    exit 1
fi

# Test 5: Verify learning updated truth values
echo ""
echo "[TEST 5] Verifying learned truth values..."
# production_data should have very high strength after successful syncs
if grep -A 10 "\[production_data\]" ./rsyncd.conf.enhanced | grep -q "strength=0.99"; then
    echo "✓ Learning correctly updated truth values"
else
    echo "✗ Truth value updates incorrect"
    exit 1
fi

# Test 6: Verify attention was adapted
echo ""
echo "[TEST 6] Verifying attention adaptation..."
# experimental_data should have increased STI due to poor performance
if grep -A 15 "\[experimental_data\]" ./rsyncd.conf.enhanced | grep -q "STI=[5-9][0-9]"; then
    echo "✓ Attention correctly adapted to performance"
else
    echo "✗ Attention adaptation incorrect"
    exit 1
fi

echo ""
echo "======================================="
echo "All tests passed! ✓"
echo "======================================="
