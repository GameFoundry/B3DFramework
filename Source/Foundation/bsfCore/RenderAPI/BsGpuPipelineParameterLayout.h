//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "CoreThread/BsCoreObject.h"
#include "Allocators/BsGroupAlloc.h"

namespace bs
{
	/** @addtogroup RenderAPI-Internal
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
		u32 Set = ~0u;
		u32 Slot = ~0u;
	};

	/** Contains code common to both sim and core thread implementations of GpuPipelineParameterLayout. */
	class B3D_CORE_EXPORT GpuPipelineParameterLayoutBase
	{
	public:
		/** Types of GPU parameters. */
		enum class GpuParameterType
		{
			UniformBuffer,
			SampledTexture,
			StorageTexture,
			StorageBuffer,
			Sampler,
			Count
		};

		/** Constructs the object using the provided GPU parameter descriptors. */
		GpuPipelineParameterLayoutBase(const GpuPipelineParameterLayoutCreateInformation& createInformation);
		virtual ~GpuPipelineParameterLayoutBase() = default;

		/** Gets the total number of sets. */
		u32 GetSetCount() const { return mSetCount; }

		/** Returns the total number of elements across all sets. */
		u32 GetResourceCount() const { return mResourceCount; }

		/** Returns the number of elements in all sets for the specified parameter type. */
		u32 GetResourceCount(GpuParameterType type) const { return mResourceCountPerType[(u32)type]; }

		/** Returns the total number of slots across all sets. */
		u32 GetBindingSlotCount() const { return mBindingSlotCount; }

		/** Returns the number of slots in all sets for the specified parameter type. */
		u32 GetBindingSlotCount(GpuParameterType type) { return mBindingSlotCountPerType[(u32)type]; }

		/**
		 * Converts a set/slot/array index combination into a sequential index that maps to the parameter in that parameter type's array.
		 *
		 * If the set, slot or array index is out of valid range, the method logs an error and returns ~0u. Only performs range checking in debug mode.
		 */
		u32 GetSequentialResourceIndex(GpuParameterType type, u32 set, u32 slot, u32 arrayIndex) const;

		/**
		 * Converts a set and slot combination into a sequential index that maps to the parameter in that parameter type's array. This is similar to
		 * GetSequentialResourceIndex(), but does not account for array indices.
		 *
		 * If the set or slot is out of valid range, the method logs an error and returns ~0u. Only performs range checking in debug mode.
		 */
		u32 GetSequentialBindingIndex(GpuParameterType type, u32 set, u32 slot) const;

		/** Converts a sequential binding index index into a set/slot combination. */
		void GetBinding(GpuParameterType type, u32 sequentialBindingIndex, u32& set, u32& slot) const;

		/**
		 * Finds set/slot indices of a parameter with the specified name for the specified GPU program stage. Set/slot
		 * indices are set to ~0u if a stage doesn't have a block with the specified name.
		 */
		void GetBinding(GpuProgramType progType, GpuParameterType type, const String& name, GpuParameterBinding& binding);

		/**
		 * Finds set/slot indices of a parameter with the specified name for every GPU program stage. Set/slot indices are
		 * set to -1 if a stage doesn't have a block with the specified name.
		 */
		void GetBindings(GpuParameterType type, const String& name, GpuParameterBinding (&bindings)[GPT_COUNT]);

		/** Returns the number of entries in the array at the specified binding index. */
		u32 GetArraySize(GpuParameterType type, u32 sequentialBindingIndex);

		/** Returns descriptions of individual parameters for the specified GPU program type. */
		const SPtr<GpuProgramParameterDescription>& GetParameterDescriptionForProgram(GpuProgramType type) const { return mPerProgramParameterDescriptions[(int)type]; }

	protected:
		/** Information about a single set in the param info object. */
		struct SetInfo
		{
			u32* SlotToSequentialBindingIndex = nullptr;
			u32* SlotToSequentialResourceIndex = nullptr;
			u32* SlotToSequentialSamplerBindingIndex = nullptr;
			u32* SlotToSequentialSamplerResourceIndex = nullptr;
			u32* SlotArraySizes = nullptr;
			GpuParameterType* SlotTypes = nullptr;
			u32 SlotCount = 0;
		};

		/** Information how a resource maps to a certain set/slot. */
		struct ResourceInfo
		{
			u32 Set = 0;
			u32 Slot = 0;
			u32 ArraySize = 1;
		};

		std::array<SPtr<GpuProgramParameterDescription>, 6> mPerProgramParameterDescriptions;

		u32 mSetCount = 0;
		SetInfo* mSetInfos = nullptr;

		u32 mBindingSlotCount = 0;
		u32 mBindingSlotCountPerType[(int)GpuParameterType::Count];
		u32 mResourceCount = 0;
		u32 mResourceCountPerType[(int)GpuParameterType::Count];
		ResourceInfo* mResourceInfos[(int)GpuParameterType::Count];

		GroupAlloc mAlloc;
	};

	/** Holds meta-data about a set of GPU parameters used by a single pipeline state. */
	class B3D_CORE_EXPORT GpuPipelineParameterLayout : public CoreObject, public GpuPipelineParameterLayoutBase
	{
	public:
		virtual ~GpuPipelineParameterLayout() = default;

		/**
		 * Retrieves a core implementation of this object usable only from the core thread.
		 *
		 * @note	Core thread only.
		 */
		SPtr<ct::GpuPipelineParameterLayout> GetCore() const;

		/**
		 * Constructs the object using the provided GPU parameter descriptors.
		 *
		 * @param[in]	desc	Object containing parameter descriptions for individual GPU program stages.
		 */
		static SPtr<GpuPipelineParameterLayout> Create(const GpuPipelineParameterLayoutInformation& desc);

	private:
		GpuPipelineParameterLayout(const GpuPipelineParameterLayoutInformation& desc);

		SPtr<ct::CoreObject> CreateCore() const override;
	};

	namespace ct
	{
		/** Contains information about all GPU program parameters required when binding a particular GPU pipeline for execution. */
		class B3D_CORE_EXPORT GpuPipelineParameterLayout : public CoreObject, public GpuPipelineParameterLayoutBase
		{
		public:
			virtual ~GpuPipelineParameterLayout() = default;

		protected:
			friend class RenderStateManager;

			GpuPipelineParameterLayout(const GpuPipelineParameterLayoutCreateInformation& createInformation);
		};
	} // namespace ct

	/** @} */
} // namespace bs
