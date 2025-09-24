//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"

namespace b3d
{
	/** @addtogroup Audio
	 *  @{
	 */

	/** Provides various utility functionality relating to audio. */
	class B3D_CORE_EXPORT AudioUtility
	{
	public:
		/**
		 * Converts a set of audio samples using multiple channels into a set of mono samples.
		 *
		 * @param[in]	input		A set of input samples. Per-channels samples should be interleaved. Size of each sample
		 *							is determined by @p bitDepth. Total size of the buffer should be @p numSamples *
		 *							@p numChannels * @p bitDepth / 8.
		 * @param[out]	output		Pre-allocated buffer to store the mono samples. Should be of @p numSamples *
		 *							@p bitDepth / 8 size.
		 * @param[in]	bitDepth	Size of a single sample in bits.
		 * @param[in]	numSamples	Number of samples per a single channel.
		 * @param[in]	numChannels	Number of channels in the input data.
		 */
		static void ConvertToMono(const u8* input, u8* output, u32 bitDepth, u32 numSamples, u32 numChannels);

		/**
		 * Converts a set of audio samples of a certain bit depth to a new bit depth.
		 *
		 * @param[in]	input		A set of input samples. Total size of the buffer should be @p numSamples *
		 *							@p inBitDepth / 8.
		 * @param[in]	inBitDepth	Size of a single sample in the @p input array, in bits.
		 * @param[out]	output		Pre-allocated buffer to store the output samples in. Total size of the buffer should be
		 *							@p numSamples * @p outBitDepth / 8.
		 * @param[in]	outBitDepth	Size of a single sample in the @p output array, in bits.
		 * @param[in]	numSamples	Total number of samples to process.
		 */
		static void ConvertBitDepth(const u8* input, u32 inBitDepth, u8* output, u32 outBitDepth, u32 numSamples);

		/**
		 * Converts a set of audio samples of a certain bit depth to a set of floating point samples in range [-1, 1].
		 *
		 * @param[in]	input		A set of input samples. Total size of the buffer should be @p numSamples *
		 *							@p inBitDepth / 8. All input samples should be signed integers.
		 * @param[in]	inBitDepth	Size of a single sample in the @p input array, in bits.
		 * @param[out]	output		Pre-allocated buffer to store the output samples in. Total size of the buffer should be
		 *							@p numSamples * sizeof(float).
		 * @param[in]	numSamples	Total number of samples to process.
		 */
		static void ConvertToFloat(const u8* input, u32 inBitDepth, float* output, u32 numSamples);

		/**
		 * Converts a 24-bit signed integer into a 32-bit signed integer.
		 *
		 * @param[in]	input	24-bit signed integer as an array of 3 bytes.
		 * @return				32-bit signed integer.
		 */
		static i32 Convert24To32Bits(const u8* input);
	};

	/** @} */
} // namespace b3d
