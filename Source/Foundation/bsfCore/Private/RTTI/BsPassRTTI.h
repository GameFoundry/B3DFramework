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
		const String& getRTTIName() override
		{
			static String name = "SerializedGpuProgramData";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializedGpuProgramData;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<SerializedGpuProgramData>();
		}
	};

	RTTITypeBase* SerializedGpuProgramData::getRTTIStatic()
	{
		return SerializedGpuProgramDataRTTI::instance();
	}

	RTTITypeBase* SerializedGpuProgramData::getRTTI() const
	{
		return getRTTIStatic();
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

		SerializedGpuProgramData& getVertexProgramDesc(Pass* obj)
		{
			return mVertexProgramDesc;
		}

		void setVertexProgramDesc(Pass* obj, SerializedGpuProgramData& val)
		{
			obj->mData.vertexProgramDesc = val;
		}

		SerializedGpuProgramData& getFragmentProgramDesc(Pass* obj)
		{
			return mFragmentProgramDesc;
		}

		void setFragmentProgramDesc(Pass* obj, SerializedGpuProgramData& val)
		{
			obj->mData.fragmentProgramDesc = val;
		}

		SerializedGpuProgramData& getGeometryProgramDesc(Pass* obj)
		{
			return mGeometryProgramDesc;
		}

		void setGeometryProgramDesc(Pass* obj, SerializedGpuProgramData& val)
		{
			obj->mData.geometryProgramDesc = val;
		}

		SerializedGpuProgramData& getHullProgramDesc(Pass* obj)
		{
			return mHullProgramDesc;
		}

		void setHullProgramDesc(Pass* obj, SerializedGpuProgramData& val)
		{
			obj->mData.hullProgramDesc = val;
		}

		SerializedGpuProgramData& getDomainProgramDesc(Pass* obj)
		{
			return mDomainProgramDesc;
		}

		void setDomainProgramDesc(Pass* obj, SerializedGpuProgramData& val)
		{
			obj->mData.domainProgramDesc = val;
		}

		SerializedGpuProgramData& getComputeProgramDesc(Pass* obj)
		{
			return mComputeProgramDesc;
		}

		void setComputeProgramDesc(Pass* obj, SerializedGpuProgramData& val)
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

		void onSerializationStarted(IReflectable* obj, SerializationContext* context) override
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
					prog->blockUntilCoreInitialized();
					desc.bytecode = prog->getCore()->getBytecode();
				}
			};

			const SPtr<GraphicsPipelineState>& graphicsPipeline = pass->getGraphicsPipelineState();
			if(graphicsPipeline)
			{
				initBytecode(graphicsPipeline->getVertexProgram(), mVertexProgramDesc);
				initBytecode(graphicsPipeline->getFragmentProgram(), mFragmentProgramDesc);
				initBytecode(graphicsPipeline->getGeometryProgram(), mGeometryProgramDesc);
				initBytecode(graphicsPipeline->getHullProgram(), mHullProgramDesc);
				initBytecode(graphicsPipeline->getDomainProgram(), mDomainProgramDesc);
			}
			
			const SPtr<ComputePipelineState>& computePipeline = pass->getComputePipelineState();
			if(computePipeline)
				initBytecode(computePipeline->getProgram(), mComputeProgramDesc);
		}

		void onDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			Pass* pass = static_cast<Pass*>(obj);
			pass->initialize();
		}

		const String& getRTTIName() override
		{
			static String name = "Pass";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_Pass;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return Pass::createEmpty();
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
