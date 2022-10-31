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
	VertexDeclaration* decl = new(B3DAllocate<VertexDeclaration>()) VertexDeclaration(desc->CreateElements());

	SPtr<VertexDeclaration> declPtr = B3DMakeCoreFromExisting<VertexDeclaration>(decl);
	declPtr->SetThisPtrInternal(declPtr);
	declPtr->Initialize();

	return declPtr;
}

SPtr<VertexBuffer> HardwareBufferManager::CreateVertexBuffer(const VERTEX_BUFFER_DESC& desc)
{
	SPtr<VertexBuffer> vbuf = B3DMakeCoreFromExisting<VertexBuffer>(new(B3DAllocate<VertexBuffer>()) VertexBuffer(desc));
	vbuf->SetThisPtrInternal(vbuf);
	vbuf->Initialize();
	return vbuf;
}

SPtr<IndexBuffer> HardwareBufferManager::CreateIndexBuffer(const INDEX_BUFFER_DESC& desc)
{
	SPtr<IndexBuffer> ibuf = B3DMakeCoreFromExisting<IndexBuffer>(new(B3DAllocate<IndexBuffer>()) IndexBuffer(desc));
	ibuf->SetThisPtrInternal(ibuf);
	ibuf->Initialize();
	return ibuf;
}

SPtr<GpuParamBlockBuffer> HardwareBufferManager::CreateGpuParamBlockBuffer(u32 size, GpuBufferUsage usage)
{
	SPtr<GpuParamBlockBuffer> paramBlockPtr = B3DMakeCoreFromExisting<GpuParamBlockBuffer>(new(B3DAllocate<GpuParamBlockBuffer>()) GpuParamBlockBuffer(size, usage));
	paramBlockPtr->SetThisPtrInternal(paramBlockPtr);
	paramBlockPtr->Initialize();
	return paramBlockPtr;
}

SPtr<GpuBuffer> HardwareBufferManager::CreateGpuBuffer(const GPU_BUFFER_DESC& desc)
{
	SPtr<GpuBuffer> gbuf = B3DMakeCoreFromExisting<GpuBuffer>(new(B3DAllocate<GpuBuffer>()) GpuBuffer(desc));
	gbuf->SetThisPtrInternal(gbuf);
	gbuf->Initialize();

	return gbuf;
}

SPtr<GpuParams> HardwareBufferManager::CreateGpuParams(const SPtr<GpuPipelineParamInfo>& paramInfo)
{
	GpuParams* params = new(B3DAllocate<GpuParams>()) GpuParams(paramInfo);
	SPtr<GpuParams> paramsPtr = B3DMakeCoreFromExisting<GpuParams>(params);
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
	VertexDeclaration* decl = new(B3DAllocate<VertexDeclaration>()) VertexDeclaration(elements, deviceMask);

	SPtr<VertexDeclaration> ret = B3DMakeSharedFromExisting<VertexDeclaration>(decl);
	ret->SetThisPtrInternal(ret);

	return ret;
}

SPtr<GpuParams> HardwareBufferManager::CreateGpuParamsInternal(
	const SPtr<GpuPipelineParamInfo>& paramInfo, GpuDeviceFlags deviceMask)
{
	GpuParams* params = new(B3DAllocate<GpuParams>()) GpuParams(paramInfo, deviceMask);
	SPtr<GpuParams> paramsPtr = B3DMakeSharedFromExisting<GpuParams>(params);
	paramsPtr->SetThisPtrInternal(paramsPtr);

	return paramsPtr;
}
}}
