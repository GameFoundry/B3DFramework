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
	struct SerializedGpuProgramData : GPU_PROGRAM_DESC, IReflectable
	{
		const SerializedGpuProgramData& operator=(const GPU_PROGRAM_DESC& desc)
		{
			source = desc.source;
			entryPoint = desc.entryPoint;
			language = desc.language;
			type = desc.type;
			requiresAdjacency = desc.requiresAdjacency;
			bytecode = desc.bytecode;

			return *this;
		}

		/************************************************************************/
		/*									RTTI								*/
		/************************************************************************/
	public:
		friend class SerializedGpuProgramDataRTTI;
		inline static RTTITypeBase* getRTTIStatic();
		inline RTTITypeBase* getRTTI() const override;
	};

	class BS_CORE_EXPORT SerializedGpuProgramDataRTTI : public RTTIType<SerializedGpuProgramData, IReflectable, SerializedGpuProgramDataRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(source, 0)
			BS_RTTI_MEMBER_PLAIN(entryPoint, 1)
			BS_RTTI_MEMBER_PLAIN(language, 2)
			BS_RTTI_MEMBER_PLAIN(type, 3)
			BS_RTTI_MEMBER_PLAIN(requiresAdjacency, 4)
			BS_RTTI_MEMBER_REFLPTR(bytecode, 5)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRTTIName() override
		{
			static String name = "SerializedGpuProgramData";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_SerializedGpuProgramData;
		}

		SPtr<IReflectable> NewRTTIObject() override
		{
			return bs_shared_ptr_new<SerializedGpuProgramData>();
		}
	};

	RTTITypeBase* SerializedGpuProgramData::getRTTIStatic()
	{
		return SerializedGpuProgramDataRTTI::Instance();
	}

	RTTITypeBase* SerializedGpuProgramData::getRTTI() const
	{
		return GetRTTIStatic();
	}

	class BS_CORE_EXPORT PassRTTI : public RTTIType<Pass, IReflectable, PassRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(blendStateDesc, mData.blendStateDesc, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(rasterizerStateDesc, mData.rasterizerStateDesc, 1)
			BS_RTTI_MEMBER_PLAIN_NAMED(depthStencilState, mData.depthStencilStateDesc, 2)

			BS_RTTI_MEMBER_PLAIN_NAMED(stencilRefValue, mData.stencilRefValue, 9)
		BS_END_RTTI_MEMBERS

		SerializedGpuProgramData& GetVertexProgramDesc(Pass* obj)
		{
			return mVertexProgramDesc;
		}

		void SetVertexProgramDesc(Pass* obj, SerializedGpuProgramData& val)
		{
			obj->mData.vertexProgramDesc = val;
		}

		SerializedGpuProgramData& GetFragmentProgramDesc(Pass* obj)
		{
			return mFragmentProgramDesc;
		}

		void SetFragmentProgramDesc(Pass* obj, SerializedGpuProgramData& val)
		{
			obj->mData.fragmentProgramDesc = val;
		}

		SerializedGpuProgramData& GetGeometryProgramDesc(Pass* obj)
		{
			return mGeometryProgramDesc;
		}

		void SetGeometryProgramDesc(Pass* obj, SerializedGpuProgramData& val)
		{
			obj->mData.geometryProgramDesc = val;
		}

		SerializedGpuProgramData& GetHullProgramDesc(Pass* obj)
		{
			return mHullProgramDesc;
		}

		void SetHullProgramDesc(Pass* obj, SerializedGpuProgramData& val)
		{
			obj->mData.hullProgramDesc = val;
		}

		SerializedGpuProgramData& GetDomainProgramDesc(Pass* obj)
		{
			return mDomainProgramDesc;
		}

		void SetDomainProgramDesc(Pass* obj, SerializedGpuProgramData& val)
		{
			obj->mData.domainProgramDesc = val;
		}

		SerializedGpuProgramData& GetComputeProgramDesc(Pass* obj)
		{
			return mComputeProgramDesc;
		}

		void SetComputeProgramDesc(Pass* obj, SerializedGpuProgramData& val)
		{
			obj->mData.computeProgramDesc = val;
		}
	public:
		PassRTTI()
		{
			addReflectableField("mVertexProgramDesc", 3, &PassRTTI::getVertexProgramDesc, &PassRTTI::setVertexProgramDesc);
			addReflectableField("mFragmentProgramDesc", 4, &PassRTTI::getFragmentProgramDesc, &PassRTTI::setFragmentProgramDesc);
			addReflectableField("mGeometryProgramDesc", 5, &PassRTTI::getGeometryProgramDesc, &PassRTTI::setGeometryProgramDesc);
			addReflectableField("mHullProgramDesc", 6, &PassRTTI::getHullProgramDesc, &PassRTTI::setHullProgramDesc);
			addReflectableField("mDomainProgramDesc", 7, &PassRTTI::getDomainProgramDesc, &PassRTTI::setDomainProgramDesc);
			addReflectableField("mComputeProgramDesc", 8, &PassRTTI::getComputeProgramDesc, &PassRTTI::setComputeProgramDesc);
		}

		void OnSerializationStarted(IReflectable* obj, SerializationContext* context) override
		{
			Pass* pass = static_cast<Pass*>(obj);

			mVertexProgramDesc = pass->mData.vertexProgramDesc;
			mFragmentProgramDesc = pass->mData.fragmentProgramDesc;
			mGeometryProgramDesc = pass->mData.geometryProgramDesc;
			mHullProgramDesc = pass->mData.hullProgramDesc;
			mDomainProgramDesc = pass->mData.domainProgramDesc;
			mComputeProgramDesc = pass->mData.computeProgramDesc;

			auto initBytecode = [](const SPtr<GpuProgram>& prog, GPU_PROGRAM_DESC& desc)
			{
				if (prog)
				{
					prog->BlockUntilCoreInitialized();
					desc.bytecode = prog->GetCore()->getBytecode();
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

		const String& GetRTTIName() override
		{
			static String name = "Pass";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_Pass;
		}

		SPtr<IReflectable> NewRTTIObject() override
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
}
