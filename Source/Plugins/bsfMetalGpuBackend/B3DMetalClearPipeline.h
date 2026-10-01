//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "Utility/B3DUtil.h"

namespace b3d
{
	namespace render
	{
		class MetalGpuDevice;

		/** @addtogroup MetalGpuBackend
		 *  @{
		 */

		/**
		 * Fragment-stage buffer slot the clear shader reads its parameters from. Parameter set argument buffers occupy the
		 * low slots, so the clear parameters use one at the top of the table.
		 */
		constexpr u32 kMetalClearParametersBufferSlot = 30;

		/**
		 * Builds the pipeline and depth-stencil states used by MetalGpuCommandBuffer::ClearViewport().
		 *
		 * Metal can only clear whole attachments through @c MTLLoadActionClear, so clearing a sub-region is done by drawing
		 * a full-screen triangle, limited to the region by the viewport and scissor. Color and depth are output by the
		 * fragment shader, while stencil is written by the depth-stencil state using the encoder's stencil reference value.
		 *
		 * @note	Thread safe.
		 */
		class MetalClearPipeline
		{
		public:
			/** Parameters the clear shader reads. Must match the @c B3DClearParameters struct in the shader source. */
			struct Parameters
			{
				/** Color written to each color attachment. Ignored for attachments not set in Key::ColorWriteMask. */
				float Color[B3D_MAXIMUM_RENDER_TARGET_COUNT][4] = {};
				float Depth = 0.0f; /**< Depth written, if Key::WritesDepth is set. */
				float Padding[3] = { 0.0f, 0.0f, 0.0f }; /**< Pads the struct to the size of its shader counterpart. */
			};

			/** Identifies one clear pipeline state. Metal pipeline states are specific to the attachment formats they render to. */
			struct Key
			{
				u16 ColorFormats[B3D_MAXIMUM_RENDER_TARGET_COUNT] = {}; /**< MTLPixelFormat per color attachment, or 0 if absent. */
				u32 DepthFormat = 0; /**< MTLPixelFormat of the depth attachment, or 0 if absent. */
				u32 StencilFormat = 0; /**< MTLPixelFormat of the stencil attachment, or 0 if absent. */
				u16 SampleCount = 1; /**< Sample count of the attachments. */
				u8 ColorWriteMask = 0; /**< Bit per color attachment, set if the clear writes it. */
				bool WritesDepth = false; /**< True if the clear writes depth. Requires a depth attachment. */

				bool operator==(const Key& rhs) const
				{
					for (u32 attachmentIndex = 0; attachmentIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; attachmentIndex++)
					{
						if (ColorFormats[attachmentIndex] != rhs.ColorFormats[attachmentIndex])
							return false;
					}

					return DepthFormat == rhs.DepthFormat
						&& StencilFormat == rhs.StencilFormat
						&& SampleCount == rhs.SampleCount
						&& ColorWriteMask == rhs.ColorWriteMask
						&& WritesDepth == rhs.WritesDepth;
				}
			};

			/** Hashes a Key. */
			struct KeyHash
			{
				size_t operator()(const Key& key) const
				{
					size_t hash = 0;
					for (u32 attachmentIndex = 0; attachmentIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; attachmentIndex++)
						B3DCombineHash(hash, key.ColorFormats[attachmentIndex]);

					B3DCombineHash(hash, key.DepthFormat);
					B3DCombineHash(hash, key.StencilFormat);
					B3DCombineHash(hash, key.SampleCount);
					B3DCombineHash(hash, key.ColorWriteMask);
					B3DCombineHash(hash, key.WritesDepth);
					return hash;
				}
			};

			explicit MetalClearPipeline(MetalGpuDevice& gpuDevice);
			~MetalClearPipeline();

#ifdef __OBJC__
			/**
			 * Returns the pipeline state for @p key, creating it on first use. Returns nil if the pipeline state could not be
			 * created, and does not retry for the same key.
			 */
			id<MTLRenderPipelineState> GetOrCreatePipelineState(const Key& key);

			/**
			 * Returns a depth-stencil state that unconditionally writes the depth and/or stencil, creating it on first use.
			 * Stencil is written from the encoder's stencil reference value, which the caller must set to the clear value.
			 */
			id<MTLDepthStencilState> GetOrCreateDepthStencilState(bool writeDepth, bool writeStencil);
#endif

		private:
			/** Holds the Objective-C state, so plain C++ translation units can include this header. */
			struct Impl;

#ifdef __OBJC__
			/** Compiles the clear shader library on first use. Returns false if compilation failed, and does not retry. */
			bool EnsureLibrary();
#endif

			MetalGpuDevice& mGpuDevice;
			TUnique<Impl> mImpl;
		};

		/** @} */
	} // namespace render
} // namespace b3d
