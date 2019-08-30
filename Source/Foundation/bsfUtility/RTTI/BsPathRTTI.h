//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Reflection/BsRTTIPlain.h"
#include "RTTI/BsStringRTTI.h"
#include "RTTI/BsStdRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Utility
	*  @{
	*/

	template<> struct RTTIPlainType<Path>
	{
		enum { id = TID_Path }; enum { hasDynamicSize = 1 };

		static uint32_t toMemory(const Path& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return rtti_write_with_size_header(stream, [&data, &stream]()
			{
				uint32_t size = 0;
				size += rtti_write(data.mDevice, stream);
				size += rtti_write(data.mNode, stream);
				size += rtti_write(data.mFilename, stream);
				size += rtti_write(data.mIsAbsolute, stream);
				size += rtti_write(data.mDirectories, stream);

				return size;
			});
		}

		static uint32_t fromMemory(Path& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			uint32_t size;
			rtti_read(size, stream);
			rtti_read(data.mDevice, stream);
			rtti_read(data.mNode, stream);
			rtti_read(data.mFilename, stream);
			rtti_read(data.mIsAbsolute, stream);
			rtti_read(data.mDirectories, stream);

			return size;
		}

		static uint32_t getDynamicSize(const Path& data)
		{
			uint64_t dataSize = (uint64_t)rtti_size(data.mDevice) + (uint64_t)rtti_size(data.mNode) + (uint64_t)rtti_size(data.mFilename) +
				(uint64_t)rtti_size(data.mIsAbsolute) + (uint64_t)rtti_size(data.mDirectories) + sizeof(uint32_t);

#if BS_DEBUG_MODE
			if (dataSize > std::numeric_limits<uint32_t>::max())
			{
				__string_throwDataOverflowException();
			}
#endif

			return (uint32_t)dataSize;
		}
	};

	/** @} */
	/** @endcond */
}
