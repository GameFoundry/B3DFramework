//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "Private/RTTI/BsGpuProgramRTTI.h"
#include "Private/RTTI/BsBlendStateRTTI.h"
#include "Private/RTTI/BsRasterizerStateRTTI.h"
#include "Private/RTTI/BsDepthStencilStateRTTI.h"
#include "Material/BsPass.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	/** Helper class used for serialization of GPU_PROGRAM_DESC. */
	struct SerializedGpuProgramData : GpuProgramCreateInformation, IReflectable
	{
		const SerializedGpuProgramData& operator=(const GpuProgramCreateInformation& desc)
		{
			Name = desc.Name;
			Source = desc.Source;
			EntryPoint = desc.EntryPoint;
			Language = desc.Language;
			Type = desc.Type;
			RequiresAdjacency = desc.RequiresAdjacency;
			Bytecode = desc.Bytecode;

			return *this;
		}

		/************************************************************************/
		/*									RTTI								*/
		/************************************************************************/
	public:
		friend class SerializedGpuProgramDataRTTI;
		inline static RTTITypeBase* GetRttiStatic();
		inline RTTITypeBase* GetRtti() const override;
	};

	class B3D_CORE_EXPORT SerializedGpuProgramDataRTTI : public RTTIType<SerializedGpuProgramData, IReflectable, SerializedGpuProgramDataRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(Source, 0)
			B3D_RTTI_MEMBER_PLAIN(EntryPoint, 1)
			B3D_RTTI_MEMBER_PLAIN(Language, 2)
			B3D_RTTI_MEMBER_PLAIN(Type, 3)
			B3D_RTTI_MEMBER_PLAIN(RequiresAdjacency, 4)
			B3D_RTTI_MEMBER_REFLPTR(Bytecode, 5)
			B3D_RTTI_MEMBER_PLAIN(Name, 6)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SerializedGpuProgramData";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_SerializedGpuProgramData;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<SerializedGpuProgramData>();
		}
	};

	RTTITypeBase* SerializedGpuProgramData::GetRttiStatic()
	{
		return SerializedGpuProgramDataRTTI::Instance();
	}

	RTTITypeBase* SerializedGpuProgramData::GetRtti() const
	{
		return GetRttiStatic();
	}

	class B3D_CORE_EXPORT PassRTTI : public RTTIType<Pass, IReflectable, PassRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN_NAMED(blendStateDesc, mData.BlendStateDesc, 0)
			B3D_RTTI_MEMBER_PLAIN_NAMED(rasterizerStateDesc, mData.RasterizerStateDesc, 1)
			B3D_RTTI_MEMBER_PLAIN_NAMED(depthStencilState, mData.DepthStencilStateDesc, 2)

			B3D_RTTI_MEMBER_PLAIN_NAMED(stencilRefValue, mData.StencilRefValue, 9)
		B3D_RTTI_END_MEMBERS

		SerializedGpuProgramData& GetVertexProgramDesc(Pass* obj)
		{
			return mVertexProgramDesc;
		}

		void SetVertexProgramDesc(Pass* obj, SerializedGpuProgramData& val)
		{
			obj->mData.VertexProgramDesc = val;
		}

		SerializedGpuProgramData& GetFragmentProgramDesc(Pass* obj)
		{
			return mFragmentProgramDesc;
		}

		void SetFragmentProgramDesc(Pass* obj, SerializedGpuProgramData& val)
		{
			obj->mData.FragmentProgramDesc = val;
		}

		SerializedGpuProgramData& GetGeometryProgramDesc(Pass* obj)
		{
			return mGeometryProgramDesc;
		}

		void SetGeometryProgramDesc(Pass* obj, SerializedGpuProgramData& val)
		{
			obj->mData.GeometryProgramDesc = val;
		}

		SerializedGpuProgramData& GetHullProgramDesc(Pass* obj)
		{
			return mHullProgramDesc;
		}

		void SetHullProgramDesc(Pass* obj, SerializedGpuProgramData& val)
		{
			obj->mData.HullProgramDesc = val;
		}

		SerializedGpuProgramData& GetDomainProgramDesc(Pass* obj)
		{
			return mDomainProgramDesc;
		}

		void SetDomainProgramDesc(Pass* obj, SerializedGpuProgramData& val)
		{
			obj->mData.DomainProgramDesc = val;
		}

		SerializedGpuProgramData& GetComputeProgramDesc(Pass* obj)
		{
			return mComputeProgramDesc;
		}

		void SetComputeProgramDesc(Pass* obj, SerializedGpuProgramData& val)
		{
			obj->mData.ComputeProgramDesc = val;
		}

	public:
		PassRTTI()
		{
			AddReflectableField("mVertexProgramDesc", 3, &PassRTTI::GetVertexProgramDesc, &PassRTTI::SetVertexProgramDesc);
			AddReflectableField("mFragmentProgramDesc", 4, &PassRTTI::GetFragmentProgramDesc, &PassRTTI::SetFragmentProgramDesc);
			AddReflectableField("mGeometryProgramDesc", 5, &PassRTTI::GetGeometryProgramDesc, &PassRTTI::SetGeometryProgramDesc);
			AddReflectableField("mHullProgramDesc", 6, &PassRTTI::GetHullProgramDesc, &PassRTTI::SetHullProgramDesc);
			AddReflectableField("mDomainProgramDesc", 7, &PassRTTI::GetDomainProgramDesc, &PassRTTI::SetDomainProgramDesc);
			AddReflectableField("mComputeProgramDesc", 8, &PassRTTI::GetComputeProgramDesc, &PassRTTI::SetComputeProgramDesc);
		}

		void OnSerializationStarted(IReflectable* obj, SerializationContext* context) override
		{
			Pass* pass = static_cast<Pass*>(obj);

			mVertexProgramDesc = pass->mData.VertexProgramDesc;
			mFragmentProgramDesc = pass->mData.FragmentProgramDesc;
			mGeometryProgramDesc = pass->mData.GeometryProgramDesc;
			mHullProgramDesc = pass->mData.HullProgramDesc;
			mDomainProgramDesc = pass->mData.DomainProgramDesc;
			mComputeProgramDesc = pass->mData.ComputeProgramDesc;

			auto initBytecode = [](const SPtr<GpuProgram>& prog, GpuProgramCreateInformation& desc)
			{
				if(prog)
				{
					prog->BlockUntilCoreInitialized();
					desc.Bytecode = prog->GetCore()->GetBytecode();
				}
			};

			const SPtr<GraphicsPipelineState>& graphicsPipeline = pass->GetGraphicsPipelineState();
			if(graphicsPipeline)
			{
				initBytecode(graphicsPipeline->GetVertexProgram(), mVertexProgramDesc);
				initBytecode(graphicsPipeline->GetFragmentProgram(), mFragmentProgramDesc);
				initBytecode(graphicsPipeline->GetGeometryProgram(), mGeometryProgramDesc);
				initBytecode(graphicsPipeline->GetHullProgram(), mHullProgramDesc);
				initBytecode(graphicsPipeline->GetDomainProgram(), mDomainProgramDesc);
			}

			const SPtr<ComputePipelineState>& computePipeline = pass->GetComputePipelineState();
			if(computePipeline)
				initBytecode(computePipeline->GetProgram(), mComputeProgramDesc);
		}

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

		SPtr<IReflectable> NewRttiObject()
		{
			return Pass::CreateEmpty();
		}

	private:
		SerializedGpuProgramData mVertexProgramDesc;
		SerializedGpuProgramData mFragmentProgramDesc;
		SerializedGpuProgramData mGeometryProgramDesc;
		SerializedGpuProgramData mHullProgramDesc;
		SerializedGpuProgramData mDomainProgramDesc;
		SerializedGpuProgramData mComputeProgramDesc;
	};

	/** @} */
	/** @endcond */
} // namespace bs
