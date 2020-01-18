#!/bin/bash

# Generates icons for iOS app assets

for size in {20,29,40,60}; do for scale in {2,3}; do
    if [[ $scale == 1 ]]; then
        filename="icon_${size}.png"
    else
        filename="icon_${size}@${scale}x.png"
    fi
    gm convert "Icon.png" -resize "$(( $scale * $size ))x$(( $scale * $size ))" "$filename"
done; done
