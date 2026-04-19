//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIPlain.h"
#include "RenderAPI/B3DGpuDevice.h"
#include "RenderAPI/B3DSamplerState.h"
#include "B3DApplication.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	B3D_ALLOW_MEMCPY_SERIALIZATION(SamplerStateInformation, TID_SamplerStateInformation);

	class B3D_EXPORT SamplerStateRTTI : public TRTTIType<SamplerState, IReflectable, SamplerStateRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mInformation, 0)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationEnded(SamplerState& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit) && !operationType.IsSet(RTTIOperationType::PreExistingObjectBit))
				object.Initialize();
		}

		const String& GetRttiName()
		{
			static String name = "SamplerState";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SamplerState;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
			if(!gpuDevice)
				return nullptr;

			return gpuDevice->CreateSamplerState(SamplerStateCreateInformation(), GpuObjectCreateFlag::DeferredInitialize);
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
