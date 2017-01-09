//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsIReflectable.h"
#include "BsCoreObject.h"
#include "BsColor.h"
#include "BsRect2I.h"
#include "BsRect2.h"
#include "BsEvent.h"

namespace bs 
{
	/** @addtogroup Implementation
	 *  @{
	 */

	/**
	 * Viewport provides you with a way to render to only a part of a RenderTarget. It also allows you to set up 
	 * color/depth/stencil clear values for that specific region.
	 */
	class BS_CORE_EXPORT ViewportBase
    {
    public:       
		virtual ~ViewportBase() { }

        /**	Gets the normalized x coordinate of the viewport, in [0, 1] range. */
        float getNormalizedX() const { return mNormArea.x; }

        /**	Gets the normalized y coordinate of the viewport, in [0, 1] range. */
        float getNormalizedY() const { return mNormArea.y; }

        /**	Gets the normalized width of the viewport, in [0, 1] range. */
        float getNormalizedWidth() const { return mNormArea.width; }

		/**	Gets the normalized height of the viewport, in [0, 1] range.  */
        float getNormalizedHeight() const { return mNormArea.height; }

        /**	Gets the actual x coordinate of the viewport in pixels, in [0, RenderTargetWidth] range. */
		INT32 getX() const;

        /**	Gets the actual y coordinate of the viewport in pixels, in [0, RenderTargetHeight] range. */
		INT32 getY() const;

		/**	Gets the actual width coordinate of the viewport in pixels, in [0, RenderTargetWidth] range. */
		INT32 getWidth() const;

		/**	Gets the actual height coordinate of the viewport in pixels, in [0, RenderTargetHeight] range. */
		INT32 getHeight() const;
               
        /**
         * Changes the area that the viewport covers.
         *
         * @note	Viewport coordinates are normalized in [0, 1] range.
         */
        void setArea(float x, float y, float width, float height);

		/**	Returns actual area of the viewport, in pixels. */
		Rect2I getArea() const;

		/**
		 * Returns the normalized area of the viewport.
		 *
		 * @note	Viewport coordinates are normalized in [0, 1] range.
		 */
		Rect2 getNormArea() const { return mNormArea; }

		/**
		 * Activates or deactivates clears for color, depth or stencil buffers. Buffers will be cleared before rendering 
		 * to this viewport is performed.
		 */
		void setRequiresClear(bool colorClear, bool depthClear, bool stencilClear);

		/**	Sets values to clear color, depth and stencil buffers to. */
		void setClearValues(const Color& clearColor, float clearDepth = 0.0f, UINT16 clearStencil = 0);

		/** Returns the color to clear the viewport color buffers to. */
		const Color& getClearColor() const { return mClearColor; }

		/**	Returns the value to clear the viewport depth buffers to. */
		float getClearDepthValue() const { return mDepthClearValue; }

		/**	Returns the value to clear the viewport stencil buffer to. */
		UINT16 getClearStencilValue() const { return mStencilClearValue; }

		/**	Returns true if viewport requires color clear before rendering. */
		bool getRequiresColorClear() const { return mRequiresColorClear; }

		/**	Returns true if viewport requires depth clear before rendering. */
		bool getRequiresDepthClear() const { return mRequiresDepthClear; }

		/**	Returns true if viewport requires stencil clear before rendering. */
		bool getRequiresStencilClear() const { return mRequiresStencilClear; }

    protected:
		ViewportBase(float x = 0.0f, float y = 0.0f, float width = 1.0f, float height = 1.0f);

		/** 
		 * Marks the core data as dirty. This causes the data from the sim thread object be synced with the core thread
		 * version of the object.
		 */
		virtual void _markCoreDirty() { }

		/** Gets the render target width. */
		virtual UINT32 getTargetWidth() const = 0;

		/**	Gets the render target width. */
		virtual UINT32 getTargetHeight() const = 0;

		Rect2 mNormArea;

		bool mRequiresColorClear;
		bool mRequiresDepthClear;
		bool mRequiresStencilClear;

		Color mClearColor;
		float mDepthClearValue;
		UINT16 mStencilClearValue;

		static const Color DEFAULT_CLEAR_COLOR;
    };

	/** @} */

	/** @addtogroup RenderAPI
	 *  @{
	 */

	/** @copydoc ViewportBase */
	class BS_CORE_EXPORT Viewport : public IReflectable, public CoreObject, public ViewportBase
    {
    public:       
        /**	Returns the render target the viewport is associated with. */
        SPtr<RenderTarget> getTarget() const { return mTarget; }

		/**	Sets the render target the viewport will be associated with. */
		void setTarget(const SPtr<RenderTarget>& target);

		/**	Retrieves a core implementation of a viewport usable only from the core thread. */
		SPtr<ct::ViewportCore> getCore() const;

	    /**
         * Creates a new viewport.
         *
         * @note	Viewport coordinates are normalized in [0, 1] range.
         */	
		static SPtr<Viewport> create(const SPtr<RenderTarget>& target, float x = 0.0f, float y = 0.0f, 
			float width = 1.0f, float height = 1.0f);

    protected:
        Viewport(const SPtr<RenderTarget>& target, float x = 0.0f, float y = 0.0f, float width = 1.0f, float height = 1.0f);

		/** @copydoc ViewportBase::_markCoreDirty */
		void _markCoreDirty() override;

		/** @copydoc ViewportBase::getTargetWidth */
		UINT32 getTargetWidth() const override;

		/** @copydoc ViewportBase::getTargetHeight */
		UINT32 getTargetHeight() const override;

		/** @copydoc CoreObject::syncToCore */
		CoreSyncData syncToCore(FrameAlloc* allocator) override;

		/** @copydoc CoreObject::getCoreDependencies */
		void getCoreDependencies(Vector<CoreObject*>& dependencies) override;

		/** @copydoc CoreObject::createCore */
		SPtr<ct::CoreObject> createCore() const override;

        SPtr<RenderTarget> mTarget;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
		Viewport();

		/** Creates an empty viewport for serialization purposes. */
		static SPtr<Viewport> createEmpty();
	public:
		friend class ViewportRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
    };

	/** @} */

	namespace ct
	{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/** @copydoc ViewportBase */
	class BS_CORE_EXPORT ViewportCore : public CoreObject, public ViewportBase
    {
    public:       
        /**	Returns the render target the viewport is associated with. */
		SPtr<RenderTargetCore> getTarget() const { return mTarget; }

		/**	Sets the render target the viewport will be associated with. */
		void setTarget(const SPtr<RenderTargetCore>& target) { mTarget = target; }

		/** @copydoc Viewport::create() */
		static SPtr<ViewportCore> create(const SPtr<RenderTargetCore>& target, float x = 0.0f, float y = 0.0f, 
			float width = 1.0f, float height = 1.0f);

    protected:
		friend class Viewport;

		ViewportCore(const SPtr<RenderTargetCore>& target, float x = 0.0f, float y = 0.0f, float width = 1.0f, float height = 1.0f);

		/** @copydoc ViewportBase::getTargetWidth */
		UINT32 getTargetWidth() const override;

		/** @copydoc ViewportBase::getTargetHeight */
		UINT32 getTargetHeight() const override;

		/** @copydoc CoreObject::syncToCore */
		void syncToCore(const CoreSyncData& data) override;

		SPtr<RenderTargetCore> mTarget;
    };

	/** @} */
		}
}