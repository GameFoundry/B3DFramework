//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsHardwareBufferManager.h"
#include "RenderAPI/BsVertexData.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "RenderAPI/BsVertexDeclaration.h"
#include "RenderAPI/BsGpuParamBlockBuffer.h"
#include "RenderAPI/BsVertexDataDesc.h"
#include "RenderAPI/BsGpuParams.h"

namespace bs
{
	SPtr<VertexDeclaration> HardwareBufferManager::CreateVertexDeclaration(const SPtr<VertexDataDesc>& desc)
	{
		VertexDeclaration* decl = new (bs_alloc<VertexDeclaration>()) VertexDeclaration(desc->CreateElements());

		SPtr<VertexDeclaration> declPtr = bs_core_ptr<VertexDeclaration>(decl);
		declPtr->_setThisPtr(declPtr);
		declPtr->Initialize();

		return declPtr;
	}

	SPtr<VertexBuffer> HardwareBufferManager::CreateVertexBuffer(const VERTEX_BUFFER_DESC& desc)
	{
		SPtr<VertexBuffer> vbuf = bs_core_ptr<VertexBuffer>(new (bs_alloc<VertexBuffer>()) VertexBuffer(desc));
		vbuf->_setThisPtr(vbuf);
		vbuf->Initialize();
		return vbuf;
	}

	SPtr<IndexBuffer> HardwareBufferManager::CreateIndexBuffer(const INDEX_BUFFER_DESC& desc)
	{
		SPtr<IndexBuffer> ibuf = bs_core_ptr<IndexBuffer>(new (bs_alloc<IndexBuffer>()) IndexBuffer(desc));
		ibuf->_setThisPtr(ibuf);
		ibuf->Initialize();
		return ibuf;

	}

	SPtr<GpuParamBlockBuffer> HardwareBufferManager::CreateGpuParamBlockBuffer(UINT32 size, GpuBufferUsage usage)
	{
		SPtr<GpuParamBlockBuffer> paramBlockPtr = bs_core_ptr<GpuParamBlockBuffer>(new (bs_alloc<GpuParamBlockBuffer>()) GpuParamBlockBuffer(size, usage));
		paramBlockPtr->_setThisPtr(paramBlockPtr);
		paramBlockPtr->Initialize();
		return paramBlockPtr;
	}

	SPtr<GpuBuffer> HardwareBufferManager::CreateGpuBuffer(const GPU_BUFFER_DESC& desc)
	{
		SPtr<GpuBuffer> gbuf = bs_core_ptr<GpuBuffer>(new (bs_alloc<GpuBuffer>()) GpuBuffer(desc));
		gbuf->_setThisPtr(gbuf);
		gbuf->Initialize();

		return gbuf;
	}

	SPtr<GpuParams> HardwareBufferManager::CreateGpuParams(const SPtr<GpuPipelineParamInfo>& paramInfo)
	{
		GpuParams* params = new (bs_alloc<GpuParams>()) GpuParams(paramInfo);
		SPtr<GpuParams> paramsPtr = bs_core_ptr<GpuParams>(params);
		paramsPtr->_setThisPtr(paramsPtr);
		paramsPtr->Initialize();

		return paramsPtr;
	}

	namespace ct
	{

	HardwareBufferManager::VertexDeclarationKey::VertexDeclarationKey(const Vector<VertexElement>& elements)
		:elements(elements)
	{ }


	size_t HardwareBufferManager::VertexDeclarationKey::HashFunction::operator()(const VertexDeclarationKey& v) const
	{
		size_t hash = 0;
		for(auto& entry : v.elements)
			bs_hash_combine(hash, VertexElement::getHash(entry));

		return hash;
	}

	bool HardwareBufferManager::VertexDeclarationKey::EqualFunction::operator()(const VertexDeclarationKey& lhs,
		const VertexDeclarationKey& rhs) const
	{
		if (lhs.elements.Size() != rhs.elements.size())
			return false;

		size_t numElements = lhs.elements.Size();
		auto iterLeft = lhs.elements.Begin();
		auto iterRight = rhs.elements.Begin();
		for(size_t i = 0; i < numElements; i++)
		{
			if (*iterLeft != *iterRight)
				return false;

			++iterLeft;
			++iterRight;
		}

		return true;
	}

	SPtr<IndexBuffer> HardwareBufferManager::createIndexBuffer(const INDEX_BUFFER_DESC& desc,
		GpuDeviceFlags deviceMask)
	{
		SPtr<IndexBuffer> ibuf = createIndexBufferInternal(desc, deviceMask);
		ibuf->Initialize();
		return ibuf;

	}

	SPtr<VertexBuffer> HardwareBufferManager::createVertexBuffer(const VERTEX_BUFFER_DESC& desc,
		GpuDeviceFlags deviceMask)
	{
		SPtr<VertexBuffer> vbuf = createVertexBufferInternal(desc, deviceMask);
		vbuf->Initialize();
		return vbuf;
	}

	SPtr<VertexDeclaration> HardwareBufferManager::createVertexDeclaration(const SPtr<VertexDataDesc>& desc,
		GpuDeviceFlags deviceMask)
	{
		Vector<VertexElement> elements = desc->CreateElements();

		return CreateVertexDeclaration(elements, deviceMask);
	}

	SPtr<GpuParams> HardwareBufferManager::createGpuParams(const SPtr<GpuPipelineParamInfo>& paramInfo,
																   GpuDeviceFlags deviceMask)
	{
		SPtr<GpuParams> params = createGpuParamsInternal(paramInfo, deviceMask);
		params->Initialize();

		return params;
	}

	SPtr<VertexDeclaration> HardwareBufferManager::createVertexDeclaration(const Vector<VertexElement>& elements,
		GpuDeviceFlags deviceMask)
	{
		VertexDeclarationKey Key(elements);

		auto iterFind = mCachedDeclarations.Find(key);
		if (iterFind != mCachedDeclarations.End())
			return iterFind->second;

		SPtr<VertexDeclaration> declPtr = createVertexDeclarationInternal(elements, deviceMask);
		declPtr->Initialize();

		mCachedDeclarations[key] = declPtr;
		return declPtr;
	}

	SPtr<GpuParamBlockBuffer> HardwareBufferManager::createGpuParamBlockBuffer(UINT32 size,
		GpuBufferUsage usage, GpuDeviceFlags deviceMask)
	{
		SPtr<GpuParamBlockBuffer> paramBlockPtr = createGpuParamBlockBufferInternal(size, usage, deviceMask);
		paramBlockPtr->Initialize();

		return paramBlockPtr;
	}

	SPtr<GpuBuffer> HardwareBufferManager::createGpuBuffer(const GPU_BUFFER_DESC& desc,
		GpuDeviceFlags deviceMask)
	{
		SPtr<GpuBuffer> gbuf = createGpuBufferInternal(desc, deviceMask);
		gbuf->Initialize();

		return gbuf;
	}

	SPtr<GpuBuffer> HardwareBufferManager::createGpuBuffer(const GPU_BUFFER_DESC& desc,
		SPtr<HardwareBuffer> underlyingBuffer)
	{
		SPtr<GpuBuffer> gbuf = createGpuBufferInternal(desc, std::move(underlyingBuffer));
		gbuf->Initialize();

		return gbuf;
	}

	SPtr<VertexDeclaration> HardwareBufferManager::createVertexDeclarationInternal(
		const Vector<VertexElement>& elements, GpuDeviceFlags deviceMask)
	{
		VertexDeclaration* decl = new (bs_alloc<VertexDeclaration>()) VertexDeclaration(elements, deviceMask);

		SPtr<VertexDeclaration> ret = bs_shared_ptr<VertexDeclaration>(decl);
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<GpuParams> HardwareBufferManager::createGpuParamsInternal(
		const SPtr<GpuPipelineParamInfo>& paramInfo, GpuDeviceFlags deviceMask)
	{
		GpuParams* params = new (bs_alloc<GpuParams>()) GpuParams(paramInfo, deviceMask);
		SPtr<GpuParams> paramsPtr = bs_shared_ptr<GpuParams>(params);
		paramsPtr->_setThisPtr(paramsPtr);

		return paramsPtr;
	}
	}
}
