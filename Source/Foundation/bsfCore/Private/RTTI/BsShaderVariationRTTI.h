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
		static uint32_t toMemory(const ShaderVariation::Param& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo)
		{
			static constexpr uint8_t VERSION = 0;

			return rtti_write_with_size_header(stream, [&data, &stream]()
			{
				uint32_t size = 0;
				size += rttiWriteElem(VERSION, stream);
				size += rttiWriteElem(data.name, stream);
				size += rttiWriteElem(data.type, stream);
				size += rttiWriteElem(data.i, stream);

				return size;
			});
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static uint32_t fromMemory(ShaderVariation::Param& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo)
		{
			uint32_t size = 0;
			rttiReadElem(size, stream);

			uint8_t version;
			rttiReadElem(version, stream);
			assert(version == 0);

			rttiReadElem(data.name, stream);
			rttiReadElem(data.type, stream);
			rttiReadElem(data.i, stream);

			return size;
		}

		/** @copydoc RTTIPlainType::getDynamicSize */
		static uint32_t getDynamicSize(const ShaderVariation::Param& data)
		{
			uint64_t dataSize = sizeof(uint8_t) + sizeof(uint32_t);
			dataSize += rttiGetElemSize(data.name);
			dataSize += rttiGetElemSize(data.type);
			dataSize += rttiGetElemSize(data.i);

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
