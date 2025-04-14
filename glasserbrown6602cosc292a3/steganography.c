#define _CRT_SECURE_NO_WARNINGS
#include "steganography.h"

/// <summary>
/// This method will hide the provided file inside of the provided image.
/// </summary>
/// <param name="imgPtr">Image to store file in</param>
/// <param name="filePtr">File to store in the image</param>
void HideInImage(IMAGE* imgPtr, FILE* filePtr)
{
	// Get the file size of the file to hide
	unsigned int fileSize = GetFileSize(filePtr);
	// Check that the file size is small enough to fit in the image
	if (imgPtr->bmHDR->dwWidth * imgPtr->bmHDR->dwHeight >= fileSize)
	{
		// Use malloc to dynamically allocate memory for the file data
		BYTE* fileData = (BYTE*)malloc(fileSize);
		// Check that malloc worked
		if (fileData != NULL)
		{
			// Read in the data from the file into the buffer
			if (fread(fileData, fileSize, 1, filePtr) != 1)
			{
				// Failed to read file data so we free fileData and return
				free(fileData);
				printf("Reading data from file did not work");
				return;
			}
			// Store the file size in dwClrImportant in the header
			imgPtr->bmHDR->dwClrImportant = fileSize;
			// Loop through the bytes in the file
			for (int i = 0; i < fileSize; i++)
			{
				// Get the blue, green, and red bytes from the file

				// Simply shift the bits to the right 4 to get the blue bits
				BYTE fileBlueByte = fileData[i] >> 4;
				// Target the green bits with the mask 00001100(12) and shift them to the right 2
				BYTE fileGreenByte = (fileData[i] & 12) >> 2;
				// Target the red bits with the mask 00000011(3) dont need to shift since its already at the end
				BYTE fileRedByte = fileData[i] & 3;
				// Now that we have the file bytes seperated out we can add them to the corresponding pixel
				// Use & to clear out the bits we want to change and | to add the bits from the file

				// Use 11110000(240) to clear out the bits we want to change and | the blue bits from the file
				imgPtr->bmData[i].bBlu = (imgPtr->bmData[i].bBlu & 240) | fileBlueByte;
				// Use 11111100(252) to clear out the bits we want to change and | the green bits from the file
				imgPtr->bmData[i].bGrn = (imgPtr->bmData[i].bGrn & 252) | fileGreenByte;
				// Use 11111100(252) to clear out the bits we want to change and | the red bits from the file
				imgPtr->bmData[i].bRed = (imgPtr->bmData[i].bRed & 252) | fileRedByte;
			}
			// Free the file data
			free(fileData);
		}
		else
		{
			printf("Failed to allocate memory for file data\n");
		}
	}
	else
	{
		printf("File is too big to hide in image\n");
	}
}

/// <summary>
/// This method will extract a file hidden in the provided image and write it to the provided file.
/// </summary>
/// <param name="imgPtr">The image with the hidden file</param>
/// <param name="filePtr">The location to store the hidden file contents</param>
void ExtractFileFromImage(IMAGE* imgPtr, FILE* filePtr)
{
	// Get the file size of the hidden file from the header's dwClrImportant field
	unsigned int fileSize = imgPtr->bmHDR->dwClrImportant;
	// Dynamically allocated memory for the fileData to be stored
	BYTE* fileData = (BYTE*)malloc(fileSize);
	// Check that malloc worked
	if (fileData != NULL)
	{
		// Loop through the images pixels extracting the file data
		for (int i = 0; i < fileSize; i++)
		{
			// Get the blue, green, and red bytes from the image
			
			// Target the blue bits with the mask 00001111(15)
			BYTE fileBlueByte = imgPtr->bmData[i].bBlu & 15;
			// Target the green bits with the mask 00000011(3)
			BYTE fileGreenByte = imgPtr->bmData[i].bGrn & 3; 
			// Target the red bits with the mask 00000011(3)
			BYTE fileRedByte = imgPtr->bmData[i].bRed & 3; 
			// Now that we have the file bytes aqueried we just have to combine them into that file byte
			// Use << 4 to shift blue to the end << 2 to shift green into it's spot and leave red as is using
			// | to add them all together into one byte
			fileData[i] = (fileBlueByte << 4) | (fileGreenByte << 2) | fileRedByte;
		}
		// Then write the fileData to the file
		if (fwrite(fileData, fileSize, 1, filePtr) != 1)
		{
			printf("Failed to write the file data\n");
		}
		// Free the file data
		free(fileData);
	}
	else
	{
		printf("Failed to allocate memory for file data\n");
	}
}

/// <summary>
/// This method will return the size of the given file in bytes.
/// I found my solution here: https://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c
/// </summary>
/// <param name="filePtr"></param>
/// <returns></returns>
unsigned int GetFileSize(FILE* filePtr)
{
	// Use fseek() to move the file pointer to the end of the file.
	fseek(filePtr, 0, SEEK_END);
	// Use ftell() to retrieve the current file pointer position,
	unsigned int fileSize = ftell(filePtr);
	// Rewind the file pointer back to the beginning of the file.
	rewind(filePtr);
	return fileSize;
}

// Read an image into the image pointer from the already opened file pointer
void ReadImage(IMAGE* imgPtr, FILE* infile)
{
	// Read in the header
	ReadHeader(imgPtr, infile);
	if (imgPtr->bmHDR != NULL) // header was read in successfully
	{
		// Read in the data
		ReadData(imgPtr, infile);
	}
}


// Read the image header into the image pointer from the already opened file pointer
void ReadHeader(IMAGE* imgPtr, FILE* infile)
{
	// Allocate memory for the bitmap header
	imgPtr->bmHDR = (BITMAPHDR*)malloc(sizeof(BITMAPHDR));

	if (imgPtr->bmHDR != NULL) // memory successfully allocated
	{
		if (fread(imgPtr->bmHDR, sizeof(BITMAPHDR), 1, infile) != 1)
		{
			// No success reading, but malloc worked
			free(imgPtr->bmHDR);
			imgPtr->bmHDR = NULL;
			printf("Reading header from file did not work\n");
		}
	}
}

// Read the image data into the image pointer from the already opened file pointer
void ReadData(IMAGE* imgPtr, FILE* infile)
{
	// The padding, in bytes, for the image is:
	unsigned int padding = imgPtr->bmHDR->dwWidth % 4;
	// Calculate the image size in bytes = size of row * number of rows
	unsigned int imageSize = (imgPtr->bmHDR->dwWidth * sizeof(PIXEL) + padding)
		* imgPtr->bmHDR->dwHeight;

	printf("Calculated image size: %d\n", imageSize);

	if (imageSize == imgPtr->bmHDR->dwImageSize)
	{
		// Allocate memory for pixel data
		imgPtr->bmData = (PIXEL*)malloc(imageSize);
		if (imgPtr->bmData != NULL) // memory successfully allocated
		{
			// Read in the image data
			if (fread(imgPtr->bmData, imageSize, 1, infile) != 1)
			{
				// Failed to read image data
				FreeImage(imgPtr);
				printf("Reading data from file did not work");
			}
		}
		else
		{
			free(imgPtr->bmHDR);
			imgPtr->bmHDR = NULL;
		}
	}
	else
	{
		printf("Invalid image\n");
		free(imgPtr->bmHDR);
		imgPtr->bmHDR = NULL;
	}
}

// Open a file given a prompt for the user (asking for the filename) and a file mode
FILE* GetFile(const char* cPrompt, const char* cMode)
{
	FILE* aFile = NULL;
	char cFileName[MAX_FILENAME_SIZE];

	// Get the name from the user
	printf("%s", cPrompt);
	gets_s(cFileName, MAX_FILENAME_SIZE);

	// Open the file with the given name and mode passed in
	aFile = fopen(cFileName, cMode);

	return aFile;
}

// Free the image header and image data from the passed-in image pointer
void FreeImage(IMAGE* imgPtr)
{
	// Free the image
	free(imgPtr->bmHDR);
	imgPtr->bmHDR = NULL;
	if (imgPtr->bmData != NULL)
	{
		free(imgPtr->bmData);
		imgPtr->bmData = NULL;
	}
}

// Write the image header and image data from the image pointer provided
// into the already opened file
void WriteImage(IMAGE* imgPtr, FILE* outfile)
{
	if (fwrite(imgPtr->bmHDR, sizeof(BITMAPHDR), 1, outfile) != 1)
	{
		printf("Failed to write image header\n");
	}
	else
	{
		// Write the image data - size has already been verified
		DWORD imageSize = imgPtr->bmHDR->dwImageSize;
		if (fwrite(imgPtr->bmData, imageSize, 1, outfile) != 1)
		{
			printf("Failed to write the image data\n");
		}
	}
}

