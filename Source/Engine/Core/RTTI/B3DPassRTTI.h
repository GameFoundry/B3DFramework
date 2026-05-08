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

	class B3D_EXPORT PassRTTI : public TRTTIType<Pass, IReflectable, PassRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(BlendStateInformation, mData.BlendStateDesc, 0)
			B3D_RTTI_MEMBER_NAMED(RasterizerStateInformation, mData.RasterizerStateDesc, 1)
			B3D_RTTI_MEMBER_NAMED(DepthStencilStateInformation, mData.DepthStencilStateDesc, 2)

			B3D_RTTI_MEMBER_NAMED(StencilReferenceValue, mData.StencilRefValue, 3)
			B3D_RTTI_MEMBER_NAMED(VertexProgramInformation, mData.VertexProgramDesc, 4)
			B3D_RTTI_MEMBER_NAMED(FragmentProgramInformation, mData.FragmentProgramDesc, 5)
			B3D_RTTI_MEMBER_NAMED(GeometryProgramInformation, mData.GeometryProgramDesc, 6)
			B3D_RTTI_MEMBER_NAMED(HullProgramInformation, mData.HullProgramDesc, 7)
			B3D_RTTI_MEMBER_NAMED(DomainProgramInformation, mData.DomainProgramDesc, 8)
			B3D_RTTI_MEMBER_NAMED(ComputeProgramInformation, mData.ComputeProgramDesc, 9)
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
			B3D_RTTI_MEMBER_NAMED(BlendStateInformation, mData.BlendStateDesc, 0)
			B3D_RTTI_MEMBER_NAMED(RasterizerStateInformation, mData.RasterizerStateDesc, 1)
			B3D_RTTI_MEMBER_NAMED(DepthStencilStateInformation, mData.DepthStencilStateDesc, 2)

			B3D_RTTI_MEMBER_NAMED(StencilReferenceValue, mData.StencilRefValue, 3)
			B3D_RTTI_MEMBER_NAMED(VertexProgramInformation, mData.VertexProgramDesc, 4)
			B3D_RTTI_MEMBER_NAMED(FragmentProgramInformation, mData.FragmentProgramDesc, 5)
			B3D_RTTI_MEMBER_NAMED(GeometryProgramInformation, mData.GeometryProgramDesc, 6)
			B3D_RTTI_MEMBER_NAMED(HullProgramInformation, mData.HullProgramDesc, 7)
			B3D_RTTI_MEMBER_NAMED(DomainProgramInformation, mData.DomainProgramDesc, 8)
			B3D_RTTI_MEMBER_NAMED(ComputeProgramInformation, mData.ComputeProgramDesc, 9)
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
