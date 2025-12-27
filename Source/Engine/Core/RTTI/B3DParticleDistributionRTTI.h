//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Particles/B3DParticleDistribution.h"
#include "RTTI/B3DAnimationCurveRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	template <>
	struct RTTIPlainType<ColorDistribution>
	{
		enum
		{
			id = TID_ColorDistribution
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const ColorDistribution& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr uint32_t kVersion = 0; // In case the data structure changes

			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += B3DRTTIWrite(kVersion, stream);
				size += B3DRTTIWrite(data.mType, stream);
				size += B3DRTTIWrite(data.mMinGradient, stream);
				size += B3DRTTIWrite(data.mMaxGradient, stream);

				return size; });
		}

		static BitLength FromMemory(ColorDistribution& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			uint32_t version;
			B3DRTTIRead(version, stream);

			switch(version)
			{
			case 0:
				B3DRTTIRead(data.mType, stream);
				B3DRTTIRead(data.mMinGradient, stream);
				B3DRTTIRead(data.mMaxGradient, stream);
				break;
			default:
				B3D_LOG(Error, RTTI, "Unknown version of TDistribution<T> data. Unable to deserialize.");
				break;
			}

			return size;
		}

		static BitLength GetSize(const ColorDistribution& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint32_t);
			dataSize += B3DRTTISize(data.mType);
			dataSize += B3DRTTISize(data.mMinGradient);
			dataSize += B3DRTTISize(data.mMaxGradient);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	template <class T>
	struct RTTIPlainType<TDistribution<T>>
	{
		enum
		{
			id = TID_TDistribution
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const TDistribution<T>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr uint32_t kVersion = 0; // In case the data structure changes

			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += B3DRTTIWrite(kVersion, stream);
				size += B3DRTTIWrite(data.mType, stream);
				size += B3DRTTIWrite(data.mMinCurve, stream);
				size += B3DRTTIWrite(data.mMaxCurve, stream);

				return size; });
		}

		static BitLength FromMemory(TDistribution<T>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			uint32_t version;
			B3DRTTIRead(version, stream);

			switch(version)
			{
			case 0:
				B3DRTTIRead(data.mType, stream);
				B3DRTTIRead(data.mMinCurve, stream);
				B3DRTTIRead(data.mMaxCurve, stream);
				break;
			default:
				B3D_LOG(Error, RTTI, "Unknown version of TDistribution<T> data. Unable to deserialize.");
				break;
			}

			return size;
		}

		static BitLength GetSize(const TDistribution<T>& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint32_t);
			dataSize += B3DRTTISize(data.mType);
			dataSize += B3DRTTISize(data.mMinCurve);
			dataSize += B3DRTTISize(data.mMaxCurve);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
