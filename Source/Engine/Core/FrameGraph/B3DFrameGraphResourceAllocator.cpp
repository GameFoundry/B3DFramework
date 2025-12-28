//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "FrameGraph/B3DFrameGraphResourceAllocator.h"
#include "RenderAPI/B3DGpuDevice.h"
#include "Image/B3DTexture.h"
#include "Image/B3DPixelUtility.h"
#include "RenderAPI/B3DGpuBuffer.h"
#include "Debug/B3DDebug.h"

using namespace b3d;
using namespace b3d::render;

namespace b3d::render
{
	FrameGraphResourceAllocator::FrameGraphResourceAllocator(GpuDevice& device)
		: mDevice(device)
	{
	}

	FrameGraphResourceAllocator::~FrameGraphResourceAllocator()
	{
		B3D_LOG(Info, LogRenderBackend, "FrameGraphResourceAllocator destroyed. Final pool sizes: {0} textures, {1} buffers",
			mTexturePool.size(), mBufferPool.size());
	}

	SPtr<Texture> FrameGraphResourceAllocator::AllocateTexture(
		const StringView& name,
		const TextureCreateInformation& createInformation)
	{
		// Try to find a compatible free texture in the pool
		SPtr<Texture> texture = FindFreeTexture(createInformation);

		if (texture != nullptr)
		{
			// Found a reusable texture - mark it as in-use
			for (auto& entry : mTexturePool)
			{
				if (entry.TextureResource == texture)
				{
					B3D_ENSURE(!entry.InUse);
					entry.InUse = true;
					mStatisticsDirty = true;

					B3D_LOG(Info, LogRenderBackend, "Reused texture '{0}' from pool ({1}x{2} {3})",
						name, createInformation.Width, createInformation.Height,
						PixelUtility::GetFormatName(createInformation.Format));

					return texture;
				}
			}

			// Should never happen - FindFreeTexture returned a texture not in our pool
			B3D_ENSURE(false);
		}

		// No compatible texture found - create a new one
		texture = mDevice.CreateTexture(createInformation);
		B3D_ENSURE(texture != nullptr);

		// Add to pool
		TexturePoolEntry entry;
		entry.TextureResource = texture;
		entry.CreateInformation = createInformation;
		entry.InUse = true;
		entry.MemorySize = CalculateTextureMemorySize(createInformation);

		mTexturePool.push_back(entry);
		mStatisticsDirty = true;

		B3D_LOG(Info, LogRenderBackend, "Created new texture '{0}' ({1}x{2} {3}, {4} bytes)",
			name, createInformation.Width, createInformation.Height,
			PixelUtility::GetFormatName(createInformation.Format), entry.MemorySize);

		return texture;
	}

	SPtr<GpuBuffer> FrameGraphResourceAllocator::AllocateBuffer(
		const StringView& name,
		const GpuBufferCreateInformation& createInformation)
	{
		// Try to find a compatible free buffer in the pool
		SPtr<GpuBuffer> buffer = FindFreeBuffer(createInformation);

		if (buffer != nullptr)
		{
			// Found a reusable buffer - mark it as in-use
			for (auto& entry : mBufferPool)
			{
				if (entry.BufferResource == buffer)
				{
					B3D_ENSURE(!entry.InUse);
					entry.InUse = true;
					mStatisticsDirty = true;

					B3D_LOG(Info, LogRenderBackend, "Reused buffer '{0}' from pool (type: {1}, {2} bytes)",
						name, (u32)createInformation.Type, entry.MemorySize);

					return buffer;
				}
			}

			// Should never happen - FindFreeBuffer returned a buffer not in our pool
			B3D_ENSURE(false);
		}

		// No compatible buffer found - create a new one
		buffer = mDevice.CreateGpuBuffer(createInformation);
		B3D_ENSURE(buffer != nullptr);

		// Add to pool
		BufferPoolEntry entry;
		entry.BufferResource = buffer;
		entry.CreateInformation = createInformation;
		entry.InUse = true;
		entry.MemorySize = CalculateBufferMemorySize(createInformation);

		mBufferPool.push_back(entry);
		mStatisticsDirty = true;

		B3D_LOG(Info, LogRenderBackend, "Created new buffer '{0}' (type: {1}, {2} bytes)",
			name, (u32)createInformation.Type, entry.MemorySize);

		return buffer;
	}

	void FrameGraphResourceAllocator::FreeTexture(const SPtr<Texture>& texture)
	{
		B3D_ENSURE(texture != nullptr);

		// Find the texture in the pool and mark it as available
		for (auto& entry : mTexturePool)
		{
			if (entry.TextureResource == texture)
			{
				B3D_ENSURE(entry.InUse);
				entry.InUse = false;
				mStatisticsDirty = true;
				return;
			}
		}

		// Texture not found in pool - this is an error
		B3D_LOG(Warning, LogRenderBackend, "Attempted to free texture that was not allocated by this allocator");
	}

	void FrameGraphResourceAllocator::FreeBuffer(const SPtr<GpuBuffer>& buffer)
	{
		B3D_ENSURE(buffer != nullptr);

		// Find the buffer in the pool and mark it as available
		for (auto& entry : mBufferPool)
		{
			if (entry.BufferResource == buffer)
			{
				B3D_ENSURE(entry.InUse);
				entry.InUse = false;
				mStatisticsDirty = true;
				return;
			}
		}

		// Buffer not found in pool - this is an error
		B3D_LOG(Warning, LogRenderBackend, "Attempted to free buffer that was not allocated by this allocator");
	}

	void FrameGraphResourceAllocator::Reset()
	{
		// Mark all resources as available for next frame
		for (auto& entry : mTexturePool)
			entry.InUse = false;

		for (auto& entry : mBufferPool)
			entry.InUse = false;

		mStatisticsDirty = true;
	}

	FrameGraphResourceAllocator::Statistics FrameGraphResourceAllocator::GetStatistics() const
	{
		if (mStatisticsDirty)
		{
			mStatistics = Statistics();

			// Count textures
			mStatistics.TotalTexturesAllocated = static_cast<u32>(mTexturePool.size());
			for (const auto& entry : mTexturePool)
			{
				if (entry.InUse)
					mStatistics.TexturesInUse++;

				mStatistics.TotalTextureMemoryBytes += entry.MemorySize;
			}

			// Count buffers
			mStatistics.TotalBuffersAllocated = static_cast<u32>(mBufferPool.size());
			for (const auto& entry : mBufferPool)
			{
				if (entry.InUse)
					mStatistics.BuffersInUse++;

				mStatistics.TotalBufferMemoryBytes += entry.MemorySize;
			}

			mStatisticsDirty = false;
		}

		return mStatistics;
	}

	SPtr<Texture> FrameGraphResourceAllocator::FindFreeTexture(const TextureCreateInformation& createInformation)
	{
		// Linear search through pool for compatible free texture
		for (const auto& entry : mTexturePool)
		{
			if (!entry.InUse && AreTextureCreateInformationCompatible(entry.CreateInformation, createInformation))
				return entry.TextureResource;
		}

		return nullptr;
	}

	SPtr<GpuBuffer> FrameGraphResourceAllocator::FindFreeBuffer(const GpuBufferCreateInformation& createInformation)
	{
		// Linear search through pool for compatible free buffer
		for (const auto& entry : mBufferPool)
		{
			if (!entry.InUse && AreBufferCreateInformationCompatible(entry.CreateInformation, createInformation))
				return entry.BufferResource;
		}

		return nullptr;
	}

	bool FrameGraphResourceAllocator::AreTextureCreateInformationCompatible(
		const TextureCreateInformation& a,
		const TextureCreateInformation& b) const
	{
		// Textures are compatible if all relevant creation parameters match exactly
		// We ignore the Name field as it's only for debugging
		// We also ignore InitialData as we're only concerned with descriptor compatibility

		return a.Type == b.Type &&
			   a.Format == b.Format &&
			   a.Width == b.Width &&
			   a.Height == b.Height &&
			   a.Depth == b.Depth &&
			   a.MipMapCount == b.MipMapCount &&
			   a.Usage == b.Usage &&
			   a.UseHardwareSRGB == b.UseHardwareSRGB &&
			   a.SampleCount == b.SampleCount &&
			   a.ArraySliceCount == b.ArraySliceCount;
	}

	bool FrameGraphResourceAllocator::AreBufferCreateInformationCompatible(
		const GpuBufferCreateInformation& a,
		const GpuBufferCreateInformation& b) const
	{
		// Buffers must have the same type
		if (a.Type != b.Type)
			return false;

		// Buffers must have the same flags
		if (a.Flags != b.Flags)
			return false;

		// Buffers must have the same suballocation count
		if (a.SuballocationCount != b.SuballocationCount)
			return false;

		// Type-specific compatibility checks
		switch (a.Type)
		{
		case GpuBufferType::Vertex:
			return a.Vertex.ElementSize == b.Vertex.ElementSize &&
				   a.Vertex.Count == b.Vertex.Count;

		case GpuBufferType::Index:
			return a.Index.Type == b.Index.Type &&
				   a.Index.Count == b.Index.Count;

		case GpuBufferType::Uniform:
			return a.Uniform.Size == b.Uniform.Size;

		case GpuBufferType::StagingWrite:
		case GpuBufferType::StagingRead:
			return a.Staging.Size == b.Staging.Size;

		case GpuBufferType::SimpleStorage:
			return a.SimpleStorage.Count == b.SimpleStorage.Count &&
				   a.SimpleStorage.Format == b.SimpleStorage.Format;

		case GpuBufferType::StructuredStorage:
			return a.StructuredStorage.Count == b.StructuredStorage.Count &&
				   a.StructuredStorage.ElementSize == b.StructuredStorage.ElementSize;

		default:
			return false;
		}
	}

	u64 FrameGraphResourceAllocator::CalculateTextureMemorySize(const TextureCreateInformation& createInformation) const
	{
		// Calculate face count
		u32 facesPerSlice = createInformation.Type == TEX_TYPE_CUBE_MAP ? 6 : 1;
		u32 faceCount = facesPerSlice * createInformation.ArraySliceCount;

		// Calculate memory for base mip level
		u64 baseMemorySize = PixelUtility::GetMemorySize(
			createInformation.Width,
			createInformation.Height,
			createInformation.Depth,
			createInformation.Format);

		// Calculate memory for all mip levels
		// Mip chain uses approximately 1.33x the base level memory (geometric series sum)
		u64 totalMemoryPerFace = baseMemorySize;
		if (createInformation.MipMapCount > 0)
		{
			u32 mipWidth = createInformation.Width;
			u32 mipHeight = createInformation.Height;
			u32 mipDepth = createInformation.Depth;

			for (u32 mipLevel = 1; mipLevel <= createInformation.MipMapCount; mipLevel++)
			{
				// Calculate mip dimensions (minimum 1)
				mipWidth = Math::Max(1u, mipWidth / 2);
				mipHeight = Math::Max(1u, mipHeight / 2);
				mipDepth = Math::Max(1u, mipDepth / 2);

				totalMemoryPerFace += PixelUtility::GetMemorySize(mipWidth, mipHeight, mipDepth, createInformation.Format);
			}
		}

		// Multiply by number of faces
		return totalMemoryPerFace * faceCount;
	}

	u64 FrameGraphResourceAllocator::CalculateBufferMemorySize(const GpuBufferCreateInformation& createInformation) const
	{
		u64 sizePerSuballocation = 0;

		// Calculate size based on buffer type
		switch (createInformation.Type)
		{
		case GpuBufferType::Vertex:
			sizePerSuballocation = static_cast<u64>(createInformation.Vertex.ElementSize) * createInformation.Vertex.Count;
			break;

		case GpuBufferType::Index:
		{
			u32 indexSize = createInformation.Index.Type == IT_16BIT ? 2 : 4;
			sizePerSuballocation = static_cast<u64>(indexSize) * createInformation.Index.Count;
			break;
		}

		case GpuBufferType::Uniform:
			sizePerSuballocation = createInformation.Uniform.Size;
			break;

		case GpuBufferType::StagingWrite:
		case GpuBufferType::StagingRead:
			sizePerSuballocation = createInformation.Staging.Size;
			break;

			// TODO - There must be code that does the calculation below elsewhere, consider reusing it
		case GpuBufferType::SimpleStorage:
		{
			// Calculate size based on format
			u32 elementSize = 0;
			switch (createInformation.SimpleStorage.Format)
			{
			case BF_16X1F:
			case BF_16X1S:
			case BF_16X1U:
				elementSize = 2;
				break;
			case BF_16X2F:
			case BF_16X2S:
			case BF_16X2U:
				elementSize = 4;
				break;
			case BF_16X4F:
			case BF_16X4S:
			case BF_16X4U:
				elementSize = 8;
				break;
			case BF_32X1F:
			case BF_32X1S:
			case BF_32X1U:
				elementSize = 4;
				break;
			case BF_32X2F:
			case BF_32X2S:
			case BF_32X2U:
				elementSize = 8;
				break;
			case BF_32X3F:
			case BF_32X3S:
			case BF_32X3U:
				elementSize = 12;
				break;
			case BF_32X4F:
			case BF_32X4S:
			case BF_32X4U:
				elementSize = 16;
				break;
			case BF_8X1:
			case BF_8X1S:
			case BF_8X1U:
				elementSize = 1;
				break;
			case BF_8X2:
			case BF_8X2S:
			case BF_8X2U:
				elementSize = 2;
				break;
			case BF_8X4:
			case BF_8X4S:
			case BF_8X4U:
				elementSize = 4;
				break;
			default:
				elementSize = 4; // Default fallback
				break;
			}
			sizePerSuballocation = static_cast<u64>(elementSize) * createInformation.SimpleStorage.Count;
			break;
		}

		case GpuBufferType::StructuredStorage:
			sizePerSuballocation = static_cast<u64>(createInformation.StructuredStorage.ElementSize) * createInformation.StructuredStorage.Count;
			break;

		default:
			return 0;
		}

		// Multiply by suballocation count
		return sizePerSuballocation * createInformation.SuballocationCount;
	}
}
