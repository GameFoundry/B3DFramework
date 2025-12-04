//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DGpuProgramParameterDescription.h"

namespace b3d
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/** Stores parameter description for all GPU programs used in a particular GPU pipeline. */
	struct GpuPipelineParameterLayoutInformation
	{
		SPtr<GpuProgramParameterDescription> Fragment;
		SPtr<GpuProgramParameterDescription> Vertex;
		SPtr<GpuProgramParameterDescription> Geometry;
		SPtr<GpuProgramParameterDescription> Hull;
		SPtr<GpuProgramParameterDescription> Domain;
		SPtr<GpuProgramParameterDescription> Compute;
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
	class B3D_EXPORT GpuPipelineParameterLayoutSet
	{
	public:
		virtual ~GpuPipelineParameterLayoutSet() = default;

		/** Initializes the object. The object should not be used before this is called. */
		virtual void Initialize() { }

		/** Returns the total number of elements in the set. */
		u32 GetResourceCount() const { return ResourceCount; }

		/** Returns the number of elements in a particular set for the specified parameter type. */
		u32 GetResourceCount(GpuParameterType type) const { return ResourceCountPerType[(u32)type]; }

		/** Returns the total number of binding slots in the set. */
		u32 GetBindingCount() const { return BindingCount; }

		/** Returns the number of binding slots in the set for the specified parameter type. */
		u32 GetBindingCount(GpuParameterType type) const { return (u32)UniformsPerType[(u32)type].Size(); }

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

		/** Returns the number of entries in the array at the specified sequential binding index. */
		u32 GetArraySize(GpuParameterType type, u32 sequentialBindingIndex) const;

		/** Returns the number of dynamic offset slots in the set. */
		u32 GetDynamicOffsetCount() const { return DynamicOffsetCount; }

		/**
		 * Returns an index that can be used for applying a dynamic offset for buffer lookup. The index can be provided
		 * to the command buffer after GpuParameterSet using this layout have been bound on the command buffer.
		 *
		 * Returns ~0u if parameter at the specific slot doesn't support dynamic offsets (supported on uniform and storage buffers),
		 * or if the parameter is not found.
		 */
		u32 GetDynamicOffsetIndex(u32 slot, u32 arrayIndex = 0) const;

		/** Returns information about a uniform parameter by the specified type and sequential index, or null if not found. */
		const UniformInformation* TryGetUniformInformation(GpuParameterType type, u32 sequentialBindingIndex) const;

		/** Returns true if the layout has a uniform buffer member with the specified name in the set. */
		bool HasUniformBufferMember(const StringView& name) const { return UniformBufferMembers.find(name) != UniformBufferMembers.end(); }

		/** Returns information about a member of a uniform buffer by the specified name, or null if not found. */
		const GpuUniformBufferMemberInformation* TryGetUniformBufferMemberInformation(const StringView& name) const;

	protected:
		GpuPipelineParameterLayoutSet(const GpuProgramParameterDescription& parameterDescription);

		Map<String, UniformInformation, std::less<>> mUniformMap; /**< A map of all uniforms. */ // TODO - Map instead of UnorderedMap to support heterogeneous lookup, until we port to C++20
		TInlineArray<UniformInformation*, 32> Uniforms; /**< Uniform for each slot index. */
		Array<TInlineArray<UniformInformation*, 16>, (u32)GpuParameterType::Count> UniformsPerType;
		Array<u32, (u32)GpuParameterType::Count> ResourceCountPerType;
		Map<String, GpuUniformBufferMemberInformation, std::less<>> UniformBufferMembers; /**< All data parameters in all uniform buffers. */ // TODO - Map instead of UnorderedMap to support heterogeneous lookup, until we port to C++20
		u32 ResourceCount = 0;
		u32 BindingCount = 0;
		u32 DynamicOffsetCount = 0; /**< Number of dynamic offset slots in this set. */
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

		/** Initializes the object. The object should not be used before this is called. */
		virtual void Initialize() { }

		/** Gets the total number of sets. */
		u32 GetSetCount() const { return (u32)mSets.Size(); }

		/** Returns the total number of elements in a particular set. */
		u32 GetResourceCount(u32 set) const { return mSets[set].ResourceCount; }

		/** Returns the number of elements in a particular set for the specified parameter type. */
		u32 GetResourceCount(u32 set, GpuParameterType type) const { return mSets[set].ResourceCountPerType[(u32)type]; }

		/** Returns the total number of binding slots in a particular set. */
		u32 GetBindingCount(u32 set) const { return mSets[set].BindingCount; }

		/** Returns the number of binding slots in a particular set for the specified parameter type. */
		u32 GetBindingCount(u32 set, GpuParameterType type) const { return (u32)mSets[set].UniformsPerType[(u32)type].Size(); }

		/**
		 * Converts a set/slot/array index combination into a sequential index that maps to the parameter in that parameter type's array. The sequential
		 * index is relative to the provided set.
		 *
		 * If the set, slot or array index is out of valid range, the method logs an error and returns ~0u. Only performs range checking in debug mode.
		 */
		u32 GetSequentialResourceIndex(u32 set, u32 slot, u32 arrayIndex) const;

		/**
		 * Converts a set and slot combination into a sequential index that maps to the parameter in that parameter type's array. This is similar to
		 * GetSequentialResourceIndex(), but does not account for array indices. The sequential index is relative to the provided set.
		 *
		 * If the set or slot is out of valid range, the method logs an error and returns ~0u. Only performs range checking in debug mode.
		 */
		u32 GetSequentialBindingIndex(u32 set, u32 slot) const;

		/** Retrieves a slot from a set and sequential binding index. */
		u32 GetSlot(GpuParameterType type, u32 set, u32 sequentialBindingIndex) const;

		/** String specific overload until we have heterogeneous lookup in UnorderedMap, to avoid re-constructing String. */
		void GetBinding(const StringView& name, GpuParameterBinding& binding) const { binding = GetBinding(name); }

		/** Finds set/slot indices of a parameter with the specified name. Set/slot indices are set to ~0u if parameter cannot be found. */
		GpuParameterBinding GetBinding(const StringView& name) const;

		/** Returns the number of entries in the array at the specified set and sequential binding index. */
		u32 GetArraySize(GpuParameterType type, u32 set, u32 sequentialBindingIndex) const;

		/** Returns the number of dynamic offset slots in the specified set. */
		u32 GetDynamicOffsetCount(u32 set) const;

		/**
		 * Returns an index that can be used for applying a dynamic offset for buffer lookup. The index can be provided
		 * to the command buffer after GpuParameterSet using this layout have been bound on the command buffer.
		 *
		 * Returns ~0u if parameter at the specific set/slot combination doesn't support dynamic offsets (supported on uniform and storage buffers),
		 * or if the parameter is not found.
		 */
		u32 GetDynamicOffsetIndex(u32 set, u32 slot, u32 arrayIndex = 0) const;

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

		/** Returns true if the layout has a uniform with the specified name and type, in the specified set. */
		bool HasUniformOfTypeAndSet(const StringView& name, GpuParameterType type, u32 set) const;

		/** Returns information about a uniform parameter by the specified name, or null if not found. */
		const UniformInformation* TryGetUniformInformation(const StringView& name) const;

		/** Returns information about a uniform parameter by the specified type, set and sequential index, or null if not found. */
		const UniformInformation* TryGetUniformInformation(GpuParameterType type, u32 set, u32 sequentialBindingIndex) const;

		/** Returns information about a uniform parameter by the specified binding, or null if not found. */
		const UniformInformation* TryGetUniformInformation(const GpuParameterBinding& binding) const;

		/** Returns true if the layout has a uniform buffer member with the specified name, in the provided set. */
		bool HasUniformBufferMember(u32 set, const StringView& name) const { return mSets[set].UniformBufferMembers.find(name) != mSets[set].UniformBufferMembers.end(); }

		/** Returns information about a member of a uniform buffer by the specified name, or null if not found. */
		const GpuUniformBufferMemberInformation* TryGetUniformBufferMemberInformation(u32 set, const StringView& name) const;

	protected:
		GpuPipelineParameterLayout(const GpuPipelineParameterLayoutCreateInformation& createInformation);

		/** Information about a single set in the param info object. */
		struct SetInformation
		{
			TInlineArray<UniformInformation*, 32> Uniforms; /**< Uniform for each slot index. */
			Array<TInlineArray<UniformInformation*, 16>, (u32)GpuParameterType::Count> UniformsPerType;
			Array<u32, (u32)GpuParameterType::Count> ResourceCountPerType;
			Map<String, GpuUniformBufferMemberInformation, std::less<>> UniformBufferMembers; /**< All data parameters in all uniform buffers. */ // TODO - Map instead of UnorderedMap to support heterogeneous lookup, until we port to C++20
			u32 ResourceCount = 0;
			u32 BindingCount = 0;
			u32 DynamicOffsetCount = 0; /**< Number of dynamic offset slots in this set. */
		};

		Map<String, UniformInformation, std::less<>> mUniformMap; /**< A map of all uniforms. */ // TODO - Map instead of UnorderedMap to support heterogeneous lookup, until we port to C++20
		TInlineArray<SetInformation, 2> mSets;
		u32 mBindingCount = 0;
	};

	/** @} */
} // namespace b3d
