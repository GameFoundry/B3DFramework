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
	class BS_CORE_EXPORT RenderStateManager : public Module<RenderStateManager>
	{
	public:
		/**
		 * Creates and initializes a new SamplerState.
		 *
		 * @param[in]	desc	Object describing the sampler state to create.
		 */
		SPtr<SamplerState> CreateSamplerState(const SAMPLER_STATE_DESC& desc) const;

		/** Creates and initializes a new DepthStencilState. */
		SPtr<DepthStencilState> CreateDepthStencilState(const DEPTH_STENCIL_STATE_DESC& desc) const;

		/**	Creates and initializes a new RasterizerState. */
		SPtr<RasterizerState> CreateRasterizerState(const RASTERIZER_STATE_DESC& desc) const;

		/**	Creates and initializes a new BlendState. */
		SPtr<BlendState> CreateBlendState(const BLEND_STATE_DESC& desc) const;

		/**
		 * Creates and initializes a new GraphicsPipelineState.
		 *
		 * @param[in]	desc	Object describing the pipeline to create.
		 */
		SPtr<GraphicsPipelineState> CreateGraphicsPipelineState(const PIPELINE_STATE_DESC& desc) const;

		/**
		 * Creates and initializes a new ComputePipelineState.
		 *
		 * @param[in]	program		Compute GPU program to be executed by the pipeline.
		 */
		SPtr<ComputePipelineState> CreateComputePipelineState(const SPtr<GpuProgram>& program) const;

		/** Creates an uninitialized sampler state. Requires manual initialization after creation. */
		SPtr<SamplerState> CreateSamplerStatePtrInternal(const SAMPLER_STATE_DESC& desc) const;

		/** Creates an uninitialized depth-stencil state. Requires manual initialization after creation. */
		SPtr<DepthStencilState> CreateDepthStencilStatePtrInternal(const DEPTH_STENCIL_STATE_DESC& desc) const;

		/** Creates an uninitialized rasterizer state. Requires manual initialization after creation. */
		SPtr<RasterizerState> CreateRasterizerStatePtrInternal(const RASTERIZER_STATE_DESC& desc) const;

		/** Creates an uninitialized blend state. Requires manual initialization after creation. */
		SPtr<BlendState> CreateBlendStatePtrInternal(const BLEND_STATE_DESC& desc) const;

		/**	Creates an uninitialized GraphicsPipelineState. Requires manual initialization after creation. */
		virtual SPtr<GraphicsPipelineState> CreateGraphicsPipelineStateInternal(const PIPELINE_STATE_DESC& desc) const;

		/**	Creates an uninitialized ComputePipelineState. Requires manual initialization after creation. */
		virtual SPtr<ComputePipelineState> CreateComputePipelineStateInternal(const SPtr<GpuProgram>& program) const;

		/** Gets a sampler state initialized with default options. */
		const SPtr<SamplerState>& GetDefaultSamplerState() const;

		/**	Gets a blend state initialized with default options. */
		const SPtr<BlendState>& GetDefaultBlendState() const;

		/**	Gets a rasterizer state initialized with default options. */
		const SPtr<RasterizerState>& GetDefaultRasterizerState() const;

		/**	Gets a depth stencil state initialized with default options. */
		const SPtr<DepthStencilState>& GetDefaultDepthStencilState() const;

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

				CachedBlendState(u32 id)
					: Id(id)
				{}

				std::weak_ptr<BlendState> State;
				u32 Id = 0;
			};

			/**	Contains data about a cached rasterizer state. */
			struct CachedRasterizerState
			{
				CachedRasterizerState() = default;

				CachedRasterizerState(u32 id)
					: Id(id)
				{}

				std::weak_ptr<RasterizerState> State;
				u32 Id = 0;
			};

			/** Contains data about a cached depth stencil state. */
			struct CachedDepthStencilState
			{
				CachedDepthStencilState() = default;

				CachedDepthStencilState(u32 id)
					: Id(id)
				{}

				std::weak_ptr<DepthStencilState> State;
				u32 Id = 0;
			};

		public:
			RenderStateManager() = default;

			/**
			 * @copydoc bs::RenderStateManager::CreateSamplerState
			 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
			 */
			SPtr<SamplerState> CreateSamplerState(const SAMPLER_STATE_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) const;

			/** @copydoc bs::RenderStateManager::CreateDepthStencilState */
			SPtr<DepthStencilState> CreateDepthStencilState(const DEPTH_STENCIL_STATE_DESC& desc) const;

			/** @copydoc bs::RenderStateManager::CreateRasterizerState */
			SPtr<RasterizerState> CreateRasterizerState(const RASTERIZER_STATE_DESC& desc) const;

			/** @copydoc bs::RenderStateManager::CreateBlendState */
			SPtr<BlendState> CreateBlendState(const BLEND_STATE_DESC& desc) const;

			/**
			 * @copydoc bs::RenderStateManager::CreateGraphicsPipelineState
			 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
			 */
			SPtr<GraphicsPipelineState> CreateGraphicsPipelineState(const PIPELINE_STATE_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) const;

			/**
			 * @copydoc bs::RenderStateManager::CreateComputePipelineState
			 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
			 */
			SPtr<ComputePipelineState> CreateComputePipelineState(const SPtr<GpuProgram>& program, GpuDeviceFlags deviceMask = GDF_DEFAULT) const;

			/** @copydoc GpuPipelineParamInfo::Create */
			SPtr<GpuPipelineParamInfo> CreatePipelineParamInfo(const GPU_PIPELINE_PARAMS_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) const;

			/** Creates an uninitialized sampler state. Requires manual initialization after creation. */
			SPtr<SamplerState> CreateSamplerStateInternal(const SAMPLER_STATE_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) const;

			/** Creates an uninitialized depth-stencil state. Requires manual initialization after creation. */
			SPtr<DepthStencilState> CreateDepthStencilStateInternal(const DEPTH_STENCIL_STATE_DESC& desc) const;

			/** Creates an uninitialized rasterizer state. Requires manual initialization after creation. */
			SPtr<RasterizerState> CreateRasterizerStateInternal(const RASTERIZER_STATE_DESC& desc) const;

			/** Creates an uninitialized blend state. Requires manual initialization after creation. */
			SPtr<BlendState> CreateBlendStateInternal(const BLEND_STATE_DESC& desc) const;

			/**	Creates an uninitialized GraphicsPipelineState. Requires manual initialization after creation. */
			virtual SPtr<GraphicsPipelineState> CreateGraphicsPipelineStateInternal(const PIPELINE_STATE_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) const;

			/**	Creates an uninitialized ComputePipelineState. Requires manual initialization after creation. */
			virtual SPtr<ComputePipelineState> CreateComputePipelineStateInternal(const SPtr<GpuProgram>& program, GpuDeviceFlags deviceMask = GDF_DEFAULT) const;

			/**	Creates an uninitialized GpuPipelineParamInfo. Requires manual initialization after creation. */
			virtual SPtr<GpuPipelineParamInfo> CreatePipelineParamInfoInternal(const GPU_PIPELINE_PARAMS_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) const;

			/** Gets a sampler state initialized with default options. */
			const SPtr<SamplerState>& GetDefaultSamplerState() const;

			/**	Gets a blend state initialized with default options. */
			const SPtr<BlendState>& GetDefaultBlendState() const;

			/**	Gets a rasterizer state initialized with default options. */
			const SPtr<RasterizerState>& GetDefaultRasterizerState() const;

			/**	Gets a depth stencil state initialized with default options. */
			const SPtr<DepthStencilState>& GetDefaultDepthStencilState() const;

		protected:
			friend class bs::SamplerState;
			friend class bs::BlendState;
			friend class bs::RasterizerState;
			friend class bs::DepthStencilState;
			friend class SamplerState;
			friend class BlendState;
			friend class RasterizerState;
			friend class DepthStencilState;

			void OnShutDown() override;

			/** @copydoc CreateSamplerState */
			virtual SPtr<SamplerState> CreateSamplerStateInternalInternal(const SAMPLER_STATE_DESC& desc, GpuDeviceFlags deviceMask) const;

			/** @copydoc CreateBlendState */
			virtual SPtr<BlendState> CreateBlendStateInternalInternal(const BLEND_STATE_DESC& desc, u32 id) const;

			/** @copydoc CreateRasterizerState */
			virtual SPtr<RasterizerState> CreateRasterizerStateInternalInternal(const RASTERIZER_STATE_DESC& desc, u32 id) const;

			/** @copydoc CreateDepthStencilState */
			virtual SPtr<DepthStencilState> CreateDepthStencilStateInternalInternal(const DEPTH_STENCIL_STATE_DESC& desc, u32 id) const;

		private:
			/**	Triggered when a new sampler state is created. */
			void NotifySamplerStateCreated(const SAMPLER_STATE_DESC& desc, const SPtr<SamplerState>& state) const;

			/**	Triggered when a new sampler state is created. */
			void NotifyBlendStateCreated(const BLEND_STATE_DESC& desc, const CachedBlendState& state) const;

			/**	Triggered when a new sampler state is created. */
			void NotifyRasterizerStateCreated(const RASTERIZER_STATE_DESC& desc, const CachedRasterizerState& state) const;

			/**	Triggered when a new sampler state is created. */
			void NotifyDepthStencilStateCreated(const DEPTH_STENCIL_STATE_DESC& desc, const CachedDepthStencilState& state) const;

			/**
			 * Triggered when the last reference to a specific sampler state is destroyed, which means we must clear our cached
			 * version as well.
			 */
			void NotifySamplerStateDestroyed(const SAMPLER_STATE_DESC& desc) const;

			/**
			 * Attempts to find a cached sampler state corresponding to the provided descriptor. Returns null if one doesn't
			 * exist.
			 */
			SPtr<SamplerState> FindCachedState(const SAMPLER_STATE_DESC& desc) const;

			/**
			 * Attempts to find a cached blend state corresponding to the provided descriptor. Returns null if one doesn't exist.
			 */
			SPtr<BlendState> FindCachedState(const BLEND_STATE_DESC& desc, u32& id) const;

			/**
			 * Attempts to find a cached rasterizer state corresponding to the provided descriptor. Returns null if one doesn't
			 * exist.
			 */
			SPtr<RasterizerState> FindCachedState(const RASTERIZER_STATE_DESC& desc, u32& id) const;

			/**
			 * Attempts to find a cached depth-stencil state corresponding to the provided descriptor. Returns null if one
			 * doesn't exist.
			 */
			SPtr<DepthStencilState> FindCachedState(const DEPTH_STENCIL_STATE_DESC& desc, u32& id) const;

			mutable SPtr<SamplerState> mDefaultSamplerState;
			mutable SPtr<BlendState> mDefaultBlendState;
			mutable SPtr<RasterizerState> mDefaultRasterizerState;
			mutable SPtr<DepthStencilState> mDefaultDepthStencilState;

			mutable UnorderedMap<SAMPLER_STATE_DESC, std::weak_ptr<SamplerState>> mCachedSamplerStates;
			mutable UnorderedMap<BLEND_STATE_DESC, CachedBlendState> mCachedBlendStates;
			mutable UnorderedMap<RASTERIZER_STATE_DESC, CachedRasterizerState> mCachedRasterizerStates;
			mutable UnorderedMap<DEPTH_STENCIL_STATE_DESC, CachedDepthStencilState> mCachedDepthStencilStates;

			mutable u32 mNextBlendStateId = 0;
			mutable u32 mNextRasterizerStateId = 0;
			mutable u32 mNextDepthStencilStateId = 0;

			mutable Mutex mMutex;
		};
	} // namespace ct

	/** @} */
} // namespace bs
