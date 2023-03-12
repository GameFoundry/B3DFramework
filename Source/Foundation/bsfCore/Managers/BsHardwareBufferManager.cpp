//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsHardwareBufferManager.h"
#include "RenderAPI/BsVertexData.h"
#include "RenderAPI/BsGenericGpuBuffer.h"
#include "RenderAPI/BsVertexDeclaration.h"
#include "RenderAPI/BsGpuParamBlockBuffer.h"
#include "RenderAPI/BsVertexDataDesc.h"
#include "RenderAPI/BsGpuParams.h"

using namespace bs;

SPtr<VertexDeclaration> HardwareBufferManager::CreateVertexDeclaration(const SPtr<VertexDataDesc>& desc)
{
	VertexDeclaration* decl = new(B3DAllocate<VertexDeclaration>()) VertexDeclaration(desc->CreateElements());

	SPtr<VertexDeclaration> declPtr = B3DMakeCoreFromExisting<VertexDeclaration>(decl);
	declPtr->SetShared(declPtr);
	declPtr->Initialize();

	return declPtr;
}

SPtr<VertexBuffer> HardwareBufferManager::CreateVertexBuffer(const VertexBufferCreateInformation& createInformation)
{
	SPtr<VertexBuffer> vbuf = B3DMakeCoreFromExisting<VertexBuffer>(new(B3DAllocate<VertexBuffer>()) VertexBuffer(createInformation));
	vbuf->SetShared(vbuf);
	vbuf->Initialize();
	return vbuf;
}

SPtr<IndexBuffer> HardwareBufferManager::CreateIndexBuffer(const IndexBufferCreateInformation& createInformation)
{
	SPtr<IndexBuffer> ibuf = B3DMakeCoreFromExisting<IndexBuffer>(new(B3DAllocate<IndexBuffer>()) IndexBuffer(createInformation));
	ibuf->SetShared(ibuf);
	ibuf->Initialize();
	return ibuf;
}

SPtr<GpuParamBlockBuffer> HardwareBufferManager::CreateGpuParamBlockBuffer(u32 size, GpuBufferFlags flags)
{
	SPtr<GpuParamBlockBuffer> paramBlockPtr = B3DMakeCoreFromExisting<GpuParamBlockBuffer>(new(B3DAllocate<GpuParamBlockBuffer>()) GpuParamBlockBuffer(size, flags));
	paramBlockPtr->SetShared(paramBlockPtr);
	paramBlockPtr->Initialize();
	return paramBlockPtr;
}

SPtr<GenericGpuBuffer> HardwareBufferManager::CreateGpuBuffer(const GenericGpuBufferCreateInformation& createInformation)
{
	SPtr<GenericGpuBuffer> gbuf = B3DMakeCoreFromExisting<GenericGpuBuffer>(new(B3DAllocate<GenericGpuBuffer>()) GenericGpuBuffer(createInformation));
	gbuf->SetShared(gbuf);
	gbuf->Initialize();

	return gbuf;
}

SPtr<GpuParams> HardwareBufferManager::CreateGpuParams(const SPtr<GpuPipelineParamInfo>& paramInfo)
{
	GpuParams* params = new(B3DAllocate<GpuParams>()) GpuParams(paramInfo);
	SPtr<GpuParams> paramsPtr = B3DMakeCoreFromExisting<GpuParams>(params);
	paramsPtr->SetShared(paramsPtr);
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
		B3DCombineHash(hash, VertexElement::GetHash(entry));

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

SPtr<IndexBuffer> HardwareBufferManager::CreateIndexBuffer(const IndexBufferCreateInformation& createInformation, GpuDeviceFlags deviceMask)
{
	SPtr<IndexBuffer> ibuf = CreateIndexBufferInternal(createInformation, deviceMask);
	ibuf->Initialize();
	return ibuf;
}

SPtr<VertexBuffer> HardwareBufferManager::CreateVertexBuffer(const VertexBufferCreateInformation& createInformation, GpuDeviceFlags deviceMask)
{
	SPtr<VertexBuffer> vbuf = CreateVertexBufferInternal(createInformation, deviceMask);
	vbuf->Initialize();
	return vbuf;
}

SPtr<VertexDeclaration> HardwareBufferManager::CreateVertexDeclaration(const SPtr<VertexDataDesc>& createInformation, GpuDeviceFlags deviceMask)
{
	Vector<VertexElement> elements = createInformation->CreateElements();

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

SPtr<GpuParamBlockBuffer> HardwareBufferManager::CreateGpuParamBlockBuffer(u32 size, GpuBufferFlags flags, GpuDeviceFlags deviceMask)
{
	SPtr<GpuParamBlockBuffer> paramBlockPtr = CreateGpuParamBlockBufferInternal(size, flags, deviceMask);
	paramBlockPtr->Initialize();

	return paramBlockPtr;
}

SPtr<GpuParamBlockBuffer> HardwareBufferManager::CreateGpuParamBlockBuffer(const SPtr<GpuBuffer>& owner, u32 offset, u32 size)
{
	SPtr<GpuParamBlockBuffer> paramBlockPtr = CreateGpuParamBlockBufferInternal(owner, offset, size);
	paramBlockPtr->Initialize();

	return paramBlockPtr;
}

SPtr<GenericGpuBuffer> HardwareBufferManager::CreateGpuBuffer(const GenericGpuBufferCreateInformation& desc, GpuDeviceFlags deviceMask)
{
	SPtr<GenericGpuBuffer> gbuf = CreateGpuBufferInternal(desc, deviceMask);
	gbuf->Initialize();

	return gbuf;
}

SPtr<GenericGpuBuffer> HardwareBufferManager::CreateGpuBuffer(const GenericGpuBufferCreateInformation& desc, SPtr<GpuBuffer> underlyingBuffer)
{
	SPtr<GenericGpuBuffer> gbuf = CreateGpuBufferInternal(desc, std::move(underlyingBuffer));
	gbuf->Initialize();

	return gbuf;
}

SPtr<VertexDeclaration> HardwareBufferManager::CreateVertexDeclarationInternal(
	const Vector<VertexElement>& elements, GpuDeviceFlags deviceMask)
{
	VertexDeclaration* decl = new(B3DAllocate<VertexDeclaration>()) VertexDeclaration(elements, deviceMask);

	SPtr<VertexDeclaration> ret = B3DMakeSharedFromExisting<VertexDeclaration>(decl);
	ret->SetShared(ret);

	return ret;
}

SPtr<GpuParams> HardwareBufferManager::CreateGpuParamsInternal(
	const SPtr<GpuPipelineParamInfo>& paramInfo, GpuDeviceFlags deviceMask)
{
	GpuParams* params = new(B3DAllocate<GpuParams>()) GpuParams(paramInfo, deviceMask);
	SPtr<GpuParams> paramsPtr = B3DMakeSharedFromExisting<GpuParams>(params);
	paramsPtr->SetShared(paramsPtr);

	return paramsPtr;
}
}}
