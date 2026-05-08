//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Script/B3DIScriptExportable.h"
#include "Image/B3DPixelUtility.h"
#include "GpuBackend/B3DViewport.h"
#include "CoreObject/B3DCoreObject.h"
#include "Threading/B3DAsyncOp.h"
#include "Utility/B3DEvent.h"

namespace b3d
{
	/** @addtogroup GpuBackend
	 *  @{
	 */

	/** Structure that contains information about what part of the texture represents the render surface. */
	struct B3D_EXPORT RenderSurfaceInformation
	{
		RenderSurfaceInformation() = default;

		HTexture Texture;

		/** First face of the texture to bind (array index in texture arrays, or Z slice in 3D textures). */
		u32 Face = 0;

		/**
		 * Number of faces to bind (entries in a texture array, or Z slices in 3D textures). When zero the entire resource
		 * will be bound.
		 */
		u32 FaceCount = 0;

		/** If the texture has multiple mips, which one to bind (only one can be bound for rendering). */
		u32 MipLevel = 0;
	};

	namespace render
	{
		/** @copydoc b3d::RenderSurfaceInformation */
		struct B3D_EXPORT RenderSurfaceInformation
		{
			RenderSurfaceInformation() = default;

			TShared<Texture> Texture;

			/** First face of the texture to bind (array index in texture arrays, or Z slice in 3D textures). */
			u32 Face = 0;

			/**
			 * Number of faces to bind (entries in a texture array, or Z slices in 3D textures). When zero the entire resource
			 * will be bound.
			 */
			u32 FaceCount = 0;

			/** If the texture has multiple mips, which one to bind (only one can be bound for rendering). */
			u32 MipLevel = 0;
		};
	} // namespace render

	/** Contains various properties that describe a render target. */
	class B3D_EXPORT RenderTargetProperties
	{
	public:
		virtual ~RenderTargetProperties() = default;

		/** Width of the render target, in pixels. */
		u32 Width = 0;

		/** Height of the render target, in pixels. */
		u32 Height = 0;

		/**
		 * Controls in what order is the render target rendered to compared to other render targets. Targets with higher
		 * priority will be rendered before ones with lower priority.
		 */
		i32 Priority = 0;

		/**
		 * Returns currently set DPI scale. Scale of 1.0 corresponds to 96 DPI.
		 *  physical pixel = logical pixel * DPI scale
		 *  logical pixel = physical pixel / DPI scale;
		 */
		float DPIScale = 1.0f;

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

	/** Render target is a frame buffer or a texture that the render system renders the scene to. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) RenderTarget : public IReflectable, public IScriptExportable, public CoreObject
	{
	public:
		RenderTarget();
		virtual ~RenderTarget() = default;

		/**
		 * @copydoc render::RenderTarget::SetPriority
		 *
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		void SetPriority(i32 priority);

		/** Returns properties that describe the render target. */
		const RenderTargetProperties& GetProperties() const { return mRenderTargetProperties; }

		/** Event that gets triggered whenever the render target is resized. */
		mutable Event<void()> OnResized;

		/** Triggered when the DPI scale changes. */
		mutable Event<void()> OnDPIScaleChanged;

	protected:
		friend class render::RenderTarget;

		RenderTargetProperties mRenderTargetProperties;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class RenderTargetRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */

	namespace render
	{
		/** @addtogroup GpuBackend
		 *  @{
		 */

		/**
		 * Provides access to internal render target implementation usable only from the render thread.
		 *
		 * @note	Render thread only.
		 */
		class B3D_EXPORT RenderTarget : public RenderProxy
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

			/**	Returns properties that describe the render target. */
			const RenderTargetProperties& GetProperties() const { return mRenderTargetProperties; }

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

			/**
			 * Reads the contents of this render target. Issues a copy command into the command buffer
			 * and returns an async operation that triggers when the data has been read.
			 *
			 * @param	commandBuffer		Command buffer to issue copy commands into.
			 * @param	colorSurfaceIndex	Which color surface to read (default 0).
			 * @param	mipLevel			Mip level to read (default 0).
			 * @param	arrayLayer			Array layer to read (default 0).
			 * @return						Async operation that triggers when the read operation is complete. May retun null PixelData if reading is not supported.
			 */
			virtual TAsyncOp<TShared<PixelData>> ReadAsync(GpuCommandBuffer& commandBuffer, u32 colorSurfaceIndex = 0, u32 mipLevel = 0, u32 arrayLayer = 0);

			/** @} */
		protected:
			friend class b3d::RenderTarget;

			u64 mUpdateCount = 0;
			RenderTargetProperties mRenderTargetProperties;
		};

		/** @} */
	} // namespace render
} // namespace b3d
