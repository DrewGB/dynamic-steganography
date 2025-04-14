#include "steganography.h"

void testExtractFileFromImage();
void testHideInImage();

int main()
{
	testHideInImage();
	testExtractFileFromImage();

	return EXIT_SUCCESS;
}

void testHideInImage()
{
	FILE* infile = NULL;
	FILE* hidefile = NULL;
	FILE* outfile = NULL;

	infile = GetFile("Enter image to hide file in: ", "rb");
	hidefile = GetFile("Enter file to hide: ", "rb");

	// Create an image struct instance
	IMAGE img = { NULL, NULL };

	// Read in the image
	ReadImage(&img, infile);
	// close the file
	fclose(infile);

	// If the hidden file will fit within the image
	if (img.bmHDR->dwWidth * img.bmHDR->dwHeight >= GetFileSize(hidefile))
	{
		HideInImage(&img, hidefile);
		fclose(hidefile);

		// Write the image with hidden data out to file
		outfile = GetFile("Enter name of image with hidden file: ", "wb");
		WriteImage(&img, outfile);
		fclose(outfile);
	}
	else
	{
		printf("File is too big to hide in image\n");
		fclose(hidefile);
	}

	// Free any memory required
	FreeImage(&img);
}


void testExtractFileFromImage()
{
	FILE* infile = GetFile("Enter image to extract from: ", "rb");
	FILE* outfile = GetFile("Enter filename to extract to: ", "wb");
	IMAGE img = { NULL, NULL };

	// Read in the image
	ReadImage(&img, infile);
	// Close the file
	fclose(infile);

	ExtractFileFromImage(&img, outfile);
	fclose(outfile);

	// Free any memory required
	FreeImage(&img);
}

