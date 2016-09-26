//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsPass.h"

namespace BansheeEngine
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT PassRTTI : public RTTIType<Pass, IReflectable, PassRTTI>
	{
	private:
		SPtr<BlendState> getBlendState(Pass* obj) { return obj->mData.blendState; }
		void setBlendState(Pass* obj, SPtr<BlendState> val) { obj->mData.blendState = val; }

		SPtr<RasterizerState> getRasterizerState(Pass* obj) { return obj->mData.rasterizerState; }
		void setRasterizerState(Pass* obj, SPtr<RasterizerState> val) { obj->mData.rasterizerState = val; }

		SPtr<DepthStencilState> getDepthStencilState(Pass* obj) { return obj->mData.depthStencilState; }
		void setDepthStencilState(Pass* obj, SPtr<DepthStencilState> val) { obj->mData.depthStencilState = val; }

		UINT32& getStencilRefValue(Pass* obj) { return obj->mData.stencilRefValue; }
		void setStencilRefValue(Pass* obj, UINT32& val) { obj->mData.stencilRefValue = val; }

		SPtr<GpuProgram> getVertexProgram(Pass* obj) { return obj->mData.vertexProgram; }
		void setVertexProgram(Pass* obj, SPtr<GpuProgram> val) { obj->mData.vertexProgram = val; }

		SPtr<GpuProgram> getFragmentProgram(Pass* obj) { return obj->mData.fragmentProgram; }
		void setFragmentProgram(Pass* obj, SPtr<GpuProgram> val) { obj->mData.fragmentProgram = val; }

		SPtr<GpuProgram> getGeometryProgram(Pass* obj) { return obj->mData.geometryProgram; }
		void setGeometryProgram(Pass* obj, SPtr<GpuProgram> val) { obj->mData.geometryProgram = val; }

		SPtr<GpuProgram> getHullProgram(Pass* obj) { return obj->mData.hullProgram; }
		void setHullProgram(Pass* obj, SPtr<GpuProgram> val) { obj->mData.hullProgram = val; }

		SPtr<GpuProgram> getDomainProgram(Pass* obj) { return obj->mData.domainProgram; }
		void setDomainProgram(Pass* obj, SPtr<GpuProgram> val) { obj->mData.domainProgram = val; }

		SPtr<GpuProgram> getComputeProgram(Pass* obj) { return obj->mData.computeProgram; }
		void setComputeProgram(Pass* obj, SPtr<GpuProgram> val) { obj->mData.computeProgram = val; }
	public:
		PassRTTI()
		{
			addReflectablePtrField("mBlendState", 0, &PassRTTI::getBlendState, &PassRTTI::setBlendState);
			addReflectablePtrField("mRasterizerState", 1, &PassRTTI::getRasterizerState, &PassRTTI::setRasterizerState);
			addReflectablePtrField("mDepthStencilState", 2, &PassRTTI::getDepthStencilState, &PassRTTI::setDepthStencilState);

			addReflectablePtrField("mVertexProgram", 3, &PassRTTI::getVertexProgram, &PassRTTI::setVertexProgram);
			addReflectablePtrField("mFragmentProgram", 4, &PassRTTI::getFragmentProgram, &PassRTTI::setFragmentProgram);
			addReflectablePtrField("mGeometryProgram", 5, &PassRTTI::getGeometryProgram, &PassRTTI::setGeometryProgram);
			addReflectablePtrField("mHullProgram", 6, &PassRTTI::getHullProgram, &PassRTTI::setHullProgram);
			addReflectablePtrField("mDomainProgram", 7, &PassRTTI::getDomainProgram, &PassRTTI::setDomainProgram);
			addReflectablePtrField("mComputeProgram", 8, &PassRTTI::getComputeProgram, &PassRTTI::setComputeProgram);

			addPlainField("mStencilRefValue", 9, &PassRTTI::getStencilRefValue, &PassRTTI::setStencilRefValue);
		}

		void onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
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
	};

	/** @} */
	/** @endcond */
}