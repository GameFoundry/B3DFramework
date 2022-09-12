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
	struct BS_CORE_EXPORT RENDER_TARGET_BLEND_STATE_DESC
	{
		bool operator==(const RENDER_TARGET_BLEND_STATE_DESC& rhs) const;

		/**
		 * Queries is blending enabled for the specified render target. Blending allows you to combine the color from
		 * current and previous pixel based on some value.
		 */
		bool blendEnable = false;

		/**
		 * Determines what should the source blend factor be. This value determines what will the color being generated
		 * currently be multiplied by.
		 */
		BlendFactor srcBlend = BF_ONE;

		/**
		 * Determines what should the destination blend factor be. This value determines what will the color already in
		 * render target be multiplied by.
		 */
		BlendFactor dstBlend = BF_ZERO;

		/**
		 * Determines how are source and destination colors combined (after they are multiplied by their respective blend
		 * factors).
		 */
		BlendOperation blendOp = BO_ADD;

		/**
		 * Determines what should the alpha source blend factor be. This value determines what will the alpha value being
		 * generated currently be multiplied by.
		 */
		BlendFactor srcBlendAlpha = BF_ONE;

		/**
		 * Determines what should the alpha destination blend factor be. This value determines what will the alpha value
		 * already in render target be multiplied by.
		 */
		BlendFactor dstBlendAlpha = BF_ZERO;

		/**
		 * Determines how are source and destination alpha values combined (after they are multiplied by their respective
		 * blend factors).
		 */
		BlendOperation blendOpAlpha = BO_ADD;

		/**
		 * Render target write mask allows to choose which pixel components should the pixel shader output.
		 * 			
		 * Only the first four bits are used. First bit representing red, second green, third blue and fourth alpha value.
		 * Set bits means pixel shader will output those channels.
		 */
		UINT8 renderTargetWriteMask = 0xFF;
	};

	/** Structure that describes render pipeline blend states. Used for initializing BlendState. */
	struct BS_CORE_EXPORT BLEND_STATE_DESC
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
		bool alphaToCoverageEnable = false;

		/**
		 * When not set, only the first render target blend descriptor will be used for all render targets. If set each
		 * render target will use its own blend descriptor.
		 */
		bool independantBlendEnable = false;

		RENDER_TARGET_BLEND_STATE_DESC renderTargetDesc[BS_MAX_MULTIPLE_RENDER_TARGETS];
	};

	/** Properties of a BlendState. Shared between sim and core thread versions of BlendState. */
	class BS_CORE_EXPORT BlendProperties
	{
	public:
		BlendProperties(const BLEND_STATE_DESC& desc);

		/** @copydoc BLEND_STATE_DESC::alphaToCoverageEnable */
		bool GetAlphaToCoverageEnabled() const { return mData.alphaToCoverageEnable; }

		/** @copydoc BLEND_STATE_DESC::independantBlendEnable */
		bool GetIndependantBlendEnable() const { return mData.independantBlendEnable; }

		/** @copydoc RENDER_TARGET_BLEND_STATE_DESC::blendEnable */
		bool GetBlendEnabled(UINT32 renderTargetIdx) const;

		/** @copydoc RENDER_TARGET_BLEND_STATE_DESC::srcBlend */
		BlendFactor GetSrcBlend(UINT32 renderTargetIdx) const;

		/** @copydoc RENDER_TARGET_BLEND_STATE_DESC::dstBlend */
		BlendFactor GetDstBlend(UINT32 renderTargetIdx) const;

		/** @copydoc RENDER_TARGET_BLEND_STATE_DESC::blendOp */
		BlendOperation GetBlendOperation(UINT32 renderTargetIdx) const;

		/** @copydoc RENDER_TARGET_BLEND_STATE_DESC::srcBlendAlpha */
		BlendFactor GetAlphaSrcBlend(UINT32 renderTargetIdx) const;

		/** @copydoc RENDER_TARGET_BLEND_STATE_DESC::dstBlendAlpha */
		BlendFactor GetAlphaDstBlend(UINT32 renderTargetIdx) const;

		/** @copydoc RENDER_TARGET_BLEND_STATE_DESC::blendOpAlpha */
		BlendOperation GetAlphaBlendOperation(UINT32 renderTargetIdx) const;

		/** @copydoc RENDER_TARGET_BLEND_STATE_DESC::renderTargetWriteMask */
		UINT8 GetRenderTargetWriteMask(UINT32 renderTargetIdx) const;

		/** Returns the hash value generated from the blend state properties. */
		UINT64 GetHash() const { return mHash; }

	protected:
		friend class BlendState;
		friend class ct::BlendState;
		friend class BlendStateRTTI;

		BLEND_STATE_DESC mData;
		UINT64 mHash;
	};

	/**
	 * Render system pipeline state that allows you to modify how an object is rendered. More exactly this state allows to
	 * you to control how is a rendered object blended with any previously rendered objects.
	 * 			
	 * @note	Blend states are immutable. Sim thread only.
	 */
	class BS_CORE_EXPORT BlendState : public IReflectable, public CoreObject
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
		static UINT64 GenerateHash(const BLEND_STATE_DESC& desc);

	protected:
		friend class RenderStateManager;

		BlendState(const BLEND_STATE_DESC& desc);

		/** @copydoc CoreObject::createCore */
		SPtr<ct::CoreObject> CreateCore() const override;

		BlendProperties mProperties;
		mutable UINT32 mId;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class BlendStateRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;	
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
	class BS_CORE_EXPORT BlendState : public CoreObject
	{
	public:
		virtual ~BlendState();

		/** Returns information about the blend state. */
		const BlendProperties& GetProperties() const;

		/** Returns a unique state ID. Only the lowest 10 bits are used. */
		UINT32 GetId() const { return mId; }

		/**	Creates a new blend state using the specified blend state description structure. */
		static SPtr<BlendState> Create(const BLEND_STATE_DESC& desc);

		/**	Returns the default blend state that you may use when no other is available. */
		static const SPtr<BlendState>& GetDefault();

	protected:
		friend class RenderStateManager;

		BlendState(const BLEND_STATE_DESC& desc, UINT32 id);

		/** @copydoc CoreObject::initialize */
		void Initialize() override;

		/**	Creates any API-specific state objects. */
		virtual void CreateInternal() { }

		BlendProperties mProperties;
		UINT32 mId;
	};

	/** @} */
	}
}

/** @cond STDLIB */
/** @addtogroup RenderAPI
 *  @{
 */

namespace std
{
/**	Hash value generator for BLEND_STATE_DESC. */
template<>
struct hash<bs::BLEND_STATE_DESC>
{
	size_t operator()(const bs::BLEND_STATE_DESC& value) const
	{
		return (size_t)bs::BlendState::generateHash(value);
	}
};
}

/** @} */
/** @endcond */
