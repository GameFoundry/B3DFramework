//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Reflection/BsRTTIPlain.h"
#include "RTTI/BsColorRTTI.h"
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

		static BitLength toMemory(const ColorGradient& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr uint32_t VERSION = 0;

			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
			{
				BitLength size = 0;
				size += rtti_write(VERSION, stream);

				for (uint32_t i = 0; i < ColorGradient::MAX_KEYS; i++)
				{
					size += rtti_write(data.mColors[i], stream);
					size += rtti_write(data.mTimes[i], stream);
				}

				size += rtti_write(data.mNumKeys, stream);
				size += rtti_write(data.mDuration, stream);

				return size;
			});
		}

		static BitLength fromMemory(ColorGradient& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			uint32_t version;
			rtti_read(version, stream);

			switch(version)
			{
			case 0:
				for (uint32_t i = 0; i < ColorGradient::MAX_KEYS; i++)
				{
					rtti_read(data.mColors[i], stream);
					rtti_read(data.mTimes[i], stream);
				}

				rtti_read(data.mNumKeys, stream);
				rtti_read(data.mDuration, stream);
				break;
			default:
				BS_LOG(Error, RTTI, "Unknown version of ColorGradient data. Unable to deserialize.");
				break;
			}

			return size;
		}

		static BitLength getSize(const ColorGradient& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize =
				rtti_size(data.mColors[0]) * ColorGradient::MAX_KEYS +
				rtti_size(data.mTimes[0]) * ColorGradient::MAX_KEYS +
				rtti_size(data.mNumKeys) + rtti_size(data.mDuration) + sizeof(uint32_t);

			rtti_add_header_size(dataSize, compress);
			return dataSize;
		}
	};

	template<> struct RTTIPlainType<ColorGradientHDR>
	{
		enum { id = TID_ColorGradientHDR }; enum { hasDynamicSize = 1 };

		static BitLength toMemory(const ColorGradientHDR& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr uint32_t VERSION = 0;

			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
			{
				BitLength size = 0;
				size += rtti_write(VERSION, stream);

				for(uint32_t i = 0; i < ColorGradientHDR::MAX_KEYS; i++)
				{
					size += rtti_write(data.mColors[i], stream);
					size += rtti_write(data.mTimes[i], stream);
				}

				size += rtti_write(data.mNumKeys, stream);
				size += rtti_write(data.mDuration, stream);

				return size;
			});
		}

		static BitLength fromMemory(ColorGradientHDR& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			uint32_t version;
			rtti_read(version, stream);

			switch(version)
			{
			case 0:
				for (uint32_t i = 0; i < ColorGradientHDR::MAX_KEYS; i++)
				{
					rtti_read(data.mColors[i], stream);
					rtti_read(data.mTimes[i], stream);
				}

				rtti_read(data.mNumKeys, stream);
				rtti_read(data.mDuration, stream);
				break;
			default:
				BS_LOG(Error, RTTI, "Unknown version of ColorGradientHDR data. Unable to deserialize.");
				break;
			}

			return size;
		}

		static BitLength getSize(const ColorGradientHDR& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize =
				rtti_size(data.mColors[0]) * ColorGradientHDR::MAX_KEYS +
				rtti_size(data.mTimes[0]) * ColorGradientHDR::MAX_KEYS +
				rtti_size(data.mNumKeys) + rtti_size(data.mDuration) + sizeof(uint32_t);

			rtti_add_header_size(dataSize, compress);
			return dataSize;
		}
	};

	/** @} */
	/** @endcond */
}
