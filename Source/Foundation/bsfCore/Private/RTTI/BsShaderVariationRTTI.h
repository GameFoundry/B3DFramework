//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStringIDRTTI.h"
#include "RTTI/BsStdRTTI.h"
#include "Material/BsShaderVariation.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	template<>
	struct RTTIPlainType<ShaderVariation::Param>
	{
		enum { id = TID_ShaderVariationParam }; enum { hasDynamicSize = 1 };

		/** @copydoc RTTIPlainType::toMemory */
		static uint32_t toMemory(const ShaderVariation::Param& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr uint8_t VERSION = 0;

			return rtti_write_with_size_header(stream, [&data, &stream]()
			{
				uint32_t size = 0;
				size += rtti_write(VERSION, stream);
				size += rtti_write(data.name, stream);
				size += rtti_write(data.type, stream);
				size += rtti_write(data.i, stream);

				return size;
			});
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static uint32_t fromMemory(ShaderVariation::Param& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			uint32_t size = 0;
			rtti_read(size, stream);

			uint8_t version;
			rtti_read(version, stream);
			assert(version == 0);

			rtti_read(data.name, stream);
			rtti_read(data.type, stream);
			rtti_read(data.i, stream);

			return size;
		}

		/** @copydoc RTTIPlainType::getDynamicSize */
		static uint32_t getDynamicSize(const ShaderVariation::Param& data)
		{
			uint64_t dataSize = sizeof(uint8_t) + sizeof(uint32_t);
			dataSize += rtti_size(data.name);
			dataSize += rtti_size(data.type);
			dataSize += rtti_size(data.i);

			assert(dataSize <= std::numeric_limits<uint32_t>::max());

			return (uint32_t)dataSize;
		}
	};

	class BS_CORE_EXPORT ShaderVariationRTTI : public RTTIType <ShaderVariation, IReflectable, ShaderVariationRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mParams, 0)
		BS_END_RTTI_MEMBERS
	public:
		const String& getRTTIName() override
		{
			static String name = "ShaderVariation";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ShaderVariation;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ShaderVariation>();
		}
	};

	/** @} */
	/** @endcond */
}
