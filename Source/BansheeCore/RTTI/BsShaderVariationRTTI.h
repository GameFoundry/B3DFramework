//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2017 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
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
		static void toMemory(const ShaderVariation::Param& data, char* memory)
		{
			UINT32 size = sizeof(UINT32);
			char* memoryStart = memory;
			memory += sizeof(UINT32);

			UINT8 version = 0;
			memory = rttiWriteElem(version, memory, size);
			memory = rttiWriteElem(data.name, memory, size);
			memory = rttiWriteElem(data.type, memory, size);
			memory = rttiWriteElem(data.i, memory, size);

			memcpy(memoryStart, &size, sizeof(UINT32));
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static UINT32 fromMemory(ShaderVariation::Param& data, char* memory)
		{
			UINT32 size = 0;
			memory = rttiReadElem(size, memory);

			UINT8 version;
			memory = rttiReadElem(version, memory);
			assert(version == 0);

			memory = rttiReadElem(data.name, memory);
			memory = rttiReadElem(data.type, memory);
			memory = rttiReadElem(data.i, memory);

			return size;
		}

		/** @copydoc RTTIPlainType::getDynamicSize */
		static UINT32 getDynamicSize(const ShaderVariation::Param& data)
		{
			UINT64 dataSize = sizeof(UINT8) + sizeof(UINT32);
			dataSize += rttiGetElemSize(data.name);
			dataSize += rttiGetElemSize(data.type);
			dataSize += rttiGetElemSize(data.i);

			assert(dataSize <= std::numeric_limits<UINT32>::max());

			return (UINT32)dataSize;
		}
	};

	class BS_CORE_EXPORT ShaderVariationRTTI : public RTTIType <ShaderVariation, IReflectable, ShaderVariationRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mParams, 0)
		BS_END_RTTI_MEMBERS
	public:
		ShaderVariationRTTI()
			:mInitMembers(this)
		{
			
		}

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
