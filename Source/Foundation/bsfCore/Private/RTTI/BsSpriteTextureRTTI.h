//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "Private/RTTI/BsSpriteImageRTTI.h"
#include "Image/BsSpriteTexture.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_CORE_EXPORT SpriteTextureRTTI : public RTTIType<SpriteTexture, SpriteImage, SpriteTextureRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_REFL(mAtlasTexture, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "SpriteTexture";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SpriteTexture;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return SpriteTexture::CreateEmpty();
		}

	private:
		void OnDeserializationEnded(IReflectable* obj, RTTIOperationContext* context)
		{
			SpriteTexture* texture = static_cast<SpriteTexture*>(obj);
			texture->Initialize();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
