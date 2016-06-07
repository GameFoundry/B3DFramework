//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGpuBuffer.h"
#include "BsException.h"
#include "BsRenderAPI.h"
#include "BsHardwareBufferManager.h"

namespace BansheeEngine
{
	GpuBufferProperties::GpuBufferProperties(UINT32 elementCount, UINT32 elementSize, GpuBufferType type, 
		GpuBufferFormat format, GpuBufferUsage usage, bool randomGpuWrite, bool useCounter)
		: mType(type), mUsage(usage), mFormat(format), mRandomGpuWrite(randomGpuWrite), mUseCounter(useCounter)
		, mElementCount(elementCount), mElementSize(elementSize)
	{
		if(type == GBT_STANDARD)
			mElementSize = GpuBuffer::getFormatSize(format);
	}

	GpuBufferCore::GpuBufferCore(UINT32 elementCount, UINT32 elementSize, GpuBufferType type, GpuBufferFormat format, 
		GpuBufferUsage usage, bool randomGpuWrite, bool useCounter)
		: mProperties(elementCount, elementSize, type, format, usage, randomGpuWrite, useCounter)
	{
	}

	GpuBufferCore::~GpuBufferCore()
	{
		// Make sure that derived classes call clearBufferViews
		// I can't call it here since it needs a virtual method call
	}

	void GpuBufferCore::clearBufferViews()
	{
		for (auto iter = mBufferViews.begin(); iter != mBufferViews.end(); ++iter)
		{
			destroyView(iter->second->view);
			bs_delete(iter->second);
		}

		mBufferViews.clear();
	}

	GpuBufferView* GpuBufferCore::requestView(const SPtr<GpuBufferCore>& buffer, UINT32 firstElement,
		UINT32 numElements, GpuViewUsage usage)
	{
		const auto& props = buffer->getProperties();

		GPU_BUFFER_DESC key;
		key.firstElement = firstElement;
		key.elementWidth = props.getElementSize();
		key.numElements = numElements;
		key.usage = usage;
		key.format = props.getFormat();
		key.useCounter = props.getUseCounter();

		auto iterFind = buffer->mBufferViews.find(key);
		if (iterFind == buffer->mBufferViews.end())
		{
			GpuBufferView* newView = buffer->createView();
			newView->initialize(buffer, key);
			buffer->mBufferViews[key] = bs_new<GpuBufferReference>(newView);

			iterFind = buffer->mBufferViews.find(key);
		}

		iterFind->second->refCount++;
		return iterFind->second->view;
	}

	void GpuBufferCore::releaseView(GpuBufferView* view)
	{
		SPtr<GpuBufferCore> buffer = view->getBuffer();

		auto iterFind = buffer->mBufferViews.find(view->getDesc());
		if (iterFind == buffer->mBufferViews.end())
		{
			BS_EXCEPT(InternalErrorException, "Trying to release a buffer view that doesn't exist!");
		}

		iterFind->second->refCount--;

		if (iterFind->second->refCount == 0)
		{
			GpuBufferReference* toRemove = iterFind->second;

			buffer->mBufferViews.erase(iterFind);

			buffer->destroyView(toRemove->view);
			bs_delete(toRemove);
		}
	}

	GpuBuffer::GpuBuffer(UINT32 elementCount, UINT32 elementSize, GpuBufferType type, GpuBufferFormat format, 
		GpuBufferUsage usage, bool randomGpuWrite, bool useCounter)
		:mProperties(elementCount, elementSize, type, format, usage, randomGpuWrite, useCounter)
	{  
	}

	SPtr<GpuBufferCore> GpuBuffer::getCore() const
	{
		return std::static_pointer_cast<GpuBufferCore>(mCoreSpecific);
	}

	SPtr<CoreObjectCore> GpuBuffer::createCore() const
	{
		return HardwareBufferCoreManager::instance().createGpuBufferInternal(mProperties.getElementCount(), 
			mProperties.getElementSize(), mProperties.getType(), mProperties.getFormat(), mProperties.getUsage(), 
			mProperties.getRandomGpuWrite(), mProperties.getUseCounter());
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
}