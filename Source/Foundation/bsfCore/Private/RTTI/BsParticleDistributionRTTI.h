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
		static BitLength toMemory(const ColorDistribution& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr uint32_t VERSION = 0; // In case the data structure changes

			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
			{
				BitLength size = 0;
				size += rtti_write(VERSION, stream);
				size += rtti_write(data.mType, stream);
				size += rtti_write(data.mMinGradient, stream);
				size += rtti_write(data.mMaxGradient, stream);

				return size;
			});
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static BitLength fromMemory(ColorDistribution& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			uint32_t version;
			rtti_read(version, stream);

			switch(version)
			{
			case 0:
				rtti_read(data.mType, stream);
				rtti_read(data.mMinGradient, stream);
				rtti_read(data.mMaxGradient, stream);
				break;
			default:
				BS_LOG(Error, RTTI, "Unknown version of TDistribution<T> data. Unable to deserialize.");
				break;
			}

			return size;
		}

		/** @copydoc RTTIPlainType::getSize */
		static BitLength getSize(const ColorDistribution& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint32_t);
			dataSize += rtti_size(data.mType);
			dataSize += rtti_size(data.mMinGradient);
			dataSize += rtti_size(data.mMaxGradient);

			rtti_add_header_size(dataSize, compress);
			return dataSize;
		}
	};

	template<class T> struct RTTIPlainType<TDistribution<T>>
	{
		enum { id = TID_TDistribution }; enum { hasDynamicSize = 1 };

		/** @copydoc RTTIPlainType::toMemory */
		static BitLength toMemory(const TDistribution<T>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr uint32_t VERSION = 0; // In case the data structure changes

			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
			{
				BitLength size = 0;
				size += rtti_write(VERSION, stream);
				size += rtti_write(data.mType, stream);
				size += rtti_write(data.mMinCurve, stream);
				size += rtti_write(data.mMaxCurve, stream);

				return size;
			});
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static BitLength fromMemory(TDistribution<T>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			uint32_t version;
			rtti_read(version, stream);

			switch(version)
			{
			case 0:
				rtti_read(data.mType, stream);
				rtti_read(data.mMinCurve, stream);
				rtti_read(data.mMaxCurve, stream);
				break;
			default:
				BS_LOG(Error, RTTI, "Unknown version of TDistribution<T> data. Unable to deserialize.");
				break;
			}

			return size;
		}

		/** @copydoc RTTIPlainType::getSize */
		static BitLength getSize(const TDistribution<T>& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint32_t);
			dataSize += rtti_size(data.mType);
			dataSize += rtti_size(data.mMinCurve);
			dataSize += rtti_size(data.mMaxCurve);

			rtti_add_header_size(dataSize, compress);
			return dataSize;
		}
	};

	/** @} */
	/** @endcond */
}
