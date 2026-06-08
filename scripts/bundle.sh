# This script builds and bundles a macOS .app for a Qt application.
#
# Required structure before running:
#
# - Root directory must contain:
#     - assets/                         → assets used by the app
#       └── appIcon/icon.png           → the app icon (used to generate icon.icns)
#     - scripts/bundle.sh              → this script (must be run from the root directory)
#
# - Usage:
#     ./scripts/bundle.sh <app_bundle_name>
#     (Example: ./scripts/bundle.sh "Manga Planner")
#
# - What it does:
#     - Builds the binary using cmake and make in a temporary 'build/' directory
#     - Creates a .app bundle named after the argument provided
#     - Converts assets/appIcon/icon.png into icon.icns and includes it
#     - Copies the compiled binary (default: MovieTracker) into the bundle
#     - Copies the assets/ folder into the bundle's Resources
#     - Runs macdeployqt to embed Qt dependencies
#     - Cleans up: deletes build/ and icon.iconset/ even if the script fails
#
# - Result:
#     - A clean repo, unchanged except for the newly added <app_bundle_name>.app bundle
#     - Ready to run via: open "<app_bundle_name>.app"
#
# - Requirements:
#     - Qt’s macdeployqt must be in your PATH
#     - CMakeLists.txt must produce a binary named "MovieTracker"


#!/bin/bash

set -e

if [ -z "$1" ]; then
	echo "Usage: $0 <bundle_name>"
	exit 1
fi

BUNDLE_NAME="$1"
BINARY_NAME="MovieTracker"
ROOT_DIR="$(pwd)"
BUILD_DIR="$ROOT_DIR/build"
ICONSET_DIR="$ROOT_DIR/icon.iconset"
APP_BUNDLE="$ROOT_DIR/$BUNDLE_NAME.app"
ICON_SRC="$ROOT_DIR/assets/appIcon/icon.png"

cleanup() {
	echo "Cleaning up..."
	rm -rf "$BUILD_DIR"
	rm -rf "$ICONSET_DIR"
	rm -rf "$APP_BUNDLE"
}
trap cleanup ERR

# Build binary
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake ..
make
cd "$ROOT_DIR"

# Create .app structure
mkdir -p "$APP_BUNDLE/Contents/MacOS"
mkdir -p "$APP_BUNDLE/Contents/Resources"

# Copy binary
cp "$BUILD_DIR/$BINARY_NAME" "$APP_BUNDLE/Contents/MacOS/"
chmod +x "$APP_BUNDLE/Contents/MacOS/$BINARY_NAME"

# Create icon.icns
mkdir -p "$ICONSET_DIR"
sips -z 16 16     "$ICON_SRC" --out "$ICONSET_DIR/icon_16x16.png"
sips -z 32 32     "$ICON_SRC" --out "$ICONSET_DIR/icon_16x16@2x.png"
sips -z 32 32     "$ICON_SRC" --out "$ICONSET_DIR/icon_32x32.png"
sips -z 64 64     "$ICON_SRC" --out "$ICONSET_DIR/icon_32x32@2x.png"
sips -z 128 128   "$ICON_SRC" --out "$ICONSET_DIR/icon_128x128.png"
sips -z 256 256   "$ICON_SRC" --out "$ICONSET_DIR/icon_128x128@2x.png"
sips -z 256 256   "$ICON_SRC" --out "$ICONSET_DIR/icon_256x256.png"
sips -z 512 512   "$ICON_SRC" --out "$ICONSET_DIR/icon_256x256@2x.png"
sips -z 512 512   "$ICON_SRC" --out "$ICONSET_DIR/icon_512x512.png"
cp "$ICON_SRC" "$ICONSET_DIR/icon_512x512@2x.png"
iconutil -c icns "$ICONSET_DIR" -o "$APP_BUNDLE/Contents/Resources/icon.icns"

# Info.plist
cat > "$APP_BUNDLE/Contents/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<plist version="1.0">
<dict>
	<key>CFBundleName</key>
	<string>$BUNDLE_NAME</string>
	<key>CFBundleDisplayName</key>
	<string>$BUNDLE_NAME</string>
	<key>CFBundleIdentifier</key>
	<string>com.yourcompany.$BUNDLE_NAME</string>
	<key>CFBundleVersion</key>
	<string>1.0.0</string>
	<key>CFBundleExecutable</key>
	<string>$BINARY_NAME</string>
	<key>CFBundleIconFile</key>
	<string>icon.icns</string>
	<key>LSApplicationCategoryType</key>
	<string>public.app-category.utilities</string>
</dict>
</plist>
EOF

# Copy assets
cp -R assets "$APP_BUNDLE/Contents/Resources/"

# Deploy Qt
macdeployqt "$APP_BUNDLE"

# Cleanup temp dirs after success
rm -rf "$BUILD_DIR"
rm -rf "$ICONSET_DIR"

echo "Bundling complete: $APP_BUNDLE"
