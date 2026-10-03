//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalVertexInputManager.h"
#include "B3DMetalUtility.h"
#include "Debug/B3DLog.h"

#include "GpuBackend/B3DGpuVertexInputManager.inl"

namespace b3d
{
	namespace render
	{
		MetalVertexInput::MetalVertexInput(u32 id, const GpuVertexInputLayout& layout)
			: mId(id), mStreamCount(layout.StreamCount), mUsedStreamMask(layout.UsedStreamMask), mNullStreamIndex(layout.NullStreamIndex)
		{
			MTLVertexDescriptor* descriptor = [[MTLVertexDescriptor alloc] init];

			for (const GpuVertexInputAttribute& attribute : layout.Attributes)
			{
				const u32 attributeIndex = attribute.ShaderInput->GetOffset();

				if (attribute.BufferElement != nullptr)
				{
					// Matched input: fetch from the bound vertex buffer's stream, using the buffer
					// element's type and byte offset within that stream.
					const VertexElement& bufferElement = *attribute.BufferElement;
					descriptor.attributes[attributeIndex].format = MetalUtility::GetVertexFormat(bufferElement.GetType());
					descriptor.attributes[attributeIndex].offset = bufferElement.GetOffset();
					descriptor.attributes[attributeIndex].bufferIndex = kMetalVertexBufferSlotBase + attribute.StreamIndex;
				}
				else
				{
					// Unmatched input: fetch zeroes from the null stream. The shader element's own type
					// determines the fetch format; the null stream is a constant-step zero buffer so any
					// element type reads all-zero data at offset 0.
					descriptor.attributes[attributeIndex].format = MetalUtility::GetVertexFormat(attribute.ShaderInput->GetType());
					descriptor.attributes[attributeIndex].offset = 0;
					descriptor.attributes[attributeIndex].bufferIndex = kMetalVertexBufferSlotBase + layout.NullStreamIndex;
				}
			}

			// Per-stream stride/step info for streams actually served by the bound vertex buffer. The
			// resolved layout records each stream's stride in StreamStrides. The null stream is not in the
			// used mask and gets its dedicated layout below.
			for (u32 streamIndex = 0; streamIndex < layout.StreamCount; streamIndex++)
			{
				if ((layout.UsedStreamMask & (1u << streamIndex)) == 0)
					continue;

				const u32 stride = layout.StreamStrides[streamIndex];
				if (stride == 0)
					continue;

				const u32 layoutIndex = kMetalVertexBufferSlotBase + streamIndex;
				descriptor.layouts[layoutIndex].stride = stride;
				descriptor.layouts[layoutIndex].stepFunction = MTLVertexStepFunctionPerVertex;
				descriptor.layouts[layoutIndex].stepRate = 1;
			}

			// Override with instance-step info from any matched buffer element that requests it.
			for (const GpuVertexInputAttribute& attribute : layout.Attributes)
			{
				if (attribute.BufferElement != nullptr && attribute.BufferElement->GetInstanceStepRate() > 0)
				{
					const u32 layoutIndex = kMetalVertexBufferSlotBase + attribute.StreamIndex;
					descriptor.layouts[layoutIndex].stepFunction = MTLVertexStepFunctionPerInstance;
					descriptor.layouts[layoutIndex].stepRate = attribute.BufferElement->GetInstanceStepRate();
				}
			}

			// Null stream layout: constant step function (Metal requires stepRate 0 with it) so every
			// vertex and instance reads the same zeroed bytes at offset 0, independent of vertex count.
			// The command buffer binds the device's shared zero-filled buffer at this slot.
			if (layout.NullStreamIndex != GpuVertexInputLayout::kNoNullStream)
			{
				const u32 layoutIndex = kMetalVertexBufferSlotBase + layout.NullStreamIndex;
				descriptor.layouts[layoutIndex].stride = kMetalNullVertexStreamStride;
				descriptor.layouts[layoutIndex].stepFunction = MTLVertexStepFunctionConstant;
				descriptor.layouts[layoutIndex].stepRate = 0;
			}

			mVertexDescriptor = descriptor;
		}

		MetalVertexInputManager::~MetalVertexInputManager()
		{
			ReleaseAll();
		}

		TShared<MetalVertexInput> MetalVertexInputManager::CreateVertexInput(const GpuVertexInputLayout& layout)
		{
			// Validate that the resolved layout is expressible on Metal before allocating anything.
			if (kMetalVertexBufferSlotBase + layout.StreamCount > kMetalVertexBufferSlotEnd)
			{
				B3D_LOG(Error, LogRenderBackend, "Vertex input requires {0} streams, exceeding the {1} vertex-stage buffer slots available on Metal.",
					layout.StreamCount, kMetalVertexBufferSlotEnd - kMetalVertexBufferSlotBase);
				return nullptr;
			}

			for (const GpuVertexInputAttribute& attribute : layout.Attributes)
			{
				const VertexElementType elementType = attribute.BufferElement != nullptr
					? attribute.BufferElement->GetType()
					: attribute.ShaderInput->GetType();
				if (MetalUtility::GetVertexFormat(elementType) == MTLVertexFormatInvalid)
				{
					B3D_LOG(Error, LogRenderBackend, "Unsupported vertex element type {0} on the Metal backend.", (u32)elementType);
					return nullptr;
				}

				if (attribute.ShaderInput->GetOffset() > kMetalMaxVertexAttributeIndex)
				{
					B3D_LOG(Error, LogRenderBackend, "Vertex shader input location {0} exceeds Metal's maximum attribute index {1}.",
						attribute.ShaderInput->GetOffset(), kMetalMaxVertexAttributeIndex);
					return nullptr;
				}
			}

			return B3DMakeShared<MetalVertexInput>(mNextId++, layout);
		}

		void MetalVertexInputManager::DestroyVertexInput(TShared<MetalVertexInput>& vertexInput)
		{
			vertexInput = nullptr;
		}

		template class TGpuVertexInputManager<MetalVertexInputManager, TShared<MetalVertexInput>>;
	} // namespace render
} // namespace b3d
