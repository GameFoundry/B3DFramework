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

		static void toMemory(const Path& data, char* memory)
		{
			UINT32 size = getDynamicSize(data);
			memcpy(memory, &size, sizeof(UINT32));
			memory += sizeof(UINT32);

			memory = rttiWriteElem(data.mDevice, memory);
			memory = rttiWriteElem(data.mNode, memory);
			memory = rttiWriteElem(data.mFilename, memory);
			memory = rttiWriteElem(data.mIsAbsolute, memory);
			rttiWriteElem(data.mDirectories, memory);
		}

		static UINT32 fromMemory(Path& data, char* memory)
		{
			UINT32 size;
			memcpy(&size, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			memory = rttiReadElem(data.mDevice, memory);
			memory = rttiReadElem(data.mNode, memory);
			memory = rttiReadElem(data.mFilename, memory);
			memory = rttiReadElem(data.mIsAbsolute, memory);
			rttiReadElem(data.mDirectories, memory);

			return size;
		}

		static UINT32 getDynamicSize(const Path& data)
		{
			UINT64 dataSize = (UINT64)rttiGetElemSize(data.mDevice) + (UINT64)rttiGetElemSize(data.mNode) + (UINT64)rttiGetElemSize(data.mFilename) +
				(UINT64)rttiGetElemSize(data.mIsAbsolute) + (UINT64)rttiGetElemSize(data.mDirectories) + sizeof(UINT32);

#if BS_DEBUG_MODE
			if (dataSize > std::numeric_limits<UINT32>::max())
			{
				__string_throwDataOverflowException();
			}
#endif

			return (UINT32)dataSize;
		}
	};

	/** @} */
	/** @endcond */
}
