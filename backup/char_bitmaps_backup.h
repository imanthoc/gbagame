
//{{BLOCK(char)

//======================================================================
//
//	char, 16x32@4, 
//	Transparent color : 00,00,00
//	+ palette 256 entries, not compressed
//	+ 9 tiles (t|f reduced) not compressed
//	+ regular map (flat), not compressed, 2x4 
//	Total size: 512 + 288 + 16 = 816
//
//	Time-stamp: 2026-02-16, 23:06:29
//	Exported by Cearn's GBA Image Transmogrifier, v0.9.2
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_CHAR_H
#define GRIT_CHAR_H

#define charTilesLen 288
extern const unsigned int charTiles[72];

#define charMapLen 16
extern const unsigned short charMap[8];

#define charPalLen 512
extern const unsigned short charPal[256];

#endif // GRIT_CHAR_H

//}}BLOCK(char)
