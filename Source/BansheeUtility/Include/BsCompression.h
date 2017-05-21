//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisitesUtil.h"

namespace bs
{
	/** @addtogroup Utility-Core
	 *  @{
	 */

	/** Performs generic compression and decompression on raw data. */
	class BS_UTILITY_EXPORT Compression
	{
	public:
		/** Compresses the data from the provided data stream and outputs the new stream with compressed data. */
		static SPtr<MemoryDataStream> compress(SPtr<DataStream>& input);

		/** Decompresses the data from the provided data stream and outputs the new stream with decompressed data. */
		static SPtr<MemoryDataStream> decompress(SPtr<DataStream>& input);
	};

	/** @} */
}