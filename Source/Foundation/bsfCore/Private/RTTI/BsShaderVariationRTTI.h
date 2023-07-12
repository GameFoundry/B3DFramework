//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStringIDRTTI.h"
#include "RTTI/BsStdRTTI.h"
#include "RTTI/BsSmallVectorRTTI.h"
#include "Material/BsShaderVariation.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	template <>
	struct RTTIPlainType<ShaderVariationParameter>
	{
		enum
		{
			id = TID_ShaderVariationParam
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const ShaderVariationParameter& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr uint8_t kVersion = 0;

			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += B3DRTTIWrite(kVersion, stream);
				size += B3DRTTIWrite(data.Name, stream);
				size += B3DRTTIWrite(data.Type, stream);
				size += B3DRTTIWrite(data.I, stream);

				return size; });
		}

		static BitLength FromMemory(ShaderVariationParameter& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			uint8_t version;
			B3DRTTIRead(version, stream);
			B3D_ASSERT(version == 0);

			B3DRTTIRead(data.Name, stream);
			B3DRTTIRead(data.Type, stream);
			B3DRTTIRead(data.I, stream);

			return size;
		}

		static BitLength GetSize(const ShaderVariationParameter& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint8_t);
			dataSize += B3DRTTISize(data.Name);
			dataSize += B3DRTTISize(data.Type);
			dataSize += B3DRTTISize(data.I);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	class B3D_CORE_EXPORT ShaderVariationRTTI : public RTTIType<ShaderVariationParameters, IReflectable, ShaderVariationRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(mParams, 0)
		B3D_RTTI_END_MEMBERS
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
			return B3DMakeShared<ShaderVariationParameters>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
