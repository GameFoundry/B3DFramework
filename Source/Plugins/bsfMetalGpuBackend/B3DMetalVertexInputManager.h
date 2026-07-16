//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "GpuBackend/B3DGpuVertexInputManager.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup MetalGpuBackend
		 *  @{
		 */

		/**
		 * Base Metal buffer-table slot at which vertex-stream buffers are bound on the vertex stage.
		 *
		 * Parameter-set argument buffers occupy the low slots [0, setCount) of the vertex-stage buffer
		 * table, so vertex streams must live above them. The base is a fixed constant (rather than the
		 * per-pipeline parameter-set count) so a cached MTLVertexDescriptor remains valid across
		 * pipelines with differing set counts, and so the command buffer can compute a stream's slot
		 * without consulting the bound pipeline's layout. Both the vertex descriptor built by
		 * MetalVertexInput and the command buffer's setVertexBuffer calls offset stream indices by
		 * this value.
		 */
		constexpr u32 kMetalVertexBufferSlotBase = 16;

		/**
		 * One-past-the-highest vertex-stage buffer slot usable for vertex streams. Metal exposes 31
		 * vertex-stage buffer slots [0, 31); slot 30 is left free for SPIRV-Cross auxiliary buffers
		 * (for example the texture swizzle buffer), which default to the top of the table.
		 */
		constexpr u32 kMetalVertexBufferSlotEnd = 30;

		/** Largest attribute index programmable in a MTLVertexDescriptor (Metal exposes attributes [0, 31)). */
		constexpr u32 kMetalMaxVertexAttributeIndex = 30;

		/**
		 * Byte stride of the null vertex stream. Sized for the largest vertex element type (float4 /
		 * uint4 = 16 bytes) so any unmatched shader input can fetch a full zeroed element from offset
		 * zero. The device provides a shared zero-filled MTLBuffer of at least this size which the
		 * command buffer binds at the null stream's slot before drawing.
		 */
		constexpr u32 kMetalNullVertexStreamStride = 16;

		/**
		 * Contains data describing vertex inputs for a Metal graphics pipeline. Owns the
		 * MTLVertexDescriptor built by resolving a vertex-buffer layout against the vertex shader's
		 * declared inputs, with unmatched shader inputs redirected to a zero-filled null stream.
		 *
		 * Metal fuses the vertex descriptor into the compiled MTLRenderPipelineState, so the identifier
		 * returned by GetId() participates in the pipeline-variant cache key (see
		 * MetalPipelineVariantKey::VertexInputId).
		 */
		class MetalVertexInput
		{
		public:
			/**
			 * @param	id			Identifier which uniquely represents this vertex input configuration.
			 * @param	layout		Vertex buffer layout resolved against the vertex shader inputs.
			 */
			MetalVertexInput(u32 id, const GpuVertexInputLayout& layout);
			~MetalVertexInput();

			/** Returns an identifier which uniquely represents this vertex input configuration. */
			u32 GetId() const { return mId; }

			/** Returns the number of vertex-buffer streams referenced by the input, including the null stream when present. */
			u32 GetVertexBufferBindingCount() const { return mStreamCount; }

			/** Returns a bitmask of streams fetched by attributes with a matching vertex buffer element. */
			u32 GetUsedStreamMask() const { return mUsedStreamMask; }

			/**
			 * Returns the stream reserved for shader inputs with no matching vertex buffer element, or
			 * GpuVertexInputLayout::kNoNullStream when every shader input matched. When present, the
			 * command buffer must bind the device's zero-filled null vertex buffer at slot
			 * kMetalVertexBufferSlotBase + GetNullStreamIndex() before drawing.
			 */
			u32 GetNullStreamIndex() const { return mNullStreamIndex; }

			/** Returns true when the layout reserves a null stream for unmatched shader inputs. */
			bool HasNullStream() const { return mNullStreamIndex != GpuVertexInputLayout::kNoNullStream; }

#ifdef __OBJC__
			/**
			 * Returns the vertex descriptor for this input configuration. Remains valid for the lifetime
			 * of this object. Callers assign it to MTLRenderPipelineDescriptor.vertexDescriptor, which is
			 * a copying property, so the descriptor owned here is never mutated after construction.
			 */
			MTLVertexDescriptor* GetVertexDescriptor() const;
#endif

		private:
			struct Impl;

			u32 mId;
			u32 mStreamCount = 0;
			u32 mUsedStreamMask = 0;
			u32 mNullStreamIndex = GpuVertexInputLayout::kNoNullStream;
			TUnique<Impl> mImpl;
		};

		class MetalVertexInputManager;
		extern template class TGpuVertexInputManager<MetalVertexInputManager, TShared<MetalVertexInput>>;

		/**
		 * Maps vertex buffer structure and vertex shader inputs in order to create vertex input
		 * descriptions usable by Metal.
		 */
		class MetalVertexInputManager : public Module<MetalVertexInputManager>, public TGpuVertexInputManager<MetalVertexInputManager, TShared<MetalVertexInput>>
		{
		public:
			~MetalVertexInputManager();

		private:
			friend class TGpuVertexInputManager<MetalVertexInputManager, TShared<MetalVertexInput>>;

			/** Creates a vertex input object for the provided resolved layout. Returns null if the layout cannot be expressed on Metal. */
			TShared<MetalVertexInput> CreateVertexInput(const GpuVertexInputLayout& layout);

			/** Releases a vertex input object evicted from the cache. */
			void DestroyVertexInput(TShared<MetalVertexInput>& vertexInput);

			u32 mNextId = 1;
		};

		/** @} */
	} // namespace render
} // namespace b3d
