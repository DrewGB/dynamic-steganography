#ifndef STEGANOGRAPHY_H
#define STEGANOGRAPHY_H

#include <stdio.h>
#include <stdlib.h>

typedef unsigned char BYTE;  // 1 byte or 8 bits, unsigned
typedef unsigned short WORD; // 2 bytes or 16 bits, unsigned
typedef unsigned int DWORD;  // 4 bytes or 32 bits, unsigned
typedef signed int LONG;     // 4 bytes or 32 bits, signed
#define MAX_FILENAME_SIZE 256
#pragma pack(push) // Used to store the default byte alignment
#pragma pack(1) // Set the byte alignment to 1

typedef struct
{
	// Bitmap file header - 14 bytes
	WORD wType; // Should be 'B' 'M'
	DWORD dwFileSize;
	WORD wReserved1;
	WORD wReserved2;
	DWORD dwDataOffset; // Should be 54 for our application
	// DIB header (bitmap information header)
	DWORD dwHeaderSize; // Should be 40 for our application
	LONG dwWidth;
	LONG dwHeight;
	WORD wPlanes; // Should be 1
	WORD wBitCount; // Should be 24 for our application
	DWORD dwCompression; // Should be 0 for our application
	DWORD dwImageSize;
	LONG lXPelsPerMeter;
	LONG lYPelsPerMeter;
	DWORD dwClrUsed;
	DWORD dwClrImportant;
} BITMAPHDR;

typedef struct
{
	BYTE bBlu, bGrn, bRed;
} PIXEL;


#pragma pack(pop) // Used to reset the default byte alignment

typedef struct
{
	BITMAPHDR* bmHDR; // Bitmap header
	PIXEL* bmData;   // Pointer to pixel data
} IMAGE;

void HideInImage(IMAGE* imgPtr, FILE* filePtr);
void ExtractFileFromImage(IMAGE* imgPtr, FILE* filePtr);
unsigned int GetFileSize(FILE* filePtr);
void ReadImage(IMAGE* imgPtr, FILE* infile);
void ReadHeader(IMAGE* imgPtr, FILE* infile);
void ReadData(IMAGE* imgPtr, FILE* infile);
FILE* GetFile(const char* cPrompt, const char* cMode);
void FreeImage(IMAGE* imgPtr);
void WriteImage(IMAGE* imgPtr, FILE* outfile);
typedef void (*BM_FUNC_PTR)(PIXEL*);


#endif