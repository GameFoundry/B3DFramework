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

	template <>
	struct RTTIPlainType<ShaderVariation::Param>
	{
		enum
		{
			id = TID_ShaderVariationParam
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const ShaderVariation::Param& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr uint8_t kVersion = 0;

			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += rtti_write(kVersion, stream);
				size += rtti_write(data.Name, stream);
				size += rtti_write(data.Type, stream);
				size += rtti_write(data.I, stream);

				return size; });
		}

		static BitLength FromMemory(ShaderVariation::Param& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			uint8_t version;
			rtti_read(version, stream);
			assert(version == 0);

			rtti_read(data.Name, stream);
			rtti_read(data.Type, stream);
			rtti_read(data.I, stream);

			return size;
		}

		static BitLength GetSize(const ShaderVariation::Param& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint8_t);
			dataSize += rtti_size(data.Name);
			dataSize += rtti_size(data.Type);
			dataSize += rtti_size(data.I);

			rtti_add_header_size(dataSize, compress);
			return dataSize;
		}
	};

	class BS_CORE_EXPORT ShaderVariationRTTI : public RTTIType<ShaderVariation, IReflectable, ShaderVariationRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mParams, 0)
		BS_END_RTTI_MEMBERS
	public:
		const String& GetRttiName()
		{
			static String name = "ShaderVariation";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_ShaderVariation;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return bs_shared_ptr_new<ShaderVariation>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
