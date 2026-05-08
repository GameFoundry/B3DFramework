//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Material/B3DMaterial.h"
#include "Material/B3DShader.h"
#include "Material/B3DPass.h"

namespace b3d
{
	/** @addtogroup Material-Internal
	 *  @{
	 */

	/** Shared code between render and main thread versions of MaterialParameterAdapter. */
	template <bool IsRenderProxy>
	class B3D_EXPORT TMaterialParameterAdapter
	{
		using GpuParametersType = CoreVariantType<GpuParameterSet, IsRenderProxy>;
		using MaterialParametersType = CoreVariantType<MaterialParameters, IsRenderProxy>;
		using MaterialType = CoreVariantHandleType<Material, IsRenderProxy>;
		using UniformBufferPointerType = TShared<CoreVariantType<GpuBuffer, IsRenderProxy>>;
		using VariationType = CoreVariantType<Variation, IsRenderProxy>;
		using ShaderType = CoreVariantHandleType<Shader, IsRenderProxy>;
		using PassType = CoreVariantType<Pass, IsRenderProxy>;
		using UniformBufferType = CoreVariantType<GpuBuffer, IsRenderProxy>;
		using TextureType = CoreVariantHandleType<Texture, IsRenderProxy>;
		using BufferType = TShared<CoreVariantType<GpuBuffer, IsRenderProxy>>;

		/** Information about how a data parameter maps from a material parameter into a uniform buffer. */
		struct DataParamInfo
		{
			u32 ParameterIndex;
			u32 UniformBufferIndex;
			u32 Offset;
			u32 ArrayStride;
		};

		/** Information about how an object parameter maps from a material parameter to a GPU stage slot. */
		struct ObjectParamInfo
		{
			u32 ParameterIndex;
			u32 SlotIndex;
			u32 SetIndex;
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
		TMaterialParameterAdapter() = default;
		TMaterialParameterAdapter(const TShared<VariationType>& variation, const ShaderType& shader, const TShared<MaterialParametersType>& materialParameters);
		~TMaterialParameterAdapter();

		/**
		 * Returns a set of GPU parameters for the specified pass and descriptor set.
		 *
		 * @param passIndex		Pass in which to look the GPU program for in.
		 * @param setIndex		Descriptor set index to retrieve parameters for.
		 * @return				GPU parameters object that can be used for setting parameters of all GPU programs
		 *						in a pass. Returns null if pass or set doesn't exist.
		 */
		TShared<GpuParametersType> GetGpuParameterSet(u32 passIndex = 0, u32 setIndex = 0);

		/**
		 * Searches for a parameter uniform buffer with the specified name, and returns an index you can use for accessing it.
		 * Returns ~0u if buffer was not found.
		 */
		u32 GetUniformBufferIndex(const String& name) const;

		/**
		 * Assign a uniform buffer with the specified index to all the relevant child GpuParameterSet objects.
		 *
		 * @param index				Index of the buffer, as retrieved from GetUniformBufferIndex().
		 * @param buffer			Uniform buffer to assign.
		 * @param ignoreInUpdate	If true the buffer will not be updated during the Update() call. This is useful
		 *							if the caller wishes to manually update the buffer contents externally, to prevent
		 *							overwriting manually written data during update.
		 */
		void SetUniformBuffer(u32 index, const UniformBufferPointerType& buffer, bool ignoreInUpdate = false);

		/**
		 * Assign a uniform buffer with the specified name to all the relevant child GpuParameterSet objects.
		 *
		 * @param name				Name of the buffer to set.
		 * @param buffer			Uniform buffer to assign.
		 * @param ignoreInUpdate	If true the buffer will not be updated during the Update() call. This is useful
		 *							if the caller wishes to manually update the buffer contents externally, to prevent
		 *							overwriting manually written data during update.
		 */
		void SetUniformBuffer(const String& name, const UniformBufferPointerType& buffer, bool ignoreInUpdate = false);

		/** Returns the number of passes the set contains the parameters for. */
		u32 GetPassCount() const { return (u32)mGpuParametersPerPass.size(); }

		/** Returns the number of sets in the specified pass. */
		u32 GetSetCount(u32 passIndex);

		/**
		 * Updates parameter data in underlying GpuParameter objects from the provided material parameters object.
		 *
		 * @param material				Material containing parameters to update from. This object must have been created
		 *								from the same material (or the same shader/variation combination).
		 * @param t						Time to evaluate animated parameters at (if any).
		 * @param updateAll				Normally the system will track dirty parameters since the last call to this method,
		 *								and only update the dirty ones. Set this to true if you want to force all parameters
		 *								to update, regardless of their dirty state.
		 */
		void Update(const MaterialType& material, float t = 0.0f, bool updateAll = false);

		static const u32 kNumStages;

	protected:
		template <bool IsRenderProxy2>
		friend class TMaterial;

		/** Binding location for a single GPU uniform buffer. */
		struct UniformBufferBinding
		{
			u32 Set;
			u32 Slot;
		};

		/** All bindings for GPU uniform buffers, for a single pass. */
		struct PassUniformBufferBindings
		{
			UniformBufferBinding Bindings[GPT_COUNT];
		};

		/** Information about a uniform buffer. */
		struct UniformBufferInfo
		{
			UniformBufferInfo(const String& name, u32 set, u32 slot, const UniformBufferPointerType& buffer, u32 suballocationByteOffset, bool shareable)
				: Name(name), Set(set), Slot(slot), Buffer(buffer), SuballocationByteOffset(suballocationByteOffset), Shareable(shareable), AllowUpdate(true), IsUsed(true), PassData(nullptr)
			{}

			String Name;
			u32 Set;
			u32 Slot;
			UniformBufferPointerType Buffer;
			u32 SuballocationByteOffset;
			bool Shareable : 1;
			bool AllowUpdate : 1;
			bool IsUsed : 1;

			PassUniformBufferBindings* PassData;
		};

		Vector<TInlineArray<TShared<GpuParametersType>, 4>> mGpuParametersPerPass;
		Vector<UniformBufferInfo> mUniformBuffers;
		Vector<DataParamInfo> mDataParamInfos;
		PassParamInfo* mPassParamInfos;

		u64 mParamVersion;
		u8* mData;
	};

	/** Used for transferring material parameters to GpuParameterSet. */
	class B3D_EXPORT MaterialParameterAdapter : public TMaterialParameterAdapter<false>
	{
	public:
		MaterialParameterAdapter() = default;

		MaterialParameterAdapter(const TShared<Variation>& variation, const HShader& shader, const TShared<MaterialParameters>& params)
			: TMaterialParameterAdapter(variation, shader, params)
		{}
	};

	namespace render
	{
		/** Render thread version of MaterialParameterAdapter. */
		class B3D_EXPORT MaterialParameterAdapter : public TMaterialParameterAdapter<true>
		{
		public:
			MaterialParameterAdapter() = default;

			MaterialParameterAdapter(const TShared<Variation>& variation, const TShared<Shader>& shader, const TShared<MaterialParameters>& materialParameters)
				: TMaterialParameterAdapter(variation, shader, materialParameters)
			{}

			using TMaterialParameterAdapter::SetUniformBuffer;

			/**
			 * Assign a uniform buffer with the specified index to all the relevant child GpuParameterSet objects.
			 *
			 * @param index					Index of the buffer, as retrieved from GetUniformBufferIndex().
			 * @param bufferSuballocation	Uniform buffer to assign.
			 * @param ignoreInUpdate		If true the buffer will not be updated during the Update() call. This is useful
			 *								if the caller wishes to manually update the buffer contents externally, to prevent
			 *								overwriting manually written data during update.
			 */
			void SetUniformBuffer(u32 index, const GpuBufferSuballocation& bufferSuballocation, bool ignoreInUpdate = false);

			/**
			 * Assign a uniform buffer with the specified name to all the relevant child GpuParameterSet objects.
			 *
			 * @param name					Name of the buffer to set.
			 * @param bufferSuballocation	Uniform buffer to assign.
			 * @param ignoreInUpdate		If true the buffer will not be updated during the Update() call. This is useful
			 *								if the caller wishes to manually update the buffer contents externally, to prevent
			 *								overwriting manually written data during update.
			 */
			void SetUniformBuffer(const String& name, const GpuBufferSuballocation& bufferSuballocation, bool ignoreInUpdate = false);
		};
	} // namespace render

	/** @} */
} // namespace b3d
