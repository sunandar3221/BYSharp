#!/bin/sh
set -e

if [ -n "$TERMUX_VERSION" ] || [ -d "/data/data/com.termux" ]; then
    IS_TERMUX=1
else
    IS_TERMUX=0
fi

if command -v clang++ >/dev/null 2>&1; then
    CXX="clang++"
elif command -v g++ >/dev/null 2>&1; then
    CXX="g++"
else
    if [ "$IS_TERMUX" -eq 1 ]; then
        echo "Menginstal clang dan make di Termux..."
        pkg update -y && pkg install -y clang make
        CXX="clang++"
    elif command -v apt-get >/dev/null 2>&1; then
        echo "Menginstal compiler g++ di Debian/Ubuntu..."
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
        echo "Error: C++17 compiler (g++ atau clang++) tidak ditemukan."
        exit 1
    fi
fi

if [ "$IS_TERMUX" -eq 1 ]; then
    INSTALL_DIR="${PREFIX:-/data/data/com.termux/files/usr}/bin"
    SUDO=""
else
    INSTALL_DIR="${PREFIX:-/usr/local}/bin"
    if [ "$(id -u)" -ne 0 ]; then
        SUDO="sudo"
    else
        SUDO=""
    fi
fi

echo "=========================================="
echo "    BY# (BYSharp) Installer Linux/Termux  "
echo "=========================================="
echo "Compiler : $CXX"
echo "Target   : $INSTALL_DIR/bys dan $INSTALL_DIR/by#"
echo "Mengompilasi source code BY#..."

$CXX -std=c++17 -O2 -Isrc \
    src/main.cpp \
    src/Lexer.cpp \
    src/Parser.cpp \
    src/Evaluator.cpp \
    -o bys

echo "Menyalin executable ke direktori sistem..."
$SUDO mkdir -p "$INSTALL_DIR"
$SUDO cp bys "$INSTALL_DIR/bys"
$SUDO cp bys "$INSTALL_DIR/by#"
$SUDO chmod 755 "$INSTALL_DIR/bys"
$SUDO chmod 755 "$INSTALL_DIR/by#"

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
