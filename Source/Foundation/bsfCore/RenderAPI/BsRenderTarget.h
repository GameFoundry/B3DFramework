//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Image/BsPixelUtil.h"
#include "RenderAPI/BsViewport.h"
#include "CoreThread/BsCoreObject.h"
#include "Utility/BsEvent.h"

namespace bs 
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

	namespace ct
	{
	/**
	 * @see		bs::RENDER_SURFACE_DESC
	 *
	 * @note	References core textures instead of texture handles.
	 */
	struct BS_CORE_EXPORT RENDER_SURFACE_DESC
	{
		RENDER_SURFACE_DESC() { }

		SPtr<Texture> texture;

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
	}

	/** Contains various properties that describe a render target. */
	class BS_CORE_EXPORT RenderTargetProperties
	{
	public:
		virtual ~RenderTargetProperties() = default;

		/** Width of the render target, in pixels. */
		UINT32 width = 0;

		/** Height of the render target, in pixels. */
		UINT32 height = 0;

		/** 
		 * Number of three dimensional slices of the render target. This will be number of layers for array
		 * textures or number of faces cube textures.
		 */
		UINT32 numSlices = 0;

		/**
		 * Controls in what order is the render target rendered to compared to other render targets. Targets with higher 
		 * priority will be rendered before ones with lower priority.
		 */
		INT32 priority = 0;

		/**
		 * True if the render target will wait for vertical sync before swapping buffers. This will eliminate 
		 * tearing but may increase input latency.
		 */
		bool vsync = false;

		/**
		 * Controls how often should the frame be presented in respect to display device refresh rate. Normal value is 1 
		 * where it will match the refresh rate. Higher values will decrease the frame rate (for example present interval of
		 * 2 on 60Hz refresh rate will display at most 30 frames per second).
		 */
		UINT32 vsyncInterval = 1;

		/** True if pixels written to the render target will be gamma corrected. */
		bool hwGamma = false;

		/**
		 * Does the texture need to be vertically flipped because of different screen space coordinate systems.	(Determines
		 * is origin top left or bottom left. Engine default is top left.)
		 */
		bool requiresTextureFlipping = false;

		/** True if the target is a window, false if an offscreen target. */
		bool isWindow = false;

		/** Controls how many samples are used for multisampling. (0 or 1 if multisampling is not used). */
		UINT32 multisampleCount = 0;
	};

	/**
	 * Render target is a frame buffer or a texture that the render system renders the scene to.
	 *
	 * @note	
	 * Sim thread unless noted otherwise. Retrieve core implementation from getCore() for core thread only functionality.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Rendering) RenderTarget : public CoreObject
	{
	public:
		RenderTarget();
		virtual ~RenderTarget() { }

		/** Queries the render target for a custom attribute. This may be anything and is implementation specific. */
		virtual void getCustomAttribute(const String& name, void* pData) const;

		/** 
		 * @copydoc ct::RenderTarget::setPriority 
		 *
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		void setPriority(INT32 priority);

		/**
		 * Returns properties that describe the render target.
		 *
		 * @note	Sim thread only.
		 */
		const RenderTargetProperties& getProperties() const;

		/** Retrieves a core implementation of a render target usable only from the core thread. */
		SPtr<ct::RenderTarget> getCore() const;

		/**
		 * Event that gets triggered whenever the render target is resized.
		 *
		 * @note	Sim thread only.
		 */
		mutable Event<void()> onResized;

	protected:
		friend class ct::RenderTarget;

		/**	Returns properties that describe the render target. */
		virtual const RenderTargetProperties& getPropertiesInternal() const = 0;
	};

	/** @} */

	namespace ct
	{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/**
	 * Provides access to internal render target implementation usable only from the core thread.
	 *
	 * @note	Core thread only.
	 */
	class BS_CORE_EXPORT RenderTarget : public CoreObject
	{
	public:
		/** Frame buffer type when double-buffering is used. */
		enum FrameBuffer
		{
			FB_FRONT,
			FB_BACK,
			FB_AUTO
		};

		RenderTarget();
		virtual ~RenderTarget() { }

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
		 *							However if certain queues are performing non-rendering operations, or operations not 
		 *							related to this render target, you can exclude them from the sync mask for potentially 
		 *							better performance. You can use CommandSyncMask to generate a valid sync mask.
		 */
		virtual void swapBuffers(UINT32 syncMask = 0xFFFFFFFF) {}

		/** Queries the render target for a custom attribute. This may be anything and is implementation specific. */
		virtual void getCustomAttribute(const String& name, void* pData) const;

		/**	Returns properties that describe the render target. */
		const RenderTargetProperties& getProperties() const;

	protected:
		friend class bs::RenderTarget;

		/**	Returns properties that describe the render target. */
		virtual const RenderTargetProperties& getPropertiesInternal() const = 0;
	};

	/** @} */
	}
}
