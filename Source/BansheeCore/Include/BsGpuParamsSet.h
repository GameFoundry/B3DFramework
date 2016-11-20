//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsMaterial.h"
#include "BsShader.h"
#include "BsPass.h"

namespace bs
{
	/** @addtogroup Implementation
	 *  @{
	 */

	template<bool Core> struct TGpuParamsType { };
	template<> struct TGpuParamsType<false> { typedef GpuParams Type; };
	template<> struct TGpuParamsType<true> { typedef GpuParamsCore Type; };

	/** Contains a set of GpuParams used for a single technique within a Material. */
	template<bool Core>
	class BS_CORE_EXPORT TGpuParamsSet
	{
		typedef typename TGpuParamsType<Core>::Type GpuParamsType;
		typedef typename TMaterialParamsType<Core>::Type MaterialParamsType;
		typedef typename TGpuParamBlockBufferPtrType<Core>::Type ParamBlockPtrType;
		typedef typename TTechniqueType<Core>::Type TechniqueType;
		typedef typename TShaderType<Core>::Type ShaderType;
		typedef typename TPassType<Core>::Type PassType;
		typedef typename TGpuProgramType<Core>::Type GpuProgramPtrType;
		typedef typename TGpuParamBlockBufferType<Core>::Type ParamBlockType;
		typedef typename TGpuParamTextureType<Core>::Type TextureType;
		typedef typename TGpuBufferType<Core>::Type BufferType;
		typedef typename TGpuParamSamplerStateType<Core>::Type SamplerStateType;
		typedef typename TPassTypes<Core>::GraphicsPipelineStateType GraphicsPipelineStateType;
		typedef typename TPassTypes<Core>::ComputePipelineStateType ComputePipelineStateType;

		/** Information about a parameter block buffer. */
		struct BlockInfo
		{
			BlockInfo(const String& name, const ParamBlockPtrType& buffer, bool shareable)
				:name(name), buffer(buffer), shareable(shareable), allowUpdate(true), isUsed(true)
			{ }

			String name;
			ParamBlockPtrType buffer;
			bool shareable;
			bool allowUpdate;
			bool isUsed;
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
			StageParamInfo stages[6];
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
		 * Updates internal GPU params for all passes and stages from the provided material parameters object.
		 *
		 * @param[in]	params			Object containing the parameter data to update from. Layout of the object must match the
		 *								object used for creating this object (be created for the same shader).
		 * @param[in]	dirtyBitIdx		Index to use when checking if parameters are dirty. Must be in range [0, 31]. Allows
		 *								the same material params to record dirty state for multiple sets of GPU params
		 *								(each with their own index).
		 * @param[in]	updateAll		By default the system will only update parameters marked as dirty in @p params. If this
		 *								is set to true, all parameters will be updated instead.
		 */
		void update(const SPtr<MaterialParamsType>& params, UINT32 dirtyBitIdx, bool updateAll = false);

		static const UINT32 NUM_STAGES;
	private:
		template<bool Core2> friend class TMaterial;

		Vector<SPtr<GpuParamsType>> mPassParams;
		Vector<BlockInfo> mBlocks;
		Vector<DataParamInfo> mDataParamInfos;
		PassParamInfo* mPassParamInfos;
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

	/** Core thread version of TGpuParamsSet<Core>. */
	class BS_CORE_EXPORT GpuParamsSetCore : public TGpuParamsSet<true>
	{
	public:
		GpuParamsSetCore() { }
		GpuParamsSetCore(const SPtr<TechniqueCore>& technique, const SPtr<ShaderCore>& shader,
			const SPtr<MaterialParamsCore>& params)
			:TGpuParamsSet(technique, shader, params)
		{ }
	};

	/** @} */
}