//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsAnimationCurve.h"

namespace BansheeEngine
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	template<class T> struct RTTIPlainType<TKeyframe<T>>
	{
		enum { id = TID_KeyFrame }; enum { hasDynamicSize = 0 };

		/** @copydoc RTTIPlainType::toMemory */
		static void toMemory(const TKeyframe<T>& data, char* memory)
		{
			memory = rttiWriteElem(data.value, memory);
			memory = rttiWriteElem(data.inTangent, memory);
			memory = rttiWriteElem(data.outTangent, memory);
			memory = rttiWriteElem(data.time, memory);
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static UINT32 fromMemory(TKeyframe<T>& data, char* memory)
		{
			memory = rttiReadElem(data.value, memory);
			memory = rttiReadElem(data.inTangent, memory);
			memory = rttiReadElem(data.outTangent, memory);
			memory = rttiReadElem(data.time, memory);
			
			return sizeof(TKeyframe<T>);
		}

		/** @copydoc RTTIPlainType::getDynamicSize */
		static UINT32 getDynamicSize(const TKeyframe<T>& data)
		{
			assert(false);
			return sizeof(TKeyframe<T>);
		}
	};

	template<class T> struct RTTIPlainType<TAnimationCurve<T>>
	{
		enum { id = TID_AnimationCurve }; enum { hasDynamicSize = 1 };

		/** @copydoc RTTIPlainType::toMemory */
		static void toMemory(const TAnimationCurve<T>& data, char* memory)
		{
			UINT32 size = sizeof(UINT32);
			char* memoryStart = memory;
			memory += sizeof(UINT32);

			UINT32 version = 0; // In case the data structure changes
			memory = rttiWriteElem(version, memory, size);
			memory = rttiWriteElem(data.mStart, memory, size);
			memory = rttiWriteElem(data.mEnd, memory, size);
			memory = rttiWriteElem(data.mLength, memory, size);
			memory = rttiWriteElem(data.mKeyframes, memory, size);

			memcpy(memoryStart, &size, sizeof(UINT32));
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static UINT32 fromMemory(TAnimationCurve<T>& data, char* memory)
		{
			UINT32 size = 0;
			memory = rttiReadElem(size, memory);

			UINT32 version;
			memory = rttiReadElem(version, memory);

			memory = rttiReadElem(data.mStart, memory);
			memory = rttiReadElem(data.mEnd, memory);
			memory = rttiReadElem(data.mLength, memory);
			memory = rttiReadElem(data.mKeyframes, memory);

			return size;
		}

		/** @copydoc RTTIPlainType::getDynamicSize */
		static UINT32 getDynamicSize(const TAnimationCurve<T>& data)
		{
			UINT64 dataSize = sizeof(UINT32) + sizeof(UINT32);
			dataSize += rttiGetElemSize(data.mStart);
			dataSize += rttiGetElemSize(data.mEnd);
			dataSize += rttiGetElemSize(data.mLength);
			dataSize += rttiGetElemSize(data.mKeyframes);

			assert(dataSize <= std::numeric_limits<UINT32>::max());

			return (UINT32)dataSize;
		}
	};

	template<class T> struct RTTIPlainType<TNamedAnimationCurve<T>>
	{
		enum { id = TID_NamedAnimationCurve }; enum { hasDynamicSize = 1 };

		/** @copydoc RTTIPlainType::toMemory */
		static void toMemory(const TNamedAnimationCurve<T>& data, char* memory)
		{
			UINT32 size = sizeof(UINT32);
			char* memoryStart = memory;
			memory += sizeof(UINT32);

			memory = rttiWriteElem(data.name, memory, size);
			memory = rttiWriteElem(data.flags, memory, size);
			memory = rttiWriteElem(data.curve, memory, size);

			memcpy(memoryStart, &size, sizeof(UINT32));
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static UINT32 fromMemory(TNamedAnimationCurve<T>& data, char* memory)
		{
			UINT32 size = 0;
			memory = rttiReadElem(size, memory);

			memory = rttiReadElem(data.name, memory);
			memory = rttiReadElem(data.flags, memory);
			memory = rttiReadElem(data.curve, memory);

			return size;
		}

		/** @copydoc RTTIPlainType::getDynamicSize */
		static UINT32 getDynamicSize(const TNamedAnimationCurve<T>& data)
		{
			UINT64 dataSize = sizeof(UINT32);
			dataSize += rttiGetElemSize(data.name);
			dataSize += rttiGetElemSize(data.flags);
			dataSize += rttiGetElemSize(data.curve);

			assert(dataSize <= std::numeric_limits<UINT32>::max());

			return (UINT32)dataSize;
		}
	};
	
	/** @} */
	/** @endcond */
}