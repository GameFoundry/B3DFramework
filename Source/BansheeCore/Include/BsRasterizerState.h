//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsIReflectable.h"
#include "BsCoreObject.h"

namespace BansheeEngine
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/** Structure that describes pipeline rasterizer state. Used for initializing a RasterizerState. */
	struct BS_CORE_EXPORT RASTERIZER_STATE_DESC
	{
		RASTERIZER_STATE_DESC()
			: polygonMode(PM_SOLID)
			, cullMode(CULL_COUNTERCLOCKWISE)
			, depthBias(0)
			, depthBiasClamp(0.0f)
			, slopeScaledDepthBias(0.0f)
			, depthClipEnable(true)
			, scissorEnable(false)
			, multisampleEnable(true)
			, antialiasedLineEnable(false)
		{ }

		bool operator==(const RASTERIZER_STATE_DESC& rhs) const;

		PolygonMode polygonMode;
		CullingMode cullMode;

		float depthBias;
		float depthBiasClamp;
		float slopeScaledDepthBias;

		bool depthClipEnable;
		bool scissorEnable;
		bool multisampleEnable;
		bool antialiasedLineEnable;
	};

	/** Properties of RasterizerState. Shared between sim and core thread versions of RasterizerState. */
	class BS_CORE_EXPORT RasterizerProperties
	{
	public:
		RasterizerProperties(const RASTERIZER_STATE_DESC& desc);

		/** Polygon mode allows you to draw polygons as solid objects or as wireframe by just drawing their edges. */
		PolygonMode getPolygonMode() const { return mData.polygonMode; }

		/**
		 * Sets vertex winding order. Faces that contain vertices with this order will be culled and not rasterized. Used 
		 * primarily for saving cycles by not rendering backfacing faces.
		 */
		CullingMode getCullMode() const { return mData.cullMode; }

		/**
		 * Represents a constant depth bias that will offset the depth values of new pixels by the specified amount.
		 *
		 * @note		This is useful if you want to avoid z fighting for objects at the same or similar depth.
		 */
		float getDepthBias() const { return mData.depthBias; }

		/**	Maximum depth bias value. */
		float getDepthBiasClamp() const { return mData.depthBiasClamp; }

		/**
		 * Represents a dynamic depth bias that increases as the slope of the rendered polygons surface increases. 
		 * Resulting value offsets depth values of new pixels. This offset will be added on top of the constant depth bias.
		 *
		 * @note	This is useful if you want to avoid z fighting for objects at the same or similar depth.
		 */
		float getSlopeScaledDepthBias() const { return mData.slopeScaledDepthBias; }

		/**
		 * If true, clipping of polygons past the far Z plane is enabled. This ensures proper Z ordering for polygons 
		 * outside of valid depth range (otherwise they all have the same depth). It can be useful to disable if you are 
		 * performing stencil operations that count on objects having a front and a back (like stencil shadow) and don't 
		 * want to clip the back.
		 */
		bool getDepthClipEnable() const { return mData.depthClipEnable; }

		/**
		 * Scissor rectangle allows you to cull all pixels outside of the scissor rectangle.
		 *			
		 * @see		RenderAPICore::setScissorRect
		 */
		bool getScissorEnable() const { return mData.scissorEnable; }

		/**
		 * Determines how are samples in multi-sample render targets handled. If disabled all samples in the render target 
		 * will be written the same value, and if enabled each sample will be generated separately.
		 *			
		 * @note	In order to get an antialiased image you need to both enable this option and use a MSAA render target.
		 */
		bool getMultisampleEnable() const { return mData.multisampleEnable; }

		/**
		 * Determines should the lines be antialiased. This is separate from multi-sample antialiasing setting as lines can
		 * be antialiased without multi-sampling.
		 *
		 * @note	This setting is usually ignored if MSAA is used, as that provides sufficient antialiasing.
		 */
		bool getAntialiasedLineEnable() const { return mData.antialiasedLineEnable; }

		/** Returns the hash value generated from the rasterizer state properties. */
		UINT64 getHash() const { return mHash; }

	protected:
		friend class RasterizerState;
		friend class RasterizerStateCore;
		friend class RasterizerStateRTTI;

		RASTERIZER_STATE_DESC mData;
		UINT64 mHash;
	};

	/**
	 * Render system pipeline state that allows you to modify how an object is rasterized (how are polygons converted
	 * to pixels).
	 *
	 * @note	Rasterizer states are immutable. Sim thread only.
	 */
	class BS_CORE_EXPORT RasterizerState : public IReflectable, public CoreObject
	{
	public:
		virtual ~RasterizerState();

		/**	Returns information about the rasterizer state. */
		const RasterizerProperties& getProperties() const;

		/** Retrieves a core implementation of the rasterizer state usable only from the core thread. */
		SPtr<RasterizerStateCore> getCore() const;

		/** Creates a new rasterizer state using the specified rasterizer state descriptor structure. */
		static SPtr<RasterizerState> create(const RASTERIZER_STATE_DESC& desc);

		/**	Returns the default rasterizer state. */
		static const SPtr<RasterizerState>& getDefault();

		/**	Generates a hash value from a rasterizer state descriptor. */
		static UINT64 generateHash(const RASTERIZER_STATE_DESC& desc);

	protected:
		friend class RenderStateManager;

		RasterizerState(const RASTERIZER_STATE_DESC& desc);
		
		/** @copydoc CoreObject::createCore */
		SPtr<CoreObjectCore> createCore() const override;

		RasterizerProperties mProperties;
		mutable UINT32 mId;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class RasterizerStateRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;	
	};

	/** @} */

	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/**
	 * Core thread version of RasterizerState.
	 *
	 * @note	Core thread.
	 */
	class BS_CORE_EXPORT RasterizerStateCore : public CoreObjectCore
	{
	public:
		virtual ~RasterizerStateCore();

		/** Returns information about the rasterizer state. */
		const RasterizerProperties& getProperties() const;

		/**	Returns a unique state ID. Only the lowest 10 bits are used. */
		UINT32 getId() const { return mId; }

		/** Creates a new rasterizer state using the specified rasterizer state descriptor structure. */
		static SPtr<RasterizerStateCore> create(const RASTERIZER_STATE_DESC& desc);

		/** Returns the default rasterizer state. */
		static const SPtr<RasterizerStateCore>& getDefault();

	protected:
		friend class RenderStateCoreManager;

		RasterizerStateCore(const RASTERIZER_STATE_DESC& desc, UINT32 id);

		/** @copydoc CoreObjectCore::initialize */
		void initialize() override;

		/**	Creates any API-specific state objects. */
		virtual void createInternal() { }

		RasterizerProperties mProperties;
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
/**	Hash value generator for RASTERIZER_STATE_DESC. */
template<>
struct hash<BansheeEngine::RASTERIZER_STATE_DESC>
{
	size_t operator()(const BansheeEngine::RASTERIZER_STATE_DESC& value) const
	{
		return (size_t)BansheeEngine::RasterizerState::generateHash(value);
	}
};
}

/** @} */
/** @endcond */
