
//{{BLOCK(chart)

//======================================================================
//
//	chart, 16x32@4, 
//	Transparent color : 00,00,00
//	+ palette 256 entries, not compressed
//	+ 9 tiles (t|f reduced) not compressed
//	+ regular map (flat), not compressed, 2x4 
//	Total size: 512 + 288 + 16 = 816
//
//	Time-stamp: 2026-03-06, 00:01:44
//	Exported by Cearn's GBA Image Transmogrifier, v0.9.2
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_CHART_H
#define GRIT_CHART_H

#define chartTilesLen 288
extern const unsigned int chartTiles[72];

#define chartMapLen 16
extern const unsigned short chartMap[8];

#define chartPalLen 512
extern const unsigned short chartPal[256];

#endif // GRIT_CHART_H

//}}BLOCK(chart)
