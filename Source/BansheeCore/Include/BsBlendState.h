//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsResource.h"

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

		/**
		 * Queries is blending enabled for the specified render target. Blending allows you to combine the color from 
		 * current and previous pixel based on some value.
		 */
		bool blendEnable;

		/**
		 * Determines what should the source blend factor be. This value determines what will the color being generated 
		 * currently be multiplied by.
		 */
		BlendFactor srcBlend;

		/**
		 * Determines what should the destination blend factor be. This value determines what will the color already in 
		 * render target be multiplied by.
		 */
		BlendFactor dstBlend;

		/**
		 * Determines how are source and destination colors combined (after they are multiplied by their respective blend 
		 * factors).
		 */
		BlendOperation blendOp;

		/**
		 * Determines what should the alpha source blend factor be. This value determines what will the alpha value being 
		 * generated currently be multiplied by.
		 */
		BlendFactor srcBlendAlpha;

		/**
		 * Determines what should the alpha destination blend factor be. This value determines what will the alpha value 
		 * already in render target be multiplied by.
		 */
		BlendFactor dstBlendAlpha;

		/**
		 * Determines how are source and destination alpha values combined (after they are multiplied by their respective
		 * blend factors).
		 */
		BlendOperation blendOpAlpha;

		/**
		 * Render target write mask allows to choose which pixel components should the pixel shader output.
		 * 			
		 * Only the first four bits are used. First bit representing red, second green, third blue and fourth alpha value. 
		 * Set bits means pixel shader will output those channels.
		 */
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
		bool alphaToCoverageEnable;

		/**
		 * When not set, only the first render target blend descriptor will be used for all render targets. If set each 
		 * render target will use its own blend descriptor.
		 */
		bool independantBlendEnable;

		RENDER_TARGET_BLEND_STATE_DESC renderTargetDesc[BS_MAX_MULTIPLE_RENDER_TARGETS];
	};

	/** Properties of a BlendState. Shared between sim and core thread versions of BlendState. */
	class BS_CORE_EXPORT BlendProperties
	{
	public:
		BlendProperties(const BLEND_STATE_DESC& desc);

		/** @copydoc BLEND_STATE_DESC::alphaToCoverageEnable */
		bool getAlphaToCoverageEnabled() const { return mData.alphaToCoverageEnable; }

		/** @copydoc BLEND_STATE_DESC::independantBlendEnable */
		bool getIndependantBlendEnable() const { return mData.independantBlendEnable; }

		/** @copydoc RENDER_TARGET_BLEND_STATE_DESC::blendEnable */
		bool getBlendEnabled(UINT32 renderTargetIdx) const;

		/** @copydoc RENDER_TARGET_BLEND_STATE_DESC::srcBlend */
		BlendFactor getSrcBlend(UINT32 renderTargetIdx) const;

		/** @copydoc RENDER_TARGET_BLEND_STATE_DESC::dstBlend */
		BlendFactor getDstBlend(UINT32 renderTargetIdx) const;

		/** @copydoc RENDER_TARGET_BLEND_STATE_DESC::blendOp */
		BlendOperation getBlendOperation(UINT32 renderTargetIdx) const;

		/** @copydoc RENDER_TARGET_BLEND_STATE_DESC::srcBlendAlpha */
		BlendFactor getAlphaSrcBlend(UINT32 renderTargetIdx) const;

		/** @copydoc RENDER_TARGET_BLEND_STATE_DESC::dstBlendAlpha */
		BlendFactor getAlphaDstBlend(UINT32 renderTargetIdx) const;

		/** @copydoc RENDER_TARGET_BLEND_STATE_DESC::blendOpAlpha */
		BlendOperation getAlphaBlendOperation(UINT32 renderTargetIdx) const;

		/** @copydoc RENDER_TARGET_BLEND_STATE_DESC::renderTargetWriteMask */
		UINT8 getRenderTargetWriteMask(UINT32 renderTargetIdx) const;

		/** Returns the hash value generated from the blend state properties. */
		UINT64 getHash() const { return mHash; }

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
		const BlendProperties& getProperties() const;

		/** Retrieves a core implementation of the sampler state usable only from the core thread. */
		SPtr<ct::BlendState> getCore() const;

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
		SPtr<ct::CoreObject> createCore() const override;

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
		const BlendProperties& getProperties() const;

		/** Returns a unique state ID. Only the lowest 10 bits are used. */
		UINT32 getId() const { return mId; }

		/**	Creates a new blend state using the specified blend state description structure. */
		static SPtr<BlendState> create(const BLEND_STATE_DESC& desc);

		/**	Returns the default blend state that you may use when no other is available. */
		static const SPtr<BlendState>& getDefault();

	protected:
		friend class RenderStateManager;

		BlendState(const BLEND_STATE_DESC& desc, UINT32 id);

		/** @copydoc CoreObject::initialize */
		void initialize() override;

		/**	Creates any API-specific state objects. */
		virtual void createInternal() { }

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
