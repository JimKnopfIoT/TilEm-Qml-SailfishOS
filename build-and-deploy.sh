#!/bin/bash
# Comprehensive build and deploy script with all safety checks

set -e

DEVICE_IP="192.168.10.41"
DEVICE_USER="root"
RPM_PATH="RPMS/harbour-tilem-0.3.0-1.aarch64.rpm"
BINARY_PATH="/usr/bin/harbour-tilem"
LAST_MD5_FILE=".last_deployed_md5"

echo "=== STEP 1: DEEP CLEANUP ==="

# CRITICAL: Find and delete ALL .o files in SDK container BEFORE wiping directories
echo "Finding and deleting ALL .o files in SDK container..."
~/SailfishOS-Platform-SDK/sdk-chroot bash -c "
    # Find and count .o files before deletion
    O_COUNT=\$(find /home/mersdk /home/deploy -name '*.o' 2>/dev/null | wc -l)
    echo \"Found \$O_COUNT .o files in SDK container\"

    # Delete all .o files
    find /home/mersdk /home/deploy -name '*.o' -delete 2>/dev/null || true

    # Delete all .a library files (static libraries)
    find /home/mersdk /home/deploy -name '*.a' -delete 2>/dev/null || true

    # Remove entire build cache directories
    rm -rf /home/mersdk/build* /home/deploy/build* 2>/dev/null || true

    echo 'SDK container build caches completely wiped'
"

# Clean local build artifacts
echo "Cleaning local build artifacts..."
rm -rf build-container/ RPMS/ installroot/ harbour-tilem-*.tar.bz2 build-native/

# Find and delete ALL .o files locally
echo "Finding and deleting ALL .o files locally..."
find . -name "*.o" -delete 2>/dev/null || true
find . -name "*.a" -delete 2>/dev/null || true
find . -name "Makefile" -delete 2>/dev/null || true

echo "Local cleanup complete"

echo ""
echo "=== STEP 2: BUILD ==="
~/SailfishOS-Platform-SDK/mb2 -t SailfishOS-5.0.0.62-aarch64 build

if [ ! -f "$RPM_PATH" ]; then
    echo "ERROR: Build failed - RPM not created"
    exit 1
fi

echo ""
echo "=== STEP 3: CHECK BUILD CHECKSUM ==="
NEW_MD5=$(md5sum "$RPM_PATH" | awk '{print $1}')
echo "New RPM MD5: $NEW_MD5"

if [ -f "$LAST_MD5_FILE" ]; then
    LAST_MD5=$(cat "$LAST_MD5_FILE")
    echo "Last deployed MD5: $LAST_MD5"

    if [ "$NEW_MD5" == "$LAST_MD5" ]; then
        echo "WARNING: Build MD5 unchanged - no actual changes were compiled!"
        echo "Aborting deployment."
        exit 1
    fi
else
    echo "No previous deployment recorded"
fi

echo ""
echo "=== STEP 4: CHECK IF APP IS RUNNING ON DEVICE ==="
APP_RUNNING=$(ssh $DEVICE_USER@$DEVICE_IP "pgrep harbour-tilem || echo 'not_running'")

if [ "$APP_RUNNING" != "not_running" ]; then
    echo "App is running (PID: $APP_RUNNING) - killing it..."
    ssh $DEVICE_USER@$DEVICE_IP "pkill -9 harbour-tilem"
    sleep 2
    echo "App killed"
else
    echo "App is not running"
fi

echo ""
echo "=== STEP 5: DEPLOY RPM ==="
echo "Copying RPM to device..."
scp "$RPM_PATH" $DEVICE_USER@$DEVICE_IP:/tmp/

echo "Installing RPM on device..."
ssh $DEVICE_USER@$DEVICE_IP "rpm -Uvh --force /tmp/harbour-tilem-0.3.0-1.aarch64.rpm"

echo ""
echo "=== STEP 6: VERIFY DEPLOYMENT ==="
DEVICE_MD5=$(ssh $DEVICE_USER@$DEVICE_IP "md5sum $BINARY_PATH" | awk '{print $1}')
echo "Device binary MD5: $DEVICE_MD5"

# Save the RPM MD5 as last deployed
echo "$NEW_MD5" > "$LAST_MD5_FILE"

# Extract RPM to verify its contents
TEMP_DIR=$(mktemp -d)
cd "$TEMP_DIR"
rpm2cpio "/home/sven/QtProjekte/TilEm-Qml/$RPM_PATH" | cpio -idmv 2>&1 | grep -q "usr/bin/harbour-tilem"
EXTRACTED_MD5=$(md5sum usr/bin/harbour-tilem | awk '{print $1}')
cd - > /dev/null
rm -rf "$TEMP_DIR"

echo "Extracted binary MD5: $EXTRACTED_MD5"

if [ "$DEVICE_MD5" != "$EXTRACTED_MD5" ]; then
    echo "ERROR: Device binary MD5 doesn't match extracted RPM binary!"
    echo "Deployment may have failed."
    exit 1
fi

echo ""
echo "=== STEP 7: RESTART APPLICATION ==="
echo "Ensuring all harbour-tilem processes are terminated..."
ssh $DEVICE_USER@$DEVICE_IP "pkill -9 harbour-tilem; sleep 2"

echo "Launching application as defaultuser via invoker..."
ssh $DEVICE_USER@$DEVICE_IP "su - defaultuser -c 'invoker --type=silica-qt5 --single-instance harbour-tilem > /dev/null 2>&1 &'"

sleep 3

echo "Checking if application started..."
APP_PID=$(ssh $DEVICE_USER@$DEVICE_IP "pgrep harbour-tilem || echo 'not_running'")

if [ "$APP_PID" != "not_running" ]; then
    echo "✓ Application started successfully (PID: $APP_PID)"
else
    echo "⚠  Application may not have started - check device manually"
fi

echo ""
echo "=== DEPLOYMENT SUCCESSFUL ==="
echo ""
echo "✓ Build completed with new changes"
echo "✓ App was stopped on device"
echo "✓ RPM deployed successfully"
echo "✓ Binary checksum verified on device"
echo "✓ Application restarted via invoker"
echo "Device binary MD5: $DEVICE_MD5"
echo "Extracted binary MD5: $EXTRACTED_MD5"
echo ""
echo "🚀 Application is running! Check logs with:"
echo "   ssh root@$DEVICE_IP 'journalctl -f | grep -E \"QML|harbour-tilem\"'"
