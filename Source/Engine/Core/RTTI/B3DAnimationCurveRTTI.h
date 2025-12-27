//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DFlagsRTTI.h"
#include "RTTI/B3DStdRTTI.h"
#include "Animation/B3DAnimationCurve.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
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
			B3DRTTIWrite(data.Value, stream);
			B3DRTTIWrite(data.InTangent, stream);
			B3DRTTIWrite(data.OutTangent, stream);
			B3DRTTIWrite(data.Time, stream);

			return sizeof(TKeyframe<T>);
		}

		static BitLength FromMemory(TKeyframe<T>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			B3DRTTIRead(data.Value, stream);
			B3DRTTIRead(data.InTangent, stream);
			B3DRTTIRead(data.OutTangent, stream);
			B3DRTTIRead(data.Time, stream);

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
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				constexpr uint32_t VERSION = 0; // In case the data structure changes

				BitLength size = 0;
				size += B3DRTTIWrite(VERSION, stream);
				size += B3DRTTIWrite(data.mStart, stream);
				size += B3DRTTIWrite(data.mEnd, stream);
				size += B3DRTTIWrite(data.mLength, stream);
				size += B3DRTTIWrite(data.mKeyframes, stream);

				return size; });
		}

		static BitLength FromMemory(TAnimationCurve<T>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			uint32_t version;
			B3DRTTIRead(version, stream);

			B3DRTTIRead(data.mStart, stream);
			B3DRTTIRead(data.mEnd, stream);
			B3DRTTIRead(data.mLength, stream);
			B3DRTTIRead(data.mKeyframes, stream);

			return size;
		}

		static BitLength GetSize(const TAnimationCurve<T>& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint32_t);
			dataSize += B3DRTTISize(data.mStart);
			dataSize += B3DRTTISize(data.mEnd);
			dataSize += B3DRTTISize(data.mLength);
			dataSize += B3DRTTISize(data.mKeyframes);
			B3DRTTIAddHeaderSize(dataSize, compress);

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
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += B3DRTTIWrite(data.Name, stream);
				size += B3DRTTIWrite(data.Flags, stream);
				size += B3DRTTIWrite(data.Curve, stream);

				return size; });
		}

		static BitLength FromMemory(TNamedAnimationCurve<T>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			B3DRTTIRead(data.Name, stream);
			B3DRTTIRead(data.Flags, stream);
			B3DRTTIRead(data.Curve, stream);

			return size;
		}

		static BitLength GetSize(const TNamedAnimationCurve<T>& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize;
			dataSize += B3DRTTISize(data.Name);
			dataSize += B3DRTTISize(data.Flags);
			dataSize += B3DRTTISize(data.Curve);
			B3DRTTIAddHeaderSize(dataSize, compress);

			return dataSize;
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
