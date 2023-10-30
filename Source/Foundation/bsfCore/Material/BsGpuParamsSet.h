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

	/** Contains a set of GpuParameters used for a single technique within a Material. */
	template <bool IsRenderProxy>
	class B3D_CORE_EXPORT TGpuParamsSet
	{
		using GpuParamsType = CoreVariantType<GpuParameters, IsRenderProxy>;
		using MaterialParamsType = CoreVariantType<MaterialParams, IsRenderProxy>;
		using ParamBlockPtrType = SPtr<CoreVariantType<GpuBuffer, IsRenderProxy>>;
		using TechniqueType = CoreVariantType<Technique, IsRenderProxy>;
		using ShaderType = CoreVariantHandleType<Shader, IsRenderProxy>;
		using PassType = CoreVariantType<Pass, IsRenderProxy>;
		using ParamBlockType = CoreVariantType<GpuBuffer, IsRenderProxy>;
		using TextureType = CoreVariantHandleType<Texture, IsRenderProxy>;
		using BufferType = SPtr<CoreVariantType<GpuBuffer, IsRenderProxy>>;

		/** Binding location for a single GPU param block buffer. */
		struct BlockBinding
		{
			u32 Set;
			u32 Slot;
		};

		/** All bindings for GPU param block buffers, for a single pass. */
		struct PassBlockBindings
		{
			BlockBinding Bindings[GPT_COUNT];
		};

		/** Information about a parameter block buffer. */
		struct BlockInfo
		{
			BlockInfo(const String& name, u32 set, u32 slot, const ParamBlockPtrType& buffer, bool shareable)
				: Name(name), Set(set), Slot(slot), Buffer(buffer), Shareable(shareable), AllowUpdate(true), IsUsed(true), PassData(nullptr)
			{}

			String Name;
			u32 Set;
			u32 Slot;
			ParamBlockPtrType Buffer;
			bool Shareable;
			bool AllowUpdate;
			bool IsUsed;

			PassBlockBindings* PassData;
		};

		/** Information about how a data parameter maps from a material parameter into a parameter block buffer. */
		struct DataParamInfo
		{
			u32 ParamIdx;
			u32 BlockIdx;
			u32 Offset;
			u32 ArrayStride;
		};

		/** Information about how an object parameter maps from a material parameter to a GPU stage slot. */
		struct ObjectParamInfo
		{
			u32 ParamIdx;
			u32 SlotIdx;
			u32 SetIdx;
		};

		/** Information about all object parameters for a specific GPU programmable stage. */
		struct StageParamInfo
		{
			ObjectParamInfo* SampledTextures;
			u32 SampledTextureCount;
			ObjectParamInfo* StorageTextures;
			u32 StorageTextureCount;
			ObjectParamInfo* Buffers;
			u32 BufferCount;
			ObjectParamInfo* SamplerStates;
			u32 SamplerStateCount;
		};

		/** Information about all object parameters for a specific pass. */
		struct PassParamInfo
		{
			StageParamInfo Stages[GPT_COUNT];
		};

	public:
		TGpuParamsSet() = default;
		TGpuParamsSet(const SPtr<TechniqueType>& technique, const ShaderType& shader, const SPtr<MaterialParamsType>& params);
		~TGpuParamsSet();

		/**
		 * Returns a set of GPU parameters for the specified pass.
		 *
		 * @param[in]	passIdx		Pass in which to look the GPU program for in.
		 * @return					GPU parameters object that can be used for setting parameters of all GPU programs
		 *							in a pass. Returns null if pass doesn't exist.
		 */
		SPtr<GpuParamsType> GetGpuParams(u32 passIdx = 0);

		/**
		 * Searches for a parameter block buffer with the specified name, and returns an index you can use for accessing it.
		 * Returns -1 if buffer was not found.
		 */
		u32 GetParamBlockBufferIndex(const String& name) const;

		/**
		 * Assign a parameter block buffer with the specified index to all the relevant child GpuParameters.
		 *
		 * @param[in]	index			Index of the buffer, as retrieved from GetParamBlockBufferIndex().
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
		void SetParamBlockBuffer(u32 index, const ParamBlockPtrType& paramBlock, bool ignoreInUpdate = false);

		/**
		 * Assign a parameter block buffer with the specified name to all the relevant child GpuParameters.
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
		void SetParamBlockBuffer(const String& name, const ParamBlockPtrType& paramBlock, bool ignoreInUpdate = false);

		/** Returns the number of passes the set contains the parameters for. */
		u32 GetNumPasses() const { return (u32)mPassParams.size(); }

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
		void Update(const SPtr<MaterialParamsType>& params, float t = 0.0f, bool updateAll = false);

		static const u32 kNumStages;

	private:
		template <bool IsRenderProxy2>
		friend class TMaterial;

		Vector<SPtr<GpuParamsType>> mPassParams;
		Vector<BlockInfo> mBlocks;
		Vector<DataParamInfo> mDataParamInfos;
		PassParamInfo* mPassParamInfos;

		u64 mParamVersion;
		u8* mData;
	};

	/** Main thread version of TGpuParamsSet<IsRenderProxy>. */
	class B3D_CORE_EXPORT GpuParamsSet : public TGpuParamsSet<false>
	{
	public:
		GpuParamsSet() = default;

		GpuParamsSet(const SPtr<Technique>& technique, const HShader& shader, const SPtr<MaterialParams>& params)
			: TGpuParamsSet(technique, shader, params)
		{}
	};

	namespace ct
	{
		/** Render thread version of TGpuParamsSet<IsRenderProxy>. */
		class B3D_CORE_EXPORT GpuParamsSet : public TGpuParamsSet<true>
		{
		public:
			GpuParamsSet() = default;

			GpuParamsSet(const SPtr<Technique>& technique, const SPtr<Shader>& shader, const SPtr<MaterialParams>& params)
				: TGpuParamsSet(technique, shader, params)
			{}
		};
	} // namespace ct

	/** @} */
} // namespace bs
