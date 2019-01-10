//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsModule.h"
#include "RenderAPI/BsBlendState.h"
#include "RenderAPI/BsRasterizerState.h"
#include "RenderAPI/BsDepthStencilState.h"
#include "RenderAPI/BsSamplerState.h"
#include "RenderAPI/BsGpuPipelineState.h"
#include "RenderAPI/BsGpuPipelineParamInfo.h"

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

	namespace ct
	{
	/**	Handles creation of various render states. */
	class BS_CORE_EXPORT RenderStateManager : public Module<RenderStateManager>
	{
	private:
		/**	Contains data about a cached blend state. */
		struct CachedBlendState
		{
			CachedBlendState() = default;

			CachedBlendState(UINT32 id)
				:id(id)
			{ }

			std::weak_ptr<BlendState> state;
			UINT32 id = 0;
		};

		/**	Contains data about a cached rasterizer state. */
		struct CachedRasterizerState
		{
			CachedRasterizerState() = default;

			CachedRasterizerState(UINT32 id)
				:id(id)
			{ }

			std::weak_ptr<RasterizerState> state;
			UINT32 id = 0;
		};

		/** Contains data about a cached depth stencil state. */
		struct CachedDepthStencilState
		{
			CachedDepthStencilState() = default;

			CachedDepthStencilState(UINT32 id)
				:id(id)
			{ }

			std::weak_ptr<DepthStencilState> state;
			UINT32 id = 0;
		};

	public:
		RenderStateManager() = default;

		/** 
		 * @copydoc bs::RenderStateManager::createSamplerState 
		 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
		 */
		SPtr<SamplerState> createSamplerState(const SAMPLER_STATE_DESC& desc, 
			GpuDeviceFlags deviceMask = GDF_DEFAULT) const;

		/** @copydoc bs::RenderStateManager::createDepthStencilState */
		SPtr<DepthStencilState> createDepthStencilState(const DEPTH_STENCIL_STATE_DESC& desc) const;

		/** @copydoc bs::RenderStateManager::createRasterizerState */
		SPtr<RasterizerState> createRasterizerState(const RASTERIZER_STATE_DESC& desc) const;

		/** @copydoc bs::RenderStateManager::createBlendState */
		SPtr<BlendState> createBlendState(const BLEND_STATE_DESC& desc) const;

		/** 
		 * @copydoc bs::RenderStateManager::createGraphicsPipelineState 
		 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
		 */
		SPtr<GraphicsPipelineState> createGraphicsPipelineState(const PIPELINE_STATE_DESC& desc, 
			GpuDeviceFlags deviceMask = GDF_DEFAULT) const;

		/** 
		 * @copydoc bs::RenderStateManager::createComputePipelineState 
		 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
		 */
		SPtr<ComputePipelineState> createComputePipelineState(const SPtr<GpuProgram>& program,
			GpuDeviceFlags deviceMask = GDF_DEFAULT) const;

		/** @copydoc GpuPipelineParamInfo::create */
		SPtr<GpuPipelineParamInfo> createPipelineParamInfo(const GPU_PIPELINE_PARAMS_DESC& desc,
															   GpuDeviceFlags deviceMask = GDF_DEFAULT) const;

		/** Creates an uninitialized sampler state. Requires manual initialization after creation. */
		SPtr<SamplerState> _createSamplerState(const SAMPLER_STATE_DESC& desc, 
			GpuDeviceFlags deviceMask = GDF_DEFAULT) const;

		/** Creates an uninitialized depth-stencil state. Requires manual initialization after creation. */
		SPtr<DepthStencilState> _createDepthStencilState(const DEPTH_STENCIL_STATE_DESC& desc) const;

		/** Creates an uninitialized rasterizer state. Requires manual initialization after creation. */
		SPtr<RasterizerState> _createRasterizerState(const RASTERIZER_STATE_DESC& desc) const;

		/** Creates an uninitialized blend state. Requires manual initialization after creation. */
		SPtr<BlendState> _createBlendState(const BLEND_STATE_DESC& desc) const;

		/**	Creates an uninitialized GraphicsPipelineState. Requires manual initialization after creation. */
		virtual SPtr<GraphicsPipelineState> _createGraphicsPipelineState(const PIPELINE_STATE_DESC& desc, 
			GpuDeviceFlags deviceMask = GDF_DEFAULT) const;

		/**	Creates an uninitialized ComputePipelineState. Requires manual initialization after creation. */
		virtual SPtr<ComputePipelineState> _createComputePipelineState(const SPtr<GpuProgram>& program,
																		   GpuDeviceFlags deviceMask = GDF_DEFAULT) const;

		/**	Creates an uninitialized GpuPipelineParamInfo. Requires manual initialization after creation. */
		virtual SPtr<GpuPipelineParamInfo> _createPipelineParamInfo(const GPU_PIPELINE_PARAMS_DESC& desc,
																		GpuDeviceFlags deviceMask = GDF_DEFAULT) const;

		/** Gets a sampler state initialized with default options. */
		const SPtr<SamplerState>& getDefaultSamplerState() const;

		/**	Gets a blend state initialized with default options. */
		const SPtr<BlendState>& getDefaultBlendState() const;

		/**	Gets a rasterizer state initialized with default options. */
		const SPtr<RasterizerState>& getDefaultRasterizerState() const;

		/**	Gets a depth stencil state initialized with default options. */
		const SPtr<DepthStencilState>& getDefaultDepthStencilState() const;

	protected:
		friend class bs::SamplerState;
		friend class bs::BlendState;
		friend class bs::RasterizerState;
		friend class bs::DepthStencilState;
		friend class SamplerState;
		friend class BlendState;
		friend class RasterizerState;
		friend class DepthStencilState;

		/** @copydoc Module::onShutDown */
		void onShutDown() override;

		/** @copydoc createSamplerState */
		virtual SPtr<SamplerState> createSamplerStateInternal(const SAMPLER_STATE_DESC& desc, GpuDeviceFlags deviceMask) const;

		/** @copydoc createBlendState */
		virtual SPtr<BlendState> createBlendStateInternal(const BLEND_STATE_DESC& desc, UINT32 id) const;

		/** @copydoc createRasterizerState */
		virtual SPtr<RasterizerState> createRasterizerStateInternal(const RASTERIZER_STATE_DESC& desc, UINT32 id) const;

		/** @copydoc createDepthStencilState */
		virtual SPtr<DepthStencilState> createDepthStencilStateInternal(const DEPTH_STENCIL_STATE_DESC& desc, UINT32 id) const;

	private:
		/**	Triggered when a new sampler state is created. */
		void notifySamplerStateCreated(const SAMPLER_STATE_DESC& desc, const SPtr<SamplerState>& state) const;

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
		SPtr<SamplerState> findCachedState(const SAMPLER_STATE_DESC& desc) const;

		/**
		 * Attempts to find a cached blend state corresponding to the provided descriptor. Returns null if one doesn't exist.
		 */
		SPtr<BlendState> findCachedState(const BLEND_STATE_DESC& desc, UINT32& id) const;

		/**
		 * Attempts to find a cached rasterizer state corresponding to the provided descriptor. Returns null if one doesn't 
		 * exist.
		 */
		SPtr<RasterizerState> findCachedState(const RASTERIZER_STATE_DESC& desc, UINT32& id) const;

		/**
		 * Attempts to find a cached depth-stencil state corresponding to the provided descriptor. Returns null if one 
		 * doesn't exist.
		 */
		SPtr<DepthStencilState> findCachedState(const DEPTH_STENCIL_STATE_DESC& desc, UINT32& id) const;

		mutable SPtr<SamplerState> mDefaultSamplerState;
		mutable SPtr<BlendState> mDefaultBlendState;
		mutable SPtr<RasterizerState> mDefaultRasterizerState;
		mutable SPtr<DepthStencilState> mDefaultDepthStencilState;

		mutable UnorderedMap<SAMPLER_STATE_DESC, std::weak_ptr<SamplerState>> mCachedSamplerStates;
		mutable UnorderedMap<BLEND_STATE_DESC, CachedBlendState> mCachedBlendStates;
		mutable UnorderedMap<RASTERIZER_STATE_DESC, CachedRasterizerState> mCachedRasterizerStates;
		mutable UnorderedMap<DEPTH_STENCIL_STATE_DESC, CachedDepthStencilState> mCachedDepthStencilStates;

		mutable UINT32 mNextBlendStateId = 0;
		mutable UINT32 mNextRasterizerStateId = 0;
		mutable UINT32 mNextDepthStencilStateId = 0;

		mutable Mutex mMutex;
	};
	}

	/** @} */
}