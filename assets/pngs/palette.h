
//{{BLOCK(palette)

//======================================================================
//
//	palette, 16x8@8, 
//	+ palette 256 entries, not compressed
//	+ 3 tiles (t|f reduced) not compressed
//	+ regular map (flat), not compressed, 2x1 
//	Total size: 512 + 192 + 4 = 708
//
//	Time-stamp: 2026-03-10, 23:43:02
//	Exported by Cearn's GBA Image Transmogrifier, v0.9.2
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_PALETTE_H
#define GRIT_PALETTE_H

#define paletteTilesLen 192
extern const unsigned int paletteTiles[48];

#define paletteMapLen 4
extern const unsigned short paletteMap[2];

#define palettePalLen 512
extern const unsigned short palettePal[256];

#endif // GRIT_PALETTE_H

//}}BLOCK(palette)
