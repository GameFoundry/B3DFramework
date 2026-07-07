//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DGpuProgramParameterDescription.h"

namespace b3d
{
	struct GpuResourceTableLayout;
	struct GpuDescriptorTable;

	/** @addtogroup GpuBackend
	 *  @{
	 */

	/** Stores parameter description for all GPU programs used in a particular GPU pipeline. */
	struct GpuPipelineParameterLayoutInformation
	{
		TShared<GpuProgramParameterDescription> Fragment;
		TShared<GpuProgramParameterDescription> Vertex;
		TShared<GpuProgramParameterDescription> Geometry;
		TShared<GpuProgramParameterDescription> Hull;
		TShared<GpuProgramParameterDescription> Domain;
		TShared<GpuProgramParameterDescription> Compute;

		/**
		 * Optional reflected resource-table layouts, one per program stage (indexed by GpuProgramType), taken from
		 * each program's compiled bytecode. Backends that pack descriptors at compiler-chosen offsets
		 * consume these when building per-set layouts, so the engine writes every descriptor exactly
		 * where the shader reads it; backends that derive layouts from the parameter descriptions alone ignore them.
		 * Entries may be null (stage absent, or no reflected layout cached on its bytecode).
		 */
		Array<TShared<GpuResourceTableLayout>, GPT_COUNT> ResourceTableLayouts;
	};

	/** Descriptor structure used for initialization of a GpuPipelineParameterLayout. */
	struct GpuPipelineParameterLayoutCreateInformation : GpuPipelineParameterLayoutInformation
	{
		GpuPipelineParameterLayoutCreateInformation() = default;
		GpuPipelineParameterLayoutCreateInformation(const GpuPipelineParameterLayoutInformation& other)
			:GpuPipelineParameterLayoutInformation(other)
		{ }
	};

	/** Binding location for a single GPU program parameter. */
	struct GpuParameterBinding
	{
		GpuParameterBinding(u32 set = ~0u, u32 slot = ~0u)
			: Set(set), Slot(slot)
		{ }

		u32 Set = ~0u;
		u32 Slot = ~0u;

		bool IsValid() const { return Set != ~0u && Slot != ~0u; }
	};

	/** Information how a resource maps to a certain uniform set/slot. */
	struct UniformInformation
	{
		String Name;
		GpuParameterType Type = GpuParameterType::Unknown;
		GpuParameterObjectType ObjectType = GPOT_UNKNOWN; /**< More specialized type compared to Type. */
		GpuBufferFormat ElementType = BF_UNKNOWN; /**< For textures and non-structured buffers, type stored in each element. */
		u32 Set = 0;
		u32 Slot = 0;
		u32 ArraySize = 1;
		u32 DynamicOffsetIndex = ~0u;
		GpuProgramStageBits Usage = GpuProgramStageBit::None;

		u32 SequentialBindingIndex = ~0u; /**< Mapping into the UniformsPerType array, for the current type. */
		u32 SequentialResourceIndex = ~0u; /**< Similar to SequentialBindingIndex, but accounts for array size of each entry of the same type prior to this entry. */
		u32 SequentialSamplerBindingIndex = ~0u; /**< Mapping into the UniformsPerType array for the sampler, if this uniform is a combined texture/sampler. */
		u32 SequentialSamplerResourceIndex = ~0u; /**< Similar to SequentialSamplerBindingIndex, but accounts for array size of each entry of the same type prior to this entry. */
	};

	/**
	 * Contains information about a single GPU program parameter set.
	 *
	 * @note	Thread safe (Immutable).
	 */
	class B3D_EXPORT GpuPipelineParameterSetLayout
	{
	public:
		virtual ~GpuPipelineParameterSetLayout() = default;

		/** Returns the total number of elements in the set. */
		u32 GetResourceCount() const { return mResourceCount; }

		/** Returns the number of elements in a particular set for the specified parameter type. */
		u32 GetResourceCount(GpuParameterType type) const { return mResourceCountPerType[(u32)type]; }

		/** Returns the total number of binding slots in the set. */
		u32 GetBindingCount() const { return mBindingCount; }

		/** Returns the number of binding slots in the set for the specified parameter type. */
		u32 GetBindingCount(GpuParameterType type) const { return (u32)mUniformsPerType[(u32)type].Size(); }

		/**
		 * Converts a slot/array index combination into a sequential index that maps to the parameter in that parameter type's array. The sequential
		 * index is relative to the set.
		 *
		 * If the slot or array index is out of valid range, the method logs an error and returns ~0u. Only performs range checking in debug mode.
		 */
		u32 GetSequentialResourceIndex(u32 slot, u32 arrayIndex) const;

		/**
		 * Converts a slot into a sequential index that maps to the parameter in that parameter type's array. This is similar to
		 * GetSequentialResourceIndex(), but does not account for array indices. The sequential index is relative to the set.
		 *
		 * If the slot is out of valid range, the method logs an error and returns ~0u. Only performs range checking in debug mode.
		 */
		u32 GetSequentialBindingIndex(u32 slot) const;

		/** Retrieves a slot from sequential binding index. */
		u32 GetSlot(GpuParameterType type, u32 sequentialBindingIndex) const;

		/** Finds slot index of a parameter with the specified name. Slot index is set to ~0u if parameter cannot be found. */
		u32 GetSlot(const StringView& name) const;

		/** Returns the number of entries in the array at the specified sequential binding index. */
		u32 GetArraySize(GpuParameterType type, u32 sequentialBindingIndex) const;

		/** Returns the number of dynamic offset slots in the set. */
		u32 GetDynamicOffsetCount() const { return mDynamicOffsetCount; }

		/**
		 * Returns an index that can be used for applying a dynamic offset for buffer lookup. The index can be provided
		 * to the command buffer after GpuParameterSet using this layout have been bound on the command buffer.
		 *
		 * Returns ~0u if parameter at the specific slot doesn't support dynamic offsets (supported on uniform and storage buffers),
		 * or if the parameter is not found.
		 */
		u32 GetDynamicOffsetIndex(u32 slot, u32 arrayIndex = 0) const;

		/**
		 * Returns an index that can be used for applying a dynamic offset for buffer lookup. The index can be provided
		 * to the command buffer after GpuParameterSet using this layout have been bound on the command buffer.
		 *
		 * Returns ~0u if parameter at the specific set/slot combination doesn't support dynamic offsets (supported on uniform and storage buffers),
		 * or if the parameter is not found.
		 */
		u32 GetDynamicOffsetIndex(const StringView& name, u32 arrayIndex = 0) const;

		/** Returns true if the layout has a uniform with the specified name. */
		bool HasUniform(const StringView& name) const { return mUniformMap.find(name) != mUniformMap.end(); }

		/** Returns true if the layout has a uniform with the specified name and type. */
		bool HasUniformOfType(const StringView& name, GpuParameterType type) const;

		/** Returns information about a uniform parameter by the specified name, or null if not found. */
		const UniformInformation* TryGetUniformInformation(const StringView& name) const;

		/** Returns information about a uniform parameter by the specified type and sequential index, or null if not found. */
		const UniformInformation* TryGetUniformInformation(GpuParameterType type, u32 sequentialBindingIndex) const;

		/** Returns information about a uniform parameter by the specified slot, or null if not found. */
		const UniformInformation* TryGetUniformInformation(u32 slot) const;

		/** Returns true if the layout has a uniform buffer member with the specified name in the set. */
		bool HasUniformBufferMember(const StringView& name) const { return mUniformBufferMembers.find(name) != mUniformBufferMembers.end(); }

		/** Returns information about a member of a uniform buffer by the specified name, or null if not found. */
		const GpuUniformBufferMemberInformation* TryGetUniformBufferMemberInformation(const StringView& name) const;

	protected:
		GpuPipelineParameterSetLayout(const GpuProgramParameterDescription& parameterDescription);

		Map<String, UniformInformation, std::less<>> mUniformMap; /**< A map of all uniforms. */ // TODO - Map instead of UnorderedMap to support heterogeneous lookup, until we port to C++20
		TInlineArray<UniformInformation*, 32> mUniforms; /**< Uniform for each slot index. */
		Array<TInlineArray<UniformInformation*, 16>, (u32)GpuParameterType::Count> mUniformsPerType;
		Array<u32, (u32)GpuParameterType::Count> mResourceCountPerType;
		Map<String, GpuUniformBufferMemberInformation, std::less<>> mUniformBufferMembers; /**< All data parameters in all uniform buffers. */ // TODO - Map instead of UnorderedMap to support heterogeneous lookup, until we port to C++20
		u32 mResourceCount = 0;
		u32 mBindingCount = 0;
		u32 mDynamicOffsetCount = 0; /**< Number of dynamic offset slots in this set. */
	};

	/**
	 * Contains information about all GPU program parameters required when binding a particular GPU pipeline for execution.
	 *
	 * @note	Thread safe (Immutable).
	 */
	class B3D_EXPORT GpuPipelineParameterLayout
	{
	public:
		virtual ~GpuPipelineParameterLayout() = default;

		/** Gets the total number of sets. */
		u32 GetSetCount() const { return (u32)mSets.Size(); }

		/** Returns pipeline layout for a particular set. */
		TShared<GpuPipelineParameterSetLayout> GetSet(u32 set) const { return mSets[set]; }

	protected:
		GpuPipelineParameterLayout(GpuDevice& device, const GpuPipelineParameterLayoutCreateInformation& createInformation);

		TInlineArray<TShared<GpuPipelineParameterSetLayout>, 2> mSets;
	};

	/** @} */
} // namespace b3d
