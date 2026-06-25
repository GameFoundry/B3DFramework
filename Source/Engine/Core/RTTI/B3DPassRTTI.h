//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIPlain.h"
#include "RTTI/B3DGpuProgramRTTI.h"
#include "RTTI/B3DGpuPipelineStateRTTI.h"
#include "Material/B3DPass.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT PassInformationRTTI : public TRTTIType<PassInformation, IReflectable, PassInformationRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(BlendStateInformation, 0)
			B3D_RTTI_MEMBER(RasterizerStateInformation, 1)
			B3D_RTTI_MEMBER(DepthStencilStateInformation, 2)

			B3D_RTTI_MEMBER(StencilRefValue, 3)
			B3D_RTTI_MEMBER(VertexProgramCreateInformation, 4)
			B3D_RTTI_MEMBER(FragmentProgramCreateInformation, 5)
			B3D_RTTI_MEMBER(GeometryProgramCreateInformation, 6)
			B3D_RTTI_MEMBER(HullProgramCreateInformation, 7)
			B3D_RTTI_MEMBER(DomainProgramCreateInformation, 8)
			B3D_RTTI_MEMBER(ComputeProgramCreateInformation, 9)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "PassInformation";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_PassInformation;
		}

		TShared<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<PassInformation>();
		}
	};

	class B3D_EXPORT PassRTTI : public TRTTIType<Pass, IReflectable, PassRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mData, 0)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationEnded(Pass& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit) && !operationType.IsSet(RTTIOperationType::PreExistingObjectBit))
				object.Initialize();
		}

		const String& GetRttiName() override
		{
			static String name = "Pass";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_Pass;
		}

		TShared<IReflectable> NewRttiObject() override
		{
			return Pass::CreateEmpty();
		}
	};

	class B3D_EXPORT PassRenderProxyRTTI : public TRTTIType<render::Pass, IReflectable, PassRenderProxyRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mData, 0)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationEnded(render::Pass& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit) && !operationType.IsSet(RTTIOperationType::PreExistingObjectBit))
				object.Initialize();
		}

		const String& GetRttiName() override
		{
			static String name = "PassRenderProxy";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_PassRenderProxy;
		}

		TShared<IReflectable> NewRttiObject() override
		{
			return render::Pass::CreateEmpty();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
