
//{{BLOCK(char)

//======================================================================
//
//	char, 160x32@4, 
//	Transparent color : 00,00,00
//	+ palette 256 entries, not compressed
//	+ 68 tiles (t|f reduced) not compressed
//	+ regular map (flat), not compressed, 20x4 
//	Total size: 512 + 2176 + 160 = 2848
//
//	Time-stamp: 2026-03-05, 23:59:27
//	Exported by Cearn's GBA Image Transmogrifier, v0.9.2
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_CHAR_H
#define GRIT_CHAR_H

#define charTilesLen 2176
extern const unsigned int charTiles[544];

#define charMapLen 160
extern const unsigned short charMap[80];

#define charPalLen 512
extern const unsigned short charPal[256];

#endif // GRIT_CHAR_H

//}}BLOCK(char)
