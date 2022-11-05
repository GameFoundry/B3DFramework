//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Resources/BsResource.h"

namespace bs
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/**
	 * Structure that describes blend states for a single render target. Used internally by BLEND_STATE_DESC for
	 * initializing a BlendState.
	 *
	 * @see		BLEND_STATE_DESC
	 * @see		BlendState
	 */
	struct B3D_CORE_EXPORT RENDER_TARGET_BLEND_STATE_DESC
	{
		bool operator==(const RENDER_TARGET_BLEND_STATE_DESC& rhs) const;

		/**
		 * Queries is blending enabled for the specified render target. Blending allows you to combine the color from
		 * current and previous pixel based on some value.
		 */
		bool BlendEnable = false;

		/**
		 * Determines what should the source blend factor be. This value determines what will the color being generated
		 * currently be multiplied by.
		 */
		BlendFactor SrcBlend = BF_ONE;

		/**
		 * Determines what should the destination blend factor be. This value determines what will the color already in
		 * render target be multiplied by.
		 */
		BlendFactor DstBlend = BF_ZERO;

		/**
		 * Determines how are source and destination colors combined (after they are multiplied by their respective blend
		 * factors).
		 */
		BlendOperation BlendOp = BO_ADD;

		/**
		 * Determines what should the alpha source blend factor be. This value determines what will the alpha value being
		 * generated currently be multiplied by.
		 */
		BlendFactor SrcBlendAlpha = BF_ONE;

		/**
		 * Determines what should the alpha destination blend factor be. This value determines what will the alpha value
		 * already in render target be multiplied by.
		 */
		BlendFactor DstBlendAlpha = BF_ZERO;

		/**
		 * Determines how are source and destination alpha values combined (after they are multiplied by their respective
		 * blend factors).
		 */
		BlendOperation BlendOpAlpha = BO_ADD;

		/**
		 * Render target write mask allows to choose which pixel components should the pixel shader output.
		 *
		 * Only the first four bits are used. First bit representing red, second green, third blue and fourth alpha value.
		 * Set bits means pixel shader will output those channels.
		 */
		u8 RenderTargetWriteMask = 0xFF;
	};

	/** Structure that describes render pipeline blend states. Used for initializing BlendState. */
	struct B3D_CORE_EXPORT BLEND_STATE_DESC
	{
		bool operator==(const BLEND_STATE_DESC& rhs) const;

		/**
		 * Alpha to coverage allows you to perform blending without needing to worry about order of rendering like regular
		 * blending does. It requires multi-sampling to be active in order to work, and you need to supply an alpha texture
		 * that determines object transparency.
		 *
		 * Blending is then performed by only using sub-samples covered by the alpha texture for the current pixel and
		 * combining them with sub-samples previously stored.
		 *
		 * Be aware this is a limited technique only useful for certain situations. Unless you are having performance
		 * problems use regular blending.
		 */
		bool AlphaToCoverageEnable = false;

		/**
		 * When not set, only the first render target blend descriptor will be used for all render targets. If set each
		 * render target will use its own blend descriptor.
		 */
		bool IndependantBlendEnable = false;

		RENDER_TARGET_BLEND_STATE_DESC RenderTargetDesc[B3D_MAXIMUM_RENDER_TARGET_COUNT];
	};

	/** Properties of a BlendState. Shared between sim and core thread versions of BlendState. */
	class B3D_CORE_EXPORT BlendProperties
	{
	public:
		BlendProperties(const BLEND_STATE_DESC& desc);

		/** @copydoc BLEND_STATE_DESC::AlphaToCoverageEnable */
		bool GetAlphaToCoverageEnabled() const { return mData.AlphaToCoverageEnable; }

		/** @copydoc BLEND_STATE_DESC::IndependantBlendEnable */
		bool GetIndependantBlendEnable() const { return mData.IndependantBlendEnable; }

		/** @copydoc RENDER_TARGET_BLEND_STATE_DESC::BlendEnable */
		bool GetBlendEnabled(u32 renderTargetIdx) const;

		/** @copydoc RENDER_TARGET_BLEND_STATE_DESC::SrcBlend */
		BlendFactor GetSrcBlend(u32 renderTargetIdx) const;

		/** @copydoc RENDER_TARGET_BLEND_STATE_DESC::DstBlend */
		BlendFactor GetDstBlend(u32 renderTargetIdx) const;

		/** @copydoc RENDER_TARGET_BLEND_STATE_DESC::BlendOp */
		BlendOperation GetBlendOperation(u32 renderTargetIdx) const;

		/** @copydoc RENDER_TARGET_BLEND_STATE_DESC::SrcBlendAlpha */
		BlendFactor GetAlphaSrcBlend(u32 renderTargetIdx) const;

		/** @copydoc RENDER_TARGET_BLEND_STATE_DESC::DstBlendAlpha */
		BlendFactor GetAlphaDstBlend(u32 renderTargetIdx) const;

		/** @copydoc RENDER_TARGET_BLEND_STATE_DESC::BlendOpAlpha */
		BlendOperation GetAlphaBlendOperation(u32 renderTargetIdx) const;

		/** @copydoc RENDER_TARGET_BLEND_STATE_DESC::RenderTargetWriteMask */
		u8 GetRenderTargetWriteMask(u32 renderTargetIdx) const;

		/** Returns the hash value generated from the blend state properties. */
		u64 GetHash() const { return mHash; }

	protected:
		friend class BlendState;
		friend class ct::BlendState;
		friend class BlendStateRTTI;

		BLEND_STATE_DESC mData;
		u64 mHash;
	};

	/**
	 * Render system pipeline state that allows you to modify how an object is rendered. More exactly this state allows to
	 * you to control how is a rendered object blended with any previously rendered objects.
	 *
	 * @note	Blend states are immutable. Sim thread only.
	 */
	class B3D_CORE_EXPORT BlendState : public IReflectable, public CoreObject
	{
	public:
		virtual ~BlendState();

		/**	Returns information about a blend state. */
		const BlendProperties& GetProperties() const;

		/** Retrieves a core implementation of the sampler state usable only from the core thread. */
		SPtr<ct::BlendState> GetCore() const;

		/**	Creates a new blend state using the specified blend state description structure. */
		static SPtr<BlendState> Create(const BLEND_STATE_DESC& desc);

		/**	Returns the default blend state that you may use when no other is available. */
		static const SPtr<BlendState>& GetDefault();

		/**	Generates a hash value from a blend state descriptor. */
		static u64 GenerateHash(const BLEND_STATE_DESC& desc);

	protected:
		friend class RenderStateManager;

		BlendState(const BLEND_STATE_DESC& desc);

		SPtr<ct::CoreObject> CreateCore() const override;

		BlendProperties mProperties;
		mutable u32 mId;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class BlendStateRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** @} */

	namespace ct
	{
		/** @addtogroup RenderAPI-Internal
		 *  @{
		 */

		/**
		 * Core thread version of bs::BlendState.
		 *
		 * @note	Core thread.
		 */
		class B3D_CORE_EXPORT BlendState : public CoreObject
		{
		public:
			virtual ~BlendState();

			/** Returns information about the blend state. */
			const BlendProperties& GetProperties() const;

			/** Returns a unique state ID. Only the lowest 10 bits are used. */
			u32 GetId() const { return mId; }

			/**	Creates a new blend state using the specified blend state description structure. */
			static SPtr<BlendState> Create(const BLEND_STATE_DESC& desc);

			/**	Returns the default blend state that you may use when no other is available. */
			static const SPtr<BlendState>& GetDefault();

		protected:
			friend class RenderStateManager;

			BlendState(const BLEND_STATE_DESC& desc, u32 id);

			void Initialize() override;

			/**	Creates any API-specific state objects. */
			virtual void CreateInternal() {}

			BlendProperties mProperties;
			u32 mId;
		};

		/** @} */
	} // namespace ct
} // namespace bs

/** @cond STDLIB */
/** @addtogroup RenderAPI
 *  @{
 */

namespace std
{
	/**	Hash value generator for BLEND_STATE_DESC. */
	template <>
	struct hash<bs::BLEND_STATE_DESC>
	{
		size_t operator()(const bs::BLEND_STATE_DESC& value) const
		{
			return (size_t)bs::BlendState::GenerateHash(value);
		}
	};
} // namespace std

/** @} */
/** @endcond */
