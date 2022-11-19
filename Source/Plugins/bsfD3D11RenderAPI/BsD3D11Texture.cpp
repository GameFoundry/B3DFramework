//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11Texture.h"
#include "BsD3D11Mappings.h"
#include "BsD3D11Device.h"
#include "BsD3D11RenderAPI.h"
#include "BsD3D11TextureView.h"
#include "CoreThread/BsCoreThread.h"
#include "Error/BsException.h"
#include "Threading/BsAsyncOp.h"
#include "Profiling/BsRenderStats.h"
#include "Math/BsMath.h"
#include "BsD3D11CommandBuffer.h"

using namespace bs;
using namespace bs::ct;

D3D11Texture::D3D11Texture(const TEXTURE_DESC& desc, const SPtr<PixelData>& initialData, GpuDeviceFlags deviceMask)
	: Texture(desc, initialData, deviceMask)
{
	B3D_ASSERT((deviceMask == GDF_DEFAULT || deviceMask == GDF_PRIMARY) && "Multiple GPUs not supported natively on DirectX 11.");
}

D3D11Texture::~D3D11Texture()
{
	ClearBufferViews();

	SAFE_RELEASE(mTex);
	SAFE_RELEASE(m1DTex);
	SAFE_RELEASE(m2DTex);
	SAFE_RELEASE(m3DTex);
	SAFE_RELEASE(mStagingBuffer);

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_Texture);
}

void D3D11Texture::Initialize()
{
	THROW_IF_NOT_CORE_THREAD;

	switch(mProperties.GetTextureType())
	{
	case TEX_TYPE_1D:
		Create1DTex();
		break;
	case TEX_TYPE_2D:
	case TEX_TYPE_CUBE_MAP:
		Create2DTex();
		break;
	case TEX_TYPE_3D:
		Create3DTex();
		break;
	default:
		B3D_EXCEPT(RenderingAPIException, "Unknown texture type");
	}

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_Texture);
	Texture::Initialize();
}

void D3D11Texture::CopyImpl(const SPtr<Texture>& target, const TextureCopyInformation& desc, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [this](const SPtr<Texture>& target, const TextureCopyInformation& desc)
	{
		D3D11Texture* other = static_cast<D3D11Texture*>(target.get());

		u32 srcResIdx = D3D11CalcSubresource(desc.SourceMip, desc.SourceFace, mProperties.GetNumMipmaps() + 1);
		u32 destResIdx = D3D11CalcSubresource(desc.DestinationMip, desc.DestinationFace, target->GetProperties().GetNumMipmaps() + 1);

		D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPI::InstancePtr());
		D3D11Device& device = rs->GetPrimaryDevice();

		bool srcHasMultisample = mProperties.GetNumSamples() > 1;
		bool destHasMultisample = target->GetProperties().GetNumSamples() > 1;

		bool copyEntireSurface = desc.SourceVolume.GetWidth() == 0 ||
			desc.SourceVolume.GetHeight() == 0 ||
			desc.SourceVolume.GetDepth() == 0;

		if(srcHasMultisample && !destHasMultisample) // Resolving from MS to non-MS texture
		{
			if(copyEntireSurface)
				device.GetImmediateContext()->ResolveSubresource(other->GetDX11Resource(), destResIdx, mTex, srcResIdx, mDXGIFormat);
			else
			{
				// Need to first resolve to a temporary texture, then copy
				TEXTURE_DESC tempDesc;
				tempDesc.Width = mProperties.GetWidth();
				tempDesc.Height = mProperties.GetHeight();
				tempDesc.Format = mProperties.GetFormat();
				tempDesc.HwGamma = mProperties.IsHardwareGammaEnabled();

				SPtr<D3D11Texture> temporary = std::static_pointer_cast<D3D11Texture>(Texture::Create(tempDesc));
				device.GetImmediateContext()->ResolveSubresource(temporary->GetDX11Resource(), 0, mTex, srcResIdx, mDXGIFormat);

				TextureCopyInformation tempCopyDesc;
				tempCopyDesc.DestinationMip = desc.DestinationMip;
				tempCopyDesc.DestinationFace = desc.DestinationFace;
				tempCopyDesc.DestinationPosition = desc.DestinationPosition;

				temporary->Copy(target, tempCopyDesc);
			}
		}
		else
		{
			D3D11_BOX srcRegion;
			srcRegion.left = desc.SourceVolume.Left;
			srcRegion.right = desc.SourceVolume.Right;
			srcRegion.top = desc.SourceVolume.Top;
			srcRegion.bottom = desc.SourceVolume.Bottom;
			srcRegion.front = desc.SourceVolume.Front;
			srcRegion.back = desc.SourceVolume.Back;

			D3D11_BOX* srcRegionPtr = nullptr;
			if(!copyEntireSurface)
				srcRegionPtr = &srcRegion;

			device.GetImmediateContext()->CopySubresourceRegion(
				other->GetDX11Resource(),
				destResIdx,
				(u32)desc.DestinationPosition.X,
				(u32)desc.DestinationPosition.Y,
				(u32)desc.DestinationPosition.Z,
				mTex,
				srcResIdx,
				srcRegionPtr);

			if(device.HasError())
			{
				String errorDescription = device.GetErrorDescription();
				B3D_EXCEPT(RenderingAPIException, "D3D11 device cannot copy subresource\nError Description:" + errorDescription);
			}
		}
	};

	if(commandBuffer == nullptr)
		executeRef(target, desc);
	else
	{
		auto execute = [=]()
		{ executeRef(target, desc); };

		SPtr<D3D11CommandBuffer> cb = std::static_pointer_cast<D3D11CommandBuffer>(commandBuffer);
		cb->QueueCommand(execute);
	}
}

PixelData D3D11Texture::LockImpl(GpuLockOptions options, u32 mipLevel, u32 face, u32 deviceIdx, u32 queueIdx)
{
	if(mProperties.GetNumSamples() > 1)
		B3D_EXCEPT(InvalidStateException, "Multisampled textures cannot be accessed from the CPU directly.");

#if B3D_PROFILING_ENABLED
	if(options == GBL_READ_ONLY || options == GBL_READ_WRITE)
	{
		B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResRead, RenderStatObject_Texture);
	}

	if(options == GBL_READ_WRITE || options == GBL_WRITE_ONLY || options == GBL_WRITE_ONLY_DISCARD || options == GBL_WRITE_ONLY_NO_OVERWRITE)
	{
		B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResWrite, RenderStatObject_Texture);
	}
#endif

	u32 mipWidth = std::max(1u, mProperties.GetWidth() >> mipLevel);
	u32 mipHeight = std::max(1u, mProperties.GetHeight() >> mipLevel);
	u32 mipDepth = std::max(1u, mProperties.GetDepth() >> mipLevel);

	PixelData lockedArea(mipWidth, mipHeight, mipDepth, mInternalFormat);

	D3D11_MAP flags = D3D11Mappings::GetLockOptions(options);
	u32 rowPitch, slicePitch;
	if(flags == D3D11_MAP_READ || flags == D3D11_MAP_READ_WRITE)
	{
		u8* data = (u8*)Mapstagingbuffer(flags, face, mipLevel, rowPitch, slicePitch);
		lockedArea.SetExternalBuffer(data);
		lockedArea.SetRowPitch(rowPitch);
		lockedArea.SetSlicePitch(slicePitch);

		mLockedForReading = true;
	}
	else
	{
		if((mProperties.GetUsage() & TU_DYNAMIC) != 0)
		{
			if(flags == D3D11_MAP_WRITE)
			{
				B3D_LOG(Error, RenderBackend, "Dynamic textures only support discard or no-overwrite writes. Falling back to no-overwrite.");
				flags = D3D11_MAP_WRITE_DISCARD;
			}

			u8* data = (u8*)Map(mTex, flags, face, mipLevel, rowPitch, slicePitch);
			lockedArea.SetExternalBuffer(data);
			lockedArea.SetRowPitch(rowPitch);
			lockedArea.SetSlicePitch(slicePitch);
		}
		else
			lockedArea.SetExternalBuffer((u8*)Mapstaticbuffer(lockedArea, mipLevel, face));

		mLockedForReading = false;
	}

	return lockedArea;
}

void D3D11Texture::UnlockImpl()
{
	if(mLockedForReading)
		Unmapstagingbuffer();
	else
	{
		if((mProperties.GetUsage() & TU_DYNAMIC) != 0)
			Unmap(mTex);
		else
			Unmapstaticbuffer();
	}
}

void D3D11Texture::ReadDataImpl(PixelData& dest, u32 mipLevel, u32 face, u32 deviceIdx, u32 queueIdx)
{
	if(mProperties.GetNumSamples() > 1)
	{
		B3D_LOG(Error, RenderBackend, "Multisampled textures cannot be accessed from the CPU directly.");
		return;
	}

	PixelData myData = Lock(GBL_READ_ONLY, mipLevel, face, deviceIdx, queueIdx);
	PixelUtil::BulkPixelConversion(myData, dest);
	Unlock();
}

void D3D11Texture::WriteDataImpl(const PixelData& src, u32 mipLevel, u32 face, bool discardWholeBuffer, u32 queueIdx)
{
	PixelFormat format = mProperties.GetFormat();

	if(mProperties.GetNumSamples() > 1)
	{
		B3D_LOG(Error, RenderBackend, "Multisampled textures cannot be accessed from the CPU directly.");
		return;
	}

	mipLevel = Math::Clamp(mipLevel, (u32)mipLevel, mProperties.GetNumMipmaps());
	face = Math::Clamp(face, (u32)0, mProperties.GetNumFaces() - 1);

	if(face > 0 && mProperties.GetTextureType() == TEX_TYPE_3D)
	{
		B3D_LOG(Error, RenderBackend, "3D texture arrays are not supported.");
		return;
	}

	if((mProperties.GetUsage() & TU_DYNAMIC) != 0)
	{
		PixelData myData = Lock(discardWholeBuffer ? GBL_WRITE_ONLY_DISCARD : GBL_WRITE_ONLY, mipLevel, face, 0, queueIdx);
		PixelUtil::BulkPixelConversion(src, myData);
		Unlock();
	}
	else if((mProperties.GetUsage() & TU_DEPTHSTENCIL) == 0)
	{
		D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPI::InstancePtr());
		D3D11Device& device = rs->GetPrimaryDevice();

		UINT subresourceIdx = D3D11CalcSubresource(mipLevel, face, mProperties.GetNumMipmaps() + 1);
		u32 rowWidth = D3D11Mappings::GetSizeInBytes(format, src.GetWidth());
		u32 sliceWidth = D3D11Mappings::GetSizeInBytes(format, src.GetWidth(), src.GetHeight());

		device.GetImmediateContext()->UpdateSubresource(mTex, subresourceIdx, nullptr, src.GetData(), rowWidth, sliceWidth);

		if(device.HasError())
		{
			String errorDescription = device.GetErrorDescription();
			B3D_EXCEPT(RenderingAPIException, "D3D11 device cannot map texture\nError Description:" + errorDescription);
		}

		B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResWrite, RenderStatObject_Texture);
	}
	else
	{
		B3D_EXCEPT(RenderingAPIException, "Trying to write into a buffer with unsupported usage: " + ToString(mProperties.GetUsage()));
	}
}

void D3D11Texture::Create1DTex()
{
	u32 width = mProperties.GetWidth();
	int usage = mProperties.GetUsage();
	u32 numMips = mProperties.GetNumMipmaps();
	PixelFormat format = mProperties.GetFormat();
	bool hwGamma = mProperties.IsHardwareGammaEnabled();
	PixelFormat closestFormat = D3D11Mappings::GetClosestSupportedPf(format, TEX_TYPE_1D, usage);
	u32 numFaces = mProperties.GetNumFaces();

	// We must have those defined here
	B3D_ASSERT(width > 0);

	// Determine which D3D11 pixel format we'll use
	HRESULT hr;
	DXGI_FORMAT d3dPF = D3D11Mappings::GetPf(closestFormat, hwGamma);

	if(format != closestFormat)
	{
		B3D_LOG(Verbose, RenderBackend, "Provided pixel format is not supported by the driver: {0}. Falling back on: {1}.", format, closestFormat);
	}

	mInternalFormat = closestFormat;
	mDXGIColorFormat = d3dPF;
	mDXGIDepthStencilFormat = d3dPF;

	// TODO - Consider making this a parameter eventually
	bool readableDepth = true;

	D3D11_TEXTURE1D_DESC desc;
	desc.Width = static_cast<u32>(width);
	desc.ArraySize = numFaces == 0 ? 1 : numFaces;
	desc.Format = d3dPF;
	desc.MiscFlags = 0;

	if((usage & TU_RENDERTARGET) != 0)
	{
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MipLevels = 1;
	}
	else if((usage & TU_DEPTHSTENCIL) != 0)
	{
		desc.Usage = D3D11_USAGE_DEFAULT;

		if(readableDepth)
			desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		else
			desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		desc.CPUAccessFlags = 0;
		desc.MipLevels = 1;
		desc.Format = D3D11Mappings::GetTypelessDepthStencilPf(closestFormat);

		mDXGIColorFormat = D3D11Mappings::GetShaderResourceDepthStencilPf(closestFormat);
		mDXGIDepthStencilFormat = d3dPF;
	}
	else
	{
		desc.Usage = D3D11Mappings::GetUsage((GpuBufferUsage)usage);
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = D3D11Mappings::GetAccessFlags((GpuBufferUsage)usage);

		// Determine total number of mipmaps including main one (d3d11 convention)
		desc.MipLevels = (numMips == MIP_UNLIMITED || (1U << numMips) > width) ? 0 : numMips + 1;
	}

	if((usage & TU_LOADSTORE) != 0)
		desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

	// Create the texture
	D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPI::InstancePtr());
	D3D11Device& device = rs->GetPrimaryDevice();
	hr = device.GetD3D11Device()->CreateTexture1D(&desc, nullptr, &m1DTex);

	// Check result and except if failed
	if(FAILED(hr) || device.HasError())
	{
		String errorDescription = device.GetErrorDescription();
		B3D_EXCEPT(RenderingAPIException, "Error creating texture\nError Description:" + errorDescription);
	}

	hr = m1DTex->QueryInterface(__uuidof(ID3D11Resource), (void**)&mTex);

	if(FAILED(hr) || device.HasError())
	{
		String errorDescription = device.GetErrorDescription();
		B3D_EXCEPT(RenderingAPIException, "Can't get base texture\nError Description:" + errorDescription);
	}

	m1DTex->GetDesc(&desc);

	if(numMips != (desc.MipLevels - 1))
	{
		B3D_EXCEPT(RenderingAPIException, "Driver returned different number of mip maps than requested. "
										 "Requested: " +
					  ToString(numMips) + ". Got: " + ToString(desc.MipLevels - 1) + ".");
	}

	mDXGIFormat = desc.Format;

	// Create texture view
	if((usage & TU_DEPTHSTENCIL) == 0 || readableDepth)
	{
		TextureViewInformation viewDesc;
		viewDesc.Surface.MipLevel = 0;
		viewDesc.Surface.MipLevelCount = desc.MipLevels;
		viewDesc.Surface.Face = 0;
		viewDesc.Surface.FaceCount = desc.ArraySize;
		viewDesc.Usage = GVU_DEFAULT;

		mShaderResourceView = B3DMakeSharedFromExisting<D3D11TextureView>(new(B3DAllocate<D3D11TextureView>()) D3D11TextureView(this, viewDesc));
	}
}

void D3D11Texture::Create2DTex()
{
	u32 width = mProperties.GetWidth();
	u32 height = mProperties.GetHeight();
	int usage = mProperties.GetUsage();
	u32 numMips = mProperties.GetNumMipmaps();
	PixelFormat format = mProperties.GetFormat();
	bool hwGamma = mProperties.IsHardwareGammaEnabled();
	u32 sampleCount = mProperties.GetNumSamples();
	TextureType texType = mProperties.GetTextureType();
	PixelFormat closestFormat = D3D11Mappings::GetClosestSupportedPf(format, texType, usage);
	u32 numFaces = mProperties.GetNumFaces();

	// TODO - Consider making this a parameter eventually
	bool readableDepth = true;

	// 0-sized textures aren't supported by the API
	width = std::max(width, 1U);
	height = std::max(height, 1U);

	// Determine which D3D11 pixel format we'll use
	HRESULT hr;
	DXGI_FORMAT d3dPF = D3D11Mappings::GetPf(closestFormat, hwGamma);

	if(format != closestFormat)
	{
		B3D_LOG(Verbose, RenderBackend, "Provided pixel format is not supported by the driver: {0}. Falling back on: {1}.", format, closestFormat);
	}

	mInternalFormat = closestFormat;
	mDXGIColorFormat = d3dPF;
	mDXGIDepthStencilFormat = d3dPF;

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = static_cast<u32>(width);
	desc.Height = static_cast<u32>(height);
	desc.ArraySize = numFaces == 0 ? 1 : numFaces;
	;
	desc.Format = d3dPF;
	desc.MiscFlags = 0;

	if((usage & TU_RENDERTARGET) != 0)
	{
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; // TODO - Add flags to allow RT be created without shader resource flags (might be more optimal)
		desc.CPUAccessFlags = 0;

		DXGI_SAMPLE_DESC sampleDesc;
		D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPI::InstancePtr());
		rs->DetermineMultisampleSettings(sampleCount, d3dPF, &sampleDesc);
		desc.SampleDesc = sampleDesc;
	}
	else if((usage & TU_DEPTHSTENCIL) != 0)
	{
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
		desc.Format = D3D11Mappings::GetTypelessDepthStencilPf(closestFormat);

		if(readableDepth)
			desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		else
			desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		DXGI_SAMPLE_DESC sampleDesc;
		D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPI::InstancePtr());
		rs->DetermineMultisampleSettings(sampleCount, d3dPF, &sampleDesc);
		desc.SampleDesc = sampleDesc;

		mDXGIColorFormat = D3D11Mappings::GetShaderResourceDepthStencilPf(closestFormat);
		mDXGIDepthStencilFormat = d3dPF;
	}
	else
	{
		desc.Usage = D3D11Mappings::GetUsage((GpuBufferUsage)usage);
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = D3D11Mappings::GetAccessFlags((GpuBufferUsage)usage);

		DXGI_SAMPLE_DESC sampleDesc;
		sampleDesc.Count = 1;
		sampleDesc.Quality = 0;
		desc.SampleDesc = sampleDesc;
	}

	// Determine total number of mipmaps including main one (d3d11 convention)
	desc.MipLevels = (numMips == MIP_UNLIMITED || (1U << numMips) > width) ? 0 : numMips + 1;

	if(texType == TEX_TYPE_CUBE_MAP)
		desc.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;

	if((usage & TU_LOADSTORE) != 0)
	{
		if(desc.SampleDesc.Count <= 1)
			desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
		else
		{
			B3D_LOG(Warning, RenderBackend, "Unable to create a load-store texture with multiple samples. This is not "
										   "supported on DirectX 11. Ignoring load-store usage flag.");
		}
	}

	// Create the texture
	D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPI::InstancePtr());
	D3D11Device& device = rs->GetPrimaryDevice();
	hr = device.GetD3D11Device()->CreateTexture2D(&desc, nullptr, &m2DTex);

	// Check result and except if failed
	if(FAILED(hr) || device.HasError())
	{
		String errorDescription = device.GetErrorDescription();
		B3D_EXCEPT(RenderingAPIException, "Error creating texture\nError Description:" + errorDescription);
	}

	hr = m2DTex->QueryInterface(__uuidof(ID3D11Resource), (void**)&mTex);

	if(FAILED(hr) || device.HasError())
	{
		String errorDescription = device.GetErrorDescription();
		B3D_EXCEPT(RenderingAPIException, "Can't get base texture\nError Description:" + errorDescription);
	}

	m2DTex->GetDesc(&desc);

	if(numMips != (desc.MipLevels - 1))
	{
		B3D_EXCEPT(RenderingAPIException, "Driver returned different number of mip maps than requested. "
										 "Requested: " +
					  ToString(numMips) + ". Got: " + ToString(desc.MipLevels - 1) + ".");
	}

	mDXGIFormat = desc.Format;

	// Create shader texture view
	if((usage & TU_DEPTHSTENCIL) == 0 || readableDepth)
	{
		TextureViewInformation viewDesc;
		viewDesc.Surface.MipLevel = 0;
		viewDesc.Surface.MipLevelCount = desc.MipLevels;
		viewDesc.Surface.Face = 0;
		viewDesc.Surface.FaceCount = desc.ArraySize;
		viewDesc.Usage = GVU_DEFAULT;

		mShaderResourceView = B3DMakeSharedFromExisting<D3D11TextureView>(new(B3DAllocate<D3D11TextureView>()) D3D11TextureView(this, viewDesc));
	}
}

void D3D11Texture::Create3DTex()
{
	u32 width = mProperties.GetWidth();
	u32 height = mProperties.GetHeight();
	u32 depth = mProperties.GetDepth();
	int usage = mProperties.GetUsage();
	u32 numMips = mProperties.GetNumMipmaps();
	PixelFormat format = mProperties.GetFormat();
	bool hwGamma = mProperties.IsHardwareGammaEnabled();
	PixelFormat closestFormat = D3D11Mappings::GetClosestSupportedPf(format, TEX_TYPE_3D, usage);

	// TODO - Consider making this a parameter eventually
	bool readableDepth = true;

	// We must have those defined here
	B3D_ASSERT(width > 0 && height > 0 && depth > 0);

	// Determine which D3D11 pixel format we'll use
	HRESULT hr;
	DXGI_FORMAT d3dPF = D3D11Mappings::GetPf(closestFormat, hwGamma);

	if(format != closestFormat)
	{
		B3D_LOG(Verbose, RenderBackend, "Provided pixel format is not supported by the driver: {0}. Falling back on: {1}.", format, closestFormat);
	}

	mInternalFormat = closestFormat;
	mDXGIColorFormat = d3dPF;
	mDXGIDepthStencilFormat = d3dPF;

	D3D11_TEXTURE3D_DESC desc;
	desc.Width = static_cast<u32>(width);
	desc.Height = static_cast<u32>(height);
	desc.Depth = static_cast<u32>(depth);
	desc.Format = d3dPF;
	desc.MiscFlags = 0;

	if((mProperties.GetUsage() & TU_RENDERTARGET) != 0)
	{
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MipLevels = 1;
	}
	else if((mProperties.GetUsage() & TU_DEPTHSTENCIL) != 0)
	{
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
		desc.MipLevels = 1;

		if(readableDepth)
			desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		else
			desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		mDXGIColorFormat = D3D11Mappings::GetShaderResourceDepthStencilPf(closestFormat);
		mDXGIDepthStencilFormat = d3dPF;
	}
	else
	{
		desc.Usage = D3D11Mappings::GetUsage((GpuBufferUsage)usage);
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = D3D11Mappings::GetAccessFlags((GpuBufferUsage)usage);

		// Determine total number of mipmaps including main one (d3d11 convention)
		desc.MipLevels = (numMips == MIP_UNLIMITED || (1U << numMips) > std::max(std::max(width, height), depth)) ? 0 : numMips + 1;
	}

	if((usage & TU_LOADSTORE) != 0)
		desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

	// Create the texture
	D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPI::InstancePtr());
	D3D11Device& device = rs->GetPrimaryDevice();
	hr = device.GetD3D11Device()->CreateTexture3D(&desc, nullptr, &m3DTex);

	// Check result and except if failed
	if(FAILED(hr) || device.HasError())
	{
		String errorDescription = device.GetErrorDescription();
		B3D_EXCEPT(RenderingAPIException, "Error creating texture\nError Description:" + errorDescription);
	}

	hr = m3DTex->QueryInterface(__uuidof(ID3D11Resource), (void**)&mTex);

	if(FAILED(hr) || device.HasError())
	{
		String errorDescription = device.GetErrorDescription();
		B3D_EXCEPT(RenderingAPIException, "Can't get base texture\nError Description:" + errorDescription);
	}

	// Create texture view
	m3DTex->GetDesc(&desc);

	if(mProperties.GetNumMipmaps() != (desc.MipLevels - 1))
	{
		B3D_EXCEPT(RenderingAPIException, "Driver returned different number of mip maps than requested. "
										 "Requested: " +
					  ToString(mProperties.GetNumMipmaps()) + ". Got: " + ToString(desc.MipLevels - 1) + ".");
	}

	mDXGIFormat = desc.Format;

	if((usage & TU_DEPTHSTENCIL) == 0 || readableDepth)
	{
		TextureViewInformation viewDesc;
		viewDesc.Surface.MipLevel = 0;
		viewDesc.Surface.MipLevelCount = desc.MipLevels;
		viewDesc.Surface.Face = 0;
		viewDesc.Surface.FaceCount = 1;
		viewDesc.Usage = GVU_DEFAULT;

		mShaderResourceView = B3DMakeSharedFromExisting<D3D11TextureView>(new(B3DAllocate<D3D11TextureView>()) D3D11TextureView(this, viewDesc));
	}
}

void* D3D11Texture::Map(ID3D11Resource* res, D3D11_MAP flags, u32 mipLevel, u32 face, u32& rowPitch, u32& slicePitch)
{
	D3D11_MAPPED_SUBRESOURCE pMappedResource;
	pMappedResource.pData = nullptr;

	mipLevel = Math::Clamp(mipLevel, (u32)mipLevel, mProperties.GetNumMipmaps());
	face = Math::Clamp(face, (u32)0, mProperties.GetNumFaces() - 1);

	if(mProperties.GetTextureType() == TEX_TYPE_3D)
		face = 0;

	D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPI::InstancePtr());
	D3D11Device& device = rs->GetPrimaryDevice();

	mLockedSubresourceIdx = D3D11CalcSubresource(mipLevel, face, mProperties.GetNumMipmaps() + 1);
	device.GetImmediateContext()->Map(res, mLockedSubresourceIdx, flags, 0, &pMappedResource);

	if(device.HasError())
	{
		String errorDescription = device.GetErrorDescription();
		B3D_EXCEPT(RenderingAPIException, "D3D11 device cannot map texture\nError Description:" + errorDescription);
	}

	rowPitch = pMappedResource.RowPitch;
	slicePitch = pMappedResource.DepthPitch;

	return pMappedResource.pData;
}

void D3D11Texture::Unmap(ID3D11Resource* res)
{
	D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPI::InstancePtr());
	D3D11Device& device = rs->GetPrimaryDevice();
	device.GetImmediateContext()->Unmap(res, mLockedSubresourceIdx);

	if(device.HasError())
	{
		String errorDescription = device.GetErrorDescription();
		B3D_EXCEPT(RenderingAPIException, "D3D11 device unmap resource\nError Description:" + errorDescription);
	}
}

void* D3D11Texture::Mapstagingbuffer(D3D11_MAP flags, u32 mipLevel, u32 face, u32& rowPitch, u32& slicePitch)
{
	// Note: I am creating and destroying a staging resource every time a texture is read.
	// Consider offering a flag on init that will keep this active all the time (at the cost of double memory).
	// Reading is slow operation anyway so I don't believe doing it as we are now will influence it much.

	if(!mStagingBuffer)
		CreateStagingBuffer();

	D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPI::InstancePtr());
	D3D11Device& device = rs->GetPrimaryDevice();
	device.GetImmediateContext()->CopyResource(mStagingBuffer, mTex);

	return Map(mStagingBuffer, flags, face, mipLevel, rowPitch, slicePitch);
}

void D3D11Texture::Unmapstagingbuffer()
{
	Unmap(mStagingBuffer);
	SAFE_RELEASE(mStagingBuffer);
}

void* D3D11Texture::Mapstaticbuffer(PixelData lock, u32 mipLevel, u32 face)
{
	u32 sizeOfImage = lock.GetConsecutiveSize();
	mLockedSubresourceIdx = D3D11CalcSubresource(mipLevel, face, mProperties.GetNumMipmaps() + 1);

	mStaticBuffer = B3DNew<PixelData>(lock.GetWidth(), lock.GetHeight(), lock.GetDepth(), lock.GetFormat());
	mStaticBuffer->AllocateInternalBuffer();

	return mStaticBuffer->GetData();
}

void D3D11Texture::Unmapstaticbuffer()
{
	u32 rowWidth = D3D11Mappings::GetSizeInBytes(mStaticBuffer->GetFormat(), mStaticBuffer->GetWidth());
	u32 sliceWidth = D3D11Mappings::GetSizeInBytes(mStaticBuffer->GetFormat(), mStaticBuffer->GetWidth(), mStaticBuffer->GetHeight());

	D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPI::InstancePtr());
	D3D11Device& device = rs->GetPrimaryDevice();
	device.GetImmediateContext()->UpdateSubresource(mTex, mLockedSubresourceIdx, nullptr, mStaticBuffer->GetData(), rowWidth, sliceWidth);

	if(device.HasError())
	{
		String errorDescription = device.GetErrorDescription();
		B3D_EXCEPT(RenderingAPIException, "D3D11 device cannot map texture\nError Description:" + errorDescription);
	}

	if(mStaticBuffer != nullptr)
		B3DDelete(mStaticBuffer);
}

ID3D11ShaderResourceView* D3D11Texture::GetSrv() const
{
	return mShaderResourceView->GetSrv();
}

void D3D11Texture::CreateStagingBuffer()
{
	D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPI::InstancePtr());
	D3D11Device& device = rs->GetPrimaryDevice();
	switch(mProperties.GetTextureType())
	{
	case TEX_TYPE_1D:
		{
			D3D11_TEXTURE1D_DESC desc;
			m1DTex->GetDesc(&desc);

			desc.BindFlags = 0;
			desc.MiscFlags = 0;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
			desc.Usage = D3D11_USAGE_STAGING;

			device.GetD3D11Device()->CreateTexture1D(&desc, nullptr, (ID3D11Texture1D**)(&mStagingBuffer));
		}
		break;
	case TEX_TYPE_2D:
	case TEX_TYPE_CUBE_MAP:
		{
			D3D11_TEXTURE2D_DESC desc;
			m2DTex->GetDesc(&desc);

			desc.BindFlags = 0;
			desc.MiscFlags = 0;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
			desc.Usage = D3D11_USAGE_STAGING;

			device.GetD3D11Device()->CreateTexture2D(&desc, nullptr, (ID3D11Texture2D**)(&mStagingBuffer));
		}
		break;
	case TEX_TYPE_3D:
		{
			D3D11_TEXTURE3D_DESC desc;
			m3DTex->GetDesc(&desc);

			desc.BindFlags = 0;
			desc.MiscFlags = 0;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
			desc.Usage = D3D11_USAGE_STAGING;

			device.GetD3D11Device()->CreateTexture3D(&desc, nullptr, (ID3D11Texture3D**)(&mStagingBuffer));
		}
		break;
	}
}

SPtr<TextureView> D3D11Texture::CreateView(const TextureViewInformation& desc)
{
	return B3DMakeSharedFromExisting<D3D11TextureView>(new(B3DAllocate<D3D11TextureView>()) D3D11TextureView(this, desc));
}
