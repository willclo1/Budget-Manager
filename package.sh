#!/bin/bash

set -e

APP_NAME="BudgetManager"
BUILD_DIR="build-release"

cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR"

rm -rf "$APP_NAME.app"

mkdir -p "$APP_NAME.app/Contents/MacOS"
mkdir -p "$APP_NAME.app/Contents/Resources"

cp "$BUILD_DIR/Budgeting" \
   "$APP_NAME.app/Contents/MacOS/BudgetingBinary"

cat > "$APP_NAME.app/Contents/MacOS/$APP_NAME" <<'EOF'
#!/bin/bash

APP_DIR="$(cd "$(dirname "$0")" && pwd)"
BINARY="$APP_DIR/BudgetingBinary"

osascript <<APPLESCRIPT
tell application "Terminal"
    activate
    do script quoted form of "$BINARY"
    set bounds of front window to {0, 25, 1440, 900}
end tell
APPLESCRIPT
EOF

chmod +x "$APP_NAME.app/Contents/MacOS/$APP_NAME"
chmod +x "$APP_NAME.app/Contents/MacOS/BudgetingBinary"

if [ -f "icon.icns" ]; then
    cp icon.icns "$APP_NAME.app/Contents/Resources/icon.icns"
fi

cat > "$APP_NAME.app/Contents/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"
"http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>$APP_NAME</string>

    <key>CFBundleIdentifier</key>
    <string>com.willclore.budgetmanager</string>

    <key>CFBundleName</key>
    <string>$APP_NAME</string>

    <key>CFBundlePackageType</key>
    <string>APPL</string>

    <key>CFBundleVersion</key>
    <string>1.0</string>

    <key>CFBundleShortVersionString</key>
    <string>1.0</string>

    <key>CFBundleIconFile</key>
    <string>icon</string>
</dict>
</plist>
EOF

echo "Done."
echo "Open with:"
echo "open $APP_NAME.app"