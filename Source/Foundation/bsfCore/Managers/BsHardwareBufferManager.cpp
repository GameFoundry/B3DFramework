//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsHardwareBufferManager.h"
#include "RenderAPI/BsVertexData.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "RenderAPI/BsVertexDeclaration.h"
#include "RenderAPI/BsGpuParamBlockBuffer.h"
#include "RenderAPI/BsVertexDataDesc.h"
#include "RenderAPI/BsGpuParams.h"

using namespace bs;

SPtr<VertexDeclaration> HardwareBufferManager::CreateVertexDeclaration(const SPtr<VertexDataDesc>& desc)
{
	VertexDeclaration* decl = new(bs_alloc<VertexDeclaration>()) VertexDeclaration(desc->CreateElements());

	SPtr<VertexDeclaration> declPtr = bs_core_ptr<VertexDeclaration>(decl);
	declPtr->SetThisPtrInternal(declPtr);
	declPtr->Initialize();

	return declPtr;
}

SPtr<VertexBuffer> HardwareBufferManager::CreateVertexBuffer(const VERTEX_BUFFER_DESC& desc)
{
	SPtr<VertexBuffer> vbuf = bs_core_ptr<VertexBuffer>(new(bs_alloc<VertexBuffer>()) VertexBuffer(desc));
	vbuf->SetThisPtrInternal(vbuf);
	vbuf->Initialize();
	return vbuf;
}

SPtr<IndexBuffer> HardwareBufferManager::CreateIndexBuffer(const INDEX_BUFFER_DESC& desc)
{
	SPtr<IndexBuffer> ibuf = bs_core_ptr<IndexBuffer>(new(bs_alloc<IndexBuffer>()) IndexBuffer(desc));
	ibuf->SetThisPtrInternal(ibuf);
	ibuf->Initialize();
	return ibuf;
}

SPtr<GpuParamBlockBuffer> HardwareBufferManager::CreateGpuParamBlockBuffer(u32 size, GpuBufferUsage usage)
{
	SPtr<GpuParamBlockBuffer> paramBlockPtr = bs_core_ptr<GpuParamBlockBuffer>(new(bs_alloc<GpuParamBlockBuffer>()) GpuParamBlockBuffer(size, usage));
	paramBlockPtr->SetThisPtrInternal(paramBlockPtr);
	paramBlockPtr->Initialize();
	return paramBlockPtr;
}

SPtr<GpuBuffer> HardwareBufferManager::CreateGpuBuffer(const GPU_BUFFER_DESC& desc)
{
	SPtr<GpuBuffer> gbuf = bs_core_ptr<GpuBuffer>(new(bs_alloc<GpuBuffer>()) GpuBuffer(desc));
	gbuf->SetThisPtrInternal(gbuf);
	gbuf->Initialize();

	return gbuf;
}

SPtr<GpuParams> HardwareBufferManager::CreateGpuParams(const SPtr<GpuPipelineParamInfo>& paramInfo)
{
	GpuParams* params = new(bs_alloc<GpuParams>()) GpuParams(paramInfo);
	SPtr<GpuParams> paramsPtr = bs_core_ptr<GpuParams>(params);
	paramsPtr->SetThisPtrInternal(paramsPtr);
	paramsPtr->Initialize();

	return paramsPtr;
}

namespace bs { namespace ct
{

HardwareBufferManager::VertexDeclarationKey::VertexDeclarationKey(const Vector<VertexElement>& elements)
	: Elements(elements)
{}

size_t HardwareBufferManager::VertexDeclarationKey::HashFunction::operator()(const VertexDeclarationKey& v) const
{
	size_t hash = 0;
	for(auto& entry : v.Elements)
		bs_hash_combine(hash, VertexElement::GetHash(entry));

	return hash;
}

bool HardwareBufferManager::VertexDeclarationKey::EqualFunction::operator()(const VertexDeclarationKey& lhs, const VertexDeclarationKey& rhs) const
{
	if(lhs.Elements.size() != rhs.Elements.size())
		return false;

	size_t numElements = lhs.Elements.size();
	auto iterLeft = lhs.Elements.begin();
	auto iterRight = rhs.Elements.begin();
	for(size_t i = 0; i < numElements; i++)
	{
		if(*iterLeft != *iterRight)
			return false;

		++iterLeft;
		++iterRight;
	}

	return true;
}

SPtr<IndexBuffer> HardwareBufferManager::CreateIndexBuffer(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
{
	SPtr<IndexBuffer> ibuf = CreateIndexBufferInternal(desc, deviceMask);
	ibuf->Initialize();
	return ibuf;
}

SPtr<VertexBuffer> HardwareBufferManager::CreateVertexBuffer(const VERTEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
{
	SPtr<VertexBuffer> vbuf = CreateVertexBufferInternal(desc, deviceMask);
	vbuf->Initialize();
	return vbuf;
}

SPtr<VertexDeclaration> HardwareBufferManager::CreateVertexDeclaration(const SPtr<VertexDataDesc>& desc, GpuDeviceFlags deviceMask)
{
	Vector<VertexElement> elements = desc->CreateElements();

	return CreateVertexDeclaration(elements, deviceMask);
}

SPtr<GpuParams> HardwareBufferManager::CreateGpuParams(const SPtr<GpuPipelineParamInfo>& paramInfo, GpuDeviceFlags deviceMask)
{
	SPtr<GpuParams> params = CreateGpuParamsInternal(paramInfo, deviceMask);
	params->Initialize();

	return params;
}

SPtr<VertexDeclaration> HardwareBufferManager::CreateVertexDeclaration(const Vector<VertexElement>& elements, GpuDeviceFlags deviceMask)
{
	VertexDeclarationKey key(elements);

	auto iterFind = mCachedDeclarations.find(key);
	if(iterFind != mCachedDeclarations.end())
		return iterFind->second;

	SPtr<VertexDeclaration> declPtr = CreateVertexDeclarationInternal(elements, deviceMask);
	declPtr->Initialize();

	mCachedDeclarations[key] = declPtr;
	return declPtr;
}

SPtr<GpuParamBlockBuffer> HardwareBufferManager::CreateGpuParamBlockBuffer(u32 size, GpuBufferUsage usage, GpuDeviceFlags deviceMask)
{
	SPtr<GpuParamBlockBuffer> paramBlockPtr = CreateGpuParamBlockBufferInternal(size, usage, deviceMask);
	paramBlockPtr->Initialize();

	return paramBlockPtr;
}

SPtr<GpuBuffer> HardwareBufferManager::CreateGpuBuffer(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
{
	SPtr<GpuBuffer> gbuf = CreateGpuBufferInternal(desc, deviceMask);
	gbuf->Initialize();

	return gbuf;
}

SPtr<GpuBuffer> HardwareBufferManager::CreateGpuBuffer(const GPU_BUFFER_DESC& desc, SPtr<HardwareBuffer> underlyingBuffer)
{
	SPtr<GpuBuffer> gbuf = CreateGpuBufferInternal(desc, std::move(underlyingBuffer));
	gbuf->Initialize();

	return gbuf;
}

SPtr<VertexDeclaration> HardwareBufferManager::CreateVertexDeclarationInternal(
	const Vector<VertexElement>& elements, GpuDeviceFlags deviceMask)
{
	VertexDeclaration* decl = new(bs_alloc<VertexDeclaration>()) VertexDeclaration(elements, deviceMask);

	SPtr<VertexDeclaration> ret = bs_shared_ptr<VertexDeclaration>(decl);
	ret->SetThisPtrInternal(ret);

	return ret;
}

SPtr<GpuParams> HardwareBufferManager::CreateGpuParamsInternal(
	const SPtr<GpuPipelineParamInfo>& paramInfo, GpuDeviceFlags deviceMask)
{
	GpuParams* params = new(bs_alloc<GpuParams>()) GpuParams(paramInfo, deviceMask);
	SPtr<GpuParams> paramsPtr = bs_shared_ptr<GpuParams>(params);
	paramsPtr->SetThisPtrInternal(paramsPtr);

	return paramsPtr;
}
}}
