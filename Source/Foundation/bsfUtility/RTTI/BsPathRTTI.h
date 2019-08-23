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

		static uint32_t toMemory(const Path& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo)
		{
			return rtti_write_with_size_header(stream, [&data, &stream]()
			{
				uint32_t size = 0;
				size += rttiWriteElem(data.mDevice, stream);
				size += rttiWriteElem(data.mNode, stream);
				size += rttiWriteElem(data.mFilename, stream);
				size += rttiWriteElem(data.mIsAbsolute, stream);
				size += rttiWriteElem(data.mDirectories, stream);

				return size;
			});
		}

		static uint32_t fromMemory(Path& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo)
		{
			uint32_t size;
			rttiReadElem(size, stream);
			rttiReadElem(data.mDevice, stream);
			rttiReadElem(data.mNode, stream);
			rttiReadElem(data.mFilename, stream);
			rttiReadElem(data.mIsAbsolute, stream);
			rttiReadElem(data.mDirectories, stream);

			return size;
		}

		static uint32_t getDynamicSize(const Path& data)
		{
			uint64_t dataSize = (uint64_t)rttiGetElemSize(data.mDevice) + (uint64_t)rttiGetElemSize(data.mNode) + (uint64_t)rttiGetElemSize(data.mFilename) +
				(uint64_t)rttiGetElemSize(data.mIsAbsolute) + (uint64_t)rttiGetElemSize(data.mDirectories) + sizeof(uint32_t);

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
