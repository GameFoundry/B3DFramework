//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DFlagsRTTI.h"
#include "RTTI/B3DColorRTTI.h"
#include "RTTI/B3DMathRTTI.h"
#include "GpuBackend/B3DViewport.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT ViewportRTTI : public TRTTIType<Viewport, IReflectable, ViewportRTTI>
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

		TShared<IReflectable> NewRttiObject()
		{
			return Viewport::CreateEmpty();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
