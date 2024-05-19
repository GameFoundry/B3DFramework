//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "Private/RTTI/BsSpriteImageRTTI.h"
#include "Image/BsSpriteVectorPath.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_CORE_EXPORT SpriteVectorPathRTTI : public RTTIType<SpriteVectorPath, SpriteImage, SpriteVectorPathRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_REFL(mAtlasTexture, 0)
			B3D_RTTI_MEMBER_REFL(mVectorPath, 1)
			B3D_RTTI_MEMBER_PLAIN(mSize, 2)
			B3D_RTTI_MEMBER_PLAIN(mScalingMode, 3)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "SpriteVectorPath";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SpriteVectorPath;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return SpriteVectorPath::CreateEmpty();
		}

	private:
		void OnDeserializationEnded(IReflectable* obj, RTTIOperationContext* context)
		{
			SpriteVectorPath* spriteVectorPath = static_cast<SpriteVectorPath*>(obj);
			spriteVectorPath->Initialize();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
