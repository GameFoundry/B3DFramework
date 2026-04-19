//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
{
	/** @addtogroup Audio
	 *  @{
	 */

	/** Provides various utility functionality relating to audio. */
	class B3D_EXPORT AudioUtility
	{
	public:
		/**
		 * Converts a set of audio samples using multiple channels into a set of mono samples.
		 *
		 * @param	input			A set of input samples. Per-channels samples should be interleaved. Size of each sample
		 *							is determined by @p bitDepth. Total size of the buffer should be @p sampleCount *
		 *							@p channelCount * @p bitDepth / 8.
		 * @param	outMono			Pre-allocated buffer to store the mono samples. Should be of @p sampleCount *
		 *							@p bitDepth / 8 size.
		 * @param	bitDepth		Size of a single sample in bits.
		 * @param	sampleCount		Number of samples per a single channel.
		 * @param	channelCount	Number of channels in the input data.
		 */
		static void ConvertToMono(const u8* input, u8* outMono, u32 bitDepth, u32 sampleCount, u32 channelCount);

		/**
		 * Converts a set of audio samples of a certain bit depth to a new bit depth.
		 *
		 * @param	input			A set of input samples. Total size of the buffer should be @p sampleCount *
		 *							@p inputBitDepth / 8.
		 * @param	inputBitDepth	Size of a single sample in the @p input array, in bits.
		 * @param	outConverted	Pre-allocated buffer to store the output samples in. Total size of the buffer should be
		 *							@p sampleCount * @p outputBitDepth / 8.
		 * @param	outputBitDepth	Size of a single sample in the @p outConverted array, in bits.
		 * @param	sampleCount		Total number of samples to process.
		 */
		static void ConvertBitDepth(const u8* input, u32 inputBitDepth, u8* outConverted, u32 outputBitDepth, u32 sampleCount);

		/**
		 * Converts a set of audio samples of a certain bit depth to a set of floating point samples in range [-1, 1].
		 *
		 * @param	input			A set of input samples. Total size of the buffer should be @p sampleCount *
		 *							@p inputBitDepth / 8. All input samples should be signed integers.
		 * @param	inputBitDepth	Size of a single sample in the @p input array, in bits.
		 * @param	outFloat		Pre-allocated buffer to store the output samples in. Total size of the buffer should be
		 *							@p sampleCount * sizeof(float).
		 * @param	sampleCount		Total number of samples to process.
		 */
		static void ConvertToFloat(const u8* input, u32 inputBitDepth, float* outFloat, u32 sampleCount);

		/**
		 * Converts a 24-bit signed integer into a 32-bit signed integer.
		 *
		 * @param	input	24-bit signed integer as an array of 3 bytes.
		 * @return			32-bit signed integer.
		 */
		static i32 Convert24To32Bits(const u8* input);
	};

	/** @} */
} // namespace b3d
