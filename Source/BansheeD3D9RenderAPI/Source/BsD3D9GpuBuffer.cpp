//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D9GpuBuffer.h"
#include "BsDebug.h"

namespace BansheeEngine
{
	D3D9GpuBufferCore::D3D9GpuBufferCore(UINT32 elementCount, UINT32 elementSize, GpuBufferType type,
		GpuBufferFormat format, GpuBufferUsage usage, bool randomGpuWrite, bool useCounter)
		: GpuBufferCore(elementCount, elementSize, type, format, usage, randomGpuWrite, useCounter)
	{
	}

	D3D9GpuBufferCore::~D3D9GpuBufferCore()
	{ 
		clearBufferViews();
	}

	void D3D9GpuBufferCore::initialize()
	{
		LOGWRN("Generic buffers are not supported in DirectX 9. Creating a dummy buffer. All operations on it will either be no-op or return a nullptr.");

		GpuBufferCore::initialize();
	}

	void* D3D9GpuBufferCore::lock(UINT32 offset, UINT32 length, GpuLockOptions options)
	{
		return nullptr;
	}

	void D3D9GpuBufferCore::unlock()
	{
	}

	void D3D9GpuBufferCore::readData(UINT32 offset, UINT32 length, void* pDest)
	{
	}

	void D3D9GpuBufferCore::writeData(UINT32 offset, UINT32 length, const void* pSource, BufferWriteType writeFlags)
	{
	}

	void D3D9GpuBufferCore::copyData(GpuBufferCore& srcBuffer, UINT32 srcOffset,
		UINT32 dstOffset, UINT32 length, bool discardWholeBuffer)
	{
	}

	GpuBufferView* D3D9GpuBufferCore::createView()
	{
		return nullptr;
	}

	void D3D9GpuBufferCore::destroyView(GpuBufferView* view)
	{
	}
}