//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsGpuProgramParameterDescription.h"

namespace bs
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
	 * Contains information about all GPU program parameters required when binding a particular GPU pipeline for execution.
	 *
	 * @note	Thread safe (Immutable).
	 */
	class B3D_CORE_EXPORT GpuPipelineParameterLayout
	{
	public:
		virtual ~GpuPipelineParameterLayout() = default;

		/** Initializes the object. The object should not be used before this is called. */
		virtual void Initialize() { }

		/** Gets the total number of sets. */
		u32 GetSetCount() const { return (u32)mSets.Size(); }

		/** Returns the total number of elements across all sets. */
		u32 GetResourceCount() const { return mResourceCount; }

		/** Returns the number of elements in all sets for the specified parameter type. */
		u32 GetResourceCount(GpuParameterType type) const { return mResourceCountPerType[(u32)type]; }

		/** Returns the total number of binding slots across all sets. */
		u32 GetBindingCount() const { return mBindingCount; }

		/** Returns the number of binding slots in all sets for the specified parameter type. */
		u32 GetBindingCount(GpuParameterType type) const { return (u32)mUniformsPerType[(u32)type].Size(); }

		/**
		 * Converts a set/slot/array index combination into a sequential index that maps to the parameter in that parameter type's array.
		 *
		 * If the set, slot or array index is out of valid range, the method logs an error and returns ~0u. Only performs range checking in debug mode.
		 */
		u32 GetSequentialResourceIndex(u32 set, u32 slot, u32 arrayIndex) const;

		/**
		 * Converts a set and slot combination into a sequential index that maps to the parameter in that parameter type's array. This is similar to
		 * GetSequentialResourceIndex(), but does not account for array indices.
		 *
		 * If the set or slot is out of valid range, the method logs an error and returns ~0u. Only performs range checking in debug mode.
		 */
		u32 GetSequentialBindingIndex(u32 set, u32 slot) const;

		/** Converts a sequential binding index index into a set/slot combination. */
		void GetBinding(GpuParameterType type, u32 sequentialBindingIndex, u32& set, u32& slot) const;

		/** Converts a sequential binding index index into a set/slot combination. */
		GpuParameterBinding GetBinding(GpuParameterType type, u32 sequentialBindingIndex) const;

		/** Finds set/slot indices of a parameter with the specified name. Set/slot indices are set to ~0u if parameter cannot be found. */
		void GetBinding(const String& name, GpuParameterBinding& binding) const;

		/** Finds set/slot indices of a parameter with the specified name. Set/slot indices are set to ~0u if parameter cannot be found. */
		GpuParameterBinding GetBinding(const String& name) const;

		/** Returns the number of entries in the array at the specified binding index. */
		u32 GetArraySize(GpuParameterType type, u32 sequentialBindingIndex) const;

		/**
		 * Returns an index that can be used for applying a dynamic offset for buffer lookup. The index can be provided
		 * to the command buffer after GpuParameters using this layout have been bound on the command buffer.
		 *
		 * Returns ~0u if parameter at the specific set/slot combination doesn't support dynamic offsets (supported on uniform and storage buffers),
		 * or if the parameter is not found.
		 */
		u32 GetDynamicOffsetIndex(u32 set, u32 slot, u32 arrayIndex = 0) const;

		/**
		 * Returns an index that can be used for applying a dynamic offset for buffer lookup. The index can be provided
		 * to the command buffer after GpuParameters using this layout have been bound on the command buffer.
		 *
		 * Returns ~0u if parameter at the specific set/slot combination doesn't support dynamic offsets (supported on uniform and storage buffers),
		 * or if the parameter is not found.
		 */
		u32 GetDynamicOffsetIndex(const String& name, u32 arrayIndex = 0) const;

		/** Returns true if the layout has a uniform with the specified name. */
		bool HasUniform(const String& name) const { return mUniformMap.find(name) != mUniformMap.end(); }

		/** Returns true if the layout has a uniform with the specified name and type. */
		bool HasUniformOfType(const String& name, GpuParameterType type) const;

		/** Returns information about a uniform parameter by the specified name, or null if not found. */
		const UniformInformation* TryGetUniformInformation(const String& name) const;

		/** Returns information about a uniform parameter by the specified type and sequential index, or null if not found. */
		const UniformInformation* TryGetUniformInformation(GpuParameterType type, u32 sequentialBindingIndex) const;

		/** Returns information about a uniform parameter by the specified binding, or null if not found. */
		const UniformInformation* TryGetUniformInformation(const GpuParameterBinding& binding) const;

		/** Returns true if the layout has a uniform buffer member with the specified name. */
		bool HasUniformBufferMember(const String& name) const { return mUniformBufferMembers.find(name) != mUniformBufferMembers.end(); }

		/** Returns information about a member of a uniform buffer by the specified name, or null if not found. */
		const GpuDataParameterInformation* TryGetUniformBufferMemberInformation(const String& name) const;

	protected:
		GpuPipelineParameterLayout(const GpuPipelineParameterLayoutCreateInformation& createInformation);

		/** Information about a single set in the param info object. */
		struct SetInformation
		{
			TInlineArray<UniformInformation*, 32> Uniforms; /**< Uniform for each slot index. */
		};

		UnorderedMap<String, UniformInformation> mUniformMap; /**< A map of all uniforms. */
		UnorderedMap<String, GpuDataParameterInformation> mUniformBufferMembers; /**< All data parameters in all uniform buffers. */
		Array<TInlineArray<UniformInformation*, 16>, (u32)GpuParameterType::Count> mUniformsPerType; /**< List of uniforms per type. */
		TInlineArray<SetInformation, 2> mSets;
		u32 mResourceCount = 0;
		u32 mBindingCount = 0;
		Array<u32, (u32)GpuParameterType::Count> mResourceCountPerType;
	};

	/** @} */
} // namespace bs
