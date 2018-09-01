//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "CoreThread/BsCoreObject.h"

namespace bs 
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/** Descriptor structure used for initializing a GPU pipeline state. */
	struct PIPELINE_STATE_DESC
	{
		SPtr<BlendState> blendState;
		SPtr<RasterizerState> rasterizerState;
		SPtr<DepthStencilState> depthStencilState;

		SPtr<GpuProgram> vertexProgram;
		SPtr<GpuProgram> fragmentProgram;
		SPtr<GpuProgram> geometryProgram;
		SPtr<GpuProgram> hullProgram;
		SPtr<GpuProgram> domainProgram;
	};

	/** @} */

	namespace ct
	{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/** Descriptor structure used for initializing a GPU pipeline state. */
	struct PIPELINE_STATE_DESC
	{
		SPtr<BlendState> blendState;
		SPtr<RasterizerState> rasterizerState;
		SPtr<DepthStencilState> depthStencilState;

		SPtr<GpuProgram> vertexProgram;
		SPtr<GpuProgram> fragmentProgram;
		SPtr<GpuProgram> geometryProgram;
		SPtr<GpuProgram> hullProgram;
		SPtr<GpuProgram> domainProgram;
	};

	/** @} */
	}

	/** @addtogroup Implementation
	 *  @{
	 */

	/** Contains all data used by a GPU pipeline state, templated so it may contain both core and sim thread data. */
	template<bool Core>
	struct TGpuPipelineStateTypes
	{ };

	template<>
	struct TGpuPipelineStateTypes < false >
	{
		typedef GpuPipelineParamInfo GpuPipelineParamInfoType;
		typedef PIPELINE_STATE_DESC StateDescType;
	};

	template<>
	struct TGpuPipelineStateTypes < true >
	{
		typedef ct::GpuPipelineParamInfo GpuPipelineParamInfoType;
		typedef ct::PIPELINE_STATE_DESC StateDescType;
	};

	/** 
	 * Templated version of GraphicsPipelineState so it can be used for both core and non-core versions of the pipeline
	 * state. 
	 */
	template<bool Core>
	class BS_CORE_EXPORT TGraphicsPipelineState
	{
	public:
		using BlendStateType = SPtr<CoreVariantType<BlendState, Core>>;
		using RasterizerStateType = SPtr<CoreVariantType<RasterizerState, Core>>;
		using DepthStencilStateType = SPtr<CoreVariantType<DepthStencilState, Core>>;
		using GpuProgramType = SPtr<CoreVariantType<GpuProgram, Core>>;
		using StateDescType = typename TGpuPipelineStateTypes<Core>::StateDescType;
		using GpuPipelineParamInfoType = typename TGpuPipelineStateTypes<Core>::GpuPipelineParamInfoType;

		virtual ~TGraphicsPipelineState() { }

		bool hasVertexProgram() const { return mData.vertexProgram != nullptr; }
		bool hasFragmentProgram() const { return mData.fragmentProgram != nullptr; }
		bool hasGeometryProgram() const { return mData.geometryProgram != nullptr; }
		bool hasHullProgram() const { return mData.hullProgram != nullptr; }
		bool hasDomainProgram() const { return mData.domainProgram != nullptr; }

		BlendStateType getBlendState() const { return mData.blendState; }
		RasterizerStateType getRasterizerState() const { return mData.rasterizerState; }
		DepthStencilStateType getDepthStencilState() const { return mData.depthStencilState; }

		const GpuProgramType& getVertexProgram() const { return mData.vertexProgram; }
		const GpuProgramType& getFragmentProgram() const { return mData.fragmentProgram; }
		const GpuProgramType& getGeometryProgram() const { return mData.geometryProgram; }
		const GpuProgramType& getHullProgram() const { return mData.hullProgram; }
		const GpuProgramType& getDomainProgram() const { return mData.domainProgram; }

		/** Returns an object containing meta-data for parameters of all GPU programs used in this pipeline state. */
		const SPtr<GpuPipelineParamInfoType>& getParamInfo() const { return mParamInfo; }

	protected:
		TGraphicsPipelineState();
		TGraphicsPipelineState(const StateDescType& desc);

		StateDescType mData;
		SPtr<GpuPipelineParamInfoType> mParamInfo;
	};

	/** 
	 * Templated version of ComputePipelineState so it can be used for both core and non-core versions of the pipeline
	 * state. 
	 */
	template<bool Core>
	class BS_CORE_EXPORT TComputePipelineState
	{
	public:
		using GpuProgramType = SPtr<CoreVariantType<GpuProgram, Core>>;
		using GpuPipelineParamInfoType = typename TGpuPipelineStateTypes<Core>::GpuPipelineParamInfoType;

		virtual ~TComputePipelineState() { }

		const GpuProgramType& getProgram() const { return mProgram; }

		/** Returns an object containing meta-data for parameters of the GPU program used in this pipeline state. */
		const SPtr<GpuPipelineParamInfoType>& getParamInfo() const { return mParamInfo; }

	protected:
		TComputePipelineState();
		TComputePipelineState(const GpuProgramType& program);

		GpuProgramType mProgram;
		SPtr<GpuPipelineParamInfoType> mParamInfo;
	};

	/** @} */

	/** @addtogroup RenderAPI
	 *  @{
	 */

	/**
	 * Describes the state of the GPU pipeline that determines how are primitives rendered. It consists of programmable
	 * states (vertex, fragment, geometry, etc. GPU programs), as well as a set of fixed states (blend, rasterizer, 
	 * depth-stencil). Once created the state is immutable, and can be bound to RenderAPI for rendering.
	 */
	class BS_CORE_EXPORT GraphicsPipelineState : public CoreObject, public TGraphicsPipelineState<false>
	{
	public:
		virtual ~GraphicsPipelineState() { }

		/**
		 * Retrieves a core implementation of the pipeline object usable only from the core thread.
		 *
		 * @note	Core thread only.
		 */
		SPtr<ct::GraphicsPipelineState> getCore() const;

		/** @copydoc RenderStateManager::createGraphicsPipelineState */
		static SPtr<GraphicsPipelineState> create(const PIPELINE_STATE_DESC& desc);
	protected:
		friend class RenderStateManager;

		GraphicsPipelineState(const PIPELINE_STATE_DESC& desc);

		/** @copydoc CoreObject::createCore */
		virtual SPtr<ct::CoreObject> createCore() const;
	};

	/**
	 * Describes the state of the GPU pipeline that determines how are compute programs executed. It consists of 
	 * of a single programmable state (GPU program). Once created the state is immutable, and can be bound to RenderAPI for
	 * use.
	 */
	class BS_CORE_EXPORT ComputePipelineState : public CoreObject, public TComputePipelineState<false>
	{
	public:
		virtual ~ComputePipelineState() { }

		/**
		 * Retrieves a core implementation of the pipeline object usable only from the core thread.
		 *
		 * @note	Core thread only.
		 */
		SPtr<ct::ComputePipelineState> getCore() const;

		/** @copydoc RenderStateManager::createComputePipelineState */
		static SPtr<ComputePipelineState> create(const SPtr<GpuProgram>& program);
	protected:
		friend class RenderStateManager;

		ComputePipelineState(const SPtr<GpuProgram>& program);

		/** @copydoc CoreObject::createCore */
		virtual SPtr<ct::CoreObject> createCore() const;
	};

	/** @} */

	namespace ct
	{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/** Core thread version of a bs::GraphicsPipelineState. */
	class BS_CORE_EXPORT GraphicsPipelineState : public CoreObject, public TGraphicsPipelineState<true>
	{
	public:
		GraphicsPipelineState(const PIPELINE_STATE_DESC& desc, GpuDeviceFlags deviceMask);
		virtual ~GraphicsPipelineState() { }

		/** @copydoc CoreObject::initialize() */
		void initialize() override;

		/** @copydoc RenderStateManager::createGraphicsPipelineState */
		static SPtr<GraphicsPipelineState> create(const PIPELINE_STATE_DESC& desc, 
			GpuDeviceFlags deviceMask = GDF_DEFAULT);

	protected:
		GpuDeviceFlags mDeviceMask;
	};

	/** Core thread version of a bs::ComputePipelineState. */
	class BS_CORE_EXPORT ComputePipelineState : public CoreObject, public TComputePipelineState<true>
	{
	public:
		ComputePipelineState(const SPtr<GpuProgram>& program, GpuDeviceFlags deviceMask);
		virtual ~ComputePipelineState() { }

		/** @copydoc CoreObject::initialize() */
		void initialize() override;

		/** @copydoc RenderStateManager::createComputePipelineState */
		static SPtr<ComputePipelineState> create(const SPtr<GpuProgram>& program,
			GpuDeviceFlags deviceMask = GDF_DEFAULT);

	protected:
		GpuDeviceFlags mDeviceMask;
	};

	/** @} */
	}
}