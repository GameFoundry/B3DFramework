//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "Private/RTTI/BsGpuProgramRTTI.h"
#include "Private/RTTI/BsGpuPipelineStateRTTI.h"
#include "Material/BsPass.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT PassRTTI : public RTTIType<Pass, IReflectable, PassRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN_NAMED(BlendStateInformation, mData.BlendStateDesc, 0)
			B3D_RTTI_MEMBER_PLAIN_NAMED(RasterizerStateInformation, mData.RasterizerStateDesc, 1)
			B3D_RTTI_MEMBER_PLAIN_NAMED(DepthStencilStateInformation, mData.DepthStencilStateDesc, 2)

			B3D_RTTI_MEMBER_PLAIN_NAMED(StencilReferenceValue, mData.StencilRefValue, 3)
			B3D_RTTI_MEMBER_REFL_NAMED(VertexProgramInformation, mData.VertexProgramDesc, 4)
			B3D_RTTI_MEMBER_REFL_NAMED(FragmentProgramInformation, mData.FragmentProgramDesc, 5)
			B3D_RTTI_MEMBER_REFL_NAMED(GeometryProgramInformation, mData.GeometryProgramDesc, 6)
			B3D_RTTI_MEMBER_REFL_NAMED(HullProgramInformation, mData.HullProgramDesc, 7)
			B3D_RTTI_MEMBER_REFL_NAMED(DomainProgramInformation, mData.DomainProgramDesc, 8)
			B3D_RTTI_MEMBER_REFL_NAMED(ComputeProgramInformation, mData.ComputeProgramDesc, 9)
		B3D_RTTI_END_MEMBERS

	public:
		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			Pass* pass = static_cast<Pass*>(obj);
			pass->Initialize();
		}

		const String& GetRttiName() override
		{
			static String name = "Pass";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_Pass;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return Pass::CreateEmpty();
		}
	};

	class B3D_CORE_EXPORT PassRenderProxyRTTI : public RTTIType<ct::Pass, IReflectable, PassRenderProxyRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN_NAMED(BlendStateInformation, mData.BlendStateDesc, 0)
			B3D_RTTI_MEMBER_PLAIN_NAMED(RasterizerStateInformation, mData.RasterizerStateDesc, 1)
			B3D_RTTI_MEMBER_PLAIN_NAMED(DepthStencilStateInformation, mData.DepthStencilStateDesc, 2)

			B3D_RTTI_MEMBER_PLAIN_NAMED(StencilReferenceValue, mData.StencilRefValue, 3)
			B3D_RTTI_MEMBER_REFL_NAMED(VertexProgramInformation, mData.VertexProgramDesc, 4)
			B3D_RTTI_MEMBER_REFL_NAMED(FragmentProgramInformation, mData.FragmentProgramDesc, 5)
			B3D_RTTI_MEMBER_REFL_NAMED(GeometryProgramInformation, mData.GeometryProgramDesc, 6)
			B3D_RTTI_MEMBER_REFL_NAMED(HullProgramInformation, mData.HullProgramDesc, 7)
			B3D_RTTI_MEMBER_REFL_NAMED(DomainProgramInformation, mData.DomainProgramDesc, 8)
			B3D_RTTI_MEMBER_REFL_NAMED(ComputeProgramInformation, mData.ComputeProgramDesc, 9)
		B3D_RTTI_END_MEMBERS

	public:
		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			ct::Pass* pass = static_cast<ct::Pass*>(obj);
			pass->Initialize();
		}

		const String& GetRttiName() override
		{
			static String name = "PassRenderProxy";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_PassRenderProxy;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return ct::Pass::CreateEmpty();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
