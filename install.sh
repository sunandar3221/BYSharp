#!/bin/sh
set -e

if [ -n "$TERMUX_VERSION" ] || [ -d "/data/data/com.termux" ]; then
    IS_TERMUX=1
    INSTALL_DIR="${PREFIX:-/data/data/com.termux/files/usr}/bin"
    SUDO=""
else
    IS_TERMUX=0
    INSTALL_DIR="${PREFIX:-/usr/local}/bin"
    if [ "$(id -u)" -ne 0 ]; then
        SUDO="sudo"
    else
        SUDO=""
    fi
fi

ARCH="$(uname -m)"
case "$ARCH" in
    aarch64|arm64)
        ANDROID_BIN="bys-android-aarch64"
        LINUX_BIN="bys-linux-x86_64"
        ;;
    armv7l|armv8l|arm)
        ANDROID_BIN="bys-android-armv7a"
        LINUX_BIN=""
        ;;
    x86_64|amd64)
        ANDROID_BIN="bys-android-x86_64"
        LINUX_BIN="bys-linux-x86_64"
        ;;
    *)
        ANDROID_BIN=""
        LINUX_BIN=""
        ;;
esac

echo "=========================================="
echo "    BY# (BYSharp) Installer               "
echo "=========================================="
echo "Platform: $(uname -s) ($ARCH)"
echo "Target  : $INSTALL_DIR/bys dan $INSTALL_DIR/by#"

TMP_DIR="${TMPDIR:-/tmp}"
DOWNLOAD_OK=0

if [ "$IS_TERMUX" -eq 1 ] && [ -n "$ANDROID_BIN" ]; then
    URL="https://github.com/sunandar3221/BYSharp/releases/download/v1.0.0/$ANDROID_BIN"
    echo "Mengunduh pre-compiled binary untuk Termux Android ($ARCH)..."
    if command -v curl >/dev/null 2>&1; then
        if curl -sL "$URL" -o "$TMP_DIR/$ANDROID_BIN" 2>/dev/null && [ -s "$TMP_DIR/$ANDROID_BIN" ]; then
            DOWNLOAD_OK=1
            BIN_PATH="$TMP_DIR/$ANDROID_BIN"
        fi
    elif command -v wget >/dev/null 2>&1; then
        if wget -qO "$TMP_DIR/$ANDROID_BIN" "$URL" 2>/dev/null && [ -s "$TMP_DIR/$ANDROID_BIN" ]; then
            DOWNLOAD_OK=1
            BIN_PATH="$TMP_DIR/$ANDROID_BIN"
        fi
    fi
elif [ "$IS_TERMUX" -eq 0 ] && [ -n "$LINUX_BIN" ]; then
    URL="https://github.com/sunandar3221/BYSharp/releases/download/v1.0.0/$LINUX_BIN"
    echo "Mengunduh pre-compiled binary untuk Linux ($ARCH)..."
    if command -v curl >/dev/null 2>&1; then
        if curl -sL "$URL" -o "$TMP_DIR/$LINUX_BIN" 2>/dev/null && [ -s "$TMP_DIR/$LINUX_BIN" ]; then
            DOWNLOAD_OK=1
            BIN_PATH="$TMP_DIR/$LINUX_BIN"
        fi
    elif command -v wget >/dev/null 2>&1; then
        if wget -qO "$TMP_DIR/$LINUX_BIN" "$URL" 2>/dev/null && [ -s "$TMP_DIR/$LINUX_BIN" ]; then
            DOWNLOAD_OK=1
            BIN_PATH="$TMP_DIR/$LINUX_BIN"
        fi
    fi
fi

if [ "$DOWNLOAD_OK" -eq 1 ]; then
    $SUDO mkdir -p "$INSTALL_DIR"
    $SUDO cp "$BIN_PATH" "$INSTALL_DIR/bys"
    $SUDO cp "$BIN_PATH" "$INSTALL_DIR/by#"
    $SUDO chmod 755 "$INSTALL_DIR/bys"
    $SUDO chmod 755 "$INSTALL_DIR/by#"
    rm -f "$BIN_PATH"

    if ! "$INSTALL_DIR/bys" --version >/dev/null 2>&1; then
        if [ "$IS_TERMUX" -eq 1 ]; then
            pkg install -y libc++ >/dev/null 2>&1 || true
        fi
        if ! "$INSTALL_DIR/bys" --version >/dev/null 2>&1; then
            DOWNLOAD_OK=0
        fi
    fi

    if [ "$DOWNLOAD_OK" -eq 1 ]; then
        echo "Pre-compiled binary berhasil diverifikasi dan siap digunakan!"
    fi
fi

if [ "$DOWNLOAD_OK" -eq 0 ]; then
    echo "Mengompilasi source code secara lokal..."
    if command -v clang++ >/dev/null 2>&1; then
        CXX="clang++"
    elif command -v g++ >/dev/null 2>&1; then
        CXX="g++"
    else
        if [ "$IS_TERMUX" -eq 1 ]; then
            echo "Menginstal compiler di Termux..."
            pkg update -y && pkg install -y clang make
            CXX="clang++"
        elif command -v apt-get >/dev/null 2>&1; then
            echo "Menginstal compiler di Debian/Ubuntu..."
            if [ "$(id -u)" -eq 0 ]; then
                apt-get update -y && apt-get install -y build-essential
            else
                sudo apt-get update -y && sudo apt-get install -y build-essential
            fi
            CXX="g++"
        elif command -v pacman >/dev/null 2>&1; then
            echo "Menginstal compiler di Arch Linux..."
            if [ "$(id -u)" -eq 0 ]; then
                pacman -Sy --noconfirm gcc make
            else
                sudo pacman -Sy --noconfirm gcc make
            fi
            CXX="g++"
        elif command -v dnf >/dev/null 2>&1; then
            echo "Menginstal compiler di Fedora..."
            if [ "$(id -u)" -eq 0 ]; then
                dnf install -y gcc-c++ make
            else
                sudo dnf install -y gcc-c++ make
            fi
            CXX="g++"
        elif command -v apk >/dev/null 2>&1; then
            echo "Menginstal compiler di Alpine Linux..."
            if [ "$(id -u)" -eq 0 ]; then
                apk add g++ make
            else
                sudo apk add g++ make
            fi
            CXX="g++"
        else
            echo "Error: C++17 compiler tidak ditemukan."
            exit 1
        fi
    fi

    echo "Mengompilasi source code BY#..."
    $CXX -std=c++17 -O2 -Isrc \
        src/main.cpp \
        src/Lexer.cpp \
        src/Parser.cpp \
        src/Evaluator.cpp \
        -o bys

    $SUDO mkdir -p "$INSTALL_DIR"
    $SUDO cp bys "$INSTALL_DIR/bys"
    $SUDO cp bys "$INSTALL_DIR/by#"
    $SUDO chmod 755 "$INSTALL_DIR/bys"
    $SUDO chmod 755 "$INSTALL_DIR/by#"
fi

echo ""
echo "Verifikasi instalasi:"
"$INSTALL_DIR/bys" --version

echo ""
echo "=========================================="
echo "    Instalasi BY# Berhasil!               "
echo "=========================================="
echo "Gunakan perintah berikut di terminal:"
echo "  bys script.by#     (Menjalankan file script)"
echo "  by# script.by#     (Alias karakter hash)"
echo "  bys                (Membuka Alien REPL Console)"
