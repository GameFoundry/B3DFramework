//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11HardwareBuffer.h"
#include "BsD3D11Mappings.h"
#include "BsD3D11Device.h"
#include "Error/BsException.h"
#include "Debug/BsDebug.h"
#include "BsD3D11CommandBuffer.h"

using namespace bs;
using namespace bs::ct;

D3D11HardwareBuffer::D3D11HardwareBuffer(BufferType btype, GpuBufferUsage usage, u32 elementCount, u32 elementSize, D3D11Device& device, bool useSystemMem, bool streamOut)
	: HardwareBuffer(elementCount * elementSize, usage, GDF_DEFAULT)
	, mBufferType(btype)
	, mElementCount(elementCount)
	, mElementSize(elementSize)
	, mUsage(usage)
	, mDevice(device)
{
	bool isLoadStore = (usage & GBU_LOADSTORE) == GBU_LOADSTORE;

	if(useSystemMem)
	{
		if(isLoadStore)
		{
			BS_LOG(Warning, RenderBackend, "LoadStore usage and useSystemMem cannot be used together.");
			isLoadStore = false;
		}

		if(streamOut)
		{
			BS_LOG(Warning, RenderBackend, "useSystemMem and streamOut cannot be used together.");
			streamOut = false;
		}
	}

	if(isLoadStore)
	{
		if(btype == BT_CONSTANT)
		{
			BS_LOG(Warning, RenderBackend, "Constant buffers cannot be bound with LoadStore usage.");
			isLoadStore = false;
		}

		if(D3D11Mappings::IsDynamic(usage))
		{
			BS_LOG(Warning, RenderBackend, "Dynamic usage not supported with LoadStore usage.");
			usage = (GpuBufferUsage)(usage & ~GBU_DYNAMIC);
		}
	}

	if(streamOut)
	{
		if(btype == BT_CONSTANT)
		{
			BS_LOG(Warning, RenderBackend, "Constant buffers cannot be used with streamOut.");
			streamOut = false;
		}
	}

	mDesc.ByteWidth = GetSize();
	mDesc.MiscFlags = 0;
	mDesc.StructureByteStride = 0;

	if(useSystemMem)
	{
		mDesc.Usage = D3D11_USAGE_STAGING;
		mDesc.BindFlags = 0;
		mDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
	}
	else
	{
		mDesc.Usage = D3D11Mappings::GetUsage(usage);
		mDesc.CPUAccessFlags = D3D11Mappings::GetAccessFlags(usage);

		switch(btype)
		{
		default:
		case BT_STANDARD:
			mDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			break;
		case BT_STRUCTURED:
			mDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			mDesc.StructureByteStride = elementSize;
			mDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			break;
		case BT_RAW:
			mDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			mDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
			break;
		case BT_VERTEX:
			mDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			break;
		case BT_INDEX:
			mDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			break;
		case BT_CONSTANT:
			mDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			break;
		case BT_INDIRECTARGUMENT:
			mDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			mDesc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
			break;
		}

		if(isLoadStore)
			mDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;

		if(streamOut)
			mDesc.BindFlags |= D3D11_BIND_STREAM_OUTPUT;
	}

	HRESULT hr = device.GetD3D11Device()->CreateBuffer(&mDesc, nullptr, &mD3DBuffer);
	if(FAILED(hr) || mDevice.HasError())
	{
		String msg = device.GetErrorDescription();
		BS_EXCEPT(RenderingAPIException, "Cannot create D3D11 buffer: " + msg);
	}
}

D3D11HardwareBuffer::~D3D11HardwareBuffer()
{
	SAFE_RELEASE(mD3DBuffer);

	if(mpTempStagingBuffer != nullptr)
		B3DDelete(mpTempStagingBuffer);
}

void* D3D11HardwareBuffer::Map(u32 offset, u32 length, GpuLockOptions options, u32 deviceIdx, u32 queueIdx)
{
	if(length > mSize)
		BS_EXCEPT(RenderingAPIException, "Provided length " + toString(length) + " larger than the buffer " + toString(mSize) + ".");

	// Use direct (and faster) Map/Unmap if dynamic write, or a staging read/write
	if((mDesc.Usage == D3D11_USAGE_DYNAMIC && options != GBL_READ_ONLY) || mDesc.Usage == D3D11_USAGE_STAGING)
	{
		D3D11_MAP mapType;

		switch(options)
		{
		case GBL_WRITE_ONLY_DISCARD:
			if(mUsage & GBU_DYNAMIC)
			{
				mapType = D3D11_MAP_WRITE_DISCARD;
			}
			else
			{
				// Map cannot be called with MAP_WRITE_DISCARD access, because the Resource was not created as
				// D3D11_USAGE_DYNAMIC. D3D11_USAGE_DYNAMIC Resources must use either MAP_WRITE_DISCARD
				// or MAP_WRITE_NO_OVERWRITE with Map.
				mapType = D3D11_MAP_WRITE;
			}
			break;
		case GBL_WRITE_ONLY_NO_OVERWRITE:
			if(mBufferType == BT_INDEX || mBufferType == BT_VERTEX)
				mapType = D3D11_MAP_WRITE_NO_OVERWRITE;
			else
			{
				const auto& featureOptions = mDevice.GetFeatureOptions();

				if(mBufferType == BT_CONSTANT && featureOptions.MapNoOverwriteOnDynamicConstantBuffer)
					mapType = D3D11_MAP_WRITE_NO_OVERWRITE;
				else if(featureOptions.MapNoOverwriteOnDynamicBufferSRV)
					mapType = D3D11_MAP_WRITE_NO_OVERWRITE;
				else
					mapType = D3D11_MAP_WRITE;
			}
			break;
		case GBL_WRITE_ONLY:
			mapType = D3D11_MAP_WRITE;
			break;
		case GBL_READ_WRITE:
			if((mDesc.CPUAccessFlags & D3D11_CPU_ACCESS_READ) != 0 &&
			   (mDesc.CPUAccessFlags & D3D11_CPU_ACCESS_WRITE) != 0)
			{
				mapType = D3D11_MAP_READ_WRITE;
			}
			else if(mDesc.CPUAccessFlags & D3D11_CPU_ACCESS_WRITE)
			{
				mapType = D3D11_MAP_WRITE;
			}
			else
			{
				mapType = D3D11_MAP_READ;
			}
			break;
		default:
		case GBL_READ_ONLY:
			mapType = D3D11_MAP_READ;
			break;
		}

		if(D3D11Mappings::IsMappingRead(mapType) && (mDesc.CPUAccessFlags & D3D11_CPU_ACCESS_READ) == 0)
			BS_LOG(Error, RenderBackend, "Trying to read a buffer, but buffer wasn't created with a read access flag.");

		if(D3D11Mappings::IsMappingWrite(mapType) && (mDesc.CPUAccessFlags & D3D11_CPU_ACCESS_WRITE) == 0)
			BS_LOG(Error, RenderBackend, "Trying to write to a buffer, but buffer wasn't created with a write access flag.");

		D3D11_MAPPED_SUBRESOURCE mappedSubResource;
		mappedSubResource.pData = nullptr;
		mDevice.ClearErrors();

		HRESULT hr = mDevice.GetImmediateContext()->Map(mD3DBuffer, 0, mapType, 0, &mappedSubResource);
		if(FAILED(hr) || mDevice.HasError())
		{
			String msg = mDevice.GetErrorDescription();
			BS_EXCEPT(RenderingAPIException, "Error calling Map: " + msg);
		}

		return static_cast<void*>(static_cast<char*>(mappedSubResource.pData) + offset);
	}
	else // Otherwise create a staging buffer to do all read/write operations on. Usually try to avoid this.
	{
		mUseTempStagingBuffer = true;
		if(!mpTempStagingBuffer)
		{
			// Create another buffer instance but use system memory
			mpTempStagingBuffer = B3DNew<D3D11HardwareBuffer>(mBufferType, GBU_STATIC, 1, mSize, std::ref(mDevice), true);
		}

		// Schedule a copy to the staging
		if(options == GBL_READ_ONLY || options == GBL_READ_WRITE)
			mpTempStagingBuffer->CopyData(*this, 0, 0, mSize, true);

		// Register whether we'll need to upload on unlock
		mStagingUploadNeeded = (options != GBL_READ_ONLY);

		return mpTempStagingBuffer->Lock(offset, length, options);
	}
}

void D3D11HardwareBuffer::Unmap()
{
	if(mUseTempStagingBuffer)
	{
		mUseTempStagingBuffer = false;

		mpTempStagingBuffer->Unlock();

		if(mStagingUploadNeeded)
			CopyData(*mpTempStagingBuffer, 0, 0, mSize, true);

		if(mpTempStagingBuffer != nullptr)
		{
			B3DDelete(mpTempStagingBuffer);
			mpTempStagingBuffer = nullptr;
		}
	}
	else
	{
		mDevice.GetImmediateContext()->Unmap(mD3DBuffer, 0);
	}
}

void D3D11HardwareBuffer::CopyData(HardwareBuffer& srcBuffer, u32 srcOffset, u32 dstOffset, u32 length, bool discardWholeBuffer, const SPtr<ct::CommandBuffer>& commandBuffer)
{
	auto executeRef = [this](HardwareBuffer& srcBuffer, u32 srcOffset, u32 dstOffset, u32 length)
	{
		// If we're copying same-size buffers in their entirety
		if(srcOffset == 0 && dstOffset == 0 &&
		   length == mSize && mSize == srcBuffer.GetSize())
		{
			mDevice.GetImmediateContext()->CopyResource(mD3DBuffer, static_cast<D3D11HardwareBuffer&>(srcBuffer).GetD3DBuffer());
			if(mDevice.HasError())
			{
				String errorDescription = mDevice.GetErrorDescription();
				BS_EXCEPT(RenderingAPIException, "Cannot copy D3D11 resource\nError Description:" + errorDescription);
			}
		}
		else
		{
			// Copy subregion
			D3D11_BOX srcBox;
			srcBox.left = (UINT)srcOffset;
			srcBox.right = (UINT)srcOffset + length;
			srcBox.top = 0;
			srcBox.bottom = 1;
			srcBox.front = 0;
			srcBox.back = 1;

			mDevice.GetImmediateContext()->CopySubresourceRegion(mD3DBuffer, 0, (UINT)dstOffset, 0, 0, static_cast<D3D11HardwareBuffer&>(srcBuffer).GetD3DBuffer(), 0, &srcBox);
			if(mDevice.HasError())
			{
				String errorDescription = mDevice.GetErrorDescription();
				BS_EXCEPT(RenderingAPIException, "Cannot copy D3D11 subresource region\nError Description:" + errorDescription);
			}
		}
	};

	if(commandBuffer == nullptr)
		executeRef(srcBuffer, srcOffset, dstOffset, length);
	else
	{
		auto execute = [&]()
		{ executeRef(srcBuffer, srcOffset, dstOffset, length); };

		SPtr<D3D11CommandBuffer> cb = std::static_pointer_cast<D3D11CommandBuffer>(commandBuffer);
		cb->QueueCommand(execute);
	}
}

void D3D11HardwareBuffer::ReadData(u32 offset, u32 length, void* dest, u32 deviceIdx, u32 queueIdx)
{
	// There is no functional interface in D3D, just do via manual lock, copy & unlock
	void* pSrc = this->Lock(offset, length, GBL_READ_ONLY);
	memcpy(dest, pSrc, length);
	this->Unlock();
}

void D3D11HardwareBuffer::WriteData(u32 offset, u32 length, const void* pSource, BufferWriteType writeFlags, u32 queueIdx)
{
	if(mDesc.Usage == D3D11_USAGE_DYNAMIC || mDesc.Usage == D3D11_USAGE_STAGING)
	{
		GpuLockOptions lockOption = GBL_WRITE_ONLY;
		if(writeFlags == BWT_DISCARD)
			lockOption = GBL_WRITE_ONLY_DISCARD;
		else if(writeFlags == BTW_NO_OVERWRITE)
			lockOption = GBL_WRITE_ONLY_NO_OVERWRITE;

		void* pDst = this->Lock(offset, length, lockOption);
		memcpy(pDst, pSource, length);
		this->Unlock();
	}
	else if(mDesc.Usage == D3D11_USAGE_DEFAULT)
	{
		if(mBufferType == BT_CONSTANT)
		{
			assert(offset == 0);
			mDevice.GetImmediateContext()->UpdateSubresource(mD3DBuffer, 0, nullptr, pSource, 0, 0);
		}
		else
		{
			D3D11_BOX dstBox;
			dstBox.left = (UINT)offset;
			dstBox.right = (UINT)offset + length;
			dstBox.top = 0;
			dstBox.bottom = 1;
			dstBox.front = 0;
			dstBox.back = 1;

			mDevice.GetImmediateContext()->UpdateSubresource(mD3DBuffer, 0, &dstBox, pSource, 0, 0);
		}
	}
	else
		BS_LOG(Error, RenderBackend, "Trying to write into a buffer with unsupported usage: {0}", mDesc.Usage);
}
