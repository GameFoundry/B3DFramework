//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsFlagsRTTI.h"
#include "RTTI/BsColorRTTI.h"
#include "RTTI/BsMathRTTI.h"
#include "RenderAPI/BsViewport.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT ViewportRTTI : public RTTIType<Viewport, IReflectable, ViewportRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mNormArea, 0)
			B3D_RTTI_MEMBER(mClearColorValue, 1)
			B3D_RTTI_MEMBER(mClearDepthValue, 2)
			B3D_RTTI_MEMBER(mClearStencilValue, 3)
			B3D_RTTI_MEMBER(mClearFlags, 4)
		B3D_RTTI_END_MEMBERS
	public:
		void OnOperationEnded(Viewport& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit) && !operationType.IsSet(RTTIOperationType::PreExistingObjectBit))
				object.Initialize();
		}

		const String& GetRttiName() override
		{
			static String name = "Viewport";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_Viewport;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return Viewport::CreateEmpty();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
