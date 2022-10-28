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

	template <class T>
	struct RTTIPlainType<TKeyframe<T>>
	{
		enum
		{
			id = TID_KeyFrame
		};

		enum
		{
			hasDynamicSize = 0
		};

		static BitLength ToMemory(const TKeyframe<T>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			rtti_write(data.Value, stream);
			rtti_write(data.InTangent, stream);
			rtti_write(data.OutTangent, stream);
			rtti_write(data.Time, stream);

			return sizeof(TKeyframe<T>);
		}

		static BitLength FromMemory(TKeyframe<T>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			rtti_read(data.Value, stream);
			rtti_read(data.InTangent, stream);
			rtti_read(data.OutTangent, stream);
			rtti_read(data.Time, stream);

			return sizeof(TKeyframe<T>);
		}

		static BitLength GetSize(const TKeyframe<T>& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return sizeof(TKeyframe<T>);
		}
	};

	template <class T>
	struct RTTIPlainType<TAnimationCurve<T>>
	{
		enum
		{
			id = TID_AnimationCurve
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const TAnimationCurve<T>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
											   {
				constexpr uint32_t VERSION = 0; // In case the data structure changes

				BitLength size = 0;
				size += rtti_write(VERSION, stream);
				size += rtti_write(data.mStart, stream);
				size += rtti_write(data.mEnd, stream);
				size += rtti_write(data.mLength, stream);
				size += rtti_write(data.mKeyframes, stream);

				return size; });
		}

		static BitLength FromMemory(TAnimationCurve<T>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			uint32_t version;
			rtti_read(version, stream);

			rtti_read(data.mStart, stream);
			rtti_read(data.mEnd, stream);
			rtti_read(data.mLength, stream);
			rtti_read(data.mKeyframes, stream);

			return size;
		}

		static BitLength GetSize(const TAnimationCurve<T>& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint32_t);
			dataSize += rtti_size(data.mStart);
			dataSize += rtti_size(data.mEnd);
			dataSize += rtti_size(data.mLength);
			dataSize += rtti_size(data.mKeyframes);
			rtti_add_header_size(dataSize, compress);

			return dataSize;
		}
	};

	template <class T>
	struct RTTIPlainType<TNamedAnimationCurve<T>>
	{
		enum
		{
			id = TID_NamedAnimationCurve
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const TNamedAnimationCurve<T>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += rtti_write(data.Name, stream);
				size += rtti_write(data.Flags, stream);
				size += rtti_write(data.Curve, stream);

				return size; });
		}

		static BitLength FromMemory(TNamedAnimationCurve<T>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			rtti_read(data.Name, stream);
			rtti_read(data.Flags, stream);
			rtti_read(data.Curve, stream);

			return size;
		}

		static BitLength GetSize(const TNamedAnimationCurve<T>& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize;
			dataSize += rtti_size(data.Name);
			dataSize += rtti_size(data.Flags);
			dataSize += rtti_size(data.Curve);
			rtti_add_header_size(dataSize, compress);

			return dataSize;
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
