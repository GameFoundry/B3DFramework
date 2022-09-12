//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/BsGpuResourceData.h"
#include "Private/RTTI/BsGpuResourceDataRTTI.h"
#include "CoreThread/BsCoreThread.h"
#include "Error/BsException.h"

namespace
{
	void VerifyLockAndThread(const bs::GpuResourceData *data)
	{
		using namespace bs;
#if !BS_FORCE_SINGLETHREADED_RENDERING
		if(data->isLocked())
		{
			if(BS_THREAD_CURRENT_ID != CoreThread::instance().getCoreThreadId())
				BS_EXCEPT(InternalErrorException, "You are not allowed to access buffer data from non-core thread when the buffer is locked.");
		}
#endif
	}
} // end of anonymous namespace

namespace bs
{
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
		verifyLockAndThread(this);
		return mData;
	}

	void GpuResourceData::SetData(UPtr<UINT8[]> &data)
	{
		verifyLockAndThread(this);

		freeInternalBuffer();

		mData = data.release();
		mOwnsData = true;
	}

	void GpuResourceData::AllocateInternalBuffer()
	{
		allocateInternalBuffer(getInternalBufferSize());
	}

	void GpuResourceData::AllocateInternalBuffer(UINT32 size)
	{
		verifyLockAndThread(this);

		freeInternalBuffer();

		mData = (UINT8*)bs_alloc(size);
		mOwnsData = true;
	}

	void GpuResourceData::FreeInternalBuffer()
	{
		if(mData == nullptr || !mOwnsData)
			return;

		verifyLockAndThread(this);

		bs_free(mData);
		mData = nullptr;
	}

	void GpuResourceData::SetExternalBuffer(UINT8* data)
	{
		verifyLockAndThread(this);

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
		return GpuResourceDataRTTI::Instance();
	}

	RTTITypeBase* GpuResourceData::getRTTI() const
	{
		return GpuResourceData::GetRTTIStatic();
	}
}
