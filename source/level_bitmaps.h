
//{{BLOCK(map)

//======================================================================
//
//	map, 512x160@4, 
//	Transparent color : 00,00,00
//	+ palette 256 entries, not compressed
//	+ 15 tiles (t|f reduced) not compressed
//	+ regular map (flat), not compressed, 64x20 
//	Total size: 512 + 480 + 2560 = 3552
//
//	Time-stamp: 2026-02-16, 22:54:40
//	Exported by Cearn's GBA Image Transmogrifier, v0.9.2
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_MAP_H
#define GRIT_MAP_H

#define lvl1TilesLen 480
extern const unsigned int lvl1Tiles[120];

#define lvl1MapLen 2560
extern const unsigned short lvl1Map[1280];

#define lvl1PalLen 512
extern const unsigned short lvl1Pal[256];


#endif // GRIT_MAP_H

//}}BLOCK(map)
