//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsShaderCompiler.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "RTTI/BsStringRTTI.h"
#include "RTTI/BsTArrayRTTI.h"
#include "RTTI/BsStdRTTI.h"
#include "Private/RTTI/BsShaderVariationRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT ShaderCompilerMetaDataRTTI : public TRTTIType<ShaderCompilerMetaData, IReflectable, ShaderCompilerMetaDataRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Source, 0)
			B3D_RTTI_MEMBER(NameInCache, 1)
			B3D_RTTI_MEMBER(ShaderHash, 2)
			B3D_RTTI_MEMBER(GPUProgramTypes, 3)
			B3D_RTTI_MEMBER_CONTAINER(Variations, 4)
			B3D_RTTI_MEMBER(Defines, 5)
			B3D_RTTI_MEMBER(IncludeHashes, 6)
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

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ShaderCompilerMetaData>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
