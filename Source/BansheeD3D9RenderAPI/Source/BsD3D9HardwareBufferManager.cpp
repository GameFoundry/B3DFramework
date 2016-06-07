//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D9HardwareBufferManager.h"
#include "BsD3D9VertexBuffer.h"
#include "BsD3D9IndexBuffer.h"
#include "BsD3D9VertexDeclaration.h"
#include "BsD3D9GpuBuffer.h"
#include "BsGpuParamBlockBuffer.h"
#include "BsException.h"

namespace BansheeEngine 
{
	SPtr<VertexBufferCore> D3D9HardwareBufferCoreManager::createVertexBufferInternal(UINT32 vertexSize, UINT32 numVerts, GpuBufferUsage usage, bool streamOut)
	{
		assert(numVerts > 0);

		SPtr<D3D9VertexBufferCore> ret = bs_shared_ptr_new<D3D9VertexBufferCore>(vertexSize, numVerts, usage, streamOut);
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<IndexBufferCore> D3D9HardwareBufferCoreManager::createIndexBufferInternal(IndexType itype, UINT32 numIndices, GpuBufferUsage usage)
	{
		assert(numIndices > 0);

		SPtr<D3D9IndexBufferCore> ret = bs_shared_ptr_new<D3D9IndexBufferCore>(itype, numIndices, usage);
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<VertexDeclarationCore> D3D9HardwareBufferCoreManager::createVertexDeclarationInternal(const List<VertexElement>& elements)
	{
		D3D9VertexDeclarationCore* decl = new (bs_alloc<D3D9VertexDeclarationCore>()) D3D9VertexDeclarationCore(elements);
		
		SPtr<D3D9VertexDeclarationCore> declPtr = bs_shared_ptr<D3D9VertexDeclarationCore>(decl);
		declPtr->_setThisPtr(declPtr);

		return declPtr;
	}

	SPtr<GpuParamBlockBufferCore> D3D9HardwareBufferCoreManager::createGpuParamBlockBufferInternal(UINT32 size, GpuParamBlockUsage usage)
	{
		GenericGpuParamBlockBufferCore* paramBlockBuffer = new (bs_alloc<GenericGpuParamBlockBufferCore>()) GenericGpuParamBlockBufferCore(size, usage);

		SPtr<GpuParamBlockBufferCore> paramBlockBufferPtr = bs_shared_ptr<GenericGpuParamBlockBufferCore>(paramBlockBuffer);
		paramBlockBufferPtr->_setThisPtr(paramBlockBufferPtr);

		return paramBlockBufferPtr;
	}

	SPtr<GpuBufferCore> D3D9HardwareBufferCoreManager::createGpuBufferInternal(UINT32 elementCount, UINT32 elementSize,
		GpuBufferType type, GpuBufferFormat format, GpuBufferUsage usage, bool randomGpuWrite, bool useCounter)
	{
		D3D9GpuBufferCore* buffer = new (bs_alloc<D3D9GpuBufferCore>()) D3D9GpuBufferCore(elementCount, elementSize, type, format, usage, randomGpuWrite, useCounter);

		SPtr<GpuBufferCore> bufferPtr = bs_shared_ptr<D3D9GpuBufferCore>(buffer);
		bufferPtr->_setThisPtr(bufferPtr);

		return bufferPtr;
	}
}
