//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D11Texture.h"
#include "BsD3D11Mappings.h"
#include "BsD3D11Device.h"
#include "BsD3D11RenderAPI.h"
#include "BsD3D11TextureView.h"
#include "BsCoreThread.h"
#include "BsException.h"
#include "BsAsyncOp.h"
#include "BsRenderStats.h"
#include "BsMath.h"

namespace bs
{
	D3D11TextureCore::D3D11TextureCore(const TEXTURE_DESC& desc, const SPtr<PixelData>& initialData, 
		GpuDeviceFlags deviceMask)
		: TextureCore(desc, initialData, deviceMask),
		m1DTex(nullptr), m2DTex(nullptr), m3DTex(nullptr), mDXGIFormat(DXGI_FORMAT_UNKNOWN), mDXGIColorFormat(DXGI_FORMAT_UNKNOWN),
		mTex(nullptr), mInternalFormat(PF_UNKNOWN), mStagingBuffer(nullptr), mDXGIDepthStencilFormat(DXGI_FORMAT_UNKNOWN),
		mLockedSubresourceIdx(-1), mLockedForReading(false), mStaticBuffer(nullptr)
	{
		assert((deviceMask == GDF_DEFAULT || deviceMask == GDF_PRIMARY) && "Multiple GPUs not supported natively on DirectX 11.");
	}

	D3D11TextureCore::~D3D11TextureCore()
	{ 
		SAFE_RELEASE(mTex);
		SAFE_RELEASE(m1DTex);
		SAFE_RELEASE(m2DTex);
		SAFE_RELEASE(m3DTex);
		SAFE_RELEASE(mStagingBuffer);

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_Texture);
	}

	void D3D11TextureCore::initialize()
	{
		THROW_IF_NOT_CORE_THREAD;

		switch (mProperties.getTextureType())
		{
		case TEX_TYPE_1D:
			create1DTex();
			break;
		case TEX_TYPE_2D:
		case TEX_TYPE_CUBE_MAP:
			create2DTex();
			break;
		case TEX_TYPE_3D:
			create3DTex();
			break;
		default:
			BS_EXCEPT(RenderingAPIException, "Unknown texture type");
		}

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_Texture);
		TextureCore::initialize();
	}

	void D3D11TextureCore::copyImpl(UINT32 srcFace, UINT32 srcMipLevel, UINT32 destFace, UINT32 destMipLevel,
									const SPtr<TextureCore>& target, UINT32 queueIdx)
	{
		D3D11TextureCore* other = static_cast<D3D11TextureCore*>(target.get());

		UINT32 srcResIdx = D3D11CalcSubresource(srcMipLevel, srcFace, mProperties.getNumMipmaps() + 1);
		UINT32 destResIdx = D3D11CalcSubresource(destMipLevel, destFace, target->getProperties().getNumMipmaps() + 1);

		D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPICore::instancePtr());
		D3D11Device& device = rs->getPrimaryDevice();

		bool srcHasMultisample = mProperties.getNumSamples() > 1;
		bool destHasMultisample = target->getProperties().getNumSamples() > 1;

		if (srcHasMultisample && !destHasMultisample) // Resolving from MS to non-MS texture
		{
			device.getImmediateContext()->ResolveSubresource(other->getDX11Resource(), destResIdx, mTex, srcResIdx, mDXGIFormat);
		}
		else
		{
			if(mProperties.getNumSamples() != target->getProperties().getNumSamples())
			{
				LOGERR("When copying textures their multisample counts must match. Ignoring copy.");
				return;
			}

			device.getImmediateContext()->CopySubresourceRegion(other->getDX11Resource(), destResIdx, 0, 0, 0, mTex, srcResIdx, nullptr);

			if (device.hasError())
			{
				String errorDescription = device.getErrorDescription();
				BS_EXCEPT(RenderingAPIException, "D3D11 device cannot copy subresource\nError Description:" + errorDescription);
			}
		}
	}

	PixelData D3D11TextureCore::lockImpl(GpuLockOptions options, UINT32 mipLevel, UINT32 face, UINT32 deviceIdx,
										 UINT32 queueIdx)
	{
		if (mProperties.getNumSamples() > 1)
			BS_EXCEPT(InvalidStateException, "Multisampled textures cannot be accessed from the CPU directly.");

#if BS_PROFILING_ENABLED
		if (options == GBL_READ_ONLY || options == GBL_READ_WRITE)
		{
			BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_Texture);
		}

		if (options == GBL_READ_WRITE || options == GBL_WRITE_ONLY || options == GBL_WRITE_ONLY_DISCARD || options == GBL_WRITE_ONLY_NO_OVERWRITE)
		{
			BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_Texture);
		}
#endif

		UINT32 mipWidth = std::max(1u, mProperties.getWidth() >> mipLevel);
		UINT32 mipHeight = std::max(1u, mProperties.getHeight() >> mipLevel);
		UINT32 mipDepth = std::max(1u, mProperties.getDepth() >> mipLevel);

		PixelData lockedArea(mipWidth, mipHeight, mipDepth, mInternalFormat);

		D3D11_MAP flags = D3D11Mappings::getLockOptions(options);
		UINT32 rowPitch, slicePitch;
		if(flags == D3D11_MAP_READ || flags == D3D11_MAP_READ_WRITE)
		{
			UINT8* data = (UINT8*)mapstagingbuffer(flags, face, mipLevel, rowPitch, slicePitch);
			lockedArea.setExternalBuffer(data);
			lockedArea.setRowPitch(rowPitch);
			lockedArea.setSlicePitch(slicePitch);
			mLockedForReading = true;
		}
		else
		{
			if ((mProperties.getUsage() & TU_DYNAMIC) != 0)
			{
				UINT8* data = (UINT8*)map(mTex, flags, face, mipLevel, rowPitch, slicePitch);
				lockedArea.setExternalBuffer(data);
				lockedArea.setRowPitch(rowPitch);
				lockedArea.setSlicePitch(slicePitch);
			}
			else
				lockedArea.setExternalBuffer((UINT8*)mapstaticbuffer(lockedArea, mipLevel, face));

			mLockedForReading = false;
		}

		return lockedArea;
	}

	void D3D11TextureCore::unlockImpl()
	{
		if(mLockedForReading)
			unmapstagingbuffer();
		else
		{
			if ((mProperties.getUsage() & TU_DYNAMIC) != 0)
				unmap(mTex);
			else
				unmapstaticbuffer();
		}
	}

	void D3D11TextureCore::readDataImpl(PixelData& dest, UINT32 mipLevel, UINT32 face, UINT32 deviceIdx, UINT32 queueIdx)
	{
		if (mProperties.getNumSamples() > 1)
		{
			LOGERR("Multisampled textures cannot be accessed from the CPU directly.");
			return;
		}

		PixelData myData = lock(GBL_READ_ONLY, mipLevel, face, deviceIdx, queueIdx);
		PixelUtil::bulkPixelConversion(myData, dest);
		unlock();
	}

	void D3D11TextureCore::writeDataImpl(const PixelData& src, UINT32 mipLevel, UINT32 face, bool discardWholeBuffer,
									 UINT32 queueIdx)
	{
		PixelFormat format = mProperties.getFormat();

		if (mProperties.getNumSamples() > 1)
		{
			LOGERR("Multisampled textures cannot be accessed from the CPU directly.");
			return;
		}

		mipLevel = Math::clamp(mipLevel, (UINT32)mipLevel, mProperties.getNumMipmaps());
		face = Math::clamp(face, (UINT32)0, mProperties.getNumFaces() - 1);

		if (face > 0 && mProperties.getTextureType() == TEX_TYPE_3D)
		{
			LOGERR("3D texture arrays are not supported.");
			return;
		}

		if ((mProperties.getUsage() & TU_DYNAMIC) != 0)
		{
			PixelData myData = lock(discardWholeBuffer ? GBL_WRITE_ONLY_DISCARD : GBL_WRITE_ONLY, mipLevel, face, 0, queueIdx);
			PixelUtil::bulkPixelConversion(src, myData);
			unlock();
		}
		else if ((mProperties.getUsage() & TU_DEPTHSTENCIL) == 0)
		{
			D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPICore::instancePtr());
			D3D11Device& device = rs->getPrimaryDevice();

			UINT subresourceIdx = D3D11CalcSubresource(mipLevel, face, mProperties.getNumMipmaps() + 1);
			UINT32 rowWidth = D3D11Mappings::getSizeInBytes(format, src.getWidth());
			UINT32 sliceWidth = D3D11Mappings::getSizeInBytes(format, src.getWidth(), src.getHeight());

			device.getImmediateContext()->UpdateSubresource(mTex, subresourceIdx, nullptr, src.getData(), rowWidth, sliceWidth);

			if (device.hasError())
			{
				String errorDescription = device.getErrorDescription();
				BS_EXCEPT(RenderingAPIException, "D3D11 device cannot map texture\nError Description:" + errorDescription);
			}

			BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_Texture);
		}
		else
		{
			BS_EXCEPT(RenderingAPIException, "Trying to write into a buffer with unsupported usage: " + toString(mProperties.getUsage()));
		}
	}

	void D3D11TextureCore::create1DTex()
	{
		UINT32 width = mProperties.getWidth();
		int usage = mProperties.getUsage();
		UINT32 numMips = mProperties.getNumMipmaps();
		PixelFormat format = mProperties.getFormat();
		bool hwGamma = mProperties.isHardwareGammaEnabled();
		PixelFormat closestFormat = D3D11Mappings::getClosestSupportedPF(format, TEX_TYPE_1D, usage);
		UINT32 numFaces = mProperties.getNumFaces();

		// We must have those defined here
		assert(width > 0);

		// Determine which D3D11 pixel format we'll use
		HRESULT hr;
		DXGI_FORMAT d3dPF = D3D11Mappings::getPF(closestFormat, hwGamma);

		if (format != D3D11Mappings::getPF(d3dPF))
		{
			LOGWRN(StringUtil::format("Provided pixel format is not supported by the driver: {0}. Falling back on: {1}.",
									  format, closestFormat));
		}

		mInternalFormat = closestFormat;
		mDXGIColorFormat = d3dPF;
		mDXGIDepthStencilFormat = d3dPF;

		// TODO - Consider making this a parameter eventually
		bool readableDepth = true;

		D3D11_TEXTURE1D_DESC desc;
		desc.Width = static_cast<UINT32>(width);
		desc.ArraySize		= numFaces == 0 ? 1 : numFaces;
		desc.Format			= d3dPF;
		desc.MiscFlags		= 0;

		if((usage & TU_RENDERTARGET) != 0)
		{
			desc.Usage			= D3D11_USAGE_DEFAULT;
			desc.BindFlags		= D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = 0;
			desc.MipLevels		= 1;
		}
		else if ((usage & TU_DEPTHSTENCIL) != 0)
		{
			desc.Usage			= D3D11_USAGE_DEFAULT;
			
			if(readableDepth)
				desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
			else
				desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

			desc.CPUAccessFlags = 0;
			desc.MipLevels		= 1;
			desc.Format			= D3D11Mappings::getTypelessDepthStencilPF(closestFormat);

			mDXGIColorFormat = D3D11Mappings::getShaderResourceDepthStencilPF(closestFormat); 
			mDXGIDepthStencilFormat = d3dPF;
		}
		else
		{
			desc.Usage			= D3D11Mappings::getUsage((GpuBufferUsage)usage);
			desc.BindFlags		= D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = D3D11Mappings::getAccessFlags((GpuBufferUsage)usage);

			// Determine total number of mipmaps including main one (d3d11 convention)
			desc.MipLevels = (numMips == MIP_UNLIMITED || (1U << numMips) > width) ? 0 : numMips + 1;
		}

		if ((usage & TU_LOADSTORE) != 0)
			desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

		// Create the texture
		D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPICore::instancePtr());
		D3D11Device& device = rs->getPrimaryDevice();
		hr = device.getD3D11Device()->CreateTexture1D(&desc, nullptr, &m1DTex);

		// Check result and except if failed
		if (FAILED(hr) || device.hasError())
		{
			String errorDescription = device.getErrorDescription();
			BS_EXCEPT(RenderingAPIException, "Error creating texture\nError Description:" + errorDescription);
		}

		hr = m1DTex->QueryInterface(__uuidof(ID3D11Resource), (void **)&mTex);

		if(FAILED(hr) || device.hasError())
		{
			String errorDescription = device.getErrorDescription();
			BS_EXCEPT(RenderingAPIException, "Can't get base texture\nError Description:" + errorDescription);
		}

		m1DTex->GetDesc(&desc);

		if(numMips != (desc.MipLevels - 1))
		{
			BS_EXCEPT(RenderingAPIException, "Driver returned different number of mip maps than requested. " \
				"Requested: " + toString(numMips) + ". Got: " + toString(desc.MipLevels - 1) + ".");
		}

		mDXGIFormat = desc.Format;

		// Create texture view
		if ((usage & TU_DEPTHSTENCIL) == 0 || readableDepth)
		{
			TEXTURE_VIEW_DESC viewDesc;
			viewDesc.mostDetailMip = 0;
			viewDesc.numMips = desc.MipLevels;
			viewDesc.firstArraySlice = 0;
			viewDesc.numArraySlices = desc.ArraySize;
			viewDesc.usage = GVU_DEFAULT;

			SPtr<TextureCore> thisPtr = std::static_pointer_cast<TextureCore>(getThisPtr());
			mShaderResourceView = bs_shared_ptr<D3D11TextureView>(new (bs_alloc<D3D11TextureView>()) D3D11TextureView(thisPtr, viewDesc));
		}
	}

	void D3D11TextureCore::create2DTex()
	{
		UINT32 width = mProperties.getWidth();
		UINT32 height = mProperties.getHeight();
		int usage = mProperties.getUsage();
		UINT32 numMips = mProperties.getNumMipmaps();
		PixelFormat format = mProperties.getFormat();
		bool hwGamma = mProperties.isHardwareGammaEnabled();
		UINT32 sampleCount = mProperties.getNumSamples();
		TextureType texType = mProperties.getTextureType();
		PixelFormat closestFormat = D3D11Mappings::getClosestSupportedPF(format, texType, usage);
		UINT32 numFaces = mProperties.getNumFaces();

		// TODO - Consider making this a parameter eventually
		bool readableDepth = true;

		// We must have those defined here
		assert(width > 0 || height > 0);

		// Determine which D3D11 pixel format we'll use
		HRESULT hr;
		DXGI_FORMAT d3dPF = D3D11Mappings::getPF(closestFormat, hwGamma);

		if (format != D3D11Mappings::getPF(d3dPF))
		{
			LOGWRN(StringUtil::format("Provided pixel format is not supported by the driver: {0}. Falling back on: {1}.",
									  format, closestFormat));
		}

		mInternalFormat = closestFormat;
		mDXGIColorFormat = d3dPF;
		mDXGIDepthStencilFormat = d3dPF;

		D3D11_TEXTURE2D_DESC desc;
		desc.Width			= static_cast<UINT32>(width);
		desc.Height			= static_cast<UINT32>(height);
		desc.ArraySize		= numFaces == 0 ? 1 : numFaces;;
		desc.Format			= d3dPF;
		desc.MiscFlags		= 0;

		if((usage & TU_RENDERTARGET) != 0)
		{
			desc.Usage			= D3D11_USAGE_DEFAULT;
			desc.BindFlags		= D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; // TODO - Add flags to allow RT be created without shader resource flags (might be more optimal)
			desc.CPUAccessFlags = 0;
			desc.MipLevels		= 1;

			DXGI_SAMPLE_DESC sampleDesc;
			D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPICore::instancePtr());
			rs->determineMultisampleSettings(sampleCount, d3dPF, &sampleDesc);
			desc.SampleDesc		= sampleDesc;

			if (texType == TEX_TYPE_CUBE_MAP)
			{
				BS_EXCEPT(NotImplementedException, "Cube map not yet supported as a render target."); // TODO: Will be once I add proper texture array support
			}
		}
		else if((usage & TU_DEPTHSTENCIL) != 0)
		{
			desc.Usage			= D3D11_USAGE_DEFAULT;
			desc.CPUAccessFlags = 0;
			desc.MipLevels		= 1;
			desc.Format			= D3D11Mappings::getTypelessDepthStencilPF(closestFormat);

			if(readableDepth)
				desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
			else
				desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

			DXGI_SAMPLE_DESC sampleDesc;
			D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPICore::instancePtr());
			rs->determineMultisampleSettings(sampleCount, d3dPF, &sampleDesc);
			desc.SampleDesc		= sampleDesc;

			if (texType == TEX_TYPE_CUBE_MAP)
			{
				BS_EXCEPT(NotImplementedException, "Cube map not yet supported as a depth stencil target."); // TODO: Will be once I add proper texture array support
			}

			mDXGIColorFormat = D3D11Mappings::getShaderResourceDepthStencilPF(closestFormat);
			mDXGIDepthStencilFormat = d3dPF;
		}
		else
		{
			desc.Usage = D3D11Mappings::getUsage((GpuBufferUsage)usage);
			desc.BindFlags		= D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = D3D11Mappings::getAccessFlags((GpuBufferUsage)usage);

			// Determine total number of mipmaps including main one (d3d11 convention)
			desc.MipLevels		= (numMips == MIP_UNLIMITED || (1U << numMips) > width) ? 0 : numMips + 1;

			DXGI_SAMPLE_DESC sampleDesc;
			sampleDesc.Count	= 1;
			sampleDesc.Quality	= 0;
			desc.SampleDesc		= sampleDesc;
		}

		if (texType == TEX_TYPE_CUBE_MAP)
            desc.MiscFlags      |= D3D11_RESOURCE_MISC_TEXTURECUBE;

		if ((usage & TU_LOADSTORE) != 0)
			desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

		// Create the texture
		D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPICore::instancePtr());
		D3D11Device& device = rs->getPrimaryDevice();
		hr = device.getD3D11Device()->CreateTexture2D(&desc, nullptr, &m2DTex);

		// Check result and except if failed
		if (FAILED(hr) || device.hasError())
		{
			String errorDescription = device.getErrorDescription();
			BS_EXCEPT(RenderingAPIException, "Error creating texture\nError Description:" + errorDescription);
		}

		hr = m2DTex->QueryInterface(__uuidof(ID3D11Resource), (void **)&mTex);

		if(FAILED(hr) || device.hasError())
		{
			String errorDescription = device.getErrorDescription();
			BS_EXCEPT(RenderingAPIException, "Can't get base texture\nError Description:" + errorDescription);
		}

		m2DTex->GetDesc(&desc);

		if(numMips != (desc.MipLevels - 1))
		{
			BS_EXCEPT(RenderingAPIException, "Driver returned different number of mip maps than requested. " \
				"Requested: " + toString(numMips) + ". Got: " + toString(desc.MipLevels - 1) + ".");
		}

		mDXGIFormat = desc.Format;

		// Create shader texture view
		if((usage & TU_DEPTHSTENCIL) == 0 || readableDepth)
		{
			TEXTURE_VIEW_DESC viewDesc;
			viewDesc.mostDetailMip = 0;
			viewDesc.numMips = desc.MipLevels;
			viewDesc.firstArraySlice = 0;
			viewDesc.numArraySlices = desc.ArraySize;
			viewDesc.usage = GVU_DEFAULT;

			SPtr<TextureCore> thisPtr = std::static_pointer_cast<TextureCore>(getThisPtr());
			mShaderResourceView = bs_shared_ptr<D3D11TextureView>(new (bs_alloc<D3D11TextureView>()) D3D11TextureView(thisPtr, viewDesc));
		}
	}

	void D3D11TextureCore::create3DTex()
	{
		UINT32 width = mProperties.getWidth();
		UINT32 height = mProperties.getHeight();
		UINT32 depth = mProperties.getDepth();
		int usage = mProperties.getUsage();
		UINT32 numMips = mProperties.getNumMipmaps();
		PixelFormat format = mProperties.getFormat();
		bool hwGamma = mProperties.isHardwareGammaEnabled();
		PixelFormat closestFormat = D3D11Mappings::getClosestSupportedPF(format, TEX_TYPE_3D, usage);

		// TODO - Consider making this a parameter eventually
		bool readableDepth = true;

		// We must have those defined here
		assert(width > 0 && height > 0 && depth > 0);

		// Determine which D3D11 pixel format we'll use
		HRESULT hr;
		DXGI_FORMAT d3dPF = D3D11Mappings::getPF(closestFormat, hwGamma);
		
		if (format != D3D11Mappings::getPF(d3dPF))
		{
			LOGWRN(StringUtil::format("Provided pixel format is not supported by the driver: {0}. Falling back on: {1}.",
									  format, closestFormat));
		}

		mInternalFormat = closestFormat;
		mDXGIColorFormat = d3dPF;
		mDXGIDepthStencilFormat = d3dPF;

		D3D11_TEXTURE3D_DESC desc;
		desc.Width = static_cast<UINT32>(width);
		desc.Height = static_cast<UINT32>(height);
		desc.Depth = static_cast<UINT32>(depth);
		desc.Format	= d3dPF;
		desc.MiscFlags = 0;

		if ((mProperties.getUsage() & TU_RENDERTARGET) != 0)
		{
			desc.Usage			= D3D11_USAGE_DEFAULT;
			desc.BindFlags		= D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = 0;
			desc.MipLevels		= 1;
		}
		else if ((mProperties.getUsage() & TU_DEPTHSTENCIL) != 0)
		{
			desc.Usage			= D3D11_USAGE_DEFAULT;
			desc.CPUAccessFlags = 0;
			desc.MipLevels		= 1;

			if (readableDepth)
				desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
			else
				desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

			mDXGIColorFormat = D3D11Mappings::getShaderResourceDepthStencilPF(closestFormat);
			mDXGIDepthStencilFormat = d3dPF;
		}
		else
		{
			desc.Usage			= D3D11Mappings::getUsage((GpuBufferUsage)usage);
			desc.BindFlags		= D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = D3D11Mappings::getAccessFlags((GpuBufferUsage)usage);

			// Determine total number of mipmaps including main one (d3d11 convention)
			desc.MipLevels		= (numMips == MIP_UNLIMITED || (1U << numMips)
				> std::max(std::max(width, height), depth)) ? 0 : numMips + 1;
		}

		if ((usage & TU_LOADSTORE) != 0)
			desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

		// Create the texture
		D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPICore::instancePtr());
		D3D11Device& device = rs->getPrimaryDevice();
		hr = device.getD3D11Device()->CreateTexture3D(&desc, nullptr, &m3DTex);

		// Check result and except if failed
		if (FAILED(hr) || device.hasError())
		{
			String errorDescription = device.getErrorDescription();
			BS_EXCEPT(RenderingAPIException, "Error creating texture\nError Description:" + errorDescription);
		}

		hr = m3DTex->QueryInterface(__uuidof(ID3D11Resource), (void **)&mTex);

		if(FAILED(hr) || device.hasError())
		{
			String errorDescription = device.getErrorDescription();
			BS_EXCEPT(RenderingAPIException, "Can't get base texture\nError Description:" + errorDescription);
		}

		// Create texture view
		m3DTex->GetDesc(&desc);

		if (mProperties.getNumMipmaps() != (desc.MipLevels - 1))
		{
			BS_EXCEPT(RenderingAPIException, "Driver returned different number of mip maps than requested. " \
				"Requested: " + toString(mProperties.getNumMipmaps()) + ". Got: " + toString(desc.MipLevels - 1) + ".");
		}

		mDXGIFormat = desc.Format;

		if ((usage & TU_DEPTHSTENCIL) == 0 || readableDepth)
		{
			TEXTURE_VIEW_DESC viewDesc;
			viewDesc.mostDetailMip = 0;
			viewDesc.numMips = desc.MipLevels;
			viewDesc.firstArraySlice = 0;
			viewDesc.numArraySlices = 1;
			viewDesc.usage = GVU_DEFAULT;

			SPtr<TextureCore> thisPtr = std::static_pointer_cast<TextureCore>(getThisPtr());
			mShaderResourceView = bs_shared_ptr<D3D11TextureView>(new (bs_alloc<D3D11TextureView>()) D3D11TextureView(thisPtr, viewDesc));
		}
	}

	void* D3D11TextureCore::map(ID3D11Resource* res, D3D11_MAP flags, UINT32 mipLevel, UINT32 face, UINT32& rowPitch, UINT32& slicePitch)
	{
		D3D11_MAPPED_SUBRESOURCE pMappedResource;
		pMappedResource.pData = nullptr;

		mipLevel = Math::clamp(mipLevel, (UINT32)mipLevel, mProperties.getNumMipmaps());
		face = Math::clamp(face, (UINT32)0, mProperties.getNumFaces() - 1);

		if (mProperties.getTextureType() == TEX_TYPE_3D)
			face = 0;

		D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPICore::instancePtr());
		D3D11Device& device = rs->getPrimaryDevice();

		mLockedSubresourceIdx = D3D11CalcSubresource(mipLevel, face, mProperties.getNumMipmaps() + 1);
		device.getImmediateContext()->Map(res, mLockedSubresourceIdx, flags, 0, &pMappedResource);

		if (device.hasError())
		{
			String errorDescription = device.getErrorDescription();
			BS_EXCEPT(RenderingAPIException, "D3D11 device cannot map texture\nError Description:" + errorDescription);
		}

		UINT32 bytesPerPixel = PixelUtil::getNumElemBytes(mProperties.getFormat());
		rowPitch = pMappedResource.RowPitch / bytesPerPixel;
		slicePitch = pMappedResource.DepthPitch / bytesPerPixel;

		return pMappedResource.pData;
	}

	void D3D11TextureCore::unmap(ID3D11Resource* res)
	{
		D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPICore::instancePtr());
		D3D11Device& device = rs->getPrimaryDevice();
		device.getImmediateContext()->Unmap(res, mLockedSubresourceIdx);

		if (device.hasError())
		{
			String errorDescription = device.getErrorDescription();
			BS_EXCEPT(RenderingAPIException, "D3D11 device unmap resource\nError Description:" + errorDescription);
		}
	}

	void* D3D11TextureCore::mapstagingbuffer(D3D11_MAP flags, UINT32 mipLevel, UINT32 face, UINT32& rowPitch, UINT32& slicePitch)
	{
		// Note: I am creating and destroying a staging resource every time a texture is read. 
		// Consider offering a flag on init that will keep this active all the time (at the cost of double memory).
		// Reading is slow operation anyway so I don't believe doing it as we are now will influence it much.

		if(!mStagingBuffer)
			createStagingBuffer(); 

		D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPICore::instancePtr());
		D3D11Device& device = rs->getPrimaryDevice();
		device.getImmediateContext()->CopyResource(mStagingBuffer, mTex);

		return map(mStagingBuffer, flags, face, mipLevel, rowPitch, slicePitch);
	}

	void D3D11TextureCore::unmapstagingbuffer()
	{
		unmap(mStagingBuffer);
		SAFE_RELEASE(mStagingBuffer);
	}

	void* D3D11TextureCore::mapstaticbuffer(PixelData lock, UINT32 mipLevel, UINT32 face)
	{
		UINT32 sizeOfImage = lock.getConsecutiveSize();
		mLockedSubresourceIdx = D3D11CalcSubresource(mipLevel, face, mProperties.getNumMipmaps()+1);

		mStaticBuffer = bs_new<PixelData>(lock.getWidth(), lock.getHeight(), lock.getDepth(), lock.getFormat());
		mStaticBuffer->allocateInternalBuffer();

		return mStaticBuffer->getData();
	}

	void D3D11TextureCore::unmapstaticbuffer()
	{
		UINT32 rowWidth = D3D11Mappings::getSizeInBytes(mStaticBuffer->getFormat(), mStaticBuffer->getWidth());
		UINT32 sliceWidth = D3D11Mappings::getSizeInBytes(mStaticBuffer->getFormat(), mStaticBuffer->getWidth(), mStaticBuffer->getHeight());

		D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPICore::instancePtr());
		D3D11Device& device = rs->getPrimaryDevice();
		device.getImmediateContext()->UpdateSubresource(mTex, mLockedSubresourceIdx, nullptr, mStaticBuffer->getData(), rowWidth, sliceWidth);

		if (device.hasError())
		{
			String errorDescription = device.getErrorDescription();
			BS_EXCEPT(RenderingAPIException, "D3D11 device cannot map texture\nError Description:" + errorDescription);
		}

		if(mStaticBuffer != nullptr)
			bs_delete(mStaticBuffer);
	}

	ID3D11ShaderResourceView* D3D11TextureCore::getSRV() const
	{
		return mShaderResourceView->getSRV();
	}

	void D3D11TextureCore::createStagingBuffer()
	{
		D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPICore::instancePtr());
		D3D11Device& device = rs->getPrimaryDevice();
		switch (mProperties.getTextureType())
		{
		case TEX_TYPE_1D:
			{
				D3D11_TEXTURE1D_DESC desc;
				m1DTex->GetDesc(&desc);

				desc.BindFlags = 0;
				desc.MiscFlags = 0;
				desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
				desc.Usage = D3D11_USAGE_STAGING;

				device.getD3D11Device()->CreateTexture1D(&desc, nullptr, (ID3D11Texture1D**)(&mStagingBuffer));
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

				device.getD3D11Device()->CreateTexture2D(&desc, nullptr, (ID3D11Texture2D**)(&mStagingBuffer));
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

				device.getD3D11Device()->CreateTexture3D(&desc, nullptr, (ID3D11Texture3D**)(&mStagingBuffer));
			}
			break;
		}
	}

	SPtr<TextureView> D3D11TextureCore::createView(const SPtr<TextureCore>& texture, const TEXTURE_VIEW_DESC& desc)
	{
		return bs_shared_ptr<D3D11TextureView>(new (bs_alloc<D3D11TextureView>()) D3D11TextureView(texture, desc));
	}
}