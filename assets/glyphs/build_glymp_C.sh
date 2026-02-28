#!/bin/sh

OUT_C=font.c
OUT_H=font.h

echo "/* Auto-generated GBA font */" > $OUT_C
echo "#ifndef FONT_H" > $OUT_H
echo "#define FONT_H" >> $OUT_H

echo "#include <gba_types.h>" >> $OUT_H
echo "" >> $OUT_H

echo "#include \"$OUT_H\"" >> $OUT_C
echo "" >> $OUT_C

TILE_INDEX=0

for i in glyph_*.png; do
    BASE=$(basename "$i" .png)

    grit "$i" \
        -gB4 \
        -gt \
        -p \
        -pe16 \
        -ftc \
        -o "$BASE"

    # ---- C FILE ----
    echo "// $BASE" >> $OUT_C
    sed -n '/Tiles\[/,/};/p' "$BASE.c" >> $OUT_C
    echo "" >> $OUT_C

    # ---- H FILE ----
    sed -n '/extern const unsigned int.*Tiles\[/p' "$BASE.h" >> $OUT_H

    # Palette only once (first glyph)
    if [ "$TILE_INDEX" -eq 0 ]; then
        sed -n '/Pal\[/,/};/p' "$BASE.c" >> $OUT_C
        sed -n '/extern const unsigned short.*Pal\[/p' "$BASE.h" >> $OUT_H
    fi

    TILE_INDEX=$((TILE_INDEX + 1))
done

echo "" >> $OUT_H
echo "#endif // FONT_H" >> $OUT_H

echo "Done. Generated font.c and font.h"
