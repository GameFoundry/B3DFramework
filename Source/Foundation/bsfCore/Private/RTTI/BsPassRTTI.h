//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Private/RTTI/BsGpuProgramRTTI.h"
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
		/* 								RTTI		                     		*/
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
		SerializedGpuProgramDataRTTI()
			:mInitMembers(this)
		{ }

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
		struct GpuProgramInfo
		{
			SerializedGpuProgramData vertexProgramDesc;
			SerializedGpuProgramData fragmentProgramDesc;
			SerializedGpuProgramData geometryProgramDesc;
			SerializedGpuProgramData hullProgramDesc;
			SerializedGpuProgramData domainProgramDesc;
			SerializedGpuProgramData computeProgramDesc;
		};

		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(blendStateDesc, mData.blendStateDesc, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(rasterizerStateDesc, mData.rasterizerStateDesc, 1)
			BS_RTTI_MEMBER_PLAIN_NAMED(depthStencilState, mData.depthStencilStateDesc, 2)

			BS_RTTI_MEMBER_PLAIN_NAMED(stencilRefValue, mData.stencilRefValue, 9)
		BS_END_RTTI_MEMBERS

		SerializedGpuProgramData& getVertexProgramDesc(Pass* obj)
		{
			GpuProgramInfo& info = any_cast_ref<GpuProgramInfo>(obj->mRTTIData);
			return info.vertexProgramDesc;
		}

		void setVertexProgramDesc(Pass* obj, SerializedGpuProgramData& val)
		{
			obj->mData.vertexProgramDesc = val;
		}

		SerializedGpuProgramData& getFragmentProgramDesc(Pass* obj) 
		{
			GpuProgramInfo& info = any_cast_ref<GpuProgramInfo>(obj->mRTTIData);
			return info.fragmentProgramDesc;
		}

		void setFragmentProgramDesc(Pass* obj, SerializedGpuProgramData& val)
		{
			obj->mData.fragmentProgramDesc = val;
		}

		SerializedGpuProgramData& getGeometryProgramDesc(Pass* obj)
		{
			GpuProgramInfo& info = any_cast_ref<GpuProgramInfo>(obj->mRTTIData);
			return info.geometryProgramDesc;
		}

		void setGeometryProgramDesc(Pass* obj, SerializedGpuProgramData& val)
		{
			obj->mData.geometryProgramDesc = val;
		}

		SerializedGpuProgramData& getHullProgramDesc(Pass* obj)
		{
			GpuProgramInfo& info = any_cast_ref<GpuProgramInfo>(obj->mRTTIData);
			return info.hullProgramDesc;
		}

		void setHullProgramDesc(Pass* obj, SerializedGpuProgramData& val)
		{
			obj->mData.hullProgramDesc = val;
		}

		SerializedGpuProgramData& getDomainProgramDesc(Pass* obj)
		{
			GpuProgramInfo& info = any_cast_ref<GpuProgramInfo>(obj->mRTTIData);
			return info.domainProgramDesc;
		}

		void setDomainProgramDesc(Pass* obj, SerializedGpuProgramData& val)
		{
			obj->mData.domainProgramDesc = val;
		}

		SerializedGpuProgramData& getComputeProgramDesc(Pass* obj)
		{
			GpuProgramInfo& info = any_cast_ref<GpuProgramInfo>(obj->mRTTIData);
			return info.computeProgramDesc;
		}

		void setComputeProgramDesc(Pass* obj, SerializedGpuProgramData& val)
		{
			obj->mData.computeProgramDesc = val;
		}
	public:
		PassRTTI()
			:mInitMembers(this)
		{
			addReflectableField("mVertexProgramDesc", 3, &PassRTTI::getVertexProgramDesc, &PassRTTI::setVertexProgramDesc);
			addReflectableField("mFragmentProgramDesc", 4, &PassRTTI::getFragmentProgramDesc, &PassRTTI::setFragmentProgramDesc);
			addReflectableField("mGeometryProgramDesc", 5, &PassRTTI::getGeometryProgramDesc, &PassRTTI::setGeometryProgramDesc);
			addReflectableField("mHullProgramDesc", 6, &PassRTTI::getHullProgramDesc, &PassRTTI::setHullProgramDesc);
			addReflectableField("mDomainProgramDesc", 7, &PassRTTI::getDomainProgramDesc, &PassRTTI::setDomainProgramDesc);
			addReflectableField("mComputeProgramDesc", 8, &PassRTTI::getComputeProgramDesc, &PassRTTI::setComputeProgramDesc);
		}

		void onSerializationStarted(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			Pass* pass = static_cast<Pass*>(obj);

			GpuProgramInfo info;
			info.vertexProgramDesc = pass->mData.vertexProgramDesc;
			info.fragmentProgramDesc = pass->mData.fragmentProgramDesc;
			info.geometryProgramDesc = pass->mData.geometryProgramDesc;
			info.hullProgramDesc = pass->mData.hullProgramDesc;
			info.domainProgramDesc = pass->mData.domainProgramDesc;
			info.computeProgramDesc = pass->mData.computeProgramDesc;

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
				initBytecode(graphicsPipeline->getVertexProgram(), info.vertexProgramDesc);
				initBytecode(graphicsPipeline->getFragmentProgram(), info.fragmentProgramDesc);
				initBytecode(graphicsPipeline->getGeometryProgram(), info.geometryProgramDesc);
				initBytecode(graphicsPipeline->getHullProgram(), info.hullProgramDesc);
				initBytecode(graphicsPipeline->getDomainProgram(), info.domainProgramDesc);
			}
			
			const SPtr<ComputePipelineState>& computePipeline = pass->getComputePipelineState();
			if(computePipeline)
				initBytecode(computePipeline->getProgram(), info.computeProgramDesc);

			pass->mRTTIData = info;
		}

		void onSerializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			Pass* pass = static_cast<Pass*>(obj);
			pass->mRTTIData = nullptr;
		}

		void onDeserializationStarted(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			Pass* pass = static_cast<Pass*>(obj);
			pass->mRTTIData = GpuProgramInfo();
		}

		void onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			Pass* pass = static_cast<Pass*>(obj);
			pass->initialize();

			pass->mRTTIData = nullptr;
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
	};

	/** @} */
	/** @endcond */
}