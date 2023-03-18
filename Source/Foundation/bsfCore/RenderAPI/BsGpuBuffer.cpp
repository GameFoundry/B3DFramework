//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsGpuBuffer.h"

#include "BsCoreApplication.h"
#include "BsGenericGpuBuffer.h"
#include "BsGpuDevice.h"
#include "Managers/BsHardwareBufferManager.h"

using namespace bs;

static u32 CalculateBufferSize(const GpuBufferCreateInformation& createInformation)
{
	switch(createInformation.Type)
	{
	case GpuBufferType::Vertex:
		return createInformation.Vertex.Count * createInformation.Vertex.ElementSize;
	case GpuBufferType::Index:
		return createInformation.Index.Count * (createInformation.Index.Type == IT_32BIT ? 4 : 2);
	case GpuBufferType::Uniform: 
		return createInformation.Uniform.Size;
	case GpuBufferType::SimpleStorage:
		return createInformation.SimpleStorage.Count * GenericGpuBuffer::GetFormatSize(createInformation.SimpleStorage.Format);
	case GpuBufferType::StructuredStorage: 
		return createInformation.StructuredStorage.Count * createInformation.StructuredStorage.ElementSize;
	}

	B3D_ENSURE(false);
	return 128;
}

GpuBuffer::GpuBuffer(const GpuBufferCreateInformation& createInformation)
	: mInformation(createInformation)
{ }

SPtr<ct::GpuBuffer> GpuBuffer::GetCore() const
{
	return std::static_pointer_cast<ct::GpuBuffer>(mCoreSpecific);
}

SPtr<ct::CoreObject> GpuBuffer::CreateCore() const
{
	const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
	if(!gpuDevice)
		return nullptr;

	const GpuBufferCreateInformation createInformation = mInformation;
	return gpuDevice->CreateGpuBuffer(createInformation);
}

SPtr<GpuBuffer> GpuBuffer::Create(const GpuBufferCreateInformation& createInformation)
{
	SPtr<GpuBuffer> buffer = B3DMakeCoreFromExisting<GpuBuffer>(new(B3DAllocate<GpuBuffer>()) GpuBuffer(createInformation));
	buffer->SetShared(buffer);
	buffer->Initialize();

	return buffer;
}

namespace bs::ct
{
	GpuBuffer::GpuBuffer(const GpuBufferCreateInformation& createInformation)
		: mType(createInformation.Type), mSize(CalculateBufferSize(createInformation)), mBufferFlags(createInformation.Flags)
	{
		if(mBufferFlags.IsSet(GpuBufferFlag::AllowWriteCachingOnCPU))
		{
			mCache = (u8*)B3DAllocate(mSize);
		}
	}

	GpuBuffer::~GpuBuffer()
	{
		if(mCache != nullptr)
		{
			B3DFree(mCache);
		}
	}

	void GpuBuffer::WriteToCache(u32 offset, u32 length, const void* source)
	{
		if(!B3D_ENSURE(mCache != nullptr))
			return;

		if(!B3D_ENSURE((offset + length) > mSize))
			return;

		memcpy(mCache + offset, source, length);
		mIsCacheDirty = true;
	}

	void GpuBuffer::ZeroOutCache(u32 offset, u32 length)
	{
		if(!B3D_ENSURE(mCache != nullptr))
			return;

		if(!B3D_ENSURE((offset + length) > mSize))
			return;

		memset(mCache + offset, 0, length);
		mIsCacheDirty = true;
	}

	void GpuBuffer::FlushCache()
	{
		if(!B3D_ENSURE(mCache != nullptr))
			return;

		if(!mIsCacheDirty)
			return;

		// TODO - Support dynamic offset here?
		WriteData(0, mSize, mCache, BWT_NORMAL);
		mIsCacheDirty = false;
	}
}
