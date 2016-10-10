//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsCoreObject.h"

namespace BansheeEngine 
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

	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/** Descriptor structure used for initializing a GPU pipeline state. */
	struct PIPELINE_STATE_CORE_DESC
	{
		SPtr<BlendStateCore> blendState;
		SPtr<RasterizerStateCore> rasterizerState;
		SPtr<DepthStencilStateCore> depthStencilState;

		SPtr<GpuProgramCore> vertexProgram;
		SPtr<GpuProgramCore> fragmentProgram;
		SPtr<GpuProgramCore> geometryProgram;
		SPtr<GpuProgramCore> hullProgram;
		SPtr<GpuProgramCore> domainProgram;
	};

	/** @} */

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
		typedef SPtr<BlendState> BlendStateType;
		typedef SPtr<RasterizerState> RasterizerStateType;
		typedef SPtr<DepthStencilState> DepthStencilStateType;
		typedef SPtr<GpuProgram> GpuProgramType;
		typedef PIPELINE_STATE_DESC StateDescType;
	};

	template<>
	struct TGpuPipelineStateTypes < true >
	{
		typedef SPtr<BlendStateCore> BlendStateType;
		typedef SPtr<RasterizerStateCore> RasterizerStateType;
		typedef SPtr<DepthStencilStateCore> DepthStencilStateType;
		typedef SPtr<GpuProgramCore> GpuProgramType;
		typedef PIPELINE_STATE_CORE_DESC StateDescType;
	};

	/** 
	 * Templated version of GpuPipelineState so it can be used for both core and non-core versions of the pipeline state. 
	 */
	template<bool Core>
	class BS_CORE_EXPORT TGpuPipelineState
    {
    public:
		typedef typename TGpuPipelineStateTypes<Core>::BlendStateType BlendStateType;
		typedef typename TGpuPipelineStateTypes<Core>::RasterizerStateType RasterizerStateType;
		typedef typename TGpuPipelineStateTypes<Core>::DepthStencilStateType DepthStencilStateType;
		typedef typename TGpuPipelineStateTypes<Core>::GpuProgramType GpuProgramType;
		typedef typename TGpuPipelineStateTypes<Core>::StateDescType StateDescType;

		virtual ~TGpuPipelineState() { }

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

	protected:
		TGpuPipelineState();
		TGpuPipelineState(const StateDescType& desc);

		StateDescType mData;
    };

	/** @} */

	/** @addtogroup RenderAPI
	 *  @{
	 */

	class GpuPipelineStateCore;

	/**
	 * Describes the state of the GPU pipeline that determines how are primitives rendered. It consists of programmable
	 * states (vertex, fragment, geometry, etc. GPU programs), as well as a set of fixed states (blend, rasterizer, 
	 * depth-stencil). Once created the state is immutable, and can be bound to RenderAPI for rendering.
	 */
    class BS_CORE_EXPORT GpuPipelineState : public CoreObject, public TGpuPipelineState<false>
    {
	public:
		virtual ~GpuPipelineState() { }

		/**
		 * Retrieves a core implementation of the pipeline object usable only from the core thread.
		 *
		 * @note	Core thread only.
		 */
		SPtr<GpuPipelineStateCore> getCore() const;

		/** @copydoc RenderStateManager::createPipelineState */
		static SPtr<GpuPipelineState> create(const PIPELINE_STATE_DESC& desc);
	protected:
		friend class RenderStateManager;

		GpuPipelineState(const PIPELINE_STATE_DESC& desc);

		/** @copydoc CoreObject::createCore */
		virtual SPtr<CoreObjectCore> createCore() const;
    };

	/** @} */

	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/** Core thread version of a GpuPipelineState. */
	class BS_CORE_EXPORT GpuPipelineStateCore : public CoreObjectCore, public TGpuPipelineState<true>
	{
	public:
		GpuPipelineStateCore(const PIPELINE_STATE_CORE_DESC& desc);
		virtual ~GpuPipelineStateCore() { }

		/** @copydoc RenderStateManager::createPipelineState */
		static SPtr<GpuPipelineStateCore> create(const PIPELINE_STATE_CORE_DESC& desc);
	};

	/** @} */
}