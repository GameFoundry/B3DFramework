//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsBitmapWriter.h"

namespace bs
{
#pragma pack(push, 2) // Align to 2byte boundary so we don't get extra 2 bytes for this struct

	struct BMP_HEADER
	{
		UINT16 BM;
		UINT32 size_of_file;
		UINT32 reserve;
		UINT32 offset_of_pixel_data;
		UINT32 size_of_header;
		UINT32 width;
		UINT32 hight;
		UINT16 num_of_color_plane;
		UINT16 num_of_bit_per_pix;
		UINT32 compression;
		UINT32 size_of_pix_data;
		UINT32 h_resolution;
		UINT32 v_resolution;
		UINT32 num_of_color_in_palette;
		UINT32 important_colors;

	};

#pragma pack(pop)

	void BitmapWriter::rawPixelsToBMP(const UINT8* input, UINT8* output, UINT32 width, UINT32 height, UINT32 bytesPerPixel)
	{
		UINT16 bmpBytesPerPixel = 3;
		if(bytesPerPixel >= 4)
			bmpBytesPerPixel = 4;

		UINT32 padding = (width * bmpBytesPerPixel) % 4;
		if(padding != 0)
			padding = 4 - padding;

		UINT32 rowPitch = (width * bmpBytesPerPixel) + padding;
		UINT32 dataSize = height * rowPitch;

		BMP_HEADER header;
		header.BM = 0x4d42;
		header.size_of_file =  sizeof(header) + dataSize;
		header.reserve = 0000; 
		header.offset_of_pixel_data = 54;
		header.size_of_header = 40;
		header.width = width;
		header.hight = height;
		header.num_of_color_plane = 1;
		header.num_of_bit_per_pix = bmpBytesPerPixel * 8;
		header.compression = 0;
		header.size_of_pix_data = dataSize;
		header.h_resolution = 2835;
		header.v_resolution = 2835;
		header.num_of_color_in_palette = 0;
		header.important_colors = 0;

		// Write header
		memcpy(output, &header, sizeof(header));
		output += sizeof(header);

		// Write bytes
		UINT32 widthBytes = width * bytesPerPixel;
		 
		// BPP matches so we can just copy directly
		if(bmpBytesPerPixel == bytesPerPixel)
		{
			for(INT32 y = height - 1; y >= 0 ; y--)
			{
				UINT8* outputPtr = output + y * rowPitch;

				memcpy(outputPtr, input, widthBytes);
				memset(outputPtr + widthBytes, 0, padding);

				input += widthBytes;
			}
		}
		else if(bmpBytesPerPixel < bytesPerPixel) // More bytes in source than supported in BMP, just truncate excess data
		{
			for(INT32 y = height - 1; y >= 0 ; y--)
			{
				UINT8* outputPtr = output + y * rowPitch;

				for(UINT32 x = 0; x < width; x++)
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
			for(INT32 y = height - 1; y >= 0 ; y--)
			{
				UINT8* outputPtr = output + y * rowPitch;

				for(UINT32 x = 0; x < width; x++)
				{
					memcpy(outputPtr, input, bytesPerPixel);

					// Fill the empty bytes with the last available byte from input
					UINT32 remainingBytes = bmpBytesPerPixel - bytesPerPixel;
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

	UINT32 BitmapWriter::getBMPSize(UINT32 width, UINT32 height, UINT32 bytesPerPixel)
	{
		UINT16 bmpBytesPerPixel = 3;
		if(bytesPerPixel >= 4)
			bmpBytesPerPixel = 4;

		UINT32 padding = (width * bmpBytesPerPixel) % 4;
		if(padding != 0)
			padding = 4 - padding;

		UINT32 rowPitch = (width * bmpBytesPerPixel) + padding;
		UINT32 dataSize = height * rowPitch;

		return sizeof(BMP_HEADER) + dataSize;
	}
}