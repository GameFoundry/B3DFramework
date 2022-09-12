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

	/** Structure that describes pipeline rasterizer state. Used for initializing a RasterizerState. */
	struct BS_CORE_EXPORT RASTERIZER_STATE_DESC
	{
		bool operator==(const RASTERIZER_STATE_DESC& rhs) const;

		/** Polygon mode allows you to draw polygons as solid objects or as wireframe by just drawing their edges. */
		PolygonMode polygonMode = PM_SOLID;

		/**
		 * Sets vertex winding order. Faces that contain vertices with this order will be culled and not rasterized. Used
		 * primarily for saving cycles by not rendering backfacing faces.
		 */
		CullingMode cullMode = CULL_COUNTERCLOCKWISE;

		/**
		 * Represents a constant depth bias that will offset the depth values of new pixels by the specified amount.
		 *
		 * @note		This is useful if you want to avoid z fighting for objects at the same or similar depth.
		 */
		float depthBias = 0;

		/**	Maximum depth bias value. */
		float depthBiasClamp = 0.0f;

		/**
		 * Represents a dynamic depth bias that increases as the slope of the rendered polygons surface increases.
		 * Resulting value offsets depth values of new pixels. This offset will be added on top of the constant depth bias.
		 *
		 * @note	This is useful if you want to avoid z fighting for objects at the same or similar depth.
		 */
		float slopeScaledDepthBias = 0.0f;

		/**
		 * If true, clipping of polygons past the far Z plane is enabled. This ensures proper Z ordering for polygons
		 * outside of valid depth range (otherwise they all have the same depth). It can be useful to disable if you are
		 * performing stencil operations that count on objects having a front and a back (like stencil shadow) and don't
		 * want to clip the back.
		 */
		bool depthClipEnable = true;

		/**
		 * Scissor rectangle allows you to cull all pixels outside of the scissor rectangle.
		 *			
		 * @see		ct::RenderAPI::setScissorRect
		 */
		bool scissorEnable = false;

		/**
		 * Determines how are samples in multi-sample render targets handled. If disabled all samples in the render target
		 * will be written the same value, and if enabled each sample will be generated separately.
		 *			
		 * @note	In order to get an antialiased image you need to both enable this option and use a MSAA render target.
		 */
		bool multisampleEnable = true;

		/**
		 * Determines should the lines be antialiased. This is separate from multi-sample antialiasing setting as lines can
		 * be antialiased without multi-sampling.
		 *
		 * @note	This setting is usually ignored if MSAA is used, as that provides sufficient antialiasing.
		 */
		bool antialiasedLineEnable = false;
	};

	/** Properties of RasterizerState. Shared between sim and core thread versions of RasterizerState. */
	class BS_CORE_EXPORT RasterizerProperties
	{
	public:
		RasterizerProperties(const RASTERIZER_STATE_DESC& desc);

		/** @copydoc RASTERIZER_STATE_DESC::polygonMode */
		PolygonMode GetPolygonMode() const { return mData.polygonMode; }

		/** @copydoc RASTERIZER_STATE_DESC::cullMode */
		CullingMode GetCullMode() const { return mData.cullMode; }

		/** @copydoc RASTERIZER_STATE_DESC::depthBias */
		float GetDepthBias() const { return mData.depthBias; }

		/** @copydoc RASTERIZER_STATE_DESC::depthBiasClamp */
		float GetDepthBiasClamp() const { return mData.depthBiasClamp; }

		/** @copydoc RASTERIZER_STATE_DESC::slopeScaledDepthBias */
		float GetSlopeScaledDepthBias() const { return mData.slopeScaledDepthBias; }

		/** @copydoc RASTERIZER_STATE_DESC::depthClipEnable */
		bool GetDepthClipEnable() const { return mData.depthClipEnable; }

		/** @copydoc RASTERIZER_STATE_DESC::scissorEnable */
		bool GetScissorEnable() const { return mData.scissorEnable; }

		/** @copydoc RASTERIZER_STATE_DESC::multisampleEnable */
		bool GetMultisampleEnable() const { return mData.multisampleEnable; }

		/** @copydoc RASTERIZER_STATE_DESC::antialiasedLineEnable */
		bool GetAntialiasedLineEnable() const { return mData.antialiasedLineEnable; }

		/** Returns the hash value generated from the rasterizer state properties. */
		UINT64 GetHash() const { return mHash; }

	protected:
		friend class RasterizerState;
		friend class ct::RasterizerState;
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
		const RasterizerProperties& GetProperties() const;

		/** Retrieves a core implementation of the rasterizer state usable only from the core thread. */
		SPtr<ct::RasterizerState> GetCore() const;

		/** Creates a new rasterizer state using the specified rasterizer state descriptor structure. */
		static SPtr<RasterizerState> Create(const RASTERIZER_STATE_DESC& desc);

		/**	Returns the default rasterizer state. */
		static const SPtr<RasterizerState>& GetDefault();

		/**	Generates a hash value from a rasterizer state descriptor. */
		static UINT64 GenerateHash(const RASTERIZER_STATE_DESC& desc);

	protected:
		friend class RenderStateManager;

		RasterizerState(const RASTERIZER_STATE_DESC& desc);
		
		/** @copydoc CoreObject::createCore */
		SPtr<ct::CoreObject> CreateCore() const override;

		RasterizerProperties mProperties;
		mutable UINT32 mId;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class RasterizerStateRTTI;
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
	 * Core thread version of bs::RasterizerState.
	 *
	 * @note	Core thread.
	 */
	class BS_CORE_EXPORT RasterizerState : public CoreObject
	{
	public:
		virtual ~RasterizerState();

		/** Returns information about the rasterizer state. */
		const RasterizerProperties& GetProperties() const;

		/**	Returns a unique state ID. Only the lowest 10 bits are used. */
		UINT32 GetId() const { return mId; }

		/** Creates a new rasterizer state using the specified rasterizer state descriptor structure. */
		static SPtr<RasterizerState> Create(const RASTERIZER_STATE_DESC& desc);

		/** Returns the default rasterizer state. */
		static const SPtr<RasterizerState>& GetDefault();

	protected:
		friend class RenderStateManager;

		RasterizerState(const RASTERIZER_STATE_DESC& desc, UINT32 id);

		/** @copydoc CoreObject::initialize */
		void Initialize() override;

		/**	Creates any API-specific state objects. */
		virtual void CreateInternal() { }

		RasterizerProperties mProperties;
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
/**	Hash value generator for RASTERIZER_STATE_DESC. */
template<>
struct hash<bs::RASTERIZER_STATE_DESC>
{
	size_t operator()(const bs::RASTERIZER_STATE_DESC& value) const
	{
		return (size_t)bs::RasterizerState::generateHash(value);
	}
};
}

/** @} */
/** @endcond */
