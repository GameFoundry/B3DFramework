//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIPlain.h"
#include "RTTI/B3DSpriteImageRTTI.h"
#include "Image/B3DSpriteTexture.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT SpriteTextureRTTI : public TRTTIType<SpriteTexture, SpriteImage, SpriteTextureRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mAtlasTexture, 0)
			B3D_RTTI_MEMBER(mUVRange, 1)
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
		void OnOperationEnded(SpriteTexture& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit) && !operationType.IsSet(RTTIOperationType::PreExistingObjectBit))
				object.Initialize();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
