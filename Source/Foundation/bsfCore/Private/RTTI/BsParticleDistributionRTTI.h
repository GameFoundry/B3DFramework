//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Particles/BsParticleDistribution.h"
#include "Private/RTTI/BsAnimationCurveRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	*  @{
	*/

	template<> struct RTTIPlainType<ColorDistribution>
	{
		enum { id = TID_ColorDistribution }; enum { hasDynamicSize = 1 };

		/** @copydoc RTTIPlainType::toMemory */
		static void toMemory(const ColorDistribution& data, char* memory)
		{
			UINT32 size = sizeof(UINT32);
			char* memoryStart = memory;
			memory += sizeof(UINT32);

			UINT32 version = 0; // In case the data structure changes
			memory = rttiWriteElem(version, memory, size);
			memory = rttiWriteElem(data.mType, memory, size);
			memory = rttiWriteElem(data.mMinGradient, memory, size);
			memory = rttiWriteElem(data.mMaxGradient, memory, size);

			memcpy(memoryStart, &size, sizeof(UINT32));
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static UINT32 fromMemory(ColorDistribution& data, char* memory)
		{
			UINT32 size = 0;
			memory = rttiReadElem(size, memory);

			UINT32 version;
			memory = rttiReadElem(version, memory);

			switch(version)
			{
			case 0:
				memory = rttiReadElem(data.mType, memory);
				memory = rttiReadElem(data.mMinGradient, memory);
				memory = rttiReadElem(data.mMaxGradient, memory);
				break;
			default:
				LOGERR("Unknown version of TDistribution<T> data. Unable to deserialize.");
				break;
			}

			return size;
		}

		/** @copydoc RTTIPlainType::getDynamicSize */
		static UINT32 getDynamicSize(const ColorDistribution& data)
		{
			UINT64 dataSize = sizeof(UINT32) + sizeof(UINT32);
			dataSize += rttiGetElemSize(data.mType);
			dataSize += rttiGetElemSize(data.mMinGradient);
			dataSize += rttiGetElemSize(data.mMaxGradient);

			assert(dataSize <= std::numeric_limits<UINT32>::max());

			return (UINT32)dataSize;
		}
	};

	template<class T> struct RTTIPlainType<TDistribution<T>>
	{
		enum { id = TID_TDistribution }; enum { hasDynamicSize = 1 };

		/** @copydoc RTTIPlainType::toMemory */
		static void toMemory(const TDistribution<T>& data, char* memory)
		{
			UINT32 size = sizeof(UINT32);
			char* memoryStart = memory;
			memory += sizeof(UINT32);

			UINT32 version = 0; // In case the data structure changes
			memory = rttiWriteElem(version, memory, size);
			memory = rttiWriteElem(data.mType, memory, size);
			memory = rttiWriteElem(data.mMinCurve, memory, size);
			memory = rttiWriteElem(data.mMaxCurve, memory, size);

			memcpy(memoryStart, &size, sizeof(UINT32));
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static UINT32 fromMemory(TDistribution<T>& data, char* memory)
		{
			UINT32 size = 0;
			memory = rttiReadElem(size, memory);

			UINT32 version;
			memory = rttiReadElem(version, memory);

			switch(version)
			{
			case 0:
				memory = rttiReadElem(data.mType, memory);
				memory = rttiReadElem(data.mMinCurve, memory);
				memory = rttiReadElem(data.mMaxCurve, memory);
				break;
			default:
				LOGERR("Unknown version of TDistribution<T> data. Unable to deserialize.");
				break;
			}

			return size;
		}

		/** @copydoc RTTIPlainType::getDynamicSize */
		static UINT32 getDynamicSize(const TDistribution<T>& data)
		{
			UINT64 dataSize = sizeof(UINT32) + sizeof(UINT32);
			dataSize += rttiGetElemSize(data.mType);
			dataSize += rttiGetElemSize(data.mMinCurve);
			dataSize += rttiGetElemSize(data.mMaxCurve);

			assert(dataSize <= std::numeric_limits<UINT32>::max());

			return (UINT32)dataSize;
		}
	};

	/** @} */
	/** @endcond */
}
