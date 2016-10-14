//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsHardwareBufferManager.h"
#include "BsVertexData.h"
#include "BsGpuBuffer.h"
#include "BsVertexDeclaration.h"
#include "BsGpuParamBlockBuffer.h"
#include "BsVertexDataDesc.h"

namespace BansheeEngine 
{
    HardwareBufferManager::HardwareBufferManager()
    {
    }

    HardwareBufferManager::~HardwareBufferManager()
    {
    }

	SPtr<VertexDeclaration> HardwareBufferManager::createVertexDeclaration(const SPtr<VertexDataDesc>& desc)
    {
		VertexDeclaration* decl = new (bs_alloc<VertexDeclaration>()) VertexDeclaration(desc->createElements());

		SPtr<VertexDeclaration> declPtr = bs_core_ptr<VertexDeclaration>(decl);
		declPtr->_setThisPtr(declPtr);
		declPtr->initialize();

		return declPtr;
    }

	SPtr<VertexBuffer> HardwareBufferManager::createVertexBuffer(UINT32 vertexSize, UINT32 numVerts, GpuBufferUsage usage, bool streamOut)
	{
		assert (numVerts > 0);

		SPtr<VertexBuffer> vbuf = bs_core_ptr<VertexBuffer>(new (bs_alloc<VertexBuffer>()) VertexBuffer(vertexSize, numVerts, usage, streamOut));
		vbuf->_setThisPtr(vbuf);
		vbuf->initialize();
		return vbuf;
	}

	SPtr<IndexBuffer> HardwareBufferManager::createIndexBuffer(IndexType itype, UINT32 numIndices, GpuBufferUsage usage)
	{
		assert (numIndices > 0);

		SPtr<IndexBuffer> ibuf = bs_core_ptr<IndexBuffer>(new (bs_alloc<IndexBuffer>()) IndexBuffer(itype, numIndices, usage));
		ibuf->_setThisPtr(ibuf);
		ibuf->initialize();
		return ibuf;

	}

	SPtr<GpuParamBlockBuffer> HardwareBufferManager::createGpuParamBlockBuffer(UINT32 size, GpuParamBlockUsage usage)
	{
		SPtr<GpuParamBlockBuffer> paramBlockPtr = bs_core_ptr<GpuParamBlockBuffer>(new (bs_alloc<GpuParamBlockBuffer>()) GpuParamBlockBuffer(size, usage));
		paramBlockPtr->_setThisPtr(paramBlockPtr);
		paramBlockPtr->initialize();
		return paramBlockPtr;
	}

	SPtr<GpuBuffer> HardwareBufferManager::createGpuBuffer(UINT32 elementCount, UINT32 elementSize, 
		GpuBufferType type, GpuBufferFormat format, GpuBufferUsage usage, bool randomGpuWrite, bool useCounter)
	{
		SPtr<GpuBuffer> gbuf = bs_core_ptr<GpuBuffer>(new (bs_alloc<GpuBuffer>()) 
			GpuBuffer(elementCount, elementSize, type, format, usage, randomGpuWrite, useCounter));
		gbuf->_setThisPtr(gbuf);
		gbuf->initialize();

		return gbuf;
	}

	SPtr<GpuParams> HardwareBufferManager::createGpuParams(const GPU_PARAMS_DESC& desc)
    {
		GpuParams* params = new (bs_alloc<GpuParams>()) GpuParams(desc);
		SPtr<GpuParams> paramsPtr = bs_core_ptr<GpuParams>(params);
		paramsPtr->_setThisPtr(paramsPtr);
		paramsPtr->initialize();

		return paramsPtr;
    }

	SPtr<IndexBufferCore> HardwareBufferCoreManager::createIndexBuffer(IndexType itype, UINT32 numIndexes, GpuBufferUsage usage)
	{
		assert(numIndexes > 0);

		SPtr<IndexBufferCore> ibuf = createIndexBufferInternal(itype, numIndexes, usage);
		ibuf->initialize();
		return ibuf;

	}

	SPtr<VertexBufferCore> HardwareBufferCoreManager::createVertexBuffer(UINT32 vertexSize, UINT32 numVerts, GpuBufferUsage usage, bool streamOut)
	{
		assert(numVerts > 0);

		SPtr<VertexBufferCore> vbuf = createVertexBufferInternal(vertexSize, numVerts, usage, streamOut);
		vbuf->initialize();
		return vbuf;
	}

	SPtr<VertexDeclarationCore> HardwareBufferCoreManager::createVertexDeclaration(const SPtr<VertexDataDesc>& desc)
	{
		SPtr<VertexDeclarationCore> declPtr = createVertexDeclarationInternal(desc->createElements());
		declPtr->initialize();

		return declPtr;
	}

	SPtr<GpuParamsCore> HardwareBufferCoreManager::createGpuParams(const GPU_PARAMS_DESC& desc)
    {
		SPtr<GpuParamsCore> params = createGpuParamsInternal(desc);
		params->initialize();

		return params;
    }

	SPtr<VertexDeclarationCore> HardwareBufferCoreManager::createVertexDeclaration(const List<VertexElement>& elements)
	{
		SPtr<VertexDeclarationCore> declPtr = createVertexDeclarationInternal(elements);
		declPtr->initialize();

		return declPtr;
	}

	SPtr<GpuParamBlockBufferCore> HardwareBufferCoreManager::createGpuParamBlockBuffer(UINT32 size, GpuParamBlockUsage usage)
	{
		SPtr<GpuParamBlockBufferCore> paramBlockPtr = createGpuParamBlockBufferInternal(size, usage);
		paramBlockPtr->initialize();

		return paramBlockPtr;
	}

	SPtr<GpuBufferCore> HardwareBufferCoreManager::createGpuBuffer(UINT32 elementCount, UINT32 elementSize,
		GpuBufferType type, GpuBufferFormat format, GpuBufferUsage usage, bool randomGpuWrite, bool useCounter)
	{
		SPtr<GpuBufferCore> gbuf = createGpuBufferInternal(elementCount, elementSize, type, format, usage, randomGpuWrite, 
			useCounter);
		gbuf->initialize();

		return gbuf;
	}

	SPtr<VertexDeclarationCore> HardwareBufferCoreManager::createVertexDeclarationInternal(const List<VertexElement>& elements)
	{
		VertexDeclarationCore* decl = new (bs_alloc<VertexDeclarationCore>()) VertexDeclarationCore(elements);

		SPtr<VertexDeclarationCore> ret = bs_shared_ptr<VertexDeclarationCore>(decl);
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<GpuParamsCore> HardwareBufferCoreManager::createGpuParamsInternal(const GPU_PARAMS_DESC& desc)
    {
		GpuParamsCore* params = new (bs_alloc<GpuParamsCore>()) GpuParamsCore(desc);
		SPtr<GpuParamsCore> paramsPtr = bs_shared_ptr<GpuParamsCore>(params);
		paramsPtr->_setThisPtr(paramsPtr);

		return paramsPtr;
    }
}
