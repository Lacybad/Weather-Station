//BMP functions, from TFT_eSPI, from Bodmer
//https://github.com/Bodmer/TFT_eSPI

#ifndef BMP_functions
#define BMP_functions

//function defs
int8_t drawBmp(const char *filename, int16_t x, int16_t y);
uint16_t read16(fs::File &f);
uint32_t read32(fs::File &f);

#endif
