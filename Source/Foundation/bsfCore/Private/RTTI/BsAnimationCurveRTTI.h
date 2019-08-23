//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsFlagsRTTI.h"
#include "RTTI/BsStdRTTI.h"
#include "Animation/BsAnimationCurve.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	template<class T> struct RTTIPlainType<TKeyframe<T>>
	{
		enum { id = TID_KeyFrame }; enum { hasDynamicSize = 0 };

		/** @copydoc RTTIPlainType::toMemory */
		static uint32_t toMemory(const TKeyframe<T>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo)
		{
			rttiWriteElem(data.value, stream);
			rttiWriteElem(data.inTangent, stream);
			rttiWriteElem(data.outTangent, stream);
			rttiWriteElem(data.time, stream);

			return sizeof(TKeyframe<T>);
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static uint32_t fromMemory(TKeyframe<T>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo)
		{
			rttiReadElem(data.value, stream);
			rttiReadElem(data.inTangent, stream);
			rttiReadElem(data.outTangent, stream);
			rttiReadElem(data.time, stream);
			
			return sizeof(TKeyframe<T>);
		}

		/** @copydoc RTTIPlainType::getDynamicSize */
		static uint32_t getDynamicSize(const TKeyframe<T>& data)
		{
			assert(false);
			return sizeof(TKeyframe<T>);
		}
	};

	template<class T> struct RTTIPlainType<TAnimationCurve<T>>
	{
		enum { id = TID_AnimationCurve }; enum { hasDynamicSize = 1 };

		/** @copydoc RTTIPlainType::toMemory */
		static uint32_t toMemory(const TAnimationCurve<T>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo)
		{
			return rtti_write_with_size_header(stream, [&data, &stream]()
			{
				constexpr uint32_t VERSION = 0; // In case the data structure changes

				uint32_t size = 0;
				size += rttiWriteElem(VERSION, stream);
				size += rttiWriteElem(data.mStart, stream);
				size += rttiWriteElem(data.mEnd, stream);
				size += rttiWriteElem(data.mLength, stream);
				size += rttiWriteElem(data.mKeyframes, stream);

				return size;
			});
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static uint32_t fromMemory(TAnimationCurve<T>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo)
		{
			uint32_t size = 0;
			rttiReadElem(size, stream);

			uint32_t version;
			rttiReadElem(version, stream);

			rttiReadElem(data.mStart, stream);
			rttiReadElem(data.mEnd, stream);
			rttiReadElem(data.mLength, stream);
			rttiReadElem(data.mKeyframes, stream);

			return size;
		}

		/** @copydoc RTTIPlainType::getDynamicSize */
		static uint32_t getDynamicSize(const TAnimationCurve<T>& data)
		{
			uint64_t dataSize = sizeof(uint32_t) + sizeof(uint32_t);
			dataSize += rttiGetElemSize(data.mStart);
			dataSize += rttiGetElemSize(data.mEnd);
			dataSize += rttiGetElemSize(data.mLength);
			dataSize += rttiGetElemSize(data.mKeyframes);

			assert(dataSize <= std::numeric_limits<uint32_t>::max());

			return (uint32_t)dataSize;
		}
	};

	template<class T> struct RTTIPlainType<TNamedAnimationCurve<T>>
	{
		enum { id = TID_NamedAnimationCurve }; enum { hasDynamicSize = 1 };

		/** @copydoc RTTIPlainType::toMemory */
		static uint32_t toMemory(const TNamedAnimationCurve<T>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo)
		{
			return rtti_write_with_size_header(stream, [&data, &stream]()
			{
				uint32_t size = 0;
				size += rttiWriteElem(data.name, stream);
				size += rttiWriteElem(data.flags, stream);
				size += rttiWriteElem(data.curve, stream);

				return size;
			});
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static uint32_t fromMemory(TNamedAnimationCurve<T>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo)
		{
			UINT32 size = 0;
			rttiReadElem(size, stream);

			rttiReadElem(data.name, stream);
			rttiReadElem(data.flags, stream);
			rttiReadElem(data.curve, stream);

			return size;
		}

		/** @copydoc RTTIPlainType::getDynamicSize */
		static uint32_t getDynamicSize(const TNamedAnimationCurve<T>& data)
		{
			uint64_t dataSize = sizeof(uint32_t);
			dataSize += rttiGetElemSize(data.name);
			dataSize += rttiGetElemSize(data.flags);
			dataSize += rttiGetElemSize(data.curve);

			assert(dataSize <= std::numeric_limits<uint32_t>::max());

			return (uint32_t)dataSize;
		}
	};
	
	/** @} */
	/** @endcond */
}
