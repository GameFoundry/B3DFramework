//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//

// Template method definitions for TGpuVertexInputManager. This file is not a translation unit of its own; it is included
// by the single backend source file that explicitly instantiates TGpuVertexInputManager for its manager and vertex input
// types, after the complete derived manager and logging headers.

namespace b3d
{
	namespace render
	{

template<class TDerived, class TVertexInput>
::std::size_t TGpuVertexInputManager<TDerived, TVertexInput>::HashFunc::operator()(const VertexDeclarationKey& key) const
{
	size_t hash = 0;
	B3DCombineHash(hash, key.BufferDeclarationId);
	B3DCombineHash(hash, key.ShaderDeclarationId);

	return hash;
}

template<class TDerived, class TVertexInput>
bool TGpuVertexInputManager<TDerived, TVertexInput>::EqualFunc::operator()(const VertexDeclarationKey& a, const VertexDeclarationKey& b) const
{
	return a.BufferDeclarationId == b.BufferDeclarationId && a.ShaderDeclarationId == b.ShaderDeclarationId;
}

template<class TDerived, class TVertexInput>
TVertexInput TGpuVertexInputManager<TDerived, TVertexInput>::GetVertexInput(const TShared<VertexDescription>& vertexBufferDescription, const TShared<VertexDescription>& shaderInputDescription)
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
		newEntry.VertexInput = static_cast<TDerived*>(this)->CreateVertexInput(layout);
		if(!newEntry.VertexInput)
			return TVertexInput();

		iterFind = mVertexInputMap.insert(std::make_pair(key, std::move(newEntry))).first;
	}

	iterFind->second.LastUsedIndex = ++mLastUsedCounter;
	return iterFind->second.VertexInput;
}

template<class TDerived, class TVertexInput>
void TGpuVertexInputManager<TDerived, TVertexInput>::RemoveLeastUsed()
{
	// Note: Caller is expected to hold the manager mutex.
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

		static_cast<TDerived*>(this)->DestroyVertexInput(inputLayoutIter->second.VertexInput);
		mVertexInputMap.erase(inputLayoutIter);

		removedElementCount++;
		if(removedElementCount >= kElementCountToPrune)
			break;
	}
}

template<class TDerived, class TVertexInput>
void TGpuVertexInputManager<TDerived, TVertexInput>::ReleaseAll()
{
	Lock lock(mMutex);

	for(auto& entry : mVertexInputMap)
		static_cast<TDerived*>(this)->DestroyVertexInput(entry.second.VertexInput);

	mVertexInputMap.clear();
}

	} // namespace render
} // namespace b3d
