//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Audio/BsAudioUtility.h"

using namespace b3d;

void ConvertToMono8(const i8* input, u8* output, u32 numSamples, u32 numChannels)
{
	for(u32 i = 0; i < numSamples; i++)
	{
		i16 sum = 0;
		for(u32 j = 0; j < numChannels; j++)
		{
			sum += *input;
			++input;
		}

		*output = sum / numChannels;
		++output;
	}
}

void ConvertToMono16(const i16* input, i16* output, u32 numSamples, u32 numChannels)
{
	for(u32 i = 0; i < numSamples; i++)
	{
		i32 sum = 0;
		for(u32 j = 0; j < numChannels; j++)
		{
			sum += *input;
			++input;
		}

		*output = sum / numChannels;
		++output;
	}
}

void Convert32To24Bits(const i32 input, u8* output)
{
	u32 valToEncode = *(u32*)&input;
	output[0] = (valToEncode >> 8) & 0x000000FF;
	output[1] = (valToEncode >> 16) & 0x000000FF;
	output[2] = (valToEncode >> 24) & 0x000000FF;
}

void ConvertToMono24(const u8* input, u8* output, u32 numSamples, u32 numChannels)
{
	for(u32 i = 0; i < numSamples; i++)
	{
		i64 sum = 0;
		for(u32 j = 0; j < numChannels; j++)
		{
			sum += AudioUtility::Convert24To32Bits(input);
			input += 3;
		}

		i32 avg = (i32)(sum / numChannels);
		Convert32To24Bits(avg, output);
		output += 3;
	}
}

void ConvertToMono32(const i32* input, i32* output, u32 numSamples, u32 numChannels)
{
	for(u32 i = 0; i < numSamples; i++)
	{
		i64 sum = 0;
		for(u32 j = 0; j < numChannels; j++)
		{
			sum += *input;
			++input;
		}

		*output = (i32)(sum / numChannels);
		++output;
	}
}

void Convert8To32Bits(const i8* input, i32* output, u32 numSamples)
{
	for(u32 i = 0; i < numSamples; i++)
	{
		i8 val = input[i];
		output[i] = val << 24;
	}
}

void Convert16To32Bits(const i16* input, i32* output, u32 numSamples)
{
	for(u32 i = 0; i < numSamples; i++)
		output[i] = input[i] << 16;
}

void Convert24To32Bits(const u8* input, i32* output, u32 numSamples)
{
	for(u32 i = 0; i < numSamples; i++)
	{
		output[i] = AudioUtility::Convert24To32Bits(input);
		input += 3;
	}
}

void Convert32To8Bits(const i32* input, u8* output, u32 numSamples)
{
	for(u32 i = 0; i < numSamples; i++)
		output[i] = (i8)(input[i] >> 24);
}

void Convert32To16Bits(const i32* input, i16* output, u32 numSamples)
{
	for(u32 i = 0; i < numSamples; i++)
		output[i] = (i16)(input[i] >> 16);
}

void Convert32To24Bits(const i32* input, u8* output, u32 numSamples)
{
	for(u32 i = 0; i < numSamples; i++)
	{
		Convert32To24Bits(input[i], output);
		output += 3;
	}
}

void AudioUtility::ConvertToMono(const u8* input, u8* output, u32 bitDepth, u32 numSamples, u32 numChannels)
{
	switch(bitDepth)
	{
	case 8:
		ConvertToMono8((i8*)input, output, numSamples, numChannels);
		break;
	case 16:
		ConvertToMono16((i16*)input, (i16*)output, numSamples, numChannels);
		break;
	case 24:
		ConvertToMono24(input, output, numSamples, numChannels);
		break;
	case 32:
		ConvertToMono32((i32*)input, (i32*)output, numSamples, numChannels);
		break;
	default:
		B3D_ASSERT(false);
		break;
	}
}

void AudioUtility::ConvertBitDepth(const u8* input, u32 inBitDepth, u8* output, u32 outBitDepth, u32 numSamples)
{
	i32* srcBuffer = nullptr;

	const bool needTempBuffer = inBitDepth != 32;
	if(needTempBuffer)
		srcBuffer = (i32*)B3DStackAllocate(numSamples * sizeof(i32));
	else
		srcBuffer = (i32*)input;

	// Note: I convert to a temporary 32-bit buffer and then use that to convert to actual requested bit depth.
	//       It would be more efficient to convert directly from source to requested depth without a temporary buffer,
	//       at the cost of additional complexity. If this method ever becomes a performance issue consider that.
	switch(inBitDepth)
	{
	case 8:
		Convert8To32Bits((i8*)input, srcBuffer, numSamples);
		break;
	case 16:
		Convert16To32Bits((i16*)input, srcBuffer, numSamples);
		break;
	case 24:
		::Convert24To32Bits(input, srcBuffer, numSamples);
		break;
	case 32:
		// Do nothing
		break;
	default:
		B3D_ASSERT(false);
		break;
	}

	switch(outBitDepth)
	{
	case 8:
		Convert32To8Bits(srcBuffer, output, numSamples);
		break;
	case 16:
		Convert32To16Bits(srcBuffer, (i16*)output, numSamples);
		break;
	case 24:
		Convert32To24Bits(srcBuffer, output, numSamples);
		break;
	case 32:
		memcpy(output, srcBuffer, numSamples * sizeof(i32));
		break;
	default:
		B3D_ASSERT(false);
		break;
	}

	if(needTempBuffer)
	{
		B3DStackFree(srcBuffer);
		srcBuffer = nullptr;
	}
}

void AudioUtility::ConvertToFloat(const u8* input, u32 inBitDepth, float* output, u32 numSamples)
{
	if(inBitDepth == 8)
	{
		for(u32 i = 0; i < numSamples; i++)
		{
			i8 sample = *(i8*)input;
			output[i] = sample / 127.0f;

			input++;
		}
	}
	else if(inBitDepth == 16)
	{
		for(u32 i = 0; i < numSamples; i++)
		{
			i16 sample = *(i16*)input;
			output[i] = sample / 32767.0f;

			input += 2;
		}
	}
	else if(inBitDepth == 24)
	{
		for(u32 i = 0; i < numSamples; i++)
		{
			i32 sample = Convert24To32Bits(input);
			output[i] = sample / 2147483647.0f;

			input += 3;
		}
	}
	else if(inBitDepth == 32)
	{
		for(u32 i = 0; i < numSamples; i++)
		{
			i32 sample = *(i32*)input;
			output[i] = sample / 2147483647.0f;

			input += 4;
		}
	}
	else
		B3D_ASSERT(false);
}

i32 AudioUtility::Convert24To32Bits(const u8* input)
{
	return (input[2] << 24) | (input[1] << 16) | (input[0] << 8);
}
