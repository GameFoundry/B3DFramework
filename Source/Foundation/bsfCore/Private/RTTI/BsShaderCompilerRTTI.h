//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
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

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT ShaderCompilerMetaDataRTTI : public RTTIType<ShaderCompilerMetaData, IReflectable, ShaderCompilerMetaDataRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(Source, 0)
			B3D_RTTI_MEMBER_PLAIN(NameInCache, 1)
			B3D_RTTI_MEMBER_PLAIN(ShaderHash, 2)
			B3D_RTTI_MEMBER_PLAIN(GPUProgramTypes, 3)
			B3D_RTTI_MEMBER_REFL_ARRAY(Variations, 4)
			B3D_RTTI_MEMBER_PLAIN(Defines, 5)
			B3D_RTTI_MEMBER_PLAIN(IncludeHashes, 6)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ShaderCompilerMetaData";
			return name;
		}

		u32 GetRttiId() override
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
} // namespace bs 
