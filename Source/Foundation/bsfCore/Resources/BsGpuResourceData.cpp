//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/BsGpuResourceData.h"
#include "Private/RTTI/BsGpuResourceDataRTTI.h"
#include "CoreObject/BsRenderThread.h"
#include "Error/BsException.h"

namespace
{
void VerifyLockAndThread(const bs::GpuResourceData* data)
{
	using namespace bs;
	if(data->IsLocked())
	{
		if(B3D_CURRENT_THREAD_ID != RenderThread::Instance().GetThreadId())
			B3D_EXCEPT(InternalErrorException, "You are not allowed to access buffer data from non-render thread when the buffer is locked.");
	}
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
	VerifyLockAndThread(this);
	return mData;
}

void GpuResourceData::SetData(UPtr<u8[]>& data)
{
	VerifyLockAndThread(this);

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
	VerifyLockAndThread(this);

	FreeInternalBuffer();

	mData = (u8*)B3DAllocate(size);
	mOwnsData = true;
}

void GpuResourceData::FreeInternalBuffer()
{
	if(mData == nullptr || !mOwnsData)
		return;

	VerifyLockAndThread(this);

	B3DFree(mData);
	mData = nullptr;
}

void GpuResourceData::SetExternalBuffer(u8* data)
{
	VerifyLockAndThread(this);

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
