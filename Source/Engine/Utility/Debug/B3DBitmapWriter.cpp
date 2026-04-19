//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Debug/B3DBitmapWriter.h"

using namespace b3d;

#pragma pack(push, 2) // Align to 2byte boundary so we don't get extra 2 bytes for this struct

struct BMP_HEADER
{
	u16 BM;
	u32 SizeOfFile;
	u32 Reserve;
	u32 OffsetOfPixelData;
	u32 SizeOfHeader;
	u32 Width;
	u32 Hight;
	u16 NumOfColorPlane;
	u16 NumOfBitPerPix;
	u32 Compression;
	u32 SizeOfPixData;
	u32 HResolution;
	u32 VResolution;
	u32 NumOfColorInPalette;
	u32 ImportantColors;
};

#pragma pack(pop)

void BitmapWriter::RawPixelsToBmp(const u8* input, u8* output, u32 width, u32 height, u32 bytesPerPixel)
{
	u16 bmpBytesPerPixel = 3;
	if(bytesPerPixel >= 4)
		bmpBytesPerPixel = 4;

	u32 padding = (width * bmpBytesPerPixel) % 4;
	if(padding != 0)
		padding = 4 - padding;

	u32 rowPitch = (width * bmpBytesPerPixel) + padding;
	u32 dataSize = height * rowPitch;

	BMP_HEADER header;
	header.BM = 0x4d42;
	header.SizeOfFile = sizeof(header) + dataSize;
	header.Reserve = 0000;
	header.OffsetOfPixelData = 54;
	header.SizeOfHeader = 40;
	header.Width = width;
	header.Hight = height;
	header.NumOfColorPlane = 1;
	header.NumOfBitPerPix = bmpBytesPerPixel * 8;
	header.Compression = 0;
	header.SizeOfPixData = dataSize;
	header.HResolution = 2835;
	header.VResolution = 2835;
	header.NumOfColorInPalette = 0;
	header.ImportantColors = 0;

	// Write header
	memcpy(output, &header, sizeof(header));
	output += sizeof(header);

	// Write bytes
	u32 widthBytes = width * bytesPerPixel;

	// BPP matches so we can just copy directly
	if(bmpBytesPerPixel == bytesPerPixel)
	{
		for(i32 rowIndex = height - 1; rowIndex >= 0; rowIndex--)
		{
			u8* outputPtr = output + rowIndex * rowPitch;

			memcpy(outputPtr, input, widthBytes);
			memset(outputPtr + widthBytes, 0, padding);

			input += widthBytes;
		}
	}
	else if(bmpBytesPerPixel < bytesPerPixel) // More bytes in source than supported in BMP, just truncate excess data
	{
		for(i32 rowIndex = height - 1; rowIndex >= 0; rowIndex--)
		{
			u8* outputPtr = output + rowIndex * rowPitch;

			for(u32 columnIndex = 0; columnIndex < width; columnIndex++)
			{
				memcpy(outputPtr, input, bmpBytesPerPixel);
				outputPtr += bmpBytesPerPixel;
				input += bytesPerPixel;
			}

			memset(outputPtr, 0, padding);
		}
	}
	else // More bytes in BMP than in source (BMP must be 24bit minimum)
	{
		for(i32 rowIndex = height - 1; rowIndex >= 0; rowIndex--)
		{
			u8* outputPtr = output + rowIndex * rowPitch;

			for(u32 columnIndex = 0; columnIndex < width; columnIndex++)
			{
				memcpy(outputPtr, input, bytesPerPixel);

				// Fill the empty bytes with the last available byte from input
				u32 remainingBytes = bmpBytesPerPixel - bytesPerPixel;
				while(remainingBytes > 0)
				{
					memcpy(outputPtr + (bmpBytesPerPixel - remainingBytes), input, 1);
					remainingBytes--;
				}

				outputPtr += bmpBytesPerPixel;
				input += bytesPerPixel;
			}

			memset(outputPtr, 0, padding);
		}
	}
}

u32 BitmapWriter::GetBmpSize(u32 width, u32 height, u32 bytesPerPixel)
{
	u16 bmpBytesPerPixel = 3;
	if(bytesPerPixel >= 4)
		bmpBytesPerPixel = 4;

	u32 padding = (width * bmpBytesPerPixel) % 4;
	if(padding != 0)
		padding = 4 - padding;

	u32 rowPitch = (width * bmpBytesPerPixel) + padding;
	u32 dataSize = height * rowPitch;

	return sizeof(BMP_HEADER) + dataSize;
}
