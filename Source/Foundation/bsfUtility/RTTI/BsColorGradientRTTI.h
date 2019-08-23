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

		static uint32_t toMemory(const ColorGradient& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo)
		{
			static constexpr uint32_t VERSION = 0;

			return rtti_write_with_size_header(stream, [&data, &stream]()
			{
				uint32_t size = 0;
				size += rttiWriteElem(VERSION, stream);

				for (uint32_t i = 0; i < ColorGradient::MAX_KEYS; i++)
				{
					size += rttiWriteElem(data.mColors[i], stream);
					size += rttiWriteElem(data.mTimes[i], stream);
				}

				size += rttiWriteElem(data.mNumKeys, stream);
				size += rttiWriteElem(data.mDuration, stream);

				return size;
			});
		}

		static uint32_t fromMemory(ColorGradient& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo)
		{
			uint32_t size;
			rttiReadElem(size, stream);

			uint32_t version;
			rttiReadElem(version, stream);

			switch(version)
			{
			case 0:
				for (uint32_t i = 0; i < ColorGradient::MAX_KEYS; i++)
				{
					rttiReadElem(data.mColors[i], stream);
					rttiReadElem(data.mTimes[i], stream);
				}

				rttiReadElem(data.mNumKeys, stream);
				rttiReadElem(data.mDuration, stream);
				break;
			default:
				BS_LOG(Error, RTTI, "Unknown version of ColorGradient data. Unable to deserialize.");
				break;
			}

			return size;
		}

		static uint32_t getDynamicSize(const ColorGradient& data)
		{
			const uint64_t dataSize =
				rttiGetElemSize(data.mColors[0]) * ColorGradient::MAX_KEYS +
				rttiGetElemSize(data.mTimes[0]) * ColorGradient::MAX_KEYS +
				rttiGetElemSize(data.mNumKeys) + rttiGetElemSize(data.mDuration) + sizeof(uint32_t) * 2;

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

		static uint32_t toMemory(const ColorGradientHDR& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo)
		{
			static constexpr uint32_t VERSION = 0;

			return rtti_write_with_size_header(stream, [&data, &stream]()
			{
				uint32_t size = 0;
				size += rttiWriteElem(VERSION, stream);

				for(uint32_t i = 0; i < ColorGradientHDR::MAX_KEYS; i++)
				{
					size += rttiWriteElem(data.mColors[i], stream);
					size += rttiWriteElem(data.mTimes[i], stream);
				}

				size += rttiWriteElem(data.mNumKeys, stream);
				size += rttiWriteElem(data.mDuration, stream);

				return size;
			});
		}

		static uint32_t fromMemory(ColorGradientHDR& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo)
		{
			uint32_t size;
			rttiReadElem(size, stream);

			uint32_t version;
			rttiReadElem(version, stream);

			switch(version)
			{
			case 0:
				for (uint32_t i = 0; i < ColorGradientHDR::MAX_KEYS; i++)
				{
					rttiReadElem(data.mColors[i], stream);
					rttiReadElem(data.mTimes[i], stream);
				}

				rttiReadElem(data.mNumKeys, stream);
				rttiReadElem(data.mDuration, stream);
				break;
			default:
				BS_LOG(Error, RTTI, "Unknown version of ColorGradientHDR data. Unable to deserialize.");
				break;
			}

			return size;
		}

		static uint32_t getDynamicSize(const ColorGradientHDR& data)
		{
			const uint64_t dataSize =
				rttiGetElemSize(data.mColors[0]) * ColorGradientHDR::MAX_KEYS +
				rttiGetElemSize(data.mTimes[0]) * ColorGradientHDR::MAX_KEYS +
				rttiGetElemSize(data.mNumKeys) + rttiGetElemSize(data.mDuration) + sizeof(uint32_t) * 2;

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
