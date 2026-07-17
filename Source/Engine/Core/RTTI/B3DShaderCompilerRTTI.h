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

	class B3D_EXPORT ShaderCompilerMetaDataRTTI : public TRTTIType<ShaderCompilerMetaData, IReflectable, ShaderCompilerMetaDataRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Source, 0)
			B3D_RTTI_MEMBER(NameInCache, 1)
			B3D_RTTI_MEMBER(ShaderHash, 2)
			B3D_RTTI_MEMBER(GPUProgramTypes, 3)
			B3D_RTTI_MEMBER_CONTAINER(Variations, 4)
			B3D_RTTI_MEMBER(Defines, 5)
			B3D_RTTI_MEMBER(IncludeHashes, 6)
			B3D_RTTI_MEMBER(CompilerVersion, 7)
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
