//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCoreApplication.h"
#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "RenderAPI/BsGpuDevice.h"
#include "RenderAPI/BsSamplerState.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	B3D_ALLOW_MEMCPY_SERIALIZATION(SamplerStateInformation);

	class B3D_CORE_EXPORT SamplerStateRTTI : public RTTIType<SamplerState, IReflectable, SamplerStateRTTI>
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
			const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
			if(!gpuDevice)
				return nullptr;

			return gpuDevice->CreateSamplerState(SamplerStateCreateInformation(), true);
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
