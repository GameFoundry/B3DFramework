//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Reflection/B3DRTTIPlain.h"
#include "RTTI/B3DColorRTTI.h"
#include "Image/B3DColorGradient.h"
#include "Error/B3DException.h"
#include "Debug/B3DDebug.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Utility
	 *  @{
	 */

	template <>
	struct RTTIPlainType<ColorGradient>
	{
		enum
		{
			id = TID_ColorGradient
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const ColorGradient& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr uint32_t kVersion = 0;

			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += B3DRTTIWrite(kVersion, stream);

				for (uint32_t i = 0; i < ColorGradient::kMaxKeys; i++)
				{
					size += B3DRTTIWrite(data.mColors[i], stream);
					size += B3DRTTIWrite(data.mTimes[i], stream);
				}

				size += B3DRTTIWrite(data.mNumKeys, stream);
				size += B3DRTTIWrite(data.mDuration, stream);

				return size; });
		}

		static BitLength FromMemory(ColorGradient& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			uint32_t version;
			B3DRTTIRead(version, stream);

			switch(version)
			{
			case 0:
				for(uint32_t i = 0; i < ColorGradient::kMaxKeys; i++)
				{
					B3DRTTIRead(data.mColors[i], stream);
					B3DRTTIRead(data.mTimes[i], stream);
				}

				B3DRTTIRead(data.mNumKeys, stream);
				B3DRTTIRead(data.mDuration, stream);
				break;
			default:
				B3D_LOG(Error, RTTI, "Unknown version of ColorGradient data. Unable to deserialize.");
				break;
			}

			return size;
		}

		static BitLength GetSize(const ColorGradient& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize =
				B3DRTTISize(data.mColors[0]) * ColorGradient::kMaxKeys +
				B3DRTTISize(data.mTimes[0]) * ColorGradient::kMaxKeys +
				B3DRTTISize(data.mNumKeys) + B3DRTTISize(data.mDuration) + sizeof(uint32_t);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	template <>
	struct RTTIPlainType<ColorGradientHDR>
	{
		enum
		{
			id = TID_ColorGradientHDR
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const ColorGradientHDR& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr uint32_t kVersion = 0;

			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += B3DRTTIWrite(kVersion, stream);

				for(uint32_t i = 0; i < ColorGradientHDR::kMaxKeys; i++)
				{
					size += B3DRTTIWrite(data.mColors[i], stream);
					size += B3DRTTIWrite(data.mTimes[i], stream);
				}

				size += B3DRTTIWrite(data.mNumKeys, stream);
				size += B3DRTTIWrite(data.mDuration, stream);

				return size; });
		}

		static BitLength FromMemory(ColorGradientHDR& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			uint32_t version;
			B3DRTTIRead(version, stream);

			switch(version)
			{
			case 0:
				for(uint32_t i = 0; i < ColorGradientHDR::kMaxKeys; i++)
				{
					B3DRTTIRead(data.mColors[i], stream);
					B3DRTTIRead(data.mTimes[i], stream);
				}

				B3DRTTIRead(data.mNumKeys, stream);
				B3DRTTIRead(data.mDuration, stream);
				break;
			default:
				B3D_LOG(Error, RTTI, "Unknown version of ColorGradientHDR data. Unable to deserialize.");
				break;
			}

			return size;
		}

		static BitLength GetSize(const ColorGradientHDR& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize =
				B3DRTTISize(data.mColors[0]) * ColorGradientHDR::kMaxKeys +
				B3DRTTISize(data.mTimes[0]) * ColorGradientHDR::kMaxKeys +
				B3DRTTISize(data.mNumKeys) + B3DRTTISize(data.mDuration) + sizeof(uint32_t);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
