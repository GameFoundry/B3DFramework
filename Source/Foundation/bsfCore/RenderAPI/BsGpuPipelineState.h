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
		SPtr<BlendState> BlendState;
		SPtr<RasterizerState> RasterizerState;
		SPtr<DepthStencilState> DepthStencilState;

		SPtr<GpuProgram> VertexProgram;
		SPtr<GpuProgram> FragmentProgram;
		SPtr<GpuProgram> GeometryProgram;
		SPtr<GpuProgram> HullProgram;
		SPtr<GpuProgram> DomainProgram;
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
			SPtr<BlendState> BlendState;
			SPtr<RasterizerState> RasterizerState;
			SPtr<DepthStencilState> DepthStencilState;

			SPtr<GpuProgram> VertexProgram;
			SPtr<GpuProgram> FragmentProgram;
			SPtr<GpuProgram> GeometryProgram;
			SPtr<GpuProgram> HullProgram;
			SPtr<GpuProgram> DomainProgram;
		};

		/** @} */
	} // namespace ct

	/** @addtogroup Implementation
	 *  @{
	 */

	/** Contains all data used by a GPU pipeline state, templated so it may contain both core and sim thread data. */
	template <bool Core>
	struct TGpuPipelineStateTypes
	{};

	template <>
	struct TGpuPipelineStateTypes<false>
	{
		typedef GpuPipelineParamInfo GpuPipelineParamInfoType;
		typedef PIPELINE_STATE_DESC StateDescType;
	};

	template <>
	struct TGpuPipelineStateTypes<true>
	{
		typedef ct::GpuPipelineParamInfo GpuPipelineParamInfoType;
		typedef ct::PIPELINE_STATE_DESC StateDescType;
	};

	/**
	 * Templated version of GraphicsPipelineState so it can be used for both core and non-core versions of the pipeline
	 * state.
	 */
	template <bool Core>
	class BS_CORE_EXPORT TGraphicsPipelineState
	{
	public:
		using BlendStateType = SPtr<CoreVariantType<BlendState, Core>>;
		using RasterizerStateType = SPtr<CoreVariantType<RasterizerState, Core>>;
		using DepthStencilStateType = SPtr<CoreVariantType<DepthStencilState, Core>>;
		using GpuProgramType = SPtr<CoreVariantType<GpuProgram, Core>>;
		using StateDescType = typename TGpuPipelineStateTypes<Core>::StateDescType;
		using GpuPipelineParamInfoType = typename TGpuPipelineStateTypes<Core>::GpuPipelineParamInfoType;

		virtual ~TGraphicsPipelineState() = default;

		bool HasVertexProgram() const { return mData.VertexProgram != nullptr; }

		bool HasFragmentProgram() const { return mData.FragmentProgram != nullptr; }

		bool HasGeometryProgram() const { return mData.GeometryProgram != nullptr; }

		bool HasHullProgram() const { return mData.HullProgram != nullptr; }

		bool HasDomainProgram() const { return mData.DomainProgram != nullptr; }

		BlendStateType GetBlendState() const { return mData.BlendState; }

		RasterizerStateType GetRasterizerState() const { return mData.RasterizerState; }

		DepthStencilStateType GetDepthStencilState() const { return mData.DepthStencilState; }

		const GpuProgramType& GetVertexProgram() const { return mData.VertexProgram; }

		const GpuProgramType& GetFragmentProgram() const { return mData.FragmentProgram; }

		const GpuProgramType& GetGeometryProgram() const { return mData.GeometryProgram; }

		const GpuProgramType& GetHullProgram() const { return mData.HullProgram; }

		const GpuProgramType& GetDomainProgram() const { return mData.DomainProgram; }

		/** Returns an object containing meta-data for parameters of all GPU programs used in this pipeline state. */
		const SPtr<GpuPipelineParamInfoType>& GetParamInfo() const { return mParamInfo; }

	protected:
		TGraphicsPipelineState() = default;
		TGraphicsPipelineState(const StateDescType& desc);

		StateDescType mData;
		SPtr<GpuPipelineParamInfoType> mParamInfo;
	};

	/**
	 * Templated version of ComputePipelineState so it can be used for both core and non-core versions of the pipeline
	 * state.
	 */
	template <bool Core>
	class BS_CORE_EXPORT TComputePipelineState
	{
	public:
		using GpuProgramType = SPtr<CoreVariantType<GpuProgram, Core>>;
		using GpuPipelineParamInfoType = typename TGpuPipelineStateTypes<Core>::GpuPipelineParamInfoType;

		virtual ~TComputePipelineState() = default;

		const GpuProgramType& GetProgram() const { return mProgram; }

		/** Returns an object containing meta-data for parameters of the GPU program used in this pipeline state. */
		const SPtr<GpuPipelineParamInfoType>& GetParamInfo() const { return mParamInfo; }

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
		virtual ~GraphicsPipelineState() = default;

		/**
		 * Retrieves a core implementation of the pipeline object usable only from the core thread.
		 *
		 * @note	Core thread only.
		 */
		SPtr<ct::GraphicsPipelineState> GetCore() const;

		/** @copydoc RenderStateManager::CreateGraphicsPipelineState */
		static SPtr<GraphicsPipelineState> Create(const PIPELINE_STATE_DESC& desc);

	protected:
		friend class RenderStateManager;

		GraphicsPipelineState(const PIPELINE_STATE_DESC& desc);

		SPtr<ct::CoreObject> CreateCore() const override;
	};

	/**
	 * Describes the state of the GPU pipeline that determines how are compute programs executed. It consists of
	 * of a single programmable state (GPU program). Once created the state is immutable, and can be bound to RenderAPI for
	 * use.
	 */
	class BS_CORE_EXPORT ComputePipelineState : public CoreObject, public TComputePipelineState<false>
	{
	public:
		virtual ~ComputePipelineState() = default;

		/**
		 * Retrieves a core implementation of the pipeline object usable only from the core thread.
		 *
		 * @note	Core thread only.
		 */
		SPtr<ct::ComputePipelineState> GetCore() const;

		/** @copydoc RenderStateManager::CreateComputePipelineState */
		static SPtr<ComputePipelineState> Create(const SPtr<GpuProgram>& program);

	protected:
		friend class RenderStateManager;

		ComputePipelineState(const SPtr<GpuProgram>& program);

		SPtr<ct::CoreObject> CreateCore() const override;
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
			virtual ~GraphicsPipelineState() = default;

			void Initialize() override;

			/** @copydoc RenderStateManager::CreateGraphicsPipelineState */
			static SPtr<GraphicsPipelineState> Create(const PIPELINE_STATE_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

		protected:
			GpuDeviceFlags mDeviceMask;
		};

		/** Core thread version of a bs::ComputePipelineState. */
		class BS_CORE_EXPORT ComputePipelineState : public CoreObject, public TComputePipelineState<true>
		{
		public:
			ComputePipelineState(const SPtr<GpuProgram>& program, GpuDeviceFlags deviceMask);
			virtual ~ComputePipelineState() = default;

			void Initialize() override;

			/** @copydoc RenderStateManager::CreateComputePipelineState */
			static SPtr<ComputePipelineState> Create(const SPtr<GpuProgram>& program, GpuDeviceFlags deviceMask = GDF_DEFAULT);

		protected:
			GpuDeviceFlags mDeviceMask;
		};

		/** @} */
	} // namespace ct
} // namespace bs
