//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIPlain.h"
#include "Importer/B3DTextureImportOptions.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT TextureImportOptionsRTTI : public TRTTIType<TextureImportOptions, ImportOptions, TextureImportOptionsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Format, 0)
			B3D_RTTI_MEMBER(GenerateMips, 1)
			B3D_RTTI_MEMBER(MaxMip, 2)
			B3D_RTTI_MEMBER(CpuCached, 3)
			B3D_RTTI_MEMBER(SRgb, 4)
			B3D_RTTI_MEMBER(Cubemap, 5)
			B3D_RTTI_MEMBER(CubemapSourceType, 6)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "TextureImportOptions";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_TextureImportOptions;
		}

		TShared<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<TextureImportOptions>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
