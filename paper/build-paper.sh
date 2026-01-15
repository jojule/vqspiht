#!/bin/bash
# Build script for the vqSPIHT paper
# Requires: LaTeX, dvips, ps2pdf, ImageMagick (for image conversion)

set -e

cd "$(dirname "$0")"

echo "Preparing figures..."

# Decompress .ps.gz files if needed
for f in *.ps.gz; do
    [ -f "$f" ] || continue
    base="${f%.gz}"
    if [ ! -f "$base" ]; then
        echo "  Decompressing $f..."
        gunzip -k "$f"
    fi
done

# Convert GIF files to PS if needed
convert_gif() {
    local gif="$1"
    local ps="$2"
    if [ -f "$gif" ] && [ ! -f "$ps" ]; then
        echo "  Converting $gif to $ps..."
        magick "$gif" "$ps" 2>/dev/null || convert "$gif" "$ps"
    fi
}

convert_gif "c_orig.gif" "c_orig.ps"
convert_gif "c_poster.gif" "c_poster.ps"
convert_gif "bigroiposter.gif" "bigroi.ps"
convert_gif "m_whole_original.gif" "m_whole_original.ps"
convert_gif "small_m_poster.gif" "small_m_poster.ps"

echo "Compiling paper..."
latex -interaction=nonstopmode vqSPIHT.tex
latex -interaction=nonstopmode vqSPIHT.tex  # Run twice for references
dvips -o vqSPIHT.ps vqSPIHT.dvi
ps2pdf vqSPIHT.ps vqSPIHT.pdf

echo "Paper compiled successfully: paper/vqSPIHT.pdf"
