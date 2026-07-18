//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12Texture.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12ResourceManager.h"
#include "B3DD3D12Utility.h"
#include "Managers/B3DD3D12DescriptorManager.h"
#include "Profiling/B3DRenderStats.h"
#include "Image/B3DPixelUtility.h"
#include "Image/B3DPixelData.h"
#include "GpuBackend/B3DGpuWorkContext.h"
#include <algorithm>

namespace
{
	/**
	 * Returns an SRV-compatible DXGI format for a texture format. Depth formats cannot be read through an SRV using
	 * their depth format and must be viewed through a colour-compatible aliasing format.
	 */
	DXGI_FORMAT GetShaderReadFormat(DXGI_FORMAT format)
	{
		switch(format)
		{
		case DXGI_FORMAT_D32_FLOAT:
			return DXGI_FORMAT_R32_FLOAT;
		case DXGI_FORMAT_D16_UNORM:
			return DXGI_FORMAT_R16_UNORM;
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
			return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
			return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
		default:
			return format;
		}
	}
}

namespace b3d
{
	namespace render
	{
		D3D12ImageSubresource::D3D12ImageSubresource(D3D12ResourceManager* owner, D3D12_RESOURCE_STATES state, const StringView& name)
			: D3D12Resource(owner, name), mState(state)
		{}

		D3D12Image::D3D12Image(D3D12ResourceManager* owner, const D3D12ImageCreateInformation& createInformation)
			: TD3D12Resource<IGpuImageResource>(owner, createInformation.Name, createInformation.FaceCount,
				createInformation.MipLevelCount, createInformation.Aspect)
			, mResource(createInformation.Resource)
			, mAllocation(createInformation.Allocation)
			, mFormat(createInformation.Format)
		{
			const u32 subresourceCount = mFaceCount * mMipLevelCount;
			for(u32 i = 0; i < subresourceCount; i++)
				mSubresources[i] = owner->Create<D3D12ImageSubresource>(createInformation.InitialState);
		}

		D3D12Image::~D3D12Image()
		{
			const u32 subresourceCount = mFaceCount * mMipLevelCount;
			for(u32 i = 0; i < subresourceCount; i++)
			{
				if(mSubresources[i] != nullptr)
					mSubresources[i]->Destroy();
			}

			// The image is only destroyed once no command buffer references it, but the deferred queue guards
			// against release paths that bypass the tracker (e.g. teardown of never-tracked resources).
			if(mResource != nullptr || mAllocation != nullptr)
				GetDevice().DeferNativeRelease(mResource, mAllocation);
		}

		GpuTextureSubresourceRange D3D12Image::GetRange(const TextureSurface& surface) const
		{
			GpuTextureSubresourceRange range;
			range.BaseArrayLayer = surface.Face;
			range.ArrayLayerCount = std::min(surface.FaceCount == 0 ? mFaceCount : surface.FaceCount, mFaceCount);
			range.BaseMipLevel = surface.MipLevel;
			range.MipLevelCount = std::min(surface.MipLevelCount == 0 ? mMipLevelCount : surface.MipLevelCount, mMipLevelCount);
			range.AspectMask = GetRange().AspectMask;
			return range;
		}

		D3D12Texture::D3D12Texture(const TextureCreateInformation& createInformation, GpuDevice& device)
			: Texture(createInformation)
			, mGpuDevice(device)
		{
		}

		D3D12Texture::~D3D12Texture()
		{
			ReleaseTexture();

			B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_Texture);
		}

		void D3D12Texture::Initialize()
		{
			// The native resource must exist before the base initialize, as the latter uploads any initial data
			CreateTexture();

			B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_Texture);
			Texture::Initialize();
		}

		void D3D12Texture::ReleaseTexture()
		{
			// Views reference the (about to be freed) native resource, so drop them first.
			ReleaseViews();

			if(mImage == nullptr)
				return;

			// The GPU may still be referencing the resource through in-flight command buffers - destruction is
			// deferred until the image's bound count drops to zero.
			mImage->Destroy();
			mImage = nullptr;
		}

		void D3D12Texture::CreateTexture()
		{
			D3D12GpuDevice& device = static_cast<D3D12GpuDevice&>(mGpuDevice);

			const TextureProperties& props = GetProperties();

			// Convert pixel format to DXGI format. sRGB variants cannot be used with UAVs, so unordered-access
			// textures keep the linear variant (mirroring the Vulkan backend's storage-image behavior).
			const bool useSRGB = props.UseHardwareSRGB && !props.Usage.IsSet(TextureUsageFlag::AllowUnorderedAccessOnTheGPU);
			mDXGIFormat = D3D12Utility::GetDXGIFormat(props.Format, useSRGB);
			if (mDXGIFormat == DXGI_FORMAT_UNKNOWN)
			{
				B3D_LOG(Error, LogRenderBackend, "D3D12: Unsupported texture format");
				return;
			}

			// Determine resource dimension. Array-ness is now expressed via ArraySliceCount / GetFaceCount()
			// rather than dedicated array texture types.
			D3D12_RESOURCE_DIMENSION dimension;
			switch (props.Type)
			{
			case TEX_TYPE_1D:
				dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
				break;
			case TEX_TYPE_2D:
			case TEX_TYPE_CUBE_MAP:
				dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				break;
			case TEX_TYPE_3D:
				dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
				break;
			default:
				B3D_LOG(Error, LogRenderBackend, "D3D12: Unsupported texture type");
				return;
			}

			const u32 faceCount = props.GetFaceCount();

			// Create resource description
			D3D12_RESOURCE_DESC resourceDesc = {};
			resourceDesc.Dimension = dimension;
			resourceDesc.Alignment = 0; // Let D3D12 choose appropriate alignment
			resourceDesc.Width = props.Width;
			resourceDesc.Height = props.Height;
			resourceDesc.DepthOrArraySize = (dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D) ? (u16)props.Depth : (u16)faceCount;
			resourceDesc.MipLevels = (u16)(props.MipMapCount + 1);
			resourceDesc.Format = mDXGIFormat;
			resourceDesc.SampleDesc.Count = props.SampleCount > 0 ? props.SampleCount : 1;
			resourceDesc.SampleDesc.Quality = 0;
			resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

			// Set resource flags based on usage
			if (props.Usage.IsSet(TextureUsageFlag::RenderTarget))
			{
				if (PixelUtility::IsDepth(props.Format))
					resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
				else
					resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			}

			if (props.Usage.IsSet(TextureUsageFlag::DepthStencil))
				resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

			if (props.Usage.IsSet(TextureUsageFlag::AllowUnorderedAccessOnTheGPU))
				resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

			// Determine initial state
			D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;
			if (props.Usage.IsSet(TextureUsageFlag::RenderTarget))
			{
				if (PixelUtility::IsDepth(props.Format))
					initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
				else
					initialState = D3D12_RESOURCE_STATE_RENDER_TARGET;
			}
			else if (props.Usage.IsSet(TextureUsageFlag::DepthStencil))
			{
				initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
			}

			// Determine clear value for render targets / depth-stencil targets
			D3D12_CLEAR_VALUE clearValue = {};
			D3D12_CLEAR_VALUE* pClearValue = nullptr;

			if (props.Usage.IsSet(TextureUsageFlag::RenderTarget) || props.Usage.IsSet(TextureUsageFlag::DepthStencil))
			{
				clearValue.Format = mDXGIFormat;
				if (PixelUtility::IsDepth(props.Format))
				{
					clearValue.DepthStencil.Depth = 1.0f;
					clearValue.DepthStencil.Stencil = 0;
				}
				else
				{
					clearValue.Color[0] = 0.0f;
					clearValue.Color[1] = 0.0f;
					clearValue.Color[2] = 0.0f;
					clearValue.Color[3] = 0.0f;
				}
				pClearValue = &clearValue;
			}

			// Create the texture resource using the D3D12MA allocator, in GPU-only (default heap) memory.
			D3D12MA::ALLOCATION_DESC allocDesc = {};
			allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
			allocDesc.ExtraHeapFlags = D3D12_HEAP_FLAG_NONE;

			ComPtr<ID3D12Resource> resource;
			D3D12MA::Allocation* allocation = nullptr;
			HRESULT hr = device.GetAllocator()->CreateResource(
				&allocDesc,
				&resourceDesc,
				initialState,
				pClearValue,
				&allocation,
				IID_PPV_ARGS(&resource)
			);

			if (FAILED(hr))
			{
				B3D_LOG(Error, LogRenderBackend, "D3D12: Failed to create texture resource");
				return;
			}

			// Set debug name if available
			if (!props.Name.empty())
			{
				const WString wideName = ToWideString(props.Name);
				resource->SetName(wideName.c_str());
			}

			GpuTextureAspectFlags aspect = GpuTextureAspectFlag::Color;
			if (PixelUtility::IsDepth(props.Format))
			{
				const bool hasStencil = mDXGIFormat == DXGI_FORMAT_D24_UNORM_S8_UINT ||
					mDXGIFormat == DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

				aspect = hasStencil ? (GpuTextureAspectFlag::Depth | GpuTextureAspectFlag::Stencil)
					: GpuTextureAspectFlags(GpuTextureAspectFlag::Depth);
			}

			D3D12ImageCreateInformation imageCreateInformation;
			imageCreateInformation.Resource = std::move(resource);
			imageCreateInformation.Allocation = allocation;
			imageCreateInformation.Format = mDXGIFormat;
			imageCreateInformation.InitialState = initialState;
			imageCreateInformation.FaceCount = faceCount;
			imageCreateInformation.MipLevelCount = props.MipMapCount + 1;
			imageCreateInformation.Aspect = aspect;
			imageCreateInformation.Name = props.Name;

			mImage = device.GetResourceManager().Create<D3D12Image>(imageCreateInformation);

			B3D_LOG(Info, LogRenderBackend, "D3D12: Created texture '{0}': {1}x{2}, format={3}, mips={4}",
				props.Name, props.Width, props.Height, (u32)mDXGIFormat, props.MipMapCount + 1);
		}

		void D3D12Texture::RecreateInternalTexture()
		{
			// Note: this discards all currently written data, as documented in the base interface.
			ReleaseTexture();
			CreateTexture();
		}

		u32 D3D12Texture::GetStagingRowPitchInBytes(u32 mipLevel) const
		{
			u32 mipWidth, mipHeight, mipDepth;
			PixelUtility::GetSizeForMipLevel(mProperties.Width, mProperties.Height, mProperties.Depth, mipLevel, mipWidth, mipHeight, mipDepth);

			u32 rowPitch, depthPitch;
			PixelUtility::GetPitch(mipWidth, mipHeight, mipDepth, mProperties.Format, rowPitch, depthPitch);

			// Pad to the smallest multiple of the required pitch alignment that still holds a whole number of blocks
			const u32 blockSize = PixelUtility::GetBlockSize(mProperties.Format);
			u32 alignedRowPitch = Math::CeilToMultiple(rowPitch, (u32)D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
			while (alignedRowPitch % blockSize != 0)
				alignedRowPitch += D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;

			return alignedRowPitch;
		}

		ImageSubresourcePitch D3D12Texture::GetStagingBufferPitchForSubresource(u32 face, u32 mipLevel) const
		{
			u32 mipWidth, mipHeight, mipDepth;
			PixelUtility::GetSizeForMipLevel(mProperties.Width, mProperties.Height, mProperties.Depth, mipLevel, mipWidth, mipHeight, mipDepth);

			const u32 blockSize = PixelUtility::GetBlockSize(mProperties.Format);
			u32 rowPitchInPixels = GetStagingRowPitchInBytes(mipLevel) / blockSize;

			// Depth slices within a placed footprint are always RowPitch * rows apart, so the slice height stays at the
			// subresource's actual row count.
			u32 sliceHeight = mipHeight;
			if (PixelUtility::IsCompressed(mProperties.Format))
			{
				// For compressed formats the pitch is expressed in blocks
				const Vector2I blockDimension = PixelUtility::GetBlockDimensions(mProperties.Format);
				rowPitchInPixels *= blockDimension.X;
				sliceHeight = Math::DivideAndRoundUp(mipHeight, (u32)blockDimension.Y) * blockDimension.Y;
			}

			return ImageSubresourcePitch(rowPitchInPixels, sliceHeight);
		}

		GpuTextureMappedScope D3D12Texture::Map(u32 mipLevel, u32 arrayLayer, GpuMapOptions options)
		{
			// D3D12 textures created here live in a DEFAULT (GPU-only) heap and cannot be mapped directly. We mirror
			// the Null-backend approach: allocate a CPU-side PixelData for the subresource and hand it back through the
			// RAII scope. For read mappings the buffer is populated with the current GPU contents; for write mappings the
			// buffer is retained and uploaded to the GPU on the subsequent Flush() (via a staging buffer copy).
			//
			// The scope holds its own PixelData, but we point it at the same backing buffer we retain here (via
			// SetExternalBuffer) so the caller's writes are visible to Flush().
			const TextureProperties& props = GetProperties();

			const u32 mipWidth = std::max(1u, props.Width >> mipLevel);
			const u32 mipHeight = std::max(1u, props.Height >> mipLevel);
			const u32 mipDepth = std::max(1u, props.Depth >> mipLevel);

			// Backing buffer owned by the texture for the duration of the mapping.
			TShared<PixelData> backing = B3DMakeShared<PixelData>(mipWidth, mipHeight, mipDepth, props.Format);
			backing->AllocateInternalBuffer();

			// For read mappings, populate the backing buffer with the current GPU contents.
			if(options.IsSet(GpuMapOption::Read))
			{
				// Blocking readback. TextureUtility handles staging-buffer creation, the GPU copy, and the CPU read.
				GpuWorkContext& workContext = static_cast<D3D12GpuDevice&>(mGpuDevice).GetInternalWorkContext();
				TextureUtility::Read(workContext, std::static_pointer_cast<Texture>(GetShared()), *backing, mipLevel, arrayLayer);
			}

			// Retain the backing buffer for write mappings so Flush() can upload the caller's writes.
			if(options.IsSet(GpuMapOption::Write))
			{
				mMappedWriteData = backing;
				mMappedWriteMip = mipLevel;
				mMappedWriteLayer = arrayLayer;
			}

			// The scope's PixelData aliases the backing buffer's memory rather than owning a separate copy.
			PixelData scopeData(mipWidth, mipHeight, mipDepth, props.Format);
			scopeData.SetRowPitch(backing->GetRowPitch());
			scopeData.SetSlicePitch(backing->GetSlicePitch());
			scopeData.SetExternalBuffer(backing->GetData());

			return GpuTextureMappedScope(
				std::move(scopeData),
				std::static_pointer_cast<Texture>(GetShared()),
				GpuTextureSubresource(mipLevel, arrayLayer),
				options
			);
		}

		void D3D12Texture::Flush(u32 mipLevel, u32 arrayLayer)
		{
			// Invoked by the mapped scope on unmap for write mappings. Upload the retained CPU buffer to the GPU via a
			// staging-buffer copy (TextureUtility::Write handles staging + CopyBufferToTexture on the work context).
			if(mMappedWriteData == nullptr)
				return;

			if(mMappedWriteMip != mipLevel || mMappedWriteLayer != arrayLayer)
			{
				// Flush target does not match the retained mapping; drop it to avoid uploading stale data.
				mMappedWriteData = nullptr;
				return;
			}

			GpuWorkContext& workContext = static_cast<D3D12GpuDevice&>(mGpuDevice).GetInternalWorkContext();
			TextureUtility::Write(workContext, std::static_pointer_cast<Texture>(GetShared()), *mMappedWriteData, mipLevel, arrayLayer);

			mMappedWriteData = nullptr;
		}

		size_t D3D12Texture::ViewKeyHash::operator()(const ViewKey& key) const
		{
			size_t seed = 0;
			B3DCombineHash(seed, key.Surface.MipLevel);
			B3DCombineHash(seed, key.Surface.MipLevelCount);
			B3DCombineHash(seed, key.Surface.Face);
			B3DCombineHash(seed, key.Surface.FaceCount);
			B3DCombineHash(seed, key.Surface.IsBoundAs2DArray);
			B3DCombineHash(seed, (u32)key.Type);
			return seed;
		}

		void D3D12Texture::ReleaseViews()
		{
			if(mViews.empty())
				return;

			D3D12DescriptorManager& descriptorManager = static_cast<D3D12GpuDevice&>(mGpuDevice).GetDescriptorManager();
			for(auto& entry : mViews)
			{
				if(entry.second.ptr != 0)
					descriptorManager.FreeCPUDescriptor(D3D12DescriptorHeapType::CBV_SRV_UAV, entry.second);
			}

			mViews.clear();
		}

		D3D12_CPU_DESCRIPTOR_HANDLE D3D12Texture::GetSRVHandle(const TextureSurface& surface)
		{
			return GetOrCreateView(surface, ViewType::SRV);
		}

		D3D12_CPU_DESCRIPTOR_HANDLE D3D12Texture::GetUAVHandle(const TextureSurface& surface)
		{
			if(!GetProperties().Usage.IsSet(TextureUsageFlag::AllowUnorderedAccessOnTheGPU))
				return D3D12_CPU_DESCRIPTOR_HANDLE{ 0 };

			return GetOrCreateView(surface, ViewType::UAV);
		}

		D3D12_CPU_DESCRIPTOR_HANDLE D3D12Texture::GetOrCreateView(const TextureSurface& surface, ViewType type)
		{
			ID3D12Resource* nativeResource = GetD3D12Resource();
			if(nativeResource == nullptr)
				return D3D12_CPU_DESCRIPTOR_HANDLE{ 0 };

			const ViewKey key{ surface, type };
			auto it = mViews.find(key);
			if(it != mViews.end())
				return it->second;

			D3D12GpuDevice& device = static_cast<D3D12GpuDevice&>(mGpuDevice);
			ID3D12Device* d3d12Device = device.GetD3D12Device();
			D3D12DescriptorManager& descriptorManager = device.GetDescriptorManager();

			const TextureProperties& props = GetProperties();

			// Resolve the requested surface, treating zero counts as "all remaining".
			const u32 mipCount = props.MipMapCount + 1;
			const u32 faceCount = props.GetFaceCount();

			const u32 baseMip = surface.MipLevel;
			const u32 numMips = surface.MipLevelCount == 0 ? (mipCount - baseMip) : surface.MipLevelCount;
			const u32 baseFace = surface.Face;
			const u32 numFaces = surface.FaceCount == 0 ? (faceCount - baseFace) : surface.FaceCount;

			D3D12_CPU_DESCRIPTOR_HANDLE handle = descriptorManager.AllocateCPUDescriptor(D3D12DescriptorHeapType::CBV_SRV_UAV);
			if(handle.ptr == 0)
			{
				B3D_LOG(Error, LogRenderBackend, "D3D12: Failed to allocate descriptor for texture view");
				return handle;
			}

			const bool isCube = props.Type == TEX_TYPE_CUBE_MAP && !surface.IsBoundAs2DArray;
			const bool isArray = numFaces > 1 || (baseFace > 0) || surface.IsBoundAs2DArray;

			if(type == ViewType::SRV)
			{
				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.Format = GetShaderReadFormat(mDXGIFormat);
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

				switch(props.Type)
				{
				case TEX_TYPE_1D:
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
					srvDesc.Texture1D.MostDetailedMip = baseMip;
					srvDesc.Texture1D.MipLevels = numMips;
					break;
				case TEX_TYPE_3D:
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
					srvDesc.Texture3D.MostDetailedMip = baseMip;
					srvDesc.Texture3D.MipLevels = numMips;
					break;
				case TEX_TYPE_CUBE_MAP:
					if(isCube)
					{
						srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
						srvDesc.TextureCube.MostDetailedMip = baseMip;
						srvDesc.TextureCube.MipLevels = numMips;
					}
					else
					{
						srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
						srvDesc.Texture2DArray.MostDetailedMip = baseMip;
						srvDesc.Texture2DArray.MipLevels = numMips;
						srvDesc.Texture2DArray.FirstArraySlice = baseFace;
						srvDesc.Texture2DArray.ArraySize = numFaces;
					}
					break;
				case TEX_TYPE_2D:
				default:
					if(isArray)
					{
						srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
						srvDesc.Texture2DArray.MostDetailedMip = baseMip;
						srvDesc.Texture2DArray.MipLevels = numMips;
						srvDesc.Texture2DArray.FirstArraySlice = baseFace;
						srvDesc.Texture2DArray.ArraySize = numFaces;
					}
					else
					{
						srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
						srvDesc.Texture2D.MostDetailedMip = baseMip;
						srvDesc.Texture2D.MipLevels = numMips;
					}
					break;
				}

				// Note: For depth textures the resource was created with a typed depth format (e.g. D32_FLOAT). A
				// colour-aliased SRV over a non-typeless depth resource is invalid in D3D12.
				// TODO(d3d12-port): Create depth textures with a typeless format so a shader-read SRV can be created.
				d3d12Device->CreateShaderResourceView(nativeResource, &srvDesc, handle);
			}
			else // UAV
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				uavDesc.Format = mDXGIFormat;

				switch(props.Type)
				{
				case TEX_TYPE_1D:
					uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
					uavDesc.Texture1D.MipSlice = baseMip;
					break;
				case TEX_TYPE_3D:
					uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
					uavDesc.Texture3D.MipSlice = baseMip;
					uavDesc.Texture3D.FirstWSlice = 0;
					uavDesc.Texture3D.WSize = std::max(1u, props.Depth >> baseMip);
					break;
				case TEX_TYPE_CUBE_MAP:
				case TEX_TYPE_2D:
				default:
					if(isArray || isCube)
					{
						uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
						uavDesc.Texture2DArray.MipSlice = baseMip;
						uavDesc.Texture2DArray.FirstArraySlice = baseFace;
						uavDesc.Texture2DArray.ArraySize = isCube ? faceCount : numFaces;
					}
					else
					{
						uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
						uavDesc.Texture2D.MipSlice = baseMip;
					}
					break;
				}

				d3d12Device->CreateUnorderedAccessView(nativeResource, nullptr, &uavDesc, handle);
			}

			mViews[key] = handle;
			return handle;
		}

		GpuQueueMask D3D12Texture::GetUseMask(u32 mipLevel, u32 arrayLayer, GpuAccessFlags accessFlags) const
		{
			if(mImage == nullptr)
				return GpuQueueMask();

			// Subresource use handles are registered on the generic tracker path, which does not split read/write
			// counters, so per-subresource masks fall back to the whole-image mask filtered by access.
			(void)mipLevel;
			(void)arrayLayer;
			return mImage->GetUseInfo(accessFlags);
		}

		u32 D3D12Texture::GetBoundCount(u32 subresourceIdx) const
		{
			if(mImage == nullptr)
				return 0;

			u32 face, mip;
			mProperties.MapFromSubresourceIndex(subresourceIdx, face, mip);
			return mImage->GetSubresource(face, mip)->GetBoundCount();
		}

		u32 D3D12Texture::GetUseCount(u32 subresourceIdx) const
		{
			if(mImage == nullptr)
				return 0;

			u32 face, mip;
			mProperties.MapFromSubresourceIndex(subresourceIdx, face, mip);
			return mImage->GetSubresource(face, mip)->GetUseCount();
		}

	} // namespace render
} // namespace b3d
