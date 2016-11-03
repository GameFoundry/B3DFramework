//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGpuBuffer.h"
#include "BsException.h"
#include "BsRenderAPI.h"
#include "BsHardwareBufferManager.h"

namespace BansheeEngine
{
	GpuBufferProperties::GpuBufferProperties(const GPU_BUFFER_DESC& desc)
		: mDesc(desc)
	{
		if(mDesc.type == GBT_STANDARD)
			mDesc.elementSize = GpuBuffer::getFormatSize(mDesc.format);
	}

	GpuBufferCore::GpuBufferCore(const GPU_BUFFER_DESC& desc, UINT32 deviceMask)
		: mProperties(desc)
	{
	}

	GpuBufferCore::~GpuBufferCore()
	{
		// Make sure that derived classes call clearBufferViews
		// I can't call it here since it needs a virtual method call
	}

	SPtr<GpuBufferCore> GpuBufferCore::create(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
	{
		return HardwareBufferCoreManager::instance().createGpuBuffer(desc, deviceMask);
	}

	GpuBuffer::GpuBuffer(const GPU_BUFFER_DESC& desc)
		:mProperties(desc)
	{  
	}

	SPtr<GpuBufferCore> GpuBuffer::getCore() const
	{
		return std::static_pointer_cast<GpuBufferCore>(mCoreSpecific);
	}

	SPtr<CoreObjectCore> GpuBuffer::createCore() const
	{
		return HardwareBufferCoreManager::instance().createGpuBufferInternal(mProperties.mDesc);
	}

	UINT32 GpuBuffer::getFormatSize(GpuBufferFormat format)
	{
		static bool lookupInitialized = false;

		static UINT32 lookup[BF_COUNT];
		if (!lookupInitialized)
		{
			lookup[BF_16X1F] = 2;
			lookup[BF_16X2F] = 4;
			lookup[BF_16X4F] = 8;
			lookup[BF_32X1F] = 4;
			lookup[BF_32X2F] = 8;
			lookup[BF_32X3F] = 12;
			lookup[BF_32X4F] = 16;
			lookup[BF_8X1] = 1;
			lookup[BF_8X2] = 2;
			lookup[BF_8X4] = 4;
			lookup[BF_16X1] = 2;
			lookup[BF_16X2] = 4;
			lookup[BF_16X4] = 8;
			lookup[BF_8X1S] = 1;
			lookup[BF_8X2S] = 2;
			lookup[BF_8X4S] = 4;
			lookup[BF_16X1S] = 2;
			lookup[BF_16X2S] = 4;
			lookup[BF_16X4S] = 8;
			lookup[BF_32X1S] = 4;
			lookup[BF_32X2S] = 8;
			lookup[BF_32X3S] = 12;
			lookup[BF_32X4S] = 16;
			lookup[BF_8X1U] = 1;
			lookup[BF_8X2U] = 2;
			lookup[BF_8X4U] = 4;
			lookup[BF_16X1U] = 1;
			lookup[BF_16X2U] = 2;
			lookup[BF_16X4U] = 4;
			lookup[BF_32X1U] = 4;
			lookup[BF_32X2U] = 8;
			lookup[BF_32X3U] = 12;
			lookup[BF_32X4U] = 16;

			lookupInitialized = true;
		}

		if (format >= BF_COUNT)
			return 0;

		return lookup[(UINT32)format];
	}

	SPtr<GpuBuffer> GpuBuffer::create(const GPU_BUFFER_DESC& desc)
	{
		return HardwareBufferManager::instance().createGpuBuffer(desc);
	}
}