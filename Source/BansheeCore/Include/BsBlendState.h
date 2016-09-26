//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsResource.h"

namespace BansheeEngine
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
		RENDER_TARGET_BLEND_STATE_DESC()
			: blendEnable(false)
			, srcBlend(BF_ONE)
			, dstBlend(BF_ZERO)
			, blendOp(BO_ADD)
			, srcBlendAlpha(BF_ONE)
			, dstBlendAlpha(BF_ZERO)
			, blendOpAlpha(BO_ADD)
			, renderTargetWriteMask(0xFF)
		{ }

		bool operator==(const RENDER_TARGET_BLEND_STATE_DESC& rhs) const;

		bool blendEnable;
		BlendFactor srcBlend;
		BlendFactor dstBlend;
		BlendOperation blendOp;
		BlendFactor srcBlendAlpha;
		BlendFactor dstBlendAlpha;
		BlendOperation blendOpAlpha;
		// Enable write to RGBA channels separately by setting first four bits (0 - R, 1 - G, 2 - B, 3 - A)
		UINT8 renderTargetWriteMask;
	};

	/** Structure that describes render pipeline blend states. Used for initializing BlendState. */
	struct BS_CORE_EXPORT BLEND_STATE_DESC
	{
		BLEND_STATE_DESC()
			: alphaToCoverageEnable(false)
			, independantBlendEnable(false)
		{ }

		bool operator==(const BLEND_STATE_DESC& rhs) const;

		bool alphaToCoverageEnable;
		bool independantBlendEnable;
		RENDER_TARGET_BLEND_STATE_DESC renderTargetDesc[BS_MAX_MULTIPLE_RENDER_TARGETS];
	};

	/** Properties of a BlendState. Shared between sim and core thread versions of BlendState. */
	class BS_CORE_EXPORT BlendProperties
	{
	public:
		BlendProperties(const BLEND_STATE_DESC& desc);

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
		bool getAlphaToCoverageEnabled() const { return mData.alphaToCoverageEnable; }

		/**
		 * When not set, only the first render target blend descriptor will be used for all render targets. If set each 
		 * render target will use its own blend descriptor.
		 */
		bool getIndependantBlendEnable() const { return mData.independantBlendEnable; }

		/**
		 * Queries is blending enabled for the specified render target. Blending allows you to combine the color from 
		 * current and previous pixel based on some value.
		 */
		bool getBlendEnabled(UINT32 renderTargetIdx) const;

		/**
		 * Determines what should the source blend factor be. This value determines what will the color being generated 
		 * currently be multiplied by.
		 */
		BlendFactor getSrcBlend(UINT32 renderTargetIdx) const;

		/**
		 * Determines what should the destination blend factor be. This value determines what will the color already in 
		 * render target be multiplied by.
		 */
		BlendFactor getDstBlend(UINT32 renderTargetIdx) const;

		/**
		 * Determines how are source and destination colors combined (after they are multiplied by their respective blend 
		 * factors).
		 */
		BlendOperation getBlendOperation(UINT32 renderTargetIdx) const;

		/**
		 * Determines what should the alpha source blend factor be. This value determines what will the alpha value being 
		 * generated currently be multiplied by.
		 */
		BlendFactor getAlphaSrcBlend(UINT32 renderTargetIdx) const;

		/**
		 * Determines what should the alpha destination blend factor be. This value determines what will the alpha value 
		 * already in render target be multiplied by.
		 */
		BlendFactor getAlphaDstBlend(UINT32 renderTargetIdx) const;

		/**
		 * Determines how are source and destination alpha values combined (after they are multiplied by their respective
		 * blend factors).
		 */
		BlendOperation getAlphaBlendOperation(UINT32 renderTargetIdx) const;

		/**
		 * Render target write mask allows to choose which pixel components should the pixel shader output.
		 * 			
		 * Only the first four bits are used. First bit representing red, second green, third blue and fourth alpha value. 
		 * Set bits means pixel shader will output those channels.
		 */
		UINT8 getRenderTargetWriteMask(UINT32 renderTargetIdx) const;

		/** Returns the hash value generated from the blend state properties. */
		UINT64 getHash() const { return mHash; }

	protected:
		friend class BlendState;
		friend class BlendStateCore;
		friend class BlendStateRTTI;

		BLEND_STATE_DESC mData;
		UINT64 mHash;
	};

	/**
	 * Render system pipeline state that allows you to modify how an object is rendered. More exactly this state allows to 
	 * you to control how is a rendered object blended with any previously renderer objects.
	 * 			
	 * @note	Blend states are immutable. Sim thread only.
	 */
	class BS_CORE_EXPORT BlendState : public IReflectable, public CoreObject
	{
	public:
		virtual ~BlendState();

		/**	Returns information about a blend state. */
		const BlendProperties& getProperties() const;

		/** Retrieves a core implementation of the sampler state usable only from the core thread. */
		SPtr<BlendStateCore> getCore() const;

		/**	Creates a new blend state using the specified blend state description structure. */
		static SPtr<BlendState> create(const BLEND_STATE_DESC& desc);

		/**	Returns the default blend state that you may use when no other is available. */
		static const SPtr<BlendState>& getDefault();

		/**	Generates a hash value from a blend state descriptor. */
		static UINT64 generateHash(const BLEND_STATE_DESC& desc);

	protected:
		friend class RenderStateManager;

		BlendState(const BLEND_STATE_DESC& desc);

		/** @copydoc CoreObject::createCore */
		SPtr<CoreObjectCore> createCore() const override;

		BlendProperties mProperties;
		mutable UINT32 mId;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class BlendStateRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;	
	};

	/** @} */
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/**
	 * Core thread version of BlendState.
	 *
	 * @note	Core thread.
	 */
	class BS_CORE_EXPORT BlendStateCore : public CoreObjectCore
	{
	public:
		virtual ~BlendStateCore();

		/** Returns information about the blend state. */
		const BlendProperties& getProperties() const;

		/** Returns a unique state ID. Only the lowest 10 bits are used. */
		UINT32 getId() const { return mId; }

		/**	Creates a new blend state using the specified blend state description structure. */
		static SPtr<BlendStateCore> create(const BLEND_STATE_DESC& desc);

		/**	Returns the default blend state that you may use when no other is available. */
		static const SPtr<BlendStateCore>& getDefault();

	protected:
		friend class RenderStateCoreManager;

		BlendStateCore(const BLEND_STATE_DESC& desc, UINT32 id);

		/** @copydoc CoreObjectCore::initialize */
		void initialize() override;

		/**	Creates any API-specific state objects. */
		virtual void createInternal() { }

		BlendProperties mProperties;
		UINT32 mId;
	};

	/** @} */
}

/** @cond STDLIB */
/** @addtogroup RenderAPI
 *  @{
 */

namespace std
{
/**	Hash value generator for BLEND_STATE_DESC. */
template<>
struct hash<BansheeEngine::BLEND_STATE_DESC>
{
	size_t operator()(const BansheeEngine::BLEND_STATE_DESC& value) const
	{
		return (size_t)BansheeEngine::BlendState::generateHash(value);
	}
};
}

/** @} */
/** @endcond */
