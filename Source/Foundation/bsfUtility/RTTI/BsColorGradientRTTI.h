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

		static uint32_t toMemory(const ColorGradient& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr uint32_t VERSION = 0;

			return rtti_write_with_size_header(stream, [&data, &stream]()
			{
				uint32_t size = 0;
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

		static uint32_t fromMemory(ColorGradient& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			uint32_t size;
			rtti_read(size, stream);

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

		static uint32_t getSize(const ColorGradient& data)
		{
			const uint64_t dataSize =
				rtti_size(data.mColors[0]) * ColorGradient::MAX_KEYS +
				rtti_size(data.mTimes[0]) * ColorGradient::MAX_KEYS +
				rtti_size(data.mNumKeys) + rtti_size(data.mDuration) + sizeof(uint32_t) * 2;

#if BS_DEBUG_MODE
			if(dataSize > std::numeric_limits<uint32_t>::max())
			{
				BS_EXCEPT(InternalErrorException, "Data overflow! Size doesn't fit into 32 bits.");
			}
#endif

			return (uint32_t)dataSize;
		}
	};

	template<> struct RTTIPlainType<ColorGradientHDR>
	{
		enum { id = TID_ColorGradientHDR }; enum { hasDynamicSize = 1 };

		static uint32_t toMemory(const ColorGradientHDR& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr uint32_t VERSION = 0;

			return rtti_write_with_size_header(stream, [&data, &stream]()
			{
				uint32_t size = 0;
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

		static uint32_t fromMemory(ColorGradientHDR& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			uint32_t size;
			rtti_read(size, stream);

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

		static uint32_t getSize(const ColorGradientHDR& data)
		{
			const uint64_t dataSize =
				rtti_size(data.mColors[0]) * ColorGradientHDR::MAX_KEYS +
				rtti_size(data.mTimes[0]) * ColorGradientHDR::MAX_KEYS +
				rtti_size(data.mNumKeys) + rtti_size(data.mDuration) + sizeof(uint32_t) * 2;

#if BS_DEBUG_MODE
			if(dataSize > std::numeric_limits<uint32_t>::max())
			{
				BS_EXCEPT(InternalErrorException, "Data overflow! Size doesn't fit into 32 bits.");
			}
#endif

			return (uint32_t)dataSize;
		}
	};

	/** @} */
	/** @endcond */
}
