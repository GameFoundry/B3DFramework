//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Image/BsColorGradient.h"
#include "Error/BsException.h"
#include "Debug/BsDebug.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Utility
	*  @{
	*/

	template<> struct RTTIPlainType<ColorGradient>
	{
		enum { id = TID_ColorGradient }; enum { hasDynamicSize = 1 };

		static void toMemory(const ColorGradient& data, char* memory)
		{
			UINT32 size = getDynamicSize(data);

			const UINT32 curSize = sizeof(UINT32);
			memcpy(memory, &size, curSize);
			memory += curSize;

			const UINT32 version = 0;
			memory = rttiWriteElem(version, memory);

			for(UINT32 i = 0; i < ColorGradient::MAX_KEYS; i++)
			{
				memory = rttiWriteElem(data.mColors[i], memory);
				memory = rttiWriteElem(data.mTimes[i], memory);
			}

			memory = rttiWriteElem(data.mNumKeys, memory);
			memory = rttiWriteElem(data.mDuration, memory);
		}

		static UINT32 fromMemory(ColorGradient& data, char* memory)
		{
			UINT32 size;
			memcpy(&size, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			UINT32 version;
			memory = rttiReadElem(version, memory);

			switch(version)
			{
			case 0:
				for (UINT32 i = 0; i < ColorGradient::MAX_KEYS; i++)
				{
					memory = rttiReadElem(data.mColors[i], memory);
					memory = rttiReadElem(data.mTimes[i], memory);
				}

				memory = rttiReadElem(data.mNumKeys, memory);
				memory = rttiReadElem(data.mDuration, memory);
				break;
			default:
				LOGERR("Unknown version of ColorGradient data. Unable to deserialize.");
				break;
			}

			return size;
		}

		static UINT32 getDynamicSize(const ColorGradient& data)
		{
			const UINT64 dataSize = 
				rttiGetElemSize(data.mColors[0]) * ColorGradient::MAX_KEYS + 
				rttiGetElemSize(data.mTimes[0]) * ColorGradient::MAX_KEYS +
				rttiGetElemSize(data.mNumKeys) + rttiGetElemSize(data.mDuration) + sizeof(UINT32) * 2;

#if BS_DEBUG_MODE
			if(dataSize > std::numeric_limits<UINT32>::max())
			{
				BS_EXCEPT(InternalErrorException, "Data overflow! Size doesn't fit into 32 bits.");
			}
#endif

			return (UINT32)dataSize;
		}
	};

	/** @} */
	/** @endcond */
}
