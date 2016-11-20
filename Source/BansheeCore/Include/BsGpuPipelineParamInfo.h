//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsCoreObject.h"

namespace bs
{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/** Helper structure used for initializing GpuPipelineParamInfo. */
	struct GPU_PIPELINE_PARAMS_DESC
	{
		SPtr<GpuParamDesc> fragmentParams;
		SPtr<GpuParamDesc> vertexParams;
		SPtr<GpuParamDesc> geometryParams;
		SPtr<GpuParamDesc> hullParams;
		SPtr<GpuParamDesc> domainParams;
		SPtr<GpuParamDesc> computeParams;
	};

	/** Contains code common to both sim and core thread implementations of GpuPipelineParamInfo. */
	class BS_CORE_EXPORT GpuPipelineParamInfoBase
	{
	public:
		/** Types of GPU parameters. */
		enum class ParamType
		{
			ParamBlock, Texture, LoadStoreTexture, Buffer, SamplerState, Count
		};

		/** Constructs the object using the provided GPU parameter descriptors. */
		GpuPipelineParamInfoBase(const GPU_PIPELINE_PARAMS_DESC& desc);
		virtual ~GpuPipelineParamInfoBase();

		/** Gets the total number of sets, across all parameter types. */
		UINT32 getNumSets() const { return mTotalNumSets; }

		/** Returns the number of sets for the specified parameter type. */
		UINT32 getNumSets(ParamType type) { return mNumSets[(int)type]; }

		/** Returns the total number of elements across all sets. */
		UINT32 getNumElements() const { return mTotalNumElements; }

		/** Returns the number of elements in all sets for the specified parameter type. */
		UINT32 getNumElements(ParamType type) { return mNumElements[(int)type]; }

		/**
		* Assuming all elements for a specific parameter type are laid out sequentially and grouped by their sets,
		* returns the sequential index to the first parameter of the provided set.
		*/
		UINT32 getSetOffset(ParamType type, UINT32 set) { return mOffsets[(int)type][set]; }

		/**
		* Converts a set/slot combination into a sequential index that maps to the parameter in that parameter type's
		* array.
		*
		* If the set or slot is out of valid range, the method logs an error and returns -1. Only performs range checking
		* in debug mode.
		*/
		UINT32 getSequentialSlot(ParamType type, UINT32 set, UINT32 slot) const;

		/** Returns descriptions of individual parameters for the specified GPU program type. */
		const SPtr<GpuParamDesc>& getParamDesc(GpuProgramType type) const { return mParamDescs[(int)type]; }

	protected:
		std::array<SPtr<GpuParamDesc>, 6> mParamDescs;

		UINT32 mTotalNumSets;
		UINT32 mTotalNumElements;
		UINT32 mNumSets[(int)ParamType::Count];
		UINT32 mNumElements[(int)ParamType::Count];
		UINT32* mOffsets[(int)ParamType::Count];

		UINT8* mData;
	};

	/** Core thread version of a GpuPipelineParamInfo. */
	class BS_CORE_EXPORT GpuPipelineParamInfoCore : public CoreObjectCore, public GpuPipelineParamInfoBase
	{
	public:
		virtual ~GpuPipelineParamInfoCore() { }

		/** 
		 * @copydoc GpuPipelineParamInfo::create 
		 * @param[in]	deviceMask		Mask that determines on which GPU devices should the buffer be created on.
		 */
		static SPtr<GpuPipelineParamInfoCore> create(const GPU_PIPELINE_PARAMS_DESC& desc,
													 GpuDeviceFlags deviceMask = GDF_DEFAULT);

	protected:
		friend class RenderStateCoreManager;

		GpuPipelineParamInfoCore(const GPU_PIPELINE_PARAMS_DESC& desc, GpuDeviceFlags deviceMask);
	};

	/** Holds meta-data about a set of GPU parameters used by a single pipeline state. */
	class BS_CORE_EXPORT GpuPipelineParamInfo : public CoreObject, public GpuPipelineParamInfoBase
	{
	public:
		virtual ~GpuPipelineParamInfo() { }

		/**
		 * Retrieves a core implementation of this object usable only from the core thread.
		 *
		 * @note	Core thread only.
		 */
		SPtr<GpuPipelineParamInfoCore> getCore() const;

		/** 
		 * Constructs the object using the provided GPU parameter descriptors. 
		 * 
		 * @param[in]	desc	Object containing parameter descriptions for individual GPU program stages.
		 */
		static SPtr<GpuPipelineParamInfo> create(const GPU_PIPELINE_PARAMS_DESC& desc);

	private:
		GpuPipelineParamInfo(const GPU_PIPELINE_PARAMS_DESC& desc);

		/** @copydoc CoreObject::createCore */
		SPtr<CoreObjectCore> createCore() const override;
	};

	/** @} */
}