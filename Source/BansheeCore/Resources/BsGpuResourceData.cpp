//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Resources/BsGpuResourceData.h"
#include "Private/RTTI/BsGpuResourceDataRTTI.h"
#include "CoreThread/BsCoreThread.h"
#include "Error/BsException.h"

namespace bs
{
	GpuResourceData::GpuResourceData()
		:mData(nullptr), mOwnsData(false), mLocked(false)
	{

	}

	GpuResourceData::GpuResourceData(const GpuResourceData& copy)
	{
		mData = copy.mData;
		mLocked = copy.mLocked; // TODO - This should be shared by all copies pointing to the same data?
		mOwnsData = false;
	}

	GpuResourceData::~GpuResourceData()
	{
		freeInternalBuffer();
	}

	GpuResourceData& GpuResourceData::operator=(const GpuResourceData& rhs)
	{
		mData = rhs.mData;
		mLocked = rhs.mLocked; // TODO - This should be shared by all copies pointing to the same data?
		mOwnsData = false;

		return *this;
	}

	UINT8* GpuResourceData::getData() const
	{
#if !BS_FORCE_SINGLETHREADED_RENDERING
		if(mLocked)
		{
			if(BS_THREAD_CURRENT_ID != CoreThread::instance().getCoreThreadId())
				BS_EXCEPT(InternalErrorException, "You are not allowed to access buffer data from non-core thread when the buffer is locked.");
		}
#endif

		return mData;
	}

	void GpuResourceData::allocateInternalBuffer()
	{
		allocateInternalBuffer(getInternalBufferSize());
	}

	void GpuResourceData::allocateInternalBuffer(UINT32 size)
	{
#if !BS_FORCE_SINGLETHREADED_RENDERING
		if(mLocked)
		{
			if(BS_THREAD_CURRENT_ID != CoreThread::instance().getCoreThreadId())
				BS_EXCEPT(InternalErrorException, "You are not allowed to access buffer data from non-core thread when the buffer is locked.");
		}
#endif

		freeInternalBuffer();

		mData = (UINT8*)bs_alloc(size);
		mOwnsData = true;
	}

	void GpuResourceData::freeInternalBuffer()
	{
		if(mData == nullptr || !mOwnsData)
			return;

#if !BS_FORCE_SINGLETHREADED_RENDERING
		if(mLocked)
		{
			if(BS_THREAD_CURRENT_ID != CoreThread::instance().getCoreThreadId())
				BS_EXCEPT(InternalErrorException, "You are not allowed to access buffer data from non-core thread when the buffer is locked.");
		}
#endif

		bs_free(mData);
		mData = nullptr;
	}

	void GpuResourceData::setExternalBuffer(UINT8* data)
	{
#if !BS_FORCE_SINGLETHREADED_RENDERING
		if(mLocked)
		{
			if(BS_THREAD_CURRENT_ID != CoreThread::instance().getCoreThreadId())
				BS_EXCEPT(InternalErrorException, "You are not allowed to access buffer data from non-core thread when the buffer is locked.");
		}
#endif

		freeInternalBuffer();

		mData = data;
		mOwnsData = false;
	}

	void GpuResourceData::_lock() const
	{
		mLocked = true;
	}

	void GpuResourceData::_unlock() const
	{
		mLocked = false;
	}

	/************************************************************************/
	/* 								SERIALIZATION                      		*/
	/************************************************************************/

	RTTITypeBase* GpuResourceData::getRTTIStatic()
	{
		return GpuResourceDataRTTI::instance();
	}

	RTTITypeBase* GpuResourceData::getRTTI() const
	{
		return GpuResourceData::getRTTIStatic();
	}
}