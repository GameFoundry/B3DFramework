//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsGpuBuffer.h"
#include "Error/BsException.h"
#include "RenderAPI/BsRenderAPI.h"
#include "Managers/BsHardwareBufferManager.h"
#include "Profiling/BsRenderStats.h"

namespace bs
{
	UINT32 getBufferSize(const GPU_BUFFER_DESC& desc)
	{
		UINT32 elementSize;

		if (desc.type == GBT_STANDARD)
			elementSize = GpuBuffer::getFormatSize(desc.format);
		else
			elementSize = desc.elementSize;

		return elementSize * desc.elementCount;
	}

	GpuBufferProperties::GpuBufferProperties(const GPU_BUFFER_DESC& desc)
		: mDesc(desc)
	{
		if(mDesc.type == GBT_STANDARD)
			mDesc.elementSize = GpuBuffer::getFormatSize(mDesc.format);
	}

	GpuBuffer::GpuBuffer(const GPU_BUFFER_DESC& desc)
		:mProperties(desc)
	{  
	}

	SPtr<ct::GpuBuffer> GpuBuffer::getCore() const
	{
		return std::static_pointer_cast<ct::GpuBuffer>(mCoreSpecific);
	}

	SPtr<ct::CoreObject> GpuBuffer::createCore() const
	{
		return ct::HardwareBufferManager::instance().createGpuBufferInternal(mProperties.mDesc);
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

	namespace ct
	{
	GpuBuffer::GpuBuffer(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
		:HardwareBuffer(getBufferSize(desc), desc.usage, deviceMask), mProperties(desc)
	{
		if (desc.type != GBT_STANDARD)
			assert(desc.format == BF_UNKNOWN && "Format must be set to BF_UNKNOWN when using non-standard buffers");
		else
			assert(desc.elementSize == 0 && "No element size can be provided for standard buffer. Size is determined from format.");
	}

	GpuBuffer::GpuBuffer(const GPU_BUFFER_DESC& desc, SPtr<HardwareBuffer> underlyingBuffer)
		: HardwareBuffer(getBufferSize(desc), desc.usage, underlyingBuffer->getDeviceMask()), mProperties(desc)
		, mBuffer(underlyingBuffer.get()), mExternalBuffer(std::move(underlyingBuffer))
	{
		const auto& props = getProperties();
		assert(underlyingBuffer->getSize() == (props.getElementCount() * props.getElementSize()));

		if (desc.type != GBT_STANDARD)
			assert(desc.format == BF_UNKNOWN && "Format must be set to BF_UNKNOWN when using non-standard buffers");
		else
			assert(desc.elementSize == 0 && "No element size can be provided for standard buffer. Size is determined from format.");
	}

	GpuBuffer::~GpuBuffer()
	{
		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_GpuBuffer);
	}

	void GpuBuffer::initialize()
	{
		const GpuBufferProperties& props = getProperties();

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_GpuBuffer);
		CoreObject::initialize();
	}

	void* GpuBuffer::map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx, UINT32 queueIdx)
	{
#if BS_PROFILING_ENABLED
		if (options == GBL_READ_ONLY || options == GBL_READ_WRITE)
		{
			BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_GpuBuffer);
		}

		if (options == GBL_READ_WRITE || options == GBL_WRITE_ONLY || options == GBL_WRITE_ONLY_DISCARD || options == GBL_WRITE_ONLY_NO_OVERWRITE)
		{
			BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_GpuBuffer);
		}
#endif

		return mBuffer->lock(offset, length, options, deviceIdx, queueIdx);
	}

	void GpuBuffer::unmap()
	{
		mBuffer->unlock();
	}

	void GpuBuffer::readData(UINT32 offset, UINT32 length, void* dest, UINT32 deviceIdx, UINT32 queueIdx)
	{
		BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_GpuBuffer);

		mBuffer->readData(offset, length, dest, deviceIdx, queueIdx);
	}

	void GpuBuffer::writeData(UINT32 offset, UINT32 length, const void* source, BufferWriteType writeFlags, 
		UINT32 queueIdx)
	{
		BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_GpuBuffer);

		mBuffer->writeData(offset, length, source, writeFlags, queueIdx);
	}

	void GpuBuffer::copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, UINT32 length, 
		bool discardWholeBuffer, const SPtr<CommandBuffer>& commandBuffer)
	{
		auto& srcGpuBuffer = static_cast<GpuBuffer&>(srcBuffer);
		mBuffer->copyData(*srcGpuBuffer.mBuffer, srcOffset, dstOffset, length, discardWholeBuffer, commandBuffer);
	}

	SPtr<GpuBuffer> GpuBuffer::create(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
	{
		return HardwareBufferManager::instance().createGpuBuffer(desc, deviceMask);
	}

	SPtr<GpuBuffer> GpuBuffer::create(const GPU_BUFFER_DESC& desc, SPtr<HardwareBuffer> underlyingBuffer)
	{
		return HardwareBufferManager::instance().createGpuBuffer(desc, std::move(underlyingBuffer));
	}
	}
}
