//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsHardwareBufferManager.h"
#include "BsVertexData.h"
#include "BsGpuBuffer.h"
#include "BsVertexDeclaration.h"
#include "BsGpuParamBlockBuffer.h"
#include "BsVertexDataDesc.h"
#include "BsGpuParams.h"

namespace bs 
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

	SPtr<VertexBuffer> HardwareBufferManager::createVertexBuffer(const VERTEX_BUFFER_DESC& desc)
	{
		SPtr<VertexBuffer> vbuf = bs_core_ptr<VertexBuffer>(new (bs_alloc<VertexBuffer>()) VertexBuffer(desc));
		vbuf->_setThisPtr(vbuf);
		vbuf->initialize();
		return vbuf;
	}

	SPtr<IndexBuffer> HardwareBufferManager::createIndexBuffer(const INDEX_BUFFER_DESC& desc)
	{
		SPtr<IndexBuffer> ibuf = bs_core_ptr<IndexBuffer>(new (bs_alloc<IndexBuffer>()) IndexBuffer(desc));
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

	SPtr<GpuBuffer> HardwareBufferManager::createGpuBuffer(const GPU_BUFFER_DESC& desc)
	{
		SPtr<GpuBuffer> gbuf = bs_core_ptr<GpuBuffer>(new (bs_alloc<GpuBuffer>()) GpuBuffer(desc));
		gbuf->_setThisPtr(gbuf);
		gbuf->initialize();

		return gbuf;
	}

	SPtr<GpuParams> HardwareBufferManager::createGpuParams(const SPtr<GpuPipelineParamInfo>& paramInfo)
    {
		GpuParams* params = new (bs_alloc<GpuParams>()) GpuParams(paramInfo);
		SPtr<GpuParams> paramsPtr = bs_core_ptr<GpuParams>(params);
		paramsPtr->_setThisPtr(paramsPtr);
		paramsPtr->initialize();

		return paramsPtr;
    }

	SPtr<IndexBufferCore> HardwareBufferCoreManager::createIndexBuffer(const INDEX_BUFFER_DESC& desc, 
		GpuDeviceFlags deviceMask)
	{
		SPtr<IndexBufferCore> ibuf = createIndexBufferInternal(desc, deviceMask);
		ibuf->initialize();
		return ibuf;

	}

	SPtr<VertexBufferCore> HardwareBufferCoreManager::createVertexBuffer(const VERTEX_BUFFER_DESC& desc, 
		GpuDeviceFlags deviceMask)
	{
		SPtr<VertexBufferCore> vbuf = createVertexBufferInternal(desc, deviceMask);
		vbuf->initialize();
		return vbuf;
	}

	SPtr<VertexDeclarationCore> HardwareBufferCoreManager::createVertexDeclaration(const SPtr<VertexDataDesc>& desc, 
		GpuDeviceFlags deviceMask)
	{
		SPtr<VertexDeclarationCore> declPtr = createVertexDeclarationInternal(desc->createElements(), deviceMask);
		declPtr->initialize();

		return declPtr;
	}

	SPtr<GpuParamsCore> HardwareBufferCoreManager::createGpuParams(const SPtr<GpuPipelineParamInfoCore>& paramInfo,
																   GpuDeviceFlags deviceMask)
    {
		SPtr<GpuParamsCore> params = createGpuParamsInternal(paramInfo, deviceMask);
		params->initialize();

		return params;
    }

	SPtr<VertexDeclarationCore> HardwareBufferCoreManager::createVertexDeclaration(const List<VertexElement>& elements,
		GpuDeviceFlags deviceMask)
	{
		SPtr<VertexDeclarationCore> declPtr = createVertexDeclarationInternal(elements, deviceMask);
		declPtr->initialize();

		return declPtr;
	}

	SPtr<GpuParamBlockBufferCore> HardwareBufferCoreManager::createGpuParamBlockBuffer(UINT32 size, 
		GpuParamBlockUsage usage, GpuDeviceFlags deviceMask)
	{
		SPtr<GpuParamBlockBufferCore> paramBlockPtr = createGpuParamBlockBufferInternal(size, usage, deviceMask);
		paramBlockPtr->initialize();

		return paramBlockPtr;
	}

	SPtr<GpuBufferCore> HardwareBufferCoreManager::createGpuBuffer(const GPU_BUFFER_DESC& desc,
		GpuDeviceFlags deviceMask)
	{
		SPtr<GpuBufferCore> gbuf = createGpuBufferInternal(desc, deviceMask);
		gbuf->initialize();

		return gbuf;
	}

	SPtr<VertexDeclarationCore> HardwareBufferCoreManager::createVertexDeclarationInternal(
		const List<VertexElement>& elements, GpuDeviceFlags deviceMask)
	{
		VertexDeclarationCore* decl = new (bs_alloc<VertexDeclarationCore>()) VertexDeclarationCore(elements, deviceMask);

		SPtr<VertexDeclarationCore> ret = bs_shared_ptr<VertexDeclarationCore>(decl);
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<GpuParamsCore> HardwareBufferCoreManager::createGpuParamsInternal(
		const SPtr<GpuPipelineParamInfoCore>& paramInfo, GpuDeviceFlags deviceMask)
    {
		GpuParamsCore* params = new (bs_alloc<GpuParamsCore>()) GpuParamsCore(paramInfo, deviceMask);
		SPtr<GpuParamsCore> paramsPtr = bs_shared_ptr<GpuParamsCore>(params);
		paramsPtr->_setThisPtr(paramsPtr);

		return paramsPtr;
    }
}
