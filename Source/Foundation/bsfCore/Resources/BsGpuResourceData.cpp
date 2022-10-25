//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/BsGpuResourceData.h"
#include "Private/RTTI/BsGpuResourceDataRTTI.h"
#include "CoreThread/BsCoreThread.h"
#include "Error/BsException.h"

namespace
{
void verifyLockAndThread(const bs::GpuResourceData* data)
{
	using namespace bs;
#if !BS_FORCE_SINGLETHREADED_RENDERING
	if(data->IsLocked())
	{
		if(BS_THREAD_CURRENT_ID != CoreThread::Instance().GetCoreThreadId())
			BS_EXCEPT(InternalErrorException, "You are not allowed to access buffer data from non-core thread when the buffer is locked.");
	}
#endif
}
} // end of anonymous namespace

using namespace bs;

GpuResourceData::GpuResourceData(const GpuResourceData& copy)
{
	mData = copy.mData;
	mLocked = copy.mLocked; // TODO - This should be shared by all copies pointing to the same data?
	mOwnsData = false;
}

GpuResourceData::~GpuResourceData()
{
	FreeInternalBuffer();
}

GpuResourceData& GpuResourceData::operator=(const GpuResourceData& rhs)
{
	mData = rhs.mData;
	mLocked = rhs.mLocked; // TODO - This should be shared by all copies pointing to the same data?
	mOwnsData = false;

	return *this;
}

u8* GpuResourceData::GetData() const
{
	verifyLockAndThread(this);
	return mData;
}

void GpuResourceData::SetData(UPtr<u8[]>& data)
{
	verifyLockAndThread(this);

	FreeInternalBuffer();

	mData = data.release();
	mOwnsData = true;
}

void GpuResourceData::AllocateInternalBuffer()
{
	AllocateInternalBuffer(GetInternalBufferSize());
}

void GpuResourceData::AllocateInternalBuffer(u32 size)
{
	verifyLockAndThread(this);

	FreeInternalBuffer();

	mData = (u8*)bs_alloc(size);
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

void GpuResourceData::SetExternalBuffer(u8* data)
{
	verifyLockAndThread(this);

	FreeInternalBuffer();

	mData = data;
	mOwnsData = false;
}

void GpuResourceData::LockInternal() const
{
	mLocked = true;
}

void GpuResourceData::UnlockInternal() const
{
	mLocked = false;
}

/************************************************************************/
/* 								SERIALIZATION                      		*/
/************************************************************************/

RTTITypeBase* GpuResourceData::GetRttiStatic()
{
	return GpuResourceDataRTTI::Instance();
}

RTTITypeBase* GpuResourceData::GetRtti() const
{
	return GpuResourceData::GetRttiStatic();
}
