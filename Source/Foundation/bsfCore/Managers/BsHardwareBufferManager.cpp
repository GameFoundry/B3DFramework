//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsHardwareBufferManager.h"
#include "RenderAPI/BsVertexData.h"
#include "RenderAPI/BsVertexDeclaration.h"
#include "RenderAPI/BsVertexDataDesc.h"
#include "RenderAPI/BsGpuParams.h"

using namespace bs;

SPtr<VertexDeclaration> HardwareBufferManager::CreateVertexDeclaration(const SPtr<VertexDataDesc>& desc)
{
	VertexDeclaration* decl = new(B3DAllocate<VertexDeclaration>()) VertexDeclaration(Vector<VertexElement>(desc->GetElements().begin(), desc->GetElements().end()));

	SPtr<VertexDeclaration> declPtr = B3DMakeCoreFromExisting<VertexDeclaration>(decl);
	declPtr->SetShared(declPtr);
	declPtr->Initialize();

	return declPtr;
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
		B3DCombineHash(hash, VertexElement::CalculateHash(entry));

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

SPtr<VertexDeclaration> HardwareBufferManager::CreateVertexDeclaration(const SPtr<VertexDataDesc>& createInformation, GpuDeviceFlags deviceMask)
{
	Vector<VertexElement> elements(createInformation->GetElements().begin(), createInformation->GetElements().end());

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
