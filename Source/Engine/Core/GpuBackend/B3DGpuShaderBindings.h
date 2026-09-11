//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"
#include "Utility/B3DTArrayView.h"

namespace b3d
{
	namespace render
	{
		/** Shader accesses cached when descriptors are bound. Reuse before each draw or dispatch until bindings change. */
		class GpuShaderBindings
		{
		public:
			GpuShaderBindings() = default;

			/** Descriptor image range, layout and shader access. */
			struct ImageBinding
			{
				ImageBinding(IGpuImageResource* image, const GpuTextureSubresourceRange& range, GpuImageLayout layout, GpuResourceUseFlags usage, GpuAccessFlags access)
					: Image(image), Range(range), Layout(layout), Usage(usage), Access(access) { }

				IGpuImageResource* Image;
				GpuTextureSubresourceRange Range;
				GpuImageLayout Layout;
				GpuResourceUseFlags Usage;
				GpuAccessFlags Access;
			};

			/** Descriptor buffer access and effective byte offset. */
			struct BufferBinding
			{
				BufferBinding(IGpuBufferResource* buffer, GpuResourceUseFlags usage, GpuAccessFlags access, u32 offset)
					: Buffer(buffer), Usage(usage), Access(access), Offset(offset) { }

				IGpuBufferResource* Buffer;
				GpuResourceUseFlags Usage;
				GpuAccessFlags Access;
				u32 Offset;
			};

			/** Drops bindings while retaining allocated storage. */
			void Clear()
			{
				Images.clear();
				Buffers.clear();
				mImageIndicesWithWriteAccess.clear();
				mBufferIndicesWithWriteAccess.clear();
				Changed = true;
			}

			/** Appends an image descriptor access. TrackShaderAndAttachmentAccesses finalizes the complete set. */
			void AddImage(IGpuImageResource* image, const GpuTextureSubresourceRange& range, GpuImageLayout layout, GpuResourceUseFlags usage, GpuAccessFlags access)
			{
				Images.emplace_back(image, range, layout, usage, access);
				Changed = true;
			}

			/** Appends a buffer descriptor access. TrackShaderAndAttachmentAccesses finalizes the complete set. */
			void AddBuffer(IGpuBufferResource* buffer, GpuResourceUseFlags usage, GpuAccessFlags access, u32 offset = 0)
			{
				Buffers.emplace_back(buffer, usage, access, offset);
				Changed = true;
			}

			/** Builds the recurring accesses: writes and reads aliasing a writable resource. Paid once per binding change. */
			void Finalize()
			{
				mImageIndicesWithWriteAccess.clear();
				mBufferIndicesWithWriteAccess.clear();
				for(u32 imageIndex = 0; imageIndex < (u32)Images.size(); imageIndex++)
					if(Images[imageIndex].Access.IsSet(GpuAccessFlag::Write))
						mImageIndicesWithWriteAccess.push_back(imageIndex);

				const u32 imageWriteCount = (u32)mImageIndicesWithWriteAccess.size();
				for(u32 imageIndex = 0; imageWriteCount != 0 && imageIndex < (u32)Images.size(); imageIndex++)
				{
					if(Images[imageIndex].Access.IsSet(GpuAccessFlag::Write))
						continue;

					for(u32 writeIndex = 0; writeIndex < imageWriteCount; writeIndex++)
					{
						if(Images[mImageIndicesWithWriteAccess[writeIndex]].Image == Images[imageIndex].Image)
						{
							mImageIndicesWithWriteAccess.push_back(imageIndex);
							break;
						}
					}
				}

				for(u32 bufferIndex = 0; bufferIndex < (u32)Buffers.size(); bufferIndex++)
					if(Buffers[bufferIndex].Access.IsSet(GpuAccessFlag::Write))
						mBufferIndicesWithWriteAccess.push_back(bufferIndex);

				const u32 bufferWriteCount = (u32)mBufferIndicesWithWriteAccess.size();
				for(u32 bufferIndex = 0; bufferWriteCount != 0 && bufferIndex < (u32)Buffers.size(); bufferIndex++)
				{
					if(Buffers[bufferIndex].Access.IsSet(GpuAccessFlag::Write))
						continue;

					for(u32 writeIndex = 0; writeIndex < bufferWriteCount; writeIndex++)
					{
						if(Buffers[mBufferIndicesWithWriteAccess[writeIndex]].Buffer == Buffers[bufferIndex].Buffer)
						{
							mBufferIndicesWithWriteAccess.push_back(bufferIndex);
							break;
						}
					}
				}
			}

			/** Indices into Images, including read-only bindings to images written through another binding. Valid after Finalize until bindings change. */
			TArrayView<const u32> GetImageIndicesWithWriteAccess() const { return mImageIndicesWithWriteAccess; }

			/** Indices into Buffers, including read-only bindings to buffers written through another binding. Valid after Finalize until bindings change. */
			TArrayView<const u32> GetBufferIndicesWithWriteAccess() const { return mBufferIndicesWithWriteAccess; }

			Vector<ImageBinding> Images; /**< Image accesses collected from the bound parameter sets. */
			Vector<BufferBinding> Buffers; /**< Buffer accesses collected from the bound parameter sets. */
			bool Changed = true; /**< Set when bindings change; cleared by successful TrackShaderAndAttachmentAccesses. */

		private:
			Vector<u32> mImageIndicesWithWriteAccess;
			Vector<u32> mBufferIndicesWithWriteAccess;
		};
	}
}
