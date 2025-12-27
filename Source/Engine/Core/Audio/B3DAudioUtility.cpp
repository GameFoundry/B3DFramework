//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Audio/B3DAudioUtility.h"

using namespace b3d;

void ConvertToMono8(const i8* input, u8* output, u32 sampleCount, u32 channelCount)
{
	for(u32 sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
	{
		i16 sum = 0;
		for(u32 channelIndex = 0; channelIndex < channelCount; channelIndex++)
		{
			sum += *input;
			++input;
		}

		*output = sum / channelCount;
		++output;
	}
}

void ConvertToMono16(const i16* input, i16* output, u32 sampleCount, u32 channelCount)
{
	for(u32 sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
	{
		i32 sum = 0;
		for(u32 channelIndex = 0; channelIndex < channelCount; channelIndex++)
		{
			sum += *input;
			++input;
		}

		*output = sum / channelCount;
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

void ConvertToMono24(const u8* input, u8* output, u32 sampleCount, u32 channelCount)
{
	for(u32 sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
	{
		i64 sum = 0;
		for(u32 channelIndex = 0; channelIndex < channelCount; channelIndex++)
		{
			sum += AudioUtility::Convert24To32Bits(input);
			input += 3;
		}

		i32 average = (i32)(sum / channelCount);
		Convert32To24Bits(average, output);
		output += 3;
	}
}

void ConvertToMono32(const i32* input, i32* output, u32 sampleCount, u32 channelCount)
{
	for(u32 sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
	{
		i64 sum = 0;
		for(u32 channelIndex = 0; channelIndex < channelCount; channelIndex++)
		{
			sum += *input;
			++input;
		}

		*output = (i32)(sum / channelCount);
		++output;
	}
}

void Convert8To32Bits(const i8* input, i32* output, u32 sampleCount)
{
	for(u32 sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
	{
		i8 value = input[sampleIndex];
		output[sampleIndex] = value << 24;
	}
}

void Convert16To32Bits(const i16* input, i32* output, u32 sampleCount)
{
	for(u32 sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
		output[sampleIndex] = input[sampleIndex] << 16;
}

void Convert24To32Bits(const u8* input, i32* output, u32 sampleCount)
{
	for(u32 sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
	{
		output[sampleIndex] = AudioUtility::Convert24To32Bits(input);
		input += 3;
	}
}

void Convert32To8Bits(const i32* input, u8* output, u32 sampleCount)
{
	for(u32 sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
		output[sampleIndex] = (i8)(input[sampleIndex] >> 24);
}

void Convert32To16Bits(const i32* input, i16* output, u32 sampleCount)
{
	for(u32 sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
		output[sampleIndex] = (i16)(input[sampleIndex] >> 16);
}

void Convert32To24Bits(const i32* input, u8* output, u32 sampleCount)
{
	for(u32 sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
	{
		Convert32To24Bits(input[sampleIndex], output);
		output += 3;
	}
}

void AudioUtility::ConvertToMono(const u8* input, u8* outMono, u32 bitDepth, u32 sampleCount, u32 channelCount)
{
	switch(bitDepth)
	{
	case 8:
		ConvertToMono8((i8*)input, outMono, sampleCount, channelCount);
		break;
	case 16:
		ConvertToMono16((i16*)input, (i16*)outMono, sampleCount, channelCount);
		break;
	case 24:
		ConvertToMono24(input, outMono, sampleCount, channelCount);
		break;
	case 32:
		ConvertToMono32((i32*)input, (i32*)outMono, sampleCount, channelCount);
		break;
	default:
		B3D_ASSERT(false);
		break;
	}
}

void AudioUtility::ConvertBitDepth(const u8* input, u32 inputBitDepth, u8* outConverted, u32 outputBitDepth, u32 sampleCount)
{
	i32* sourceBuffer = nullptr;

	const bool needTempBuffer = inputBitDepth != 32;
	if(needTempBuffer)
		sourceBuffer = (i32*)B3DStackAllocate(sampleCount * sizeof(i32));
	else
		sourceBuffer = (i32*)input;

	// Note: I convert to a temporary 32-bit buffer and then use that to convert to actual requested bit depth.
	//       It would be more efficient to convert directly from source to requested depth without a temporary buffer,
	//       at the cost of additional complexity. If this method ever becomes a performance issue consider that.
	switch(inputBitDepth)
	{
	case 8:
		Convert8To32Bits((i8*)input, sourceBuffer, sampleCount);
		break;
	case 16:
		Convert16To32Bits((i16*)input, sourceBuffer, sampleCount);
		break;
	case 24:
		::Convert24To32Bits(input, sourceBuffer, sampleCount);
		break;
	case 32:
		// Do nothing
		break;
	default:
		B3D_ASSERT(false);
		break;
	}

	switch(outputBitDepth)
	{
	case 8:
		Convert32To8Bits(sourceBuffer, outConverted, sampleCount);
		break;
	case 16:
		Convert32To16Bits(sourceBuffer, (i16*)outConverted, sampleCount);
		break;
	case 24:
		Convert32To24Bits(sourceBuffer, outConverted, sampleCount);
		break;
	case 32:
		memcpy(outConverted, sourceBuffer, sampleCount * sizeof(i32));
		break;
	default:
		B3D_ASSERT(false);
		break;
	}

	if(needTempBuffer)
	{
		B3DStackFree(sourceBuffer);
		sourceBuffer = nullptr;
	}
}

void AudioUtility::ConvertToFloat(const u8* input, u32 inputBitDepth, float* outFloat, u32 sampleCount)
{
	if(inputBitDepth == 8)
	{
		for(u32 sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
		{
			i8 sample = *(i8*)input;
			outFloat[sampleIndex] = sample / 127.0f;

			input++;
		}
	}
	else if(inputBitDepth == 16)
	{
		for(u32 sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
		{
			i16 sample = *(i16*)input;
			outFloat[sampleIndex] = sample / 32767.0f;

			input += 2;
		}
	}
	else if(inputBitDepth == 24)
	{
		for(u32 sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
		{
			i32 sample = Convert24To32Bits(input);
			outFloat[sampleIndex] = sample / 2147483647.0f;

			input += 3;
		}
	}
	else if(inputBitDepth == 32)
	{
		for(u32 sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
		{
			i32 sample = *(i32*)input;
			outFloat[sampleIndex] = sample / 2147483647.0f;

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
