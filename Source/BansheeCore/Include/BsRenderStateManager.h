//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsModule.h"
#include "BsBlendState.h"
#include "BsRasterizerState.h"
#include "BsDepthStencilState.h"
#include "BsSamplerState.h"
#include "BsGpuPipelineState.h"
#include "BsGpuPipelineParamInfo.h"

namespace bs
{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/** Handles creation of various render states. */
	class BS_CORE_EXPORT RenderStateManager : public Module <RenderStateManager>
	{
	public:
		/**	
		 * Creates and initializes a new SamplerState. 
		 * 
		 * @param[in]	desc	Object describing the sampler state to create.
		 */
		SPtr<SamplerState> createSamplerState(const SAMPLER_STATE_DESC& desc) const;

		/** Creates and initializes a new DepthStencilState. */
		SPtr<DepthStencilState> createDepthStencilState(const DEPTH_STENCIL_STATE_DESC& desc) const;

		/**	Creates and initializes a new RasterizerState. */
		SPtr<RasterizerState> createRasterizerState(const RASTERIZER_STATE_DESC& desc) const;

		/**	Creates and initializes a new BlendState. */
		SPtr<BlendState> createBlendState(const BLEND_STATE_DESC& desc) const;

		/**	
		 * Creates and initializes a new GraphicsPipelineState. 
		 *
		 * @param[in]	desc	Object describing the pipeline to create.
		 */
		SPtr<GraphicsPipelineState> createGraphicsPipelineState(const PIPELINE_STATE_DESC& desc) const;

		/**	
		 * Creates and initializes a new ComputePipelineState. 
		 *
		 * @param[in]	program		Compute GPU program to be executed by the pipeline.
		 */
		SPtr<ComputePipelineState> createComputePipelineState(const SPtr<GpuProgram>& program) const;

		/** Creates an uninitialized sampler state. Requires manual initialization after creation. */
		SPtr<SamplerState> _createSamplerStatePtr(const SAMPLER_STATE_DESC& desc) const;

		/** Creates an uninitialized depth-stencil state. Requires manual initialization after creation. */
		SPtr<DepthStencilState> _createDepthStencilStatePtr(const DEPTH_STENCIL_STATE_DESC& desc) const;

		/** Creates an uninitialized rasterizer state. Requires manual initialization after creation. */
		SPtr<RasterizerState> _createRasterizerStatePtr(const RASTERIZER_STATE_DESC& desc) const;

		/** Creates an uninitialized blend state. Requires manual initialization after creation. */
		SPtr<BlendState> _createBlendStatePtr(const BLEND_STATE_DESC& desc) const;

		/**	Creates an uninitialized GraphicsPipelineState. Requires manual initialization after creation. */
		virtual SPtr<GraphicsPipelineState> _createGraphicsPipelineState(const PIPELINE_STATE_DESC& desc) const;

		/**	Creates an uninitialized ComputePipelineState. Requires manual initialization after creation. */
		virtual SPtr<ComputePipelineState> _createComputePipelineState(const SPtr<GpuProgram>& program) const;

		/** Gets a sampler state initialized with default options. */
		const SPtr<SamplerState>& getDefaultSamplerState() const;

		/**	Gets a blend state initialized with default options. */
		const SPtr<BlendState>& getDefaultBlendState() const;

		/**	Gets a rasterizer state initialized with default options. */
		const SPtr<RasterizerState>& getDefaultRasterizerState() const;

		/**	Gets a depth stencil state initialized with default options. */
		const SPtr<DepthStencilState>& getDefaultDepthStencilState() const;

	private:
		friend class SamplerState;
		friend class BlendState;
		friend class RasterizerState;
		friend class DepthStencilState;

		mutable SPtr<SamplerState> mDefaultSamplerState;
		mutable SPtr<BlendState> mDefaultBlendState;
		mutable SPtr<RasterizerState> mDefaultRasterizerState;
		mutable SPtr<DepthStencilState> mDefaultDepthStencilState;
	};

	/**	Handles creation of various render states. */
	class BS_CORE_EXPORT RenderStateCoreManager : public Module<RenderStateCoreManager>
	{
	private:
		/**	Contains data about a cached blend state. */
		struct CachedBlendState
		{
			CachedBlendState()
				:id(0)
			{ }

			CachedBlendState(UINT32 id)
				:id(id)
			{ }

			std::weak_ptr<BlendStateCore> state;
			UINT32 id;
		};

		/**	Contains data about a cached rasterizer state. */
		struct CachedRasterizerState
		{
			CachedRasterizerState()
				:id(0)
			{ }

			CachedRasterizerState(UINT32 id)
				:id(id)
			{ }

			std::weak_ptr<RasterizerStateCore> state;
			UINT32 id;
		};

		/** Contains data about a cached depth stencil state. */
		struct CachedDepthStencilState
		{
			CachedDepthStencilState()
				:id(0)
			{ }

			CachedDepthStencilState(UINT32 id)
				:id(id)
			{ }

			std::weak_ptr<DepthStencilStateCore> state;
			UINT32 id;
		};

	public:
		RenderStateCoreManager();

		/** 
		 * @copydoc RenderStateManager::createSamplerState 
		 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
		 */
		SPtr<SamplerStateCore> createSamplerState(const SAMPLER_STATE_DESC& desc, 
			GpuDeviceFlags deviceMask = GDF_DEFAULT) const;

		/** @copydoc RenderStateManager::createDepthStencilState */
		SPtr<DepthStencilStateCore> createDepthStencilState(const DEPTH_STENCIL_STATE_DESC& desc) const;

		/** @copydoc RenderStateManager::createRasterizerState */
		SPtr<RasterizerStateCore> createRasterizerState(const RASTERIZER_STATE_DESC& desc) const;

		/** @copydoc RenderStateManager::createBlendState */
		SPtr<BlendStateCore> createBlendState(const BLEND_STATE_DESC& desc) const;

		/** 
		 * @copydoc RenderStateManager::createGraphicsPipelineState 
		 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
		 */
		SPtr<GraphicsPipelineStateCore> createGraphicsPipelineState(const PIPELINE_STATE_CORE_DESC& desc, 
			GpuDeviceFlags deviceMask = GDF_DEFAULT) const;

		/** 
		 * @copydoc RenderStateManager::createComputePipelineState 
		 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
		 */
		SPtr<ComputePipelineStateCore> createComputePipelineState(const SPtr<GpuProgramCore>& program,
			GpuDeviceFlags deviceMask = GDF_DEFAULT) const;

		/** @copydoc GpuPipelineParamInfoCore::create */
		SPtr<GpuPipelineParamInfoCore> createPipelineParamInfo(const GPU_PIPELINE_PARAMS_DESC& desc,
															   GpuDeviceFlags deviceMask = GDF_DEFAULT) const;

		/** Creates an uninitialized sampler state. Requires manual initialization after creation. */
		SPtr<SamplerStateCore> _createSamplerState(const SAMPLER_STATE_DESC& desc, 
			GpuDeviceFlags deviceMask = GDF_DEFAULT) const;

		/** Creates an uninitialized depth-stencil state. Requires manual initialization after creation. */
		SPtr<DepthStencilStateCore> _createDepthStencilState(const DEPTH_STENCIL_STATE_DESC& desc) const;

		/** Creates an uninitialized rasterizer state. Requires manual initialization after creation. */
		SPtr<RasterizerStateCore> _createRasterizerState(const RASTERIZER_STATE_DESC& desc) const;

		/** Creates an uninitialized blend state. Requires manual initialization after creation. */
		SPtr<BlendStateCore> _createBlendState(const BLEND_STATE_DESC& desc) const;

		/**	Creates an uninitialized GraphicsPipelineState. Requires manual initialization after creation. */
		virtual SPtr<GraphicsPipelineStateCore> _createGraphicsPipelineState(const PIPELINE_STATE_CORE_DESC& desc, 
			GpuDeviceFlags deviceMask = GDF_DEFAULT) const;

		/**	Creates an uninitialized ComputePipelineState. Requires manual initialization after creation. */
		virtual SPtr<ComputePipelineStateCore> _createComputePipelineState(const SPtr<GpuProgramCore>& program,
																		   GpuDeviceFlags deviceMask = GDF_DEFAULT) const;

		/**	Creates an uninitialized GpuPipelineParamInfoCore. Requires manual initialization after creation. */
		virtual SPtr<GpuPipelineParamInfoCore> _createPipelineParamInfo(const GPU_PIPELINE_PARAMS_DESC& desc,
																		GpuDeviceFlags deviceMask = GDF_DEFAULT) const;

		/** Gets a sampler state initialized with default options. */
		const SPtr<SamplerStateCore>& getDefaultSamplerState() const;

		/**	Gets a blend state initialized with default options. */
		const SPtr<BlendStateCore>& getDefaultBlendState() const;

		/**	Gets a rasterizer state initialized with default options. */
		const SPtr<RasterizerStateCore>& getDefaultRasterizerState() const;

		/**	Gets a depth stencil state initialized with default options. */
		const SPtr<DepthStencilStateCore>& getDefaultDepthStencilState() const;

	protected:
		friend class SamplerState;
		friend class BlendState;
		friend class RasterizerState;
		friend class DepthStencilState;
		friend class SamplerStateCore;
		friend class BlendStateCore;
		friend class RasterizerStateCore;
		friend class DepthStencilStateCore;

		/** @copydoc Module::onShutDown */
		void onShutDown() override;

		/** @copydoc createSamplerState */
		virtual SPtr<SamplerStateCore> createSamplerStateInternal(const SAMPLER_STATE_DESC& desc, GpuDeviceFlags deviceMask) const;

		/** @copydoc createBlendState */
		virtual SPtr<BlendStateCore> createBlendStateInternal(const BLEND_STATE_DESC& desc, UINT32 id) const;

		/** @copydoc createRasterizerState */
		virtual SPtr<RasterizerStateCore> createRasterizerStateInternal(const RASTERIZER_STATE_DESC& desc, UINT32 id) const;

		/** @copydoc createDepthStencilState */
		virtual SPtr<DepthStencilStateCore> createDepthStencilStateInternal(const DEPTH_STENCIL_STATE_DESC& desc, UINT32 id) const;

	private:
		/**	Triggered when a new sampler state is created. */
		void notifySamplerStateCreated(const SAMPLER_STATE_DESC& desc, const SPtr<SamplerStateCore>& state) const;

		/**	Triggered when a new sampler state is created. */
		void notifyBlendStateCreated(const BLEND_STATE_DESC& desc, const CachedBlendState& state) const;

		/**	Triggered when a new sampler state is created. */
		void notifyRasterizerStateCreated(const RASTERIZER_STATE_DESC& desc, const CachedRasterizerState& state) const;

		/**	Triggered when a new sampler state is created. */
		void notifyDepthStencilStateCreated(const DEPTH_STENCIL_STATE_DESC& desc, const CachedDepthStencilState& state) const;

		/**
		 * Triggered when the last reference to a specific sampler state is destroyed, which means we must clear our cached
		 * version as well.
		 */
		void notifySamplerStateDestroyed(const SAMPLER_STATE_DESC& desc) const;

		/**
		 * Attempts to find a cached sampler state corresponding to the provided descriptor. Returns null if one doesn't 
		 * exist.
		 */
		SPtr<SamplerStateCore> findCachedState(const SAMPLER_STATE_DESC& desc) const;

		/**
		 * Attempts to find a cached blend state corresponding to the provided descriptor. Returns null if one doesn't exist.
		 */
		SPtr<BlendStateCore> findCachedState(const BLEND_STATE_DESC& desc, UINT32& id) const;

		/**
		 * Attempts to find a cached rasterizer state corresponding to the provided descriptor. Returns null if one doesn't 
		 * exist.
		 */
		SPtr<RasterizerStateCore> findCachedState(const RASTERIZER_STATE_DESC& desc, UINT32& id) const;

		/**
		 * Attempts to find a cached depth-stencil state corresponding to the provided descriptor. Returns null if one 
		 * doesn't exist.
		 */
		SPtr<DepthStencilStateCore> findCachedState(const DEPTH_STENCIL_STATE_DESC& desc, UINT32& id) const;

		mutable SPtr<SamplerStateCore> mDefaultSamplerState;
		mutable SPtr<BlendStateCore> mDefaultBlendState;
		mutable SPtr<RasterizerStateCore> mDefaultRasterizerState;
		mutable SPtr<DepthStencilStateCore> mDefaultDepthStencilState;

		mutable UnorderedMap<SAMPLER_STATE_DESC, std::weak_ptr<SamplerStateCore>> mCachedSamplerStates;
		mutable UnorderedMap<BLEND_STATE_DESC, CachedBlendState> mCachedBlendStates;
		mutable UnorderedMap<RASTERIZER_STATE_DESC, CachedRasterizerState> mCachedRasterizerStates;
		mutable UnorderedMap<DEPTH_STENCIL_STATE_DESC, CachedDepthStencilState> mCachedDepthStencilStates;

		mutable UINT32 mNextBlendStateId;
		mutable UINT32 mNextRasterizerStateId;
		mutable UINT32 mNextDepthStencilStateId;

		mutable Mutex mMutex;
	};

	/** @} */
}