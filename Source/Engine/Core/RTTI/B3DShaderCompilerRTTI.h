//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Material/B3DShaderCompiler.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIPlain.h"
#include "RTTI/B3DStringRTTI.h"
#include "RTTI/B3DTArrayRTTI.h"
#include "RTTI/B3DStdRTTI.h"
#include "RTTI/B3DShaderVariationRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	template <>
	struct RTTIPlainType<ShaderEntryPoint> : RTTIPlainTypeHelper<ShaderEntryPoint, TID_ShaderEntryPoint, 0>
	{
		template<class Processor>
		static void RTTIEnumerateFields(ShaderEntryPoint& object, Processor& processor, u8 version)
		{
			processor(object.PassIndex);
			processor(object.Type);
			processor(object.Name);
		}
	};

	class B3D_EXPORT ShaderCompilerMetaDataRTTI : public TRTTIType<ShaderCompilerMetaData, IReflectable, ShaderCompilerMetaDataRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Source, 0)
			B3D_RTTI_MEMBER(NameInCache, 1)
			B3D_RTTI_MEMBER(ShaderHash, 2)
			B3D_RTTI_MEMBER_CONTAINER(Variations, 4)
			B3D_RTTI_MEMBER(Defines, 5)
			B3D_RTTI_MEMBER(IncludeHashes, 6)
			B3D_RTTI_MEMBER(CompilerVersion, 7)
			B3D_RTTI_MEMBER(EntryPoints, 8)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ShaderCompilerMetaData";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ShaderCompilerMetaData;
		}

		TShared<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ShaderCompilerMetaData>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
