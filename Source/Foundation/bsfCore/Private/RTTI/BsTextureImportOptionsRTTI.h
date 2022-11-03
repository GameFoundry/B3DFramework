//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "Importer/BsTextureImportOptions.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT TextureImportOptionsRTTI : public RTTIType<TextureImportOptions, ImportOptions, TextureImportOptionsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(Format, 0)
			B3D_RTTI_MEMBER_PLAIN(GenerateMips, 1)
			B3D_RTTI_MEMBER_PLAIN(MaxMip, 2)
			B3D_RTTI_MEMBER_PLAIN(CpuCached, 3)
			B3D_RTTI_MEMBER_PLAIN(SRgb, 4)
			B3D_RTTI_MEMBER_PLAIN(Cubemap, 5)
			B3D_RTTI_MEMBER_PLAIN(CubemapSourceType, 6)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "TextureImportOptions";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_TextureImportOptions;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<TextureImportOptions>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
