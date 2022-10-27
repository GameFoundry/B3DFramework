//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11InputLayoutManager.h"
#include "BsD3D11Mappings.h"
#include "BsD3D11RenderAPI.h"
#include "BsD3D11Device.h"
#include "BsD3D11GpuProgram.h"
#include "Managers/BsHardwareBufferManager.h"
#include "Profiling/BsRenderStats.h"
#include "Debug/BsDebug.h"

using namespace bs;
using namespace bs::ct;

size_t D3D11InputLayoutManager::HashFunc::operator()(const D3D11InputLayoutManager::VertexDeclarationKey& key) const
{
	size_t hash = 0;
	bs_hash_combine(hash, key.VertxDeclId);
	bs_hash_combine(hash, key.VertexProgramId);

	return hash;
}

bool D3D11InputLayoutManager::EqualFunc::operator()(const D3D11InputLayoutManager::VertexDeclarationKey& a, const D3D11InputLayoutManager::VertexDeclarationKey& b) const

{
	if(a.VertxDeclId != b.VertxDeclId)
		return false;

	if(a.VertexProgramId != b.VertexProgramId)
		return false;

	return true;
}

D3D11InputLayoutManager::~D3D11InputLayoutManager()
{
	while(mInputLayoutMap.begin() != mInputLayoutMap.end())
	{
		auto firstElem = mInputLayoutMap.begin();

		SAFE_RELEASE(firstElem->second->InputLayout);
		bs_delete(firstElem->second);

		mInputLayoutMap.erase(firstElem);
		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_InputLayout);
	}
}

ID3D11InputLayout* D3D11InputLayoutManager::RetrieveInputLayout(const SPtr<VertexDeclaration>& vertexShaderDecl, const SPtr<VertexDeclaration>& vertexBufferDecl, D3D11GpuProgram& vertexProgram)
{
	VertexDeclarationKey pair;
	pair.VertxDeclId = vertexBufferDecl->GetId();
	pair.VertexProgramId = vertexProgram.GetProgramId();

	auto iterFind = mInputLayoutMap.find(pair);
	if(iterFind == mInputLayoutMap.end())
	{
		if(mInputLayoutMap.size() >= DECLARATION_BUFFER_SIZE)
			RemoveLeastUsed(); // Prune so the buffer doesn't just infinitely grow

		AddNewInputLayout(vertexShaderDecl, vertexBufferDecl, vertexProgram);

		iterFind = mInputLayoutMap.find(pair);

		if(iterFind == mInputLayoutMap.end()) // We failed to create input layout
			return nullptr;
	}

	iterFind->second->LastUsedIdx = ++mLastUsedCounter;
	return iterFind->second->InputLayout;
}

void D3D11InputLayoutManager::AddNewInputLayout(const SPtr<VertexDeclaration>& vertexShaderDecl, const SPtr<VertexDeclaration>& vertexBufferDecl, D3D11GpuProgram& vertexProgram)
{
	const VertexDeclarationProperties& bufferDeclProps = vertexBufferDecl->GetProperties();
	const VertexDeclarationProperties& shaderDeclProps = vertexShaderDecl->GetProperties();

	Vector<D3D11_INPUT_ELEMENT_DESC> declElements;

	const Vector<VertexElement>& bufferElems = bufferDeclProps.GetElements();
	const Vector<VertexElement>& shaderElems = shaderDeclProps.GetElements();

	i32 maxStreamIdx = -1;
	for(auto iter = bufferElems.begin(); iter != bufferElems.end(); ++iter)
	{
		declElements.push_back(D3D11_INPUT_ELEMENT_DESC());
		D3D11_INPUT_ELEMENT_DESC& elementDesc = declElements.back();

		elementDesc.SemanticName = D3D11Mappings::Get(iter->GetSemantic());
		elementDesc.SemanticIndex = iter->GetSemanticIdx();
		elementDesc.Format = D3D11Mappings::Get(iter->GetType());
		elementDesc.InputSlot = iter->GetStreamIdx();
		elementDesc.AlignedByteOffset = static_cast<WORD>(iter->GetOffset());

		if(iter->GetInstanceStepRate() == 0)
		{
			elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			elementDesc.InstanceDataStepRate = 0;
		}
		else
		{
			elementDesc.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
			elementDesc.InstanceDataStepRate = iter->GetInstanceStepRate();
		}

		maxStreamIdx = std::max(maxStreamIdx, (i32)iter->GetStreamIdx());
	}

	// Find elements missing in buffer and add a dummy stream for them
	for(auto shaderIter = shaderElems.begin(); shaderIter != shaderElems.end(); ++shaderIter)
	{
		bool foundElement = false;
		for(auto bufferIter = bufferElems.begin(); bufferIter != bufferElems.end(); ++bufferIter)
		{
			if(shaderIter->GetSemantic() == bufferIter->GetSemantic() && shaderIter->GetSemanticIdx() == bufferIter->GetSemanticIdx())
			{
				foundElement = true;
				break;
			}
		}

		if(!foundElement)
		{
			declElements.push_back(D3D11_INPUT_ELEMENT_DESC());
			D3D11_INPUT_ELEMENT_DESC& elementDesc = declElements.back();

			elementDesc.SemanticName = D3D11Mappings::Get(shaderIter->GetSemantic());
			elementDesc.SemanticIndex = shaderIter->GetSemanticIdx();
			elementDesc.Format = D3D11Mappings::Get(shaderIter->GetType());
			elementDesc.InputSlot = (u32)(maxStreamIdx + 1);
			elementDesc.AlignedByteOffset = 0;
			elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			elementDesc.InstanceDataStepRate = 0;
		}
	}

	D3D11RenderAPI* d3d11rs = static_cast<D3D11RenderAPI*>(RenderAPI::InstancePtr());
	D3D11Device& device = d3d11rs->GetPrimaryDevice();

	const DataBlob& microcode = vertexProgram.GetMicroCode();

	InputLayoutEntry* newEntry = bs_new<InputLayoutEntry>();
	newEntry->LastUsedIdx = ++mLastUsedCounter;
	newEntry->InputLayout = nullptr;
	HRESULT hr = device.GetD3D11Device()->CreateInputLayout(
		&declElements[0],
		(u32)declElements.size(),
		microcode.Data,
		microcode.Size,
		&newEntry->InputLayout);

	if(FAILED(hr) || device.HasError())
		BS_EXCEPT(RenderingAPIException, "Unable to set D3D11 vertex declaration" + device.GetErrorDescription());

	// Create key and add to the layout map
	VertexDeclarationKey pair;
	pair.VertxDeclId = vertexBufferDecl->GetId();
	pair.VertexProgramId = vertexProgram.GetProgramId();

	mInputLayoutMap[pair] = newEntry;

	BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_InputLayout);
}

void D3D11InputLayoutManager::RemoveLeastUsed()
{
	if(!mWarningShown)
	{
		BS_LOG(Warning, RenderBackend, "Input layout buffer is full, pruning last {0} elements. This is probably okay "
									   "unless you are creating a massive amount of input layouts as they will get re-created every frame. "
									   "In that case you should increase the layout buffer size. This warning won't be shown again.",
			   NUM_ELEMENTS_TO_PRUNE);

		mWarningShown = true;
	}

	Map<u32, VertexDeclarationKey> leastFrequentlyUsedMap;

	for(auto iter = mInputLayoutMap.begin(); iter != mInputLayoutMap.end(); ++iter)
		leastFrequentlyUsedMap[iter->second->LastUsedIdx] = iter->first;

	u32 elemsRemoved = 0;
	for(auto iter = leastFrequentlyUsedMap.begin(); iter != leastFrequentlyUsedMap.end(); ++iter)
	{
		auto inputLayoutIter = mInputLayoutMap.find(iter->second);

		SAFE_RELEASE(inputLayoutIter->second->InputLayout);
		bs_delete(inputLayoutIter->second);

		mInputLayoutMap.erase(inputLayoutIter);
		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_InputLayout);

		elemsRemoved++;
		if(elemsRemoved >= NUM_ELEMENTS_TO_PRUNE)
			break;
	}
}
