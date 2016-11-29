//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsGpuParamDesc.h"
#include "BsGpuParams.h"
#include "BsGpuPipelineParamInfo.h"
#include "BsRenderAPI.h"
#include "BsGpuParamBlockBuffer.h"

namespace bs
{
	/** @addtogroup Renderer-Internal
	 *  @{
	 */

// Note: Every time one of these param blocks is instantiated we generate its descriptor. It would be better to generate
// it once, and then just quickly instantiate for subsequent creations.

/**
 * Starts a new custom parameter block. Custom parameter blocks allow you to create C++ structures that map directly
 * to GPU program buffers (for example uniform buffer in OpenGL or constant buffer in DX). Must be followed by
 * BS_PARAM_BLOCK_END.
 */
#define BS_PARAM_BLOCK_BEGIN(Name)																							\
	struct Name																												\
	{																														\
		Name()																												\
		{																													\
			Vector<GpuParamDataDesc> params = getEntries();																	\
			RenderAPICore& rapi = RenderAPICore::instance();																\
																															\
			mBlockDesc = rapi.generateParamBlockDesc(#Name, params);														\
																															\
			SPtr<GpuParamDesc> paramsDesc = bs_shared_ptr_new<GpuParamDesc>();												\
			paramsDesc->paramBlocks[#Name] = mBlockDesc;																	\
			for (auto& param : params)																						\
				paramsDesc->params[param.name] = param;																		\
																															\
			GPU_PIPELINE_PARAMS_DESC pipelineParamDesc;																		\
			pipelineParamDesc.vertexParams = paramsDesc;																	\
			SPtr<GpuPipelineParamInfoCore> paramInfo = GpuPipelineParamInfoCore::create(pipelineParamDesc);					\
			mParams = GpuParamsCore::create(paramInfo);																		\
																															\
			mBuffer = GpuParamBlockBufferCore::create(mBlockDesc.blockSize * sizeof(UINT32));								\
			mParams->setParamBlockBuffer(GPT_VERTEX_PROGRAM, #Name, mBuffer);												\
			initEntries();																									\
		}																													\
																															\
		const SPtr<GpuParamBlockBufferCore>& getBuffer() const { return mBuffer; }											\
		void setBuffer(const SPtr<GpuParamBlockBufferCore>& buffer)															\
		{																													\
				mBuffer = buffer;																							\
				mParams->setParamBlockBuffer(GPT_VERTEX_PROGRAM, #Name, mBuffer);											\
		}																													\
		const GpuParamBlockDesc& getDesc() const { return mBlockDesc; }														\
		void flushToGPU(UINT32 queueIdx = 0) { mBuffer->flushToGPU(queueIdx); }												\
																															\
	private:																												\
		struct META_FirstEntry {};																							\
		static void META_GetPrevEntries(Vector<GpuParamDataDesc>& params, META_FirstEntry id) { }							\
		void META_InitPrevEntry(const SPtr<GpuParamsCore>& params, META_FirstEntry id) { }									\
																															\
		typedef META_FirstEntry 

/**
 * Registers a new entry in a parameter block. Must be called in between BS_PARAM_BLOCK_BEGIN and BS_PARAM_BLOCK_END calls.
 */
#define BS_PARAM_BLOCK_ENTRY_ARRAY(Type, Name, NumElements)																	\
		META_Entry_##Name;																									\
																															\
		struct META_NextEntry_##Name {};																					\
		static void META_GetPrevEntries(Vector<GpuParamDataDesc>& params, META_NextEntry_##Name id)							\
		{																													\
			META_GetPrevEntries(params, META_Entry_##Name());																\
																															\
			params.push_back(GpuParamDataDesc());																			\
			GpuParamDataDesc& newEntry = params.back();																		\
			newEntry.name = #Name;																							\
			newEntry.type = (GpuParamDataType)TGpuDataParamInfo<Type>::TypeId;												\
			newEntry.arraySize = NumElements;																				\
		}																													\
																															\
		void META_InitPrevEntry(const SPtr<GpuParamsCore>& params, META_NextEntry_##Name id)								\
		{																													\
			META_InitPrevEntry(params, META_Entry_##Name());																\
			params->getParam(GPT_VERTEX_PROGRAM, #Name, Name);																\
		}																													\
																															\
	public:																													\
		TGpuDataParam<Type, true> Name;																						\
																															\
	private:																												\
		typedef META_NextEntry_##Name

/** 
 * Registers a new entry in a parameter block. Must be called in between BS_PARAM_BLOCK_BEGIN and BS_PARAM_BLOCK_END calls. 
 */
#define BS_PARAM_BLOCK_ENTRY(Type, Name) BS_PARAM_BLOCK_ENTRY_ARRAY(Type, Name, 1)

/** Ends parameter block definition. See BS_PARAM_BLOCK_BEGIN. */
#define BS_PARAM_BLOCK_END																									\
		META_LastEntry;																										\
																															\
		static Vector<GpuParamDataDesc> getEntries()																		\
		{																													\
			Vector<GpuParamDataDesc> entries;																				\
			META_GetPrevEntries(entries, META_LastEntry());																	\
			return entries;																									\
		}																													\
																															\
		void initEntries()																									\
		{																													\
			META_InitPrevEntry(mParams, META_LastEntry());																	\
		}																													\
																															\
		SPtr<GpuParamsCore> mParams;																						\
		SPtr<GpuParamBlockBufferCore> mBuffer;																				\
		GpuParamBlockDesc mBlockDesc;																						\
	};

	/** @} */
}