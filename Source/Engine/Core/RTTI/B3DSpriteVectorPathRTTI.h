//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIPlain.h"
#include "RTTI/B3DSpriteImageRTTI.h"
#include "Image/B3DSpriteVectorPath.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT SpriteVectorPathRTTI : public TRTTIType<SpriteVectorPath, SpriteImage, SpriteVectorPathRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			//B3D_RTTI_MEMBER(mAtlasTexture, 0)
			B3D_RTTI_MEMBER(mVectorPath, 1)
			B3D_RTTI_MEMBER(mDefaultSize, 2)
			B3D_RTTI_MEMBER(mScalingMode, 3)
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
		void OnOperationEnded(SpriteVectorPath& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit) && !operationType.IsSet(RTTIOperationType::PreExistingObjectBit))
				object.Initialize();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
