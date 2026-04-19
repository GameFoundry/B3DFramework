//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

namespace b3d
{
	/** @addtogroup Serialization
	 *  @{
	 */

	/** Supported compression modes. */
	enum class B3D_SCRIPT_EXPORT() CompressionType
	{
		Uncompressed = 0,
		Snappy = 1,
		Default = Snappy
	};

	/** Performs generic compression and decompression on raw data. */
	class B3D_EXPORT Compression
	{
	public:
		/**
		 * Compresses the data from the provided data stream and outputs the new stream with compressed data.
		 *
		 * @param	input				Input stream from which to read the data to compress. Data will be read from the current cursor position of the stream. Stream cursor will be advanced as the data is read.
		 * @param	output				Output stream into which to write the compressed data.
		 * @param	inputDataSize		Size of the data to read from the input. Once this size is reached we will end the process. If 0, we will compress until the input stream end is reached.
		 * @param	compressionType		Type of compression to use.
		 * @param	reportProgress		Optional callback to trigger reporting of the compression progress. Reported value will be in range [0, 1].
		 * @return						Size of the compressed data written.
		 */
		static u64 Compress(DataStream& input, DataStream& output, u64 inputDataSize = 0, CompressionType compressionType = CompressionType::Default, std::function<void(float)> reportProgress = nullptr);

		/**
		 * Decompresses the data from the provided data stream and outputs the new stream with decompressed data. 
		 *
		 * @param	input				Input stream from which to read the data to decompress. Data will be read from the current cursor position of the stream. Stream cursor will be advanced as the data is read.
		 * @param	output				Output stream into which to write the decompressed data.
		 * @param	inputDataSize		Size of the data to read from the input. Once this size is reached we will end the process. If 0, we will decompress until the input stream end is reached.
		 * @param	compressionType		Type of compression used on the compressed data.
		 * @param	reportProgress		Optional callback to trigger reporting of the decompression progress. Reported value will be in range [0, 1].
		 * @return						True if successful.
		 */
		static bool Decompress(DataStream& input, DataStream& output, u64 inputDataSize = 0, CompressionType compressionType = CompressionType::Default, std::function<void(float)> reportProgress = nullptr);
	};

	/** @} */
} // namespace b3d
