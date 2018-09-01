//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Material/BsMaterial.h"
#include "Material/BsShader.h"
#include "Material/BsPass.h"

namespace bs
{
	/** @addtogroup Implementation
	 *  @{
	 */

	/** Contains a set of GpuParams used for a single technique within a Material. */
	template<bool Core>
	class BS_CORE_EXPORT TGpuParamsSet
	{
		using GpuParamsType = CoreVariantType<GpuParams, Core>;
		using MaterialParamsType = CoreVariantType<MaterialParams, Core>;
		using ParamBlockPtrType = SPtr<CoreVariantType<GpuParamBlockBuffer, Core>>;
		using TechniqueType = CoreVariantType<Technique, Core>;
		using ShaderType = CoreVariantHandleType<Shader, Core>;
		using PassType = CoreVariantType<Pass, Core>;
		using GpuProgramPtrType = SPtr<CoreVariantType<GpuProgram, Core>>;
		using ParamBlockType = CoreVariantType<GpuParamBlockBuffer, Core>;
		using TextureType = CoreVariantHandleType<Texture, Core>;
		using BufferType = SPtr<CoreVariantType<GpuBuffer, Core>>;
		using SamplerStateType = SPtr<CoreVariantType<SamplerState, Core>>;
		using GraphicsPipelineStateType = CoreVariantType<GraphicsPipelineState, Core>;
		using ComputePipelineStateType = CoreVariantType<ComputePipelineState, Core>;

		/** Binding location for a single GPU param block buffer. */
		struct BlockBinding
		{
			UINT32 set;
			UINT32 slot;
		};

		/** All bindings for GPU param block buffers, for a single pass. */
		struct PassBlockBindings
		{
			BlockBinding bindings[GPT_COUNT];
		};

		/** Information about a parameter block buffer. */
		struct BlockInfo
		{
			BlockInfo(const String& name, UINT32 set, UINT32 slot, const ParamBlockPtrType& buffer, bool shareable)
				: name(name), set(set), slot(slot), buffer(buffer), shareable(shareable), allowUpdate(true), isUsed(true)
				, passData(nullptr)
			{ }

			String name;
			UINT32 set;
			UINT32 slot;
			ParamBlockPtrType buffer;
			bool shareable;
			bool allowUpdate;
			bool isUsed;

			PassBlockBindings* passData;
		};

		/** Information about how a data parameter maps from a material parameter into a parameter block buffer. */
		struct DataParamInfo
		{
			UINT32 paramIdx;
			UINT32 blockIdx;
			UINT32 offset;
		};

		/** Information about how an object parameter maps from a material parameter to a GPU stage slot. */
		struct ObjectParamInfo
		{
			UINT32 paramIdx;
			UINT32 slotIdx;
			UINT32 setIdx;
		};

		/** Information about all object parameters for a specific GPU programmable stage. */
		struct StageParamInfo
		{
			ObjectParamInfo* textures;
			UINT32 numTextures;
			ObjectParamInfo* loadStoreTextures;
			UINT32 numLoadStoreTextures;
			ObjectParamInfo* buffers;
			UINT32 numBuffers;
			ObjectParamInfo* samplerStates;
			UINT32 numSamplerStates;
		};

		/** Information about all object parameters for a specific pass. */
		struct PassParamInfo
		{
			StageParamInfo stages[GPT_COUNT];
		};

	public:
		TGpuParamsSet() {}
		TGpuParamsSet(const SPtr<TechniqueType>& technique, const ShaderType& shader,
			const SPtr<MaterialParamsType>& params);
		~TGpuParamsSet();

		/** 
		 * Returns a set of GPU parameters for the specified pass. 
		 *
		 * @param[in]	passIdx		Pass in which to look the GPU program for in.
		 * @return					GPU parameters object that can be used for setting parameters of all GPU programs 
		 *							in a pass. Returns null if pass doesn't exist.
		 */
		SPtr<GpuParamsType> getGpuParams(UINT32 passIdx = 0);

		/** 
		 * Searches for a parameter block buffer with the specified name, and returns an index you can use for accessing it.
		 * Returns -1 if buffer was not found.
		 */
		UINT32 getParamBlockBufferIndex(const String& name) const;

		/**
		 * Assign a parameter block buffer with the specified index to all the relevant child GpuParams.
		 *
		 * @param[in]	index			Index of the buffer, as retrieved from getParamBlockBufferIndex().
		 * @param[in]	paramBlock		Parameter block to assign.
		 * @param[in]	ignoreInUpdate	If true the buffer will not be updated during the update() call. This is useful
		 *								if the caller wishes to manually update the buffer contents externally, to prevent
		 *								overwriting manually written data during update.
		 *
		 * @note	
		 * Parameter block buffers can be used as quick way of setting multiple parameters on a material at once, or
		 * potentially sharing parameters between multiple materials. This reduces driver overhead as the parameters
		 * in the buffers need only be set once and then reused multiple times.
		 */
		void setParamBlockBuffer(UINT32 index, const ParamBlockPtrType& paramBlock, bool ignoreInUpdate = false);

		/**
		 * Assign a parameter block buffer with the specified name to all the relevant child GpuParams.
		 *
		 * @param[in]	name			Name of the buffer to set.
		 * @param[in]	paramBlock		Parameter block to assign.
		 * @param[in]	ignoreInUpdate	If true the buffer will not be updated during the update() call. This is useful
		 *								if the caller wishes to manually update the buffer contents externally, to prevent
		 *								overwriting manually written data during update.
		 *
		 * @note	
		 * Parameter block buffers can be used as quick way of setting multiple parameters on a material at once, or
		 * potentially sharing parameters between multiple materials. This reduces driver overhead as the parameters
		 * in the buffers need only be set once and then reused multiple times.
		 */
		void setParamBlockBuffer(const String& name, const ParamBlockPtrType& paramBlock, bool ignoreInUpdate = false);

		/** Returns the number of passes the set contains the parameters for. */
		UINT32 getNumPasses() const { return (UINT32)mPassParams.size(); }

		/**
		 * Updates parameter data in this object from the provided material parameters object.
		 *
		 * @param[in]	params			Object containing the parameter data to update from. Layout of the object must match
		 *								the object used for creating this object (be created for the same shader).
		 * @param[in]	t				Time to evaluate animated parameters at (if any).
		 * @param[in]	updateAll		Normally the system will track dirty parameters since the last call to this method,
		 *								and only update the dirty ones. Set this to true if you want to force all parameters
		 *								to update, regardless of their dirty state.
		 */
		void update(const SPtr<MaterialParamsType>& params, float t = 0.0f, bool updateAll = false);

		static const UINT32 NUM_STAGES;
	private:
		template<bool Core2> friend class TMaterial;

		Vector<SPtr<GpuParamsType>> mPassParams;
		Vector<BlockInfo> mBlocks;
		Vector<DataParamInfo> mDataParamInfos;
		PassParamInfo* mPassParamInfos;

		UINT64 mParamVersion;
		UINT8* mData;
	};

	/** Sim thread version of TGpuParamsSet<Core>. */
	class BS_CORE_EXPORT GpuParamsSet : public TGpuParamsSet<false>
	{
	public:
		GpuParamsSet() { }
		GpuParamsSet(const SPtr<Technique>& technique, const HShader& shader, 
			const SPtr<MaterialParams>& params)
			:TGpuParamsSet(technique, shader, params)
		{ }
	};

	namespace ct
	{
	/** Core thread version of TGpuParamsSet<Core>. */
	class BS_CORE_EXPORT GpuParamsSet : public TGpuParamsSet<true>
	{
	public:
		GpuParamsSet() { }
		GpuParamsSet(const SPtr<Technique>& technique, const SPtr<Shader>& shader,
			const SPtr<MaterialParams>& params)
			:TGpuParamsSet(technique, shader, params)
		{ }
	};
	}

	/** @} */
}