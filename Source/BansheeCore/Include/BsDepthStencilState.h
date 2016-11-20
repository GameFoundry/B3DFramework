//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsIReflectable.h"
#include "BsCoreObject.h"

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
		DEPTH_STENCIL_STATE_DESC()
			: depthReadEnable(true)
			, depthWriteEnable(true)
			, depthComparisonFunc(CMPF_LESS)
			, stencilEnable(false)
			, stencilReadMask(0xFF)
			, stencilWriteMask(0xFF)
			, frontStencilFailOp(SOP_KEEP)
			, frontStencilZFailOp(SOP_KEEP)
			, frontStencilPassOp(SOP_KEEP)
			, frontStencilComparisonFunc(CMPF_ALWAYS_PASS)
			, backStencilFailOp(SOP_KEEP)
			, backStencilZFailOp(SOP_KEEP)
			, backStencilPassOp(SOP_KEEP)
			, backStencilComparisonFunc(CMPF_ALWAYS_PASS)
		{ }

		bool operator==(const DEPTH_STENCIL_STATE_DESC& rhs) const;

		bool depthReadEnable;
		bool depthWriteEnable;
		CompareFunction depthComparisonFunc;

		bool stencilEnable;
		UINT8 stencilReadMask;
		UINT8 stencilWriteMask;

		StencilOperation frontStencilFailOp;
		StencilOperation frontStencilZFailOp;
		StencilOperation frontStencilPassOp;
		CompareFunction frontStencilComparisonFunc;

		StencilOperation backStencilFailOp;
		StencilOperation backStencilZFailOp;
		StencilOperation backStencilPassOp;
		CompareFunction backStencilComparisonFunc;
	};

	/** @cond SPECIALIZATIONS */
	BS_ALLOW_MEMCPY_SERIALIZATION(DEPTH_STENCIL_STATE_DESC);
	/** @endcond */

	/** Properties of DepthStencilState. Shared between sim and core thread versions of DepthStencilState. */
	class BS_CORE_EXPORT DepthStencilProperties
	{
	public:
		DepthStencilProperties(const DEPTH_STENCIL_STATE_DESC& desc);

		/**
		 * If enabled, any pixel about to be written will be tested against the depth value currently in the buffer. If the 
		 * depth test passes (depending on the set valueand chosen depth comparison function), that pixel is written and 
		 * depth is updated (if depth write is enabled).
		 */
		bool getDepthReadEnable() const { return mData.depthReadEnable; }

		/** If enabled rendering pixels will update the depth buffer value. */
		bool getDepthWriteEnable() const { return mData.depthWriteEnable; }

		/**
		 * Determines what operation should the renderer use when comparing previous and current depth value. If the 
		 * operation passes, pixel with the current depth value will be considered visible.
		 */
		CompareFunction getDepthComparisonFunc() const { return mData.depthComparisonFunc; }

		/**
		 * If true then stencil buffer will also be updated when a pixel is written, and pixels will be tested against 
		 * the stencil buffer before rendering.
		 */
		bool getStencilEnable() const { return mData.stencilEnable; }

		/** Mask to apply to any value read from the stencil buffer, before applying the stencil comparison function. */
		UINT8 getStencilReadMask() const { return mData.stencilReadMask; }

		/**	Mask to apply to any value about to be written in the stencil buffer. */
		UINT8 getStencilWriteMask() const { return mData.stencilWriteMask; }

		/**	Operation that happens when stencil comparison function fails on a front facing polygon. */
		StencilOperation getStencilFrontFailOp() const { return mData.frontStencilFailOp; }

		/** Operation that happens when stencil comparison function passes but depth test fails on a front facing polygon. */
		StencilOperation getStencilFrontZFailOp() const { return mData.frontStencilZFailOp; }

		/**	Operation that happens when stencil comparison function passes on a front facing polygon. */
		StencilOperation getStencilFrontPassOp() const { return mData.frontStencilPassOp; }

		/**
		 * Stencil comparison function used for front facing polygons. Stencil buffer will be modified according to 
		 * previously set stencil operations depending whether this comparison passes or fails.
		 */
		CompareFunction getStencilFrontCompFunc() const { return mData.frontStencilComparisonFunc; }

		/** Operation that happens when stencil comparison function fails on a back facing polygon. */
		StencilOperation getStencilBackFailOp() const { return mData.backStencilFailOp; }

		/** Operation that happens when stencil comparison function passes but depth test fails on a back facing polygon. */
		StencilOperation getStencilBackZFailOp() const { return mData.backStencilZFailOp; }

		/**	Operation that happens when stencil comparison function passes on a back facing polygon. */
		StencilOperation getStencilBackPassOp() const { return mData.backStencilPassOp; }

		/**
		 * Stencil comparison function used for back facing polygons. Stencil buffer will be modified according	to 
		 * previously set stencil operations depending whether this comparison passes or fails.
		 */
		CompareFunction getStencilBackCompFunc() const { return mData.backStencilComparisonFunc; }

		/** Returns the hash value generated from the depth-stencil state properties. */
		UINT64 getHash() const { return mHash; }

	protected:
		friend class DepthStencilState;
		friend class DepthStencilStateCore;
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
		virtual ~DepthStencilState();

		/**	Returns information about the depth stencil state. */
		const DepthStencilProperties& getProperties() const;

		/**	Retrieves a core implementation of a sampler state usable only from the core thread. */
		SPtr<DepthStencilStateCore> getCore() const;

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
		SPtr<CoreObjectCore> createCore() const override;

		DepthStencilProperties mProperties;
		mutable UINT32 mId;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class DepthStencilStateRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;	
	};

	/** @} */

	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/**
	 * Core thread version of DepthStencilState.
	 *
	 * @note	Core thread.
	 */
	class BS_CORE_EXPORT DepthStencilStateCore : public CoreObjectCore
	{
	public:
		virtual ~DepthStencilStateCore();

		/**	Returns information about the depth stencil state. */
		const DepthStencilProperties& getProperties() const;

		/**	Returns a unique state ID. Only the lowest 10 bits are used. */
		UINT32 getId() const { return mId; }

		/**	Creates a new depth stencil state using the specified depth stencil state description structure. */
		static SPtr<DepthStencilStateCore> create(const DEPTH_STENCIL_STATE_DESC& desc);

		/**	Returns the default depth stencil state that you may use when no other is available. */
		static const SPtr<DepthStencilStateCore>& getDefault();

	protected:
		friend class RenderStateCoreManager;

		DepthStencilStateCore(const DEPTH_STENCIL_STATE_DESC& desc, UINT32 id);

		/** @copydoc CoreObjectCore::initialize */
		void initialize() override;

		/**	Creates any API-specific state objects. */
		virtual void createInternal() { }

		DepthStencilProperties mProperties;
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
