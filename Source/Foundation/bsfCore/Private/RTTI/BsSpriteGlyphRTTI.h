//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "Private/RTTI/BsSpriteImageRTTI.h"
#include "Image/BsSpriteGlyph.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_CORE_EXPORT SpriteGlyphRTTI : public RTTIType<SpriteGlyph, SpriteImage, SpriteGlyphRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_REFL(mAtlasTexture, 0)
			B3D_RTTI_MEMBER_REFL(mFont, 1)
			B3D_RTTI_MEMBER_PLAIN(mGlyph, 2)
			B3D_RTTI_MEMBER_PLAIN(mGlyphSize, 3)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "SpriteGlyph";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_SpriteGlyph;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return SpriteGlyph::CreateEmpty();
		}

	private:
		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context)
		{
			SpriteGlyph* glyph = static_cast<SpriteGlyph*>(obj);
			glyph->Initialize();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
