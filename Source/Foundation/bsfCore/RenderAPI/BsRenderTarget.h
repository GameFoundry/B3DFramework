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
		RENDER_SURFACE_DESC() {}

		HTexture Texture;

		/** First face of the texture to bind (array index in texture arrays, or Z slice in 3D textures). */
		u32 Face = 0;

		/**
		 * Number of faces to bind (entries in a texture array, or Z slices in 3D textures). When zero the entire resource
		 * will be bound.
		 */
		u32 NumFaces = 0;

		/** If the texture has multiple mips, which one to bind (only one can be bound for rendering). */
		u32 MipLevel = 0;
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
			RENDER_SURFACE_DESC() {}

			SPtr<Texture> Texture;

			/** First face of the texture to bind (array index in texture arrays, or Z slice in 3D textures). */
			u32 Face = 0;

			/**
			 * Number of faces to bind (entries in a texture array, or Z slices in 3D textures). When zero the entire resource
			 * will be bound.
			 */
			u32 NumFaces = 0;

			/** If the texture has multiple mips, which one to bind (only one can be bound for rendering). */
			u32 MipLevel = 0;
		};
	} // namespace ct

	/** Contains various properties that describe a render target. */
	class BS_CORE_EXPORT RenderTargetProperties
	{
	public:
		virtual ~RenderTargetProperties() = default;

		/** Width of the render target, in pixels. */
		u32 Width = 0;

		/** Height of the render target, in pixels. */
		u32 Height = 0;

		/**
		 * Number of three dimensional slices of the render target. This will be number of layers for array
		 * textures or number of faces cube textures.
		 */
		u32 NumSlices = 0;

		/**
		 * Controls in what order is the render target rendered to compared to other render targets. Targets with higher
		 * priority will be rendered before ones with lower priority.
		 */
		i32 Priority = 0;

		/**
		 * True if the render target will wait for vertical sync before swapping buffers. This will eliminate
		 * tearing but may increase input latency.
		 */
		bool Vsync = false;

		/**
		 * Controls how often should the frame be presented in respect to display device refresh rate. Normal value is 1
		 * where it will match the refresh rate. Higher values will decrease the frame rate (for example present interval of
		 * 2 on 60Hz refresh rate will display at most 30 frames per second).
		 */
		u32 VsyncInterval = 1;

		/** True if pixels written to the render target will be gamma corrected. */
		bool HwGamma = false;

		/**
		 * Does the texture need to be vertically flipped because of different screen space coordinate systems.	(Determines
		 * is origin top left or bottom left. Engine default is top left.)
		 */
		bool RequiresTextureFlipping = false;

		/** True if the target is a window, false if an offscreen target. */
		bool IsWindow = false;

		/** Controls how many samples are used for multisampling. (0 or 1 if multisampling is not used). */
		u32 MultisampleCount = 0;
	};

	/**
	 * Render target is a frame buffer or a texture that the render system renders the scene to.
	 *
	 * @note
	 * Sim thread unless noted otherwise. Retrieve core implementation from getCore() for core thread only functionality.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(Rendering)) RenderTarget : public IReflectable, public CoreObject
	{
	public:
		RenderTarget();
		virtual ~RenderTarget() = default;

		/** Queries the render target for a custom attribute. This may be anything and is implementation specific. */
		virtual void GetCustomAttribute(const String& name, void* pData) const;

		/**
		 * @copydoc ct::RenderTarget::SetPriority
		 *
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		void SetPriority(i32 priority);

		/**
		 * Returns properties that describe the render target.
		 *
		 * @note	Sim thread only.
		 */
		const RenderTargetProperties& GetProperties() const;

		/** Retrieves a core implementation of a render target usable only from the core thread. */
		SPtr<ct::RenderTarget> GetCore() const;

		/**
		 * Event that gets triggered whenever the render target is resized.
		 *
		 * @note	Sim thread only.
		 */
		mutable Event<void()> OnResized;

	protected:
		friend class ct::RenderTarget;

		/**	Returns properties that describe the render target. */
		virtual const RenderTargetProperties& GetPropertiesInternal() const = 0;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class RenderTargetRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
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
			virtual ~RenderTarget() = default;

			/**
			 * Sets a priority that determines in which orders the render targets the processed.
			 *
			 * @param[in]	priority	The priority. Higher value means the target will be rendered sooner.
			 */
			void SetPriority(i32 priority);

			/**
			 * Swaps the frame buffers to display the next frame.
			 *
			 * @param[in]	syncMask	Optional synchronization mask that determines for which queues should the system wait
			 *							before performing the swap buffer operation. By default the system waits for all queues.
			 *							However if certain queues are performing non-rendering operations, or operations not
			 *							related to this render target, you can exclude them from the sync mask for potentially
			 *							better performance. You can use CommandSyncMask to generate a valid sync mask.
			 */
			virtual void SwapBuffers(u32 syncMask = 0xFFFFFFFF) {}

			/** Queries the render target for a custom attribute. This may be anything and is implementation specific. */
			virtual void GetCustomAttribute(const String& name, void* pData) const;

			/**	Returns properties that describe the render target. */
			const RenderTargetProperties& GetProperties() const;

			/**
			 * Returns a number that increments each time the target is rendered to. External systems can use this to
			 * determine when the target's contents changed.
			 */
			u64 GetUpdateCount() const { return mUpdateCount; }

			/**
			 * @name Internal
			 * @{
			 */

			/** Increments the update count, letting other code know that the contents of the render target changed. */
			void TickUpdateCountInternal() { mUpdateCount++; }

			/** @} */
		protected:
			friend class bs::RenderTarget;

			/**	Returns properties that describe the render target. */
			virtual const RenderTargetProperties& GetPropertiesInternal() const = 0;

		private:
			u64 mUpdateCount = 0;
		};

		/** @} */
	} // namespace ct
} // namespace bs
