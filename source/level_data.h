
//{{BLOCK(lvl1)

//======================================================================
//
//	lvl1, 592x160@4, 
//	Transparent color : 00,00,00
//	+ palette 256 entries, not compressed
//	+ 187 tiles (t|f reduced) not compressed
//	+ regular map (flat), not compressed, 74x20 
//	Total size: 512 + 5984 + 2960 = 9456
//
//	Time-stamp: 2026-02-28, 17:50:20
//	Exported by Cearn's GBA Image Transmogrifier, v0.9.2
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_LVL1_H
#define GRIT_LVL1_H

#define LVL_CNT 2
#define MAX_BLKTL_CNT 10

///////////////////////////////////////////////////////////////////////
#define lvl1TilesLen 5984
extern const unsigned int lvl1Tiles[1496];

#define lvl1MapLen 2960
extern const unsigned short lvl1Map[1480];

#define lvl1PalLen 512
extern const unsigned short lvl1Pal[256];

///////////////////////////////////////////////////////////////////////
#define lvl2TilesLen 1216
extern const unsigned int lvl2Tiles[304];

#define lvl2MapLen 2960
extern const unsigned short lvl2Map[1480];

#define lvl2PalLen 512
extern const unsigned short lvl2Pal[256];

///////////////////////////////////////////////////////////////////////

extern const unsigned short lvlBlockingTiles[LVL_CNT][MAX_BLKTL_CNT];
extern const unsigned short lvlTrigRegion[LVL_CNT][2];
extern const unsigned short *tilemap_ptr[LVL_CNT];

extern const unsigned short *pal_ptr[LVL_CNT];

extern const unsigned int *tiles_ptr[LVL_CNT];
extern const unsigned short tiles_len[LVL_CNT];

#endif // GRIT_LVL1_H

//}}BLOCK(lvl1)
