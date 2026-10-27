//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuVertexInputManager.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuResourceTracker.h"
#include "Math/B3DMath.h"

using namespace b3d;
using namespace b3d::render;

void GpuVertexInputLayout::Resolve(const VertexDescription& vertexBufferDescription, const VertexDescription& shaderInputDescription)
{
	const TInlineArray<VertexElement, 8>& vertexBufferElements = vertexBufferDescription.GetElements();
	const TInlineArray<VertexElement, 8>& shaderInputElements = shaderInputDescription.GetElements();

	Attributes.Clear();
	StreamCount = 0;
	UsedStreamMask = 0;
	NullStreamIndex = kNoNullStream;
	StreamStrides.Clear();

	bool areAnyShaderInputsMissing = false;
	for(const VertexElement& shaderInputElement : shaderInputElements)
	{
		GpuVertexInputAttribute attribute;
		attribute.ShaderInput = &shaderInputElement;

		for(const VertexElement& vertexBufferElement : vertexBufferElements)
		{
			if(shaderInputElement.GetSemantic() == vertexBufferElement.GetSemantic() && shaderInputElement.GetSemanticIndex() == vertexBufferElement.GetSemanticIndex())
			{
				attribute.BufferElement = &vertexBufferElement;
				break;
			}
		}

		if(attribute.BufferElement != nullptr)
		{
			attribute.StreamIndex = attribute.BufferElement->GetStreamIndex();
			attribute.SteppedPerInstance = attribute.BufferElement->GetInstanceStepRate() > 0;

			if(attribute.StreamIndex < 32)
				UsedStreamMask |= 1u << attribute.StreamIndex;

			StreamCount = Math::Max(StreamCount, attribute.StreamIndex + 1);
		}
		else
			areAnyShaderInputsMissing = true;

		Attributes.Add(attribute);
	}

	// Reserve an extra stream for unmatched shader inputs; backends are expected to bind an empty vertex buffer
	// there so such inputs read zero
	if(areAnyShaderInputsMissing)
	{
		NullStreamIndex = StreamCount++;

		for(GpuVertexInputAttribute& attribute : Attributes)
		{
			if(attribute.BufferElement == nullptr)
				attribute.StreamIndex = NullStreamIndex;
		}
	}

	// Record the vertex stride of every referenced stream so backends need no further access to the vertex
	// buffer description. Strides sum all buffer elements in a stream, matched by a shader input or not, since
	// unmatched elements still occupy space between consecutive vertices. The null stream always gets a zero
	// stride: nothing is ever fetched from the empty buffer bound there, even if the buffer description happens
	// to declare (unmatched) elements under the same stream index.
	for(u32 streamIndex = 0; streamIndex < StreamCount; streamIndex++)
	{
		u32 stride = 0;
		if(streamIndex != NullStreamIndex)
		{
			for(const VertexElement& vertexBufferElement : vertexBufferElements)
			{
				if(vertexBufferElement.GetStreamIndex() == streamIndex)
					stride += vertexBufferElement.GetSize();
			}
		}

		StreamStrides.Add(stride);
	}
}

GpuVertexInputManager::GpuVertexInputManager(GpuDevice& device)
	: mDevice(device)
{ }

GpuVertexInputManager::~GpuVertexInputManager()
{
	for(auto& entry : mVertexInputMap)
		entry.second.VertexInput->Destroy();
}

::std::size_t GpuVertexInputManager::HashFunc::operator()(const VertexDeclarationKey& key) const
{
	size_t hash = 0;
	B3DCombineHash(hash, key.BufferDeclarationId);
	B3DCombineHash(hash, key.ShaderDeclarationId);

	return hash;
}

bool GpuVertexInputManager::EqualFunc::operator()(const VertexDeclarationKey& a, const VertexDeclarationKey& b) const
{
	return a.BufferDeclarationId == b.BufferDeclarationId && a.ShaderDeclarationId == b.ShaderDeclarationId;
}

GpuVertexInput* GpuVertexInputManager::GetVertexInput(const TShared<VertexDescription>& vertexBufferDescription, const TShared<VertexDescription>& shaderInputDescription,
	GpuResourceTracker& resourceTracker)
{
	Lock lock(mMutex);

	VertexDeclarationKey key;
	key.BufferDeclarationId = vertexBufferDescription->GetId();
	key.ShaderDeclarationId = shaderInputDescription->GetId();

	auto iterFind = mVertexInputMap.find(key);
	if(iterFind == mVertexInputMap.end())
	{
		if(mVertexInputMap.size() >= kDeclarationBufferSize)
			RemoveLeastUsed(); // Prune so the cache doesn't just infinitely grow

		GpuVertexInputLayout layout;
		layout.Resolve(*vertexBufferDescription, *shaderInputDescription);

		VertexInputEntry newEntry;
		newEntry.VertexInput = mDevice.CreateVertexInput(mNextId++, layout);
		if(newEntry.VertexInput == nullptr)
			return nullptr;

		iterFind = mVertexInputMap.insert(std::make_pair(key, newEntry)).first;
	}

	iterFind->second.LastUsedIndex = ++mLastUsedCounter;
	resourceTracker.TrackResourceUsage(iterFind->second.VertexInput, GpuAccessFlag::Read);

	return iterFind->second.VertexInput;
}

void GpuVertexInputManager::RemoveLeastUsed()
{
	if(!mWarningShown)
	{
		B3D_LOG(Warning, LogRenderBackend, "Vertex input cache is full, pruning last {0} elements. This is "
			"probably okay unless you are creating a massive amount of input layouts as they will get re-created "
			"every frame. In that case you should increase the layout cache size. This warning won't be shown again.",
			kElementCountToPrune);

		mWarningShown = true;
	}

	Map<u32, VertexDeclarationKey> leastFrequentlyUsedMap;

	for(auto iter = mVertexInputMap.begin(); iter != mVertexInputMap.end(); ++iter)
		leastFrequentlyUsedMap[iter->second.LastUsedIndex] = iter->first;

	u32 removedElementCount = 0;
	for(auto iter = leastFrequentlyUsedMap.begin(); iter != leastFrequentlyUsedMap.end(); ++iter)
	{
		auto inputLayoutIter = mVertexInputMap.find(iter->second);

		// Command buffers that track the object keep it alive until the GPU is done with it
		inputLayoutIter->second.VertexInput->Destroy();
		mVertexInputMap.erase(inputLayoutIter);

		removedElementCount++;
		if(removedElementCount >= kElementCountToPrune)
			break;
	}
}
