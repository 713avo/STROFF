#!/bin/bash
# STROFF Vim Syntax Installer
# Installs syntax highlighting for STROFF files (.str, .trf)

set -e

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${BLUE}STROFF Vim Syntax Installer${NC}"
echo "=============================="
echo

# Determine vim directory
if [ -n "$HOME" ]; then
    VIM_DIR="$HOME/.vim"
else
    echo -e "${RED}Error: HOME environment variable not set${NC}"
    exit 1
fi

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Create vim directories if they don't exist
echo -e "${BLUE}Creating vim directories...${NC}"
mkdir -p "$VIM_DIR/syntax"
mkdir -p "$VIM_DIR/ftdetect"

# Install syntax file
echo -e "${BLUE}Installing syntax file...${NC}"
cp "$SCRIPT_DIR/syntax/stroff.vim" "$VIM_DIR/syntax/"
echo -e "${GREEN}✓${NC} Installed: $VIM_DIR/syntax/stroff.vim"

# Install filetype detection file
echo -e "${BLUE}Installing filetype detection...${NC}"
cp "$SCRIPT_DIR/ftdetect/stroff.vim" "$VIM_DIR/ftdetect/"
echo -e "${GREEN}✓${NC} Installed: $VIM_DIR/ftdetect/stroff.vim"

echo
echo -e "${GREEN}Installation complete!${NC}"
echo
echo "STROFF syntax highlighting is now enabled for:"
echo "  - .str files"
echo "  - .trf files (legacy)"
echo
echo "Open any .str or .trf file in Vim to see syntax highlighting."
echo

# Check if neovim is installed
if command -v nvim &> /dev/null; then
    echo -e "${BLUE}Neovim detected!${NC}"
    NVIM_DIR="${XDG_CONFIG_HOME:-$HOME/.config}/nvim"

    read -p "Would you like to install for Neovim as well? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        mkdir -p "$NVIM_DIR/syntax"
        mkdir -p "$NVIM_DIR/ftdetect"
        cp "$SCRIPT_DIR/syntax/stroff.vim" "$NVIM_DIR/syntax/"
        cp "$SCRIPT_DIR/ftdetect/stroff.vim" "$NVIM_DIR/ftdetect/"
        echo -e "${GREEN}✓${NC} Installed for Neovim: $NVIM_DIR"
    fi
fi

echo
echo -e "${GREEN}Done!${NC}"
