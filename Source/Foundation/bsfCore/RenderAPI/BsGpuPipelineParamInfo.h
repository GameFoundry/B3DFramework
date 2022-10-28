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

	/** Helper structure used for initializing GpuPipelineParamInfo. */
	struct GPU_PIPELINE_PARAMS_DESC
	{
		SPtr<GpuParamDesc> FragmentParams;
		SPtr<GpuParamDesc> VertexParams;
		SPtr<GpuParamDesc> GeometryParams;
		SPtr<GpuParamDesc> HullParams;
		SPtr<GpuParamDesc> DomainParams;
		SPtr<GpuParamDesc> ComputeParams;
	};

	/** Binding location for a single GPU program parameter. */
	struct GpuParamBinding
	{
		u32 Set = (u32)-1;
		u32 Slot = (u32)-1;
	};

	/** Contains code common to both sim and core thread implementations of GpuPipelineParamInfo. */
	class BS_CORE_EXPORT GpuPipelineParamInfoBase
	{
	public:
		/** Types of GPU parameters. */
		enum class ParamType
		{
			ParamBlock,
			Texture,
			LoadStoreTexture,
			Buffer,
			SamplerState,
			Count
		};

		/** Constructs the object using the provided GPU parameter descriptors. */
		GpuPipelineParamInfoBase(const GPU_PIPELINE_PARAMS_DESC& desc);
		virtual ~GpuPipelineParamInfoBase() = default;

		/** Gets the total number of sets. */
		u32 GetNumSets() const { return mNumSets; }

		/** Returns the total number of elements across all sets. */
		u32 GetNumElements() const { return mNumElements; }

		/** Returns the number of elements in all sets for the specified parameter type. */
		u32 GetNumElements(ParamType type) { return mNumElementsPerType[(int)type]; }

		/**
		 * Converts a set/slot combination into a sequential index that maps to the parameter in that parameter type's
		 * array.
		 *
		 * If the set or slot is out of valid range, the method logs an error and returns -1. Only performs range checking
		 * in debug mode.
		 */
		u32 GetSequentialSlot(ParamType type, u32 set, u32 slot) const;

		/** Converts a sequential slot index into a set/slot combination. */
		void GetBinding(ParamType type, u32 sequentialSlot, u32& set, u32& slot) const;

		/**
		 * Finds set/slot indices of a parameter with the specified name for the specified GPU program stage. Set/slot
		 * indices are set to -1 if a stage doesn't have a block with the specified name.
		 */
		void GetBinding(GpuProgramType progType, ParamType type, const String& name, GpuParamBinding& binding);

		/**
		 * Finds set/slot indices of a parameter with the specified name for every GPU program stage. Set/slot indices are
		 * set to -1 if a stage doesn't have a block with the specified name.
		 */
		void GetBindings(ParamType type, const String& name, GpuParamBinding (&bindings)[GPT_COUNT]);

		/** Returns descriptions of individual parameters for the specified GPU program type. */
		const SPtr<GpuParamDesc>& GetParamDesc(GpuProgramType type) const { return mParamDescs[(int)type]; }

	protected:
		/** Information about a single set in the param info object. */
		struct SetInfo
		{
			u32* SlotIndices;
			ParamType* SlotTypes;
			u32* SlotSamplers;
			u32 NumSlots;
		};

		/** Information how a resource maps to a certain set/slot. */
		struct ResourceInfo
		{
			u32 Set;
			u32 Slot;
		};

		std::array<SPtr<GpuParamDesc>, 6> mParamDescs;

		u32 mNumSets;
		u32 mNumElements;
		SetInfo* mSetInfos;
		u32 mNumElementsPerType[(int)ParamType::Count];
		ResourceInfo* mResourceInfos[(int)ParamType::Count];

		GroupAlloc mAlloc;
	};

	/** Holds meta-data about a set of GPU parameters used by a single pipeline state. */
	class BS_CORE_EXPORT GpuPipelineParamInfo : public CoreObject, public GpuPipelineParamInfoBase
	{
	public:
		virtual ~GpuPipelineParamInfo() = default;

		/**
		 * Retrieves a core implementation of this object usable only from the core thread.
		 *
		 * @note	Core thread only.
		 */
		SPtr<ct::GpuPipelineParamInfo> GetCore() const;

		/**
		 * Constructs the object using the provided GPU parameter descriptors.
		 *
		 * @param[in]	desc	Object containing parameter descriptions for individual GPU program stages.
		 */
		static SPtr<GpuPipelineParamInfo> Create(const GPU_PIPELINE_PARAMS_DESC& desc);

	private:
		GpuPipelineParamInfo(const GPU_PIPELINE_PARAMS_DESC& desc);

		SPtr<ct::CoreObject> CreateCore() const override;
	};

	namespace ct
	{
		/** Core thread version of a bs::GpuPipelineParamInfo. */
		class BS_CORE_EXPORT GpuPipelineParamInfo : public CoreObject, public GpuPipelineParamInfoBase
		{
		public:
			virtual ~GpuPipelineParamInfo() = default;

			/**
			 * @copydoc bs::GpuPipelineParamInfo::Create
			 * @param[in]	deviceMask		Mask that determines on which GPU devices should the buffer be created on.
			 */
			static SPtr<GpuPipelineParamInfo> Create(const GPU_PIPELINE_PARAMS_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

		protected:
			friend class RenderStateManager;

			GpuPipelineParamInfo(const GPU_PIPELINE_PARAMS_DESC& desc, GpuDeviceFlags deviceMask);
		};
	} // namespace ct

	/** @} */
} // namespace bs
