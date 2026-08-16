//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12Texture.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12ResourceManager.h"
#include "B3DD3D12Utility.h"
#include "Managers/B3DD3D12DescriptorManager.h"
#include "Profiling/B3DRenderStats.h"
#include "Image/B3DPixelUtility.h"
#include <algorithm>
#include <numeric>

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
		D3D12ImageSubresource::D3D12ImageSubresource(D3D12ResourceManager* owner, const D3D12TextureLayout& layout, const StringView& name) : D3D12Resource(owner, name), mLayout(layout)
		{}

		const D3D12TextureLayout& D3D12ImageSubresource::GetLayout() const
		{
			AssertIfNotSubmitThread();
			return mLayout;
		}

		void D3D12ImageSubresource::SetLayout(const D3D12TextureLayout& layout)
		{
			AssertIfNotSubmitThread();
			mLayout = layout;
		}

		bool D3D12ImageSubresource::GetLayoutTransitionQueueId(GpuQueueId& outQueueId) const
		{
			AssertIfNotSubmitThread();

			if(!mHasLayoutTransitionQueue)
				return false;

			outQueueId = mLayoutTransitionQueueId;
			return true;
		}

		void D3D12ImageSubresource::SetLayoutTransitionQueueId(GpuQueueId queueId)
		{
			AssertIfNotSubmitThread();
			mLayoutTransitionQueueId = queueId;
			mHasLayoutTransitionQueue = true;
		}

		D3D12Image::D3D12Image(D3D12ResourceManager* owner, const D3D12ImageCreateInformation& createInformation) : TD3D12Resource<IGpuImageResource>(owner, createInformation.Name, createInformation.FaceCount, createInformation.MipLevelCount, createInformation.Aspect), mResource(createInformation.Resource), mAllocation(createInformation.Allocation), mFormat(createInformation.Format), mAllowConcurrentQueueReads(createInformation.AllowConcurrentQueueReads)
		{
			const u32 subresourceCount = mFaceCount * mMipLevelCount;
			for(u32 subresourceIndex = 0; subresourceIndex < subresourceCount; subresourceIndex++)
				mSubresources[subresourceIndex] = owner->Create<D3D12ImageSubresource>(createInformation.InitialLayout);
		}

		D3D12Image::~D3D12Image()
		{
			const u32 subresourceCount = mFaceCount * mMipLevelCount;
			for(u32 subresourceIndex = 0; subresourceIndex < subresourceCount; subresourceIndex++)
			{
				if(mSubresources[subresourceIndex] != nullptr)
					mSubresources[subresourceIndex]->Destroy();
			}

			mResource.Reset();
			GetDevice().FreeMemory(mAllocation);
		}

		GpuTextureSubresourceRange D3D12Image::GetRange(const TextureSurface& surface) const
		{
			const u32 remainingFaceCount = surface.Face < mFaceCount ? mFaceCount - surface.Face : 0;
			const u32 remainingMipLevelCount = surface.MipLevel < mMipLevelCount ? mMipLevelCount - surface.MipLevel : 0;

			GpuTextureSubresourceRange range;
			range.BaseArrayLayer = surface.Face;
			range.ArrayLayerCount = surface.FaceCount == 0 ? remainingFaceCount : std::min(surface.FaceCount, remainingFaceCount);
			range.BaseMipLevel = surface.MipLevel;
			range.MipLevelCount = surface.MipLevelCount == 0 ? remainingMipLevelCount : std::min(surface.MipLevelCount, remainingMipLevelCount);
			range.AspectMask = GetRange().AspectMask;
			return range;
		}

		D3D12Texture::D3D12Texture(const TextureCreateInformation& createInformation, GpuDevice& device) : Texture(createInformation), mGpuDevice(device)
		{
		}

		D3D12Texture::~D3D12Texture()
		{
			ReleaseTexture();

			B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_Texture);
		}

		void D3D12Texture::Initialize()
		{
			CreateTexture();

			B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_Texture);
			Texture::Initialize();
		}

		void D3D12Texture::ReleaseTexture()
		{
			ReleaseViews();

			if(mImage == nullptr)
				return;

			mImage->Destroy();
			mImage = nullptr;
		}

		void D3D12Texture::CreateTexture()
		{
			D3D12GpuDevice& device = static_cast<D3D12GpuDevice&>(mGpuDevice);

			const TextureProperties& properties = GetProperties();

			// Convert pixel format to DXGI format. sRGB variants cannot be used with UAVs, so unordered-access
			// textures keep the linear variant (mirroring the Vulkan backend's storage-image behavior).
			const bool useSRGB = properties.UseHardwareSRGB && !properties.Usage.IsSet(TextureUsageFlag::AllowUnorderedAccessOnTheGPU);
			mDXGIFormat = D3D12Utility::GetDXGIFormat(properties.Format, useSRGB);
			if (mDXGIFormat == DXGI_FORMAT_UNKNOWN)
			{
				B3D_LOG(Error, LogRenderBackend, "D3D12: Unsupported texture format");
				return;
			}

			// Determine resource dimension. Array-ness is expressed through the face count rather than through
			// dedicated array texture types.
			D3D12_RESOURCE_DIMENSION dimension;
			switch (properties.Type)
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

			const u32 faceCount = properties.GetFaceCount();

			// Create resource description
			D3D12_RESOURCE_DESC resourceDesc = {};
			resourceDesc.Dimension = dimension;
			resourceDesc.Alignment = 0; // Let D3D12 choose appropriate alignment
			resourceDesc.Width = properties.Width;
			resourceDesc.Height = properties.Height;
			resourceDesc.DepthOrArraySize = (dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D) ? (u16)properties.Depth : (u16)faceCount;
			resourceDesc.MipLevels = (u16)(properties.MipMapCount + 1);
			resourceDesc.Format = mDXGIFormat;
			resourceDesc.SampleDesc.Count = properties.SampleCount > 0 ? properties.SampleCount : 1;
			resourceDesc.SampleDesc.Quality = 0;
			resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

			// Set resource flags based on usage
			if (properties.Usage.IsSet(TextureUsageFlag::RenderTarget))
			{
				if (PixelUtility::IsDepth(properties.Format))
					resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
				else
					resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			}

			if (properties.Usage.IsSet(TextureUsageFlag::DepthStencil))
				resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

			if (properties.Usage.IsSet(TextureUsageFlag::AllowUnorderedAccessOnTheGPU))
				resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

			// Determine clear value for render targets / depth-stencil targets
			D3D12_CLEAR_VALUE clearValue = {};
			D3D12_CLEAR_VALUE* optimizedClearValue = nullptr;

			if (properties.Usage.IsSet(TextureUsageFlag::RenderTarget) || properties.Usage.IsSet(TextureUsageFlag::DepthStencil))
			{
				clearValue.Format = mDXGIFormat;
				if (PixelUtility::IsDepth(properties.Format))
				{
					clearValue.DepthStencil.Depth = properties.ClearDepth;
					clearValue.DepthStencil.Stencil = properties.ClearStencil;
				}
				else
				{
					clearValue.Color[0] = properties.ClearColor.R;
					clearValue.Color[1] = properties.ClearColor.G;
					clearValue.Color[2] = properties.ClearColor.B;
					clearValue.Color[3] = properties.ClearColor.A;
				}

				optimizedClearValue = &clearValue;
			}

			ComPtr<ID3D12Resource> resource;
			GpuResourceLocation allocation;
			HRESULT hr = device.CreateResource(resourceDesc, D3D12_HEAP_TYPE_DEFAULT, D3D12_BARRIER_LAYOUT_UNDEFINED, optimizedClearValue, resource, allocation);

			if (FAILED(hr))
			{
				B3D_LOG(Error, LogRenderBackend, "D3D12: Failed to create texture resource");
				return;
			}

			// Set debug name if available
			if (!properties.Name.empty())
			{
				const WString wideName = ToWideString(properties.Name);
				resource->SetName(wideName.c_str());
			}

			GpuTextureAspectFlags aspect = GpuTextureAspectFlag::Color;
			if (PixelUtility::IsDepth(properties.Format))
			{
				const bool hasStencil = mDXGIFormat == DXGI_FORMAT_D24_UNORM_S8_UINT || mDXGIFormat == DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

				aspect = hasStencil ? (GpuTextureAspectFlag::Depth | GpuTextureAspectFlag::Stencil) : GpuTextureAspectFlags(GpuTextureAspectFlag::Depth);
			}

			D3D12ImageCreateInformation imageCreateInformation;
			imageCreateInformation.Resource = std::move(resource);
			imageCreateInformation.Allocation = allocation;
			imageCreateInformation.Format = mDXGIFormat;
			imageCreateInformation.InitialLayout = D3D12TextureLayout::Undefined();
			imageCreateInformation.FaceCount = faceCount;
			imageCreateInformation.MipLevelCount = properties.MipMapCount + 1;
			imageCreateInformation.Aspect = aspect;
			imageCreateInformation.AllowConcurrentQueueReads = properties.Usage.IsSet(TextureUsageFlag::AllowConcurrentQueueReads);
			imageCreateInformation.Name = properties.Name;

			mImage = device.GetResourceManager().Create<D3D12Image>(imageCreateInformation);
			if(UsesDefaultSRV())
				mDefaultSRV = CreateView(TextureSurface::kComplete, ViewType::SRV);

			B3D_LOG(Verbose, LogRenderBackend, "D3D12: Created texture '{0}': {1}x{2}, format={3}, mips={4}", properties.Name, properties.Width, properties.Height, (u32)mDXGIFormat, properties.MipMapCount + 1);
		}

		void D3D12Texture::RecreateInternalTexture()
		{
			ReleaseTexture();
			CreateTexture();
		}

		u32 D3D12Texture::GetStagingRowPitchInBytes(u32 mipLevel) const
		{
			u32 mipWidth, mipHeight, mipDepth;
			PixelUtility::GetSizeForMipLevel(mProperties.Width, mProperties.Height, mProperties.Depth, mipLevel, mipWidth, mipHeight, mipDepth);

			u32 rowPitch, depthPitch;
			PixelUtility::GetPitch(mipWidth, mipHeight, mipDepth, mProperties.Format, rowPitch, depthPitch);

			const u32 blockSize = PixelUtility::GetBlockSize(mProperties.Format);
			B3D_ASSERT(blockSize != 0);

			// The generic staging pitch is expressed in whole pixels/blocks, so align to a multiple satisfying both D3D12 and the format block size.
			const u32 rowPitchAlignment = std::lcm((u32)D3D12_TEXTURE_DATA_PITCH_ALIGNMENT, blockSize);
			return Math::CeilToMultiple(rowPitch, rowPitchAlignment);
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

		GpuTextureMappedScope D3D12Texture::Map(u32, u32, GpuMapOptions)
		{
			// D3D12 textures use GPU-local heaps and are not directly mappable
			return GpuTextureMappedScope();
		}

		void D3D12Texture::ReleaseViews()
		{
			D3D12DescriptorManager& descriptorManager = static_cast<D3D12GpuDevice&>(mGpuDevice).GetDescriptorManager();
			if(mDefaultSRV.ptr != 0)
			{
				descriptorManager.FreeCPUDescriptor(D3D12DescriptorHeapType::CBV_SRV_UAV, mDefaultSRV);
				mDefaultSRV = {};
			}

			Lock lock(mViewMutex);
			for(const ViewEntry& entry : mViews)
			{
				if(entry.Handle.ptr != 0)
					descriptorManager.FreeCPUDescriptor(D3D12DescriptorHeapType::CBV_SRV_UAV, entry.Handle);
			}

			mViews.clear();
		}

		D3D12_CPU_DESCRIPTOR_HANDLE D3D12Texture::GetSRVHandle(const TextureSurface& surface)
		{
			if(UsesDefaultSRV() && surface == TextureSurface::kComplete)
				return mDefaultSRV;

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
			Lock lock(mViewMutex);

			for(const ViewEntry& entry : mViews)
			{
				if(entry.Type == type && entry.Surface == surface)
					return entry.Handle;
			}

			const D3D12_CPU_DESCRIPTOR_HANDLE handle = CreateView(surface, type);
			if(handle.ptr != 0)
				mViews.Add(ViewEntry(surface, type, handle));

			return handle;
		}

		D3D12_CPU_DESCRIPTOR_HANDLE D3D12Texture::CreateView(const TextureSurface& surface, ViewType type)
		{
			ID3D12Resource* nativeResource = GetD3D12Resource();
			if(nativeResource == nullptr)
				return D3D12_CPU_DESCRIPTOR_HANDLE{ 0 };

			D3D12GpuDevice& device = static_cast<D3D12GpuDevice&>(mGpuDevice);
			ID3D12Device* d3d12Device = device.GetD3D12Device();
			D3D12DescriptorManager& descriptorManager = device.GetDescriptorManager();

			const TextureProperties& properties = GetProperties();

			// Resolve the requested surface, treating zero counts as "all remaining".
			const u32 mipCount = properties.MipMapCount + 1;
			const u32 faceCount = properties.GetFaceCount();

			const u32 baseMip = surface.MipLevel;
			const u32 selectedMipCount = surface.MipLevelCount == 0 ? (mipCount - baseMip) : surface.MipLevelCount;
			const u32 baseFace = surface.Face;
			const u32 selectedFaceCount = surface.FaceCount == 0 ? (faceCount - baseFace) : surface.FaceCount;

			D3D12_CPU_DESCRIPTOR_HANDLE handle = descriptorManager.AllocateCPUDescriptor(D3D12DescriptorHeapType::CBV_SRV_UAV);
			if(handle.ptr == 0)
			{
				B3D_LOG(Error, LogRenderBackend, "D3D12: Failed to allocate descriptor for texture view");
				return handle;
			}

			const bool isCube = properties.Type == TEX_TYPE_CUBE_MAP && !surface.IsBoundAs2DArray;
			const bool isArray = selectedFaceCount > 1 || baseFace > 0 || surface.IsBoundAs2DArray;

			if(type == ViewType::SRV)
			{
				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.Format = GetShaderReadFormat(mDXGIFormat);
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

				switch(properties.Type)
				{
				case TEX_TYPE_1D:
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
					srvDesc.Texture1D.MostDetailedMip = baseMip;
					srvDesc.Texture1D.MipLevels = selectedMipCount;
					break;
				case TEX_TYPE_3D:
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
					srvDesc.Texture3D.MostDetailedMip = baseMip;
					srvDesc.Texture3D.MipLevels = selectedMipCount;
					break;
				case TEX_TYPE_CUBE_MAP:
				case TEX_TYPE_2D:
				default:
					// A cube map that isn't viewed as a cube always has IsBoundAs2DArray set, so it lands in the array branch
					if(isCube)
					{
						srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
						srvDesc.TextureCube.MostDetailedMip = baseMip;
						srvDesc.TextureCube.MipLevels = selectedMipCount;
					}
					else if(isArray)
					{
						srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
						srvDesc.Texture2DArray.MostDetailedMip = baseMip;
						srvDesc.Texture2DArray.MipLevels = selectedMipCount;
						srvDesc.Texture2DArray.FirstArraySlice = baseFace;
						srvDesc.Texture2DArray.ArraySize = selectedFaceCount;
					}
					else
					{
						srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
						srvDesc.Texture2D.MostDetailedMip = baseMip;
						srvDesc.Texture2D.MipLevels = selectedMipCount;
					}
					break;
				}

				// Note: For depth textures the resource was created with a typed depth format (e.g. D32_FLOAT). A
				// colour-aliased SRV over a non-typeless depth resource is invalid in D3D12.
				// TODO(d3d12-port): Create depth textures with a typeless format so a shader-read SRV can be created.
				d3d12Device->CreateShaderResourceView(nativeResource, &srvDesc, handle);
			}
			else
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				uavDesc.Format = mDXGIFormat;

				switch(properties.Type)
				{
				case TEX_TYPE_1D:
					uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
					uavDesc.Texture1D.MipSlice = baseMip;
					break;
				case TEX_TYPE_3D:
					uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
					uavDesc.Texture3D.MipSlice = baseMip;
					uavDesc.Texture3D.FirstWSlice = 0;
					uavDesc.Texture3D.WSize = std::max(1u, properties.Depth >> baseMip);
					break;
				case TEX_TYPE_CUBE_MAP:
				case TEX_TYPE_2D:
				default:
					if(isArray || isCube)
					{
						uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
						uavDesc.Texture2DArray.MipSlice = baseMip;
						uavDesc.Texture2DArray.FirstArraySlice = baseFace;
						uavDesc.Texture2DArray.ArraySize = selectedFaceCount;
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

			return handle;
		}

		bool D3D12Texture::UsesDefaultSRV() const
		{
			const TextureUsageFlags usage = GetProperties().Usage;
			return !usage.IsSetAny(TextureUsageFlag::RenderTarget | TextureUsageFlag::DepthStencil | TextureUsageFlag::AllowUnorderedAccessOnTheGPU);
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

		u32 D3D12Texture::GetBoundCount(u32 subresourceIndex) const
		{
			if(mImage == nullptr)
				return 0;

			u32 face, mipLevel;
			mProperties.MapFromSubresourceIndex(subresourceIndex, face, mipLevel);
			return mImage->GetSubresource(face, mipLevel)->GetBoundCount();
		}

		u32 D3D12Texture::GetUseCount(u32 subresourceIndex) const
		{
			if(mImage == nullptr)
				return 0;

			u32 face, mipLevel;
			mProperties.MapFromSubresourceIndex(subresourceIndex, face, mipLevel);
			return mImage->GetSubresource(face, mipLevel)->GetUseCount();
		}

	} // namespace render
} // namespace b3d
