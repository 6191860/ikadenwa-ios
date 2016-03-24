#!/bin/bash
set -ueo pipefail
convert -resize 58x icon_raw.png icon_58.png
convert -resize 80x icon_raw.png icon_80.png
convert -resize 120x icon_raw.png icon_120.png
convert -resize 29x icon_raw.png icon_29.png
convert -resize 40x icon_raw.png icon_40.png
convert -resize 152x icon_raw.png icon_152.png
convert -resize 167x icon_raw.png icon_167.png
convert -resize 76x icon_raw.png icon_76.png

