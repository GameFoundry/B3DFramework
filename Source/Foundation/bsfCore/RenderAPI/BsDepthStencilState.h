//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsIReflectable.h"
#include "CoreThread/BsCoreObject.h"

namespace bs
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/**
	 * Descriptor structured used for initializing DepthStencilState.
	 *
	 * @see		DepthStencilState
	 */
	struct BS_CORE_EXPORT DEPTH_STENCIL_STATE_DESC
	{
		bool operator==(const DEPTH_STENCIL_STATE_DESC& rhs) const;

		/**
		 * If enabled, any pixel about to be written will be tested against the depth value currently in the buffer. If the 
		 * depth test passes (depending on the set valueand chosen depth comparison function), that pixel is written and 
		 * depth is updated (if depth write is enabled).
		 */
		bool depthReadEnable = true;

		/** If enabled rendering pixels will update the depth buffer value. */
		bool depthWriteEnable = true;

		/**
		 * Determines what operation should the renderer use when comparing previous and current depth value. If the 
		 * operation passes, pixel with the current depth value will be considered visible.
		 */
		CompareFunction depthComparisonFunc = CMPF_LESS;

		/**
		 * If true then stencil buffer will also be updated when a pixel is written, and pixels will be tested against 
		 * the stencil buffer before rendering.
		 */
		bool stencilEnable = false;

		/** Mask to apply to any value read from the stencil buffer, before applying the stencil comparison function. */
		UINT8 stencilReadMask = 0xFF;

		/**	Mask to apply to any value about to be written in the stencil buffer. */
		UINT8 stencilWriteMask = 0xFF;

		/**	Operation that happens when stencil comparison function fails on a front facing polygon. */
		StencilOperation frontStencilFailOp = SOP_KEEP;

		/** Operation that happens when stencil comparison function passes but depth test fails on a front facing polygon. */
		StencilOperation frontStencilZFailOp = SOP_KEEP;

		/**	Operation that happens when stencil comparison function passes on a front facing polygon. */
		StencilOperation frontStencilPassOp = SOP_KEEP;

		/**
		 * Stencil comparison function used for front facing polygons. Stencil buffer will be modified according to 
		 * previously set stencil operations depending whether this comparison passes or fails.
		 */
		CompareFunction frontStencilComparisonFunc = CMPF_ALWAYS_PASS;

		/** Operation that happens when stencil comparison function fails on a back facing polygon. */
		StencilOperation backStencilFailOp = SOP_KEEP;

		/** Operation that happens when stencil comparison function passes but depth test fails on a back facing polygon. */
		StencilOperation backStencilZFailOp = SOP_KEEP;

		/**	Operation that happens when stencil comparison function passes on a back facing polygon. */
		StencilOperation backStencilPassOp = SOP_KEEP;

		/**
		 * Stencil comparison function used for back facing polygons. Stencil buffer will be modified according	to 
		 * previously set stencil operations depending whether this comparison passes or fails.
		 */
		CompareFunction backStencilComparisonFunc = CMPF_ALWAYS_PASS;
	};

	/** @cond SPECIALIZATIONS */
	BS_ALLOW_MEMCPY_SERIALIZATION(DEPTH_STENCIL_STATE_DESC);
	/** @endcond */

	/** Properties of DepthStencilState. Shared between sim and core thread versions of DepthStencilState. */
	class BS_CORE_EXPORT DepthStencilProperties
	{
	public:
		DepthStencilProperties(const DEPTH_STENCIL_STATE_DESC& desc);

		/** @copydoc DEPTH_STENCIL_STATE_DESC::depthReadEnable */
		bool getDepthReadEnable() const { return mData.depthReadEnable; }

		/** @copydoc DEPTH_STENCIL_STATE_DESC::depthWriteEnable */
		bool getDepthWriteEnable() const { return mData.depthWriteEnable; }

		/** @copydoc DEPTH_STENCIL_STATE_DESC::depthComparisonFunc */
		CompareFunction getDepthComparisonFunc() const { return mData.depthComparisonFunc; }

		/** @copydoc DEPTH_STENCIL_STATE_DESC::stencilEnable */
		bool getStencilEnable() const { return mData.stencilEnable; }

		/** @copydoc DEPTH_STENCIL_STATE_DESC::stencilReadMask */
		UINT8 getStencilReadMask() const { return mData.stencilReadMask; }

		/** @copydoc DEPTH_STENCIL_STATE_DESC::stencilWriteMask */
		UINT8 getStencilWriteMask() const { return mData.stencilWriteMask; }

		/** @copydoc DEPTH_STENCIL_STATE_DESC::frontStencilFailOp */
		StencilOperation getStencilFrontFailOp() const { return mData.frontStencilFailOp; }

		/** @copydoc DEPTH_STENCIL_STATE_DESC::frontStencilZFailOp */
		StencilOperation getStencilFrontZFailOp() const { return mData.frontStencilZFailOp; }

		/** @copydoc DEPTH_STENCIL_STATE_DESC::frontStencilPassOp */
		StencilOperation getStencilFrontPassOp() const { return mData.frontStencilPassOp; }

		/** @copydoc DEPTH_STENCIL_STATE_DESC::frontStencilComparisonFunc */
		CompareFunction getStencilFrontCompFunc() const { return mData.frontStencilComparisonFunc; }

		/** @copydoc DEPTH_STENCIL_STATE_DESC::backStencilFailOp */
		StencilOperation getStencilBackFailOp() const { return mData.backStencilFailOp; }

		/** @copydoc DEPTH_STENCIL_STATE_DESC::backStencilZFailOp */
		StencilOperation getStencilBackZFailOp() const { return mData.backStencilZFailOp; }

		/** @copydoc DEPTH_STENCIL_STATE_DESC::backStencilPassOp */
		StencilOperation getStencilBackPassOp() const { return mData.backStencilPassOp; }

		/** @copydoc DEPTH_STENCIL_STATE_DESC::backStencilComparisonFunc */
		CompareFunction getStencilBackCompFunc() const { return mData.backStencilComparisonFunc; }

		/** Returns the hash value generated from the depth-stencil state properties. */
		UINT64 getHash() const { return mHash; }

	protected:
		friend class DepthStencilState;
		friend class ct::DepthStencilState;
		friend class DepthStencilStateRTTI;

		DEPTH_STENCIL_STATE_DESC mData;
		UINT64 mHash;
	};

	/**
	 * Render system pipeline state that allows you to modify how an object is rendered. More exactly this state allows to 
	 * you to control how are depth and stencil buffers modified upon rendering.
	 *
	 * @note	Depth stencil states are immutable. Sim thread only.
	 */
	class BS_CORE_EXPORT DepthStencilState : public IReflectable, public CoreObject
	{
	public:
		virtual ~DepthStencilState() = default;

		/**	Returns information about the depth stencil state. */
		const DepthStencilProperties& getProperties() const;

		/**	Retrieves a core implementation of a sampler state usable only from the core thread. */
		SPtr<ct::DepthStencilState> getCore() const;

		/**	Creates a new depth stencil state using the specified depth stencil state description structure. */
		static SPtr<DepthStencilState> create(const DEPTH_STENCIL_STATE_DESC& desc);

		/**	Returns the default depth stencil state that you may use when no other is available. */
		static const SPtr<DepthStencilState>& getDefault();

		/**	Generates a hash value from a depth-stencil state descriptor. */
		static UINT64 generateHash(const DEPTH_STENCIL_STATE_DESC& desc);

	protected:
		friend class RenderStateManager;

		DepthStencilState(const DEPTH_STENCIL_STATE_DESC& desc);

		/** @copydoc CoreObject::createCore */
		SPtr<ct::CoreObject> createCore() const override;

		DepthStencilProperties mProperties;
		mutable UINT32 mId;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class DepthStencilStateRTTI;
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
	 * Core thread version of bs::DepthStencilState.
	 *
	 * @note	Core thread.
	 */
	class BS_CORE_EXPORT DepthStencilState : public CoreObject
	{
	public:
		virtual ~DepthStencilState();

		/**	Returns information about the depth stencil state. */
		const DepthStencilProperties& getProperties() const;

		/**	Returns a unique state ID. Only the lowest 10 bits are used. */
		UINT32 getId() const { return mId; }

		/**	Creates a new depth stencil state using the specified depth stencil state description structure. */
		static SPtr<DepthStencilState> create(const DEPTH_STENCIL_STATE_DESC& desc);

		/**	Returns the default depth stencil state that you may use when no other is available. */
		static const SPtr<DepthStencilState>& getDefault();

	protected:
		friend class RenderStateManager;

		DepthStencilState(const DEPTH_STENCIL_STATE_DESC& desc, UINT32 id);

		/** @copydoc CoreObject::initialize */
		void initialize() override;

		/**	Creates any API-specific state objects. */
		virtual void createInternal() { }

		DepthStencilProperties mProperties;
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
/**	Hash value generator for DEPTH_STENCIL_STATE_DESC. */
template<>
struct hash<bs::DEPTH_STENCIL_STATE_DESC>
{
	size_t operator()(const bs::DEPTH_STENCIL_STATE_DESC& value) const
	{
		return (size_t)bs::DepthStencilState::generateHash(value);
	}
};
}

/** @} */
/** @endcond */
