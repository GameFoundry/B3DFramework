//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsPixelUtil.h"
#include "BsViewport.h"
#include "BsCoreObject.h"
#include "BsEvent.h"

namespace BansheeEngine 
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/** Structure that contains information about what part of the texture represents the render surface. */
	struct BS_CORE_EXPORT RENDER_SURFACE_DESC
	{
		RENDER_SURFACE_DESC() { }

		HTexture texture;

		/** First face of the texture to bind (array index in texture arrays, or Z slice in 3D textures). */
		UINT32 face = 0; 

		/**
		 * Number of faces to bind (entries in a texture array, or Z slices in 3D textures). When zero the entire resource
		 * will be bound. 
		 */
		UINT32 numFaces = 0;

		/** If the texture has multiple mips, which one to bind (only one can be bound for rendering). */
		UINT32 mipLevel = 0; 
	};

	/**
	 * @see		RENDER_SURFACE_DESC
	 *
	 * @note	References core textures instead of texture handles.
	 */
	struct BS_CORE_EXPORT RENDER_SURFACE_DESC_CORE
	{
		RENDER_SURFACE_DESC_CORE() { }

		SPtr<TextureCore> texture;

		/** First face of the texture to bind (array index in texture arrays, or Z slice in 3D textures). */
		UINT32 face = 0; 

		/**
		 * Number of faces to bind (entries in a texture array, or Z slices in 3D textures). When zero the entire resource
		 * will be bound. 
		 */
		UINT32 numFaces = 0;

		/** If the texture has multiple mips, which one to bind (only one can be bound for rendering). */
		UINT32 mipLevel = 0; 
	};

	/** Contains various properties that describe a render target. */
	class BS_CORE_EXPORT RenderTargetProperties
	{
	public:
		RenderTargetProperties() { }
		virtual ~RenderTargetProperties() { }

		/**
		 * Returns width of the render target, in pixels.
		 * 
		 * @note	Sim thread only.
		 */
        UINT32 getWidth() const { return mWidth; }

		/**
		 * Returns height of the render target, in pixels.
		 *
		 * @note	Sim thread only.
		 */
        UINT32 getHeight() const { return mHeight; }

		/** Gets the number of samples used for multisampling. (0 or 1 if multisampling is not used). */
		UINT32 getMultisampleCount() const { return mMultisampleCount; }

		/**
		 * Returns true if the render target will wait for vertical sync before swapping buffers. This will eliminate 
		 * tearing but may increase input latency.
		 */
		bool getVSync() const { return mVSync; }

		/**
		 * Returns how often should the frame be presented in respect to display device refresh rate. Normal value is 1 
		 * where it will match the refresh rate. Higher values will decrease the frame rate (for example present interval of
		 * 2 on 60Hz refresh rate will display at most 30 frames per second).
		 */
		UINT32 getVSyncInterval() const { return mVSyncInterval; }

		/** Returns true if pixels written to the render target will be gamma corrected. */
		bool isHwGammaEnabled() const { return mHwGamma; }

		/**
		 * Returns true if the render target can be used for rendering.
		 *
		 * @note	Core thread only.
		 */
		bool isActive() const { return mActive; }

		/**
		 * Controls in what order is the render target rendered to compared to other render targets. Targets with higher 
		 * priority will be rendered before ones with lower priority.
		 */
		INT32 getPriority() const { return mPriority; }

		/** Returns true if the render target is a render window. */
		bool isWindow() const { return mIsWindow; }

		/**
		 * Does the texture need to be vertically flipped because of different screen space coordinate systems.	(Determines
		 * is origin top left or bottom left. Engine default is top left.)
		 */
		bool requiresTextureFlipping() const { return mRequiresTextureFlipping; }

	protected:
		friend class RenderTargetCore;
		friend class RenderTarget;

		UINT32 mWidth = 0;
		UINT32 mHeight = 0;
		UINT32 mColorDepth = 32;

		INT32 mPriority = 0;
		UINT32 mVSyncInterval = 1;

		bool mActive = true;
		bool mHwGamma = false;
		bool mVSync = false;
		bool mRequiresTextureFlipping = false;
		bool mIsWindow = false;

		UINT32 mMultisampleCount = 0;
	};

	/**
	 * Render target is a frame buffer or a texture that the render system renders the scene to.
	 *
	 * @note	
	 * Sim thread unless noted otherwise. Retrieve core implementation from getCore() for core thread only functionality.
	 */
    class BS_CORE_EXPORT RenderTarget : public CoreObject
    {
    public:
		RenderTarget();
		virtual ~RenderTarget() { }

		/** Queries the render target for a custom attribute. This may be anything and is implementation specific. */
		virtual void getCustomAttribute(const String& name, void* pData) const;

		/** 
		 * @copydoc RenderTargetCore::setPriority 
		 *
		 * @param[in]	accessor	Accessor on which will this command be queued for execution.
		 */
		void setPriority(CoreAccessor& accessor, INT32 priority);

		/**
		 * Returns properties that describe the render target.
		 *
		 * @note	Sim thread only.
		 */
		const RenderTargetProperties& getProperties() const;

		/** Retrieves a core implementation of a render target usable only from the core thread. */
		SPtr<RenderTargetCore> getCore() const;

		/**
		 * Event that gets triggered whenever the render target is resized.
		 *
		 * @note	Sim thread only.
		 */
		mutable Event<void()> onResized;

    protected:
		friend class RenderTargetCore;

		/**	Returns properties that describe the render target. */
		virtual const RenderTargetProperties& getPropertiesInternal() const = 0;
    };

	/** @} */

	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/**
	 * Provides access to internal render target implementation usable only from the core thread.
	 *
	 * @note	Core thread only.
	 */
	class BS_CORE_EXPORT RenderTargetCore : public CoreObjectCore
	{
	public:
		/** Frame buffer type when double-buffering is used. */
		enum FrameBuffer
		{
			FB_FRONT,
			FB_BACK,
			FB_AUTO
		};

		RenderTargetCore();
		virtual ~RenderTargetCore() { }

		/**
		 * Sets a priority that determines in which orders the render targets the processed.
		 * 			
		 * @param[in]	priority	The priority. Higher value means the target will be rendered sooner.
		 */
		void setPriority(INT32 priority);

		/** 
		 * Swaps the frame buffers to display the next frame. 
		 *
		 * @param[in]	syncMask	Optional synchronization mask that determines for which queues should the system wait
		 *							before performing the swap buffer operation. By default the system waits for all queues.
		 *							Howvever if certain queues are performing non-rendering operations, or operations not 
		 *							related to this render target, you can exclude them from the sync mask for potentially 
		 *							better performance. You can use CommandSyncMask to generate a valid sync mask.
		 */
		virtual void swapBuffers(UINT32 syncMask = 0xFFFFFFFF) {}

		/** Queries the render target for a custom attribute. This may be anything and is implementation specific. */
		virtual void getCustomAttribute(const String& name, void* pData) const;

		/**	Returns properties that describe the render target. */
		const RenderTargetProperties& getProperties() const;

	protected:
		friend class RenderTarget;

		/**	Returns properties that describe the render target. */
		virtual const RenderTargetProperties& getPropertiesInternal() const = 0;
	};

	/** @} */
}
