//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGUIInteractable.h"
#include "B3DGUIManager.h"
#include "GUI/B3DGUIMeshbatches.h"
#include "Mesh/B3DMesh.h"
#include "RenderAPI/B3DVertexDescription.h"
#include "Image/B3DSpriteTexture.h"

using namespace b3d;

SPtr<VertexDescription> GetGUITriangleMeshDesc()
{
	static SPtr<VertexDescription> sDesc;

	if(!sDesc)
	{
		TInlineArray<VertexElement, 8> vertexElements;
		vertexElements.Add(VertexElement(VET_FLOAT2, VES_POSITION));
		vertexElements.Add(VertexElement(VET_FLOAT2, VES_TEXCOORD));

		sDesc = B3DMakeShared<VertexDescription>(vertexElements);
	}

	return sDesc;
}

SPtr<VertexDescription> GetGUILineMeshDesc()
{
	static SPtr<VertexDescription> sDesc;

	if(!sDesc)
	{
		TInlineArray<VertexElement, 8> vertexElements;
		vertexElements.Add(VertexElement(VET_FLOAT2, VES_POSITION));

		sDesc = B3DMakeShared<VertexDescription>(vertexElements);
	}

	return sDesc;
}

GUIMeshBatches::GUIMeshBatches(GUIWidget* parentWidget)
	: mWidget(parentWidget)
{
	BatchesInDepthRange rootDepthRange;
	rootDepthRange.MinDepth = 0;
	rootDepthRange.DepthRange = std::numeric_limits<u32>::max();
	rootDepthRange.Id = mNextDepthRangeId++;

	mDepthRanges.push_back(rootDepthRange);
}

void GUIMeshBatches::Add(GUIRenderable* guiElement)
{
	const TInlineArray<GUIRenderElement, 4>& guiRenderElements = guiElement->GetRenderElements();

	BatchedGUIElement& batchedGuiElement = mElements[guiElement];
	batchedGuiElement.GUIElement = guiElement;
	batchedGuiElement.Bounds = guiElement->GetAbsoluteClippedArea().To<i32, u32>();
	batchedGuiElement.BatchPerRenderElement.Resize(guiRenderElements.Size(), ~0u);

	for(u32 elementIndex = 0; elementIndex < (u32)guiRenderElements.Size(); elementIndex++)
	{
		B3D_ASSERT(batchedGuiElement.BatchPerRenderElement[elementIndex] == ~0u);
		Add(batchedGuiElement, elementIndex);
	}

	mBatchesOutOfDateInRenderer = true;
}

void GUIMeshBatches::Add(BatchedGUIElement& batchedGuiElement, u32 renderElementIndex)
{
	GUIRenderable* const guiElement = batchedGuiElement.GUIElement;
	const TInlineArray<GUIRenderElement, 4>& renderElements = guiElement->GetRenderElements();

	const GUIRenderElement& guiRenderElement = renderElements[renderElementIndex];
	const u32 renderElementDepth = guiElement->GetDepth() + guiRenderElement.Depth;

	// Depth ranges are sorted by MinDepth
	for(u32 rangeIndex = 0; rangeIndex < (u32)mDepthRanges.size(); rangeIndex++)
	{
		if(renderElementDepth < mDepthRanges[rangeIndex].MinDepth || renderElementDepth >= (mDepthRanges[rangeIndex].MinDepth + mDepthRanges[rangeIndex].DepthRange))
			continue;

		Add(batchedGuiElement, renderElementIndex, rangeIndex);
		break;
	}
}

void GUIMeshBatches::Add(BatchedGUIElement& batchedGuiElement, u32 renderElementIndex, u32 depthRangeIndex)
{
	GUIRenderable* const guiElement = batchedGuiElement.GUIElement;
	const TInlineArray<GUIRenderElement, 4>& guiRenderElements = guiElement->GetRenderElements();

	const GUIRenderElement& guiRenderElement = guiRenderElements[renderElementIndex];
	const u32 renderElementDepth = guiElement->GetDepth() + guiRenderElement.Depth;

	SpriteMaterial* const spriteMaterial = guiRenderElement.Material;
	B3D_ASSERT(spriteMaterial != nullptr);

	const BatchedGUIRenderElement batchedGuiRenderElement(guiElement, renderElementIndex, renderElementDepth);
	const BatchedMaterial batchedMaterial = CreateBatchedMaterial(batchedGuiRenderElement);

	BatchesInDepthRange* depthRange = &mDepthRanges[depthRangeIndex];
	if(depthRange->DepthRange == 1)
	{
		Add(batchedGuiElement, batchedGuiRenderElement, batchedMaterial, depthRangeIndex);
	}
	else
	{
		Batch* foundBatch = nullptr;
		bool depthRangeHasAnotherBatch = (batchedMaterial.IsBatchingAllowed && depthRange->BatchIds.size() > 1) || (!batchedMaterial.IsBatchingAllowed && !depthRange->BatchIds.empty());
		if(!depthRangeHasAnotherBatch && !depthRange->BatchIds.empty())
		{
			for(const auto& batchId : depthRange->BatchIds)
			{
				auto found = mBatches.find(batchId);
				if(found == mBatches.end())
				{
					B3D_ASSERT(false);
					return;
				}

				const Batch& batch = found->second;
				if(batch.Material.CanBeMergedWith(batchedMaterial))
				{
					foundBatch = &found->second;
					break;
				}
			}

			if(foundBatch == nullptr)
				depthRangeHasAnotherBatch = true;
		}

		if(depthRangeHasAnotherBatch)
		{
			u32 currentDepthRangeIndex = ~0u;
			if(depthRange->MinDepth != renderElementDepth)
			{
				currentDepthRangeIndex = SplitDepthRange(depthRangeIndex, renderElementDepth);
				depthRange = nullptr; // Clear as it's possible no longer valid as the vector resized
			}
			else
			{
				currentDepthRangeIndex = depthRangeIndex;
			}

			foundBatch = Add(batchedGuiElement, batchedGuiRenderElement, batchedMaterial, currentDepthRangeIndex);

			B3D_ASSERT(currentDepthRangeIndex < (u32)mDepthRanges.size());
			BatchesInDepthRange* const currentDepthRange = &mDepthRanges[currentDepthRangeIndex];

			if(currentDepthRange->DepthRange != 1)
			{
				u32 minimumDepth = ~0u;
				bool hasFoundNextElement = false;

				for(const auto& batchId : currentDepthRange->BatchIds)
				{
					auto found = mBatches.find(batchId);
					if(found == mBatches.end())
					{
						B3D_ASSERT(false);
						return;
					}

					const Batch& batch = found->second;
					if(batch.Id == foundBatch->Id)
						continue;

					for(const auto& entry : batch.RenderElements)
					{
						if(entry.Depth >= minimumDepth)
							continue;

						minimumDepth = entry.Depth;
						hasFoundNextElement = true;

						// Early out
						if(minimumDepth == renderElementDepth)
							break;
					}
				}

				if(hasFoundNextElement)
				{
					if(minimumDepth == renderElementDepth)
					{
						B3D_ASSERT(renderElementDepth != ~0u);
						SplitDepthRange(currentDepthRangeIndex, renderElementDepth + 1);

						B3D_ASSERT(currentDepthRangeIndex < (u32)mDepthRanges.size());
						B3D_ASSERT(mDepthRanges[currentDepthRangeIndex].DepthRange == 1);
					}
					else
					{
						SplitDepthRange(currentDepthRangeIndex, minimumDepth);
					}
				}
			}
		}
		else
		{
			Add(batchedGuiElement, batchedGuiRenderElement, batchedMaterial, depthRangeIndex);
		}
	}

	B3D_ASSERT(batchedGuiElement.BatchPerRenderElement[renderElementIndex] != ~0u);
}

GUIMeshBatches::Batch* GUIMeshBatches::Add(BatchedGUIElement& batchedGuiElement, const BatchedGUIRenderElement& batchedGuiRenderElement, const BatchedMaterial& batchedMaterial, u32 depthRangeIndex)
{
	if(!B3D_ENSURE(depthRangeIndex < (u32)mDepthRanges.size()))
		return nullptr;

	BatchesInDepthRange& depthRange = mDepthRanges[depthRangeIndex];
	Batch* foundBatch = nullptr;
	if(batchedMaterial.IsBatchingAllowed)
	{
		for(const auto& batchId : depthRange.BatchIds)
		{
			auto itFoundBatch = mBatches.find(batchId);
			if(!B3D_ENSURE(itFoundBatch != mBatches.end()))
				return nullptr;

			const Batch& batch = itFoundBatch->second;
			if(batch.Material.CanBeMergedWith(batchedMaterial))
			{
				foundBatch = &itFoundBatch->second;
				break;
			}
		}
	}

	if(foundBatch == nullptr)
	{
		const u32 newBatchId = AllocateBatchId();

		Batch& newBatch = mBatches[newBatchId];
		newBatch.Id = newBatchId;
		newBatch.DepthRangeId = depthRange.Id;
		newBatch.Material = batchedMaterial;

		depthRange.BatchIds.push_back(newBatch.Id);
		foundBatch = &newBatch;
	}
	else
	{
		foundBatch->Material.Merge(batchedMaterial);
	}

	if(foundBatch->RenderElements.empty())
	{
		foundBatch->RenderElements.push_back(batchedGuiRenderElement);
	}
	else
	{
		bool hasAdded = false;
		for(auto it = foundBatch->RenderElements.begin(); it != foundBatch->RenderElements.end(); ++it)
		{
			if(it->Depth > batchedGuiRenderElement.Depth)
				continue;

			foundBatch->RenderElements.insert(it, batchedGuiRenderElement);
			hasAdded = true;
			break;
		}

		if(!hasAdded)
			foundBatch->RenderElements.push_back(batchedGuiRenderElement);
	}

	foundBatch->IsMeshDirty = true;

	batchedGuiElement.BatchPerRenderElement[batchedGuiRenderElement.RenderElementIndex] = foundBatch->Id;

	B3D_ASSERT(batchedGuiRenderElement.ParentGUIElement != nullptr);

	const Area2I bounds = batchedGuiRenderElement.ParentGUIElement->GetAbsoluteClippedArea().To<i32, u32>();
	foundBatch->Bounds.Encapsulate(bounds);

	MarkBoundsDirty(batchedGuiElement, foundBatch->Id);

	return foundBatch;
}

void GUIMeshBatches::Remove(GUIRenderable* guiElement)
{
	auto found = mElements.find(guiElement);
	if(found == mElements.end())
		return;

	for(u32 elementIndex = 0; elementIndex < found->second.BatchPerRenderElement.Size(); elementIndex++)
		Remove(found->second, elementIndex);

	mElements.erase(guiElement);
	mBatchesOutOfDateInRenderer = true;
}

void GUIMeshBatches::Remove(BatchedGUIElement& batchedGuiElement, u32 renderElementIndex)
{
	if(renderElementIndex >= (u32)batchedGuiElement.BatchPerRenderElement.Size())
		return;

	const u32 batchId = batchedGuiElement.BatchPerRenderElement[renderElementIndex];
	auto foundBatch = mBatches.find(batchId);
	if(!B3D_ENSURE(foundBatch != mBatches.end()))
		return;

	auto foundDepthRange = std::find_if(mDepthRanges.begin(), mDepthRanges.end(), [depthRangeId = foundBatch->second.DepthRangeId](const BatchesInDepthRange& depthRange)
		{ return depthRange.Id == depthRangeId; });

	if(!B3D_ENSURE(foundDepthRange != mDepthRanges.end()))
		return;

	const u32 depthRangeIndex = (u32)(foundDepthRange - mDepthRanges.begin());
	Remove(batchedGuiElement, renderElementIndex, depthRangeIndex);
}

void GUIMeshBatches::Remove(BatchedGUIElement& batchedGuiElement, u32 renderElementIndex, u32 depthRangeIndex)
{
	if(!B3D_ENSURE(depthRangeIndex < (u32)mDepthRanges.size()))
		return;

	const u32 batchId = batchedGuiElement.BatchPerRenderElement[renderElementIndex];

	const auto itFoundBatch = mBatches.find(batchId);
	if(!B3D_ENSURE(itFoundBatch != mBatches.end()))
		return;

	GUIRenderable* const guiElement = batchedGuiElement.GUIElement;
	BatchesInDepthRange& depthRange = mDepthRanges[depthRangeIndex];

	bool hasFoundElement = false;
	Batch& batch = itFoundBatch->second;
	for(auto it = batch.RenderElements.begin(); it != batch.RenderElements.end();)
	{
		if(it->ParentGUIElement == guiElement && it->RenderElementIndex == renderElementIndex)
		{
			batch.IsBoundsDirty = true;
			batch.IsMeshDirty = true;

			Area2I::AddUnique(batchedGuiElement.Bounds, batch.DirtyRegions);

			it = batch.RenderElements.erase(it);
			hasFoundElement = true;
			break;
		}
		else
		{
			++it;
		}
	}

	B3D_ASSERT(hasFoundElement);

	if(batch.RenderElements.empty())
	{
		for(auto it = depthRange.BatchIds.begin(); it != depthRange.BatchIds.end();)
		{
			if(*it == batchId)
			{
				for(const auto& dirtyRegion : batch.DirtyRegions)
					Area2I::AddUnique(dirtyRegion, mDirtyRegionsForRemovedBatches);

				FreeBatchId(batch.Id);

				mBatches.erase(itFoundBatch);
				it = depthRange.BatchIds.erase(it);

				break;
			}
			else
				++it;
		}
	}

	batchedGuiElement.BatchPerRenderElement[renderElementIndex] = ~0u;

	const bool hasCollapsedWithPreviousDepthRange = CollapseDepthRange(depthRangeIndex);

	const u32 nextDepthRangeIndex = hasCollapsedWithPreviousDepthRange ? depthRangeIndex : depthRangeIndex + 1;
	if(nextDepthRangeIndex < (u32)mDepthRanges.size())
		CollapseDepthRange(nextDepthRangeIndex);
}

GUIDrawGroupRenderDataUpdate GUIMeshBatches::RebuildDirty(bool forceRebuildMeshes)
{
	// Update dirty draw groups and mark them for redraw
	bool shouldRebuildMeshes = forceRebuildMeshes;
	for(auto& entry : mDirtyElements)
	{
		GUIRenderable* const guiElement = entry.first;

		auto itFoundElement = mElements.find(guiElement);
		if(itFoundElement == mElements.end())
			continue;

		shouldRebuildMeshes = true;

		const TInlineArray<GUIRenderElement, 4>& guiRenderElements = guiElement->GetRenderElements();
		BatchedGUIElement& batchedGuiElement = itFoundElement->second;

		bool dirtyBounds = false;
		if((entry.second & DirtyContent) != 0)
		{
			const bool renderElementsDirty = batchedGuiElement.BatchPerRenderElement.Size() != guiRenderElements.Size();

			// If render element count changed, do a full rebuild of the draw group
			if(renderElementsDirty)
			{
				Remove(guiElement);
				Add(guiElement);

				continue;
			}

			// If bounds changed, rebuild the bounds of the draw groups
			Area2I bounds = guiElement->GetAbsoluteClippedArea().To<i32, u32>();
			if(batchedGuiElement.Bounds != bounds)
			{
				MarkBoundsDirty(batchedGuiElement);

				dirtyBounds = true;
				batchedGuiElement.Bounds = bounds;

				MarkBoundsDirty(batchedGuiElement);
			}
		}

		for(u32 elementIndex = 0; elementIndex < (u32)guiRenderElements.size(); elementIndex++)
		{
			const GUIRenderElement& guiRenderElement = guiRenderElements[elementIndex];
			const u32 batchId = batchedGuiElement.BatchPerRenderElement[elementIndex];

			B3D_ASSERT(batchId != ~0u);

			auto itFoundBatch = mBatches.find(batchId);
			if(!B3D_ENSURE(itFoundBatch != mBatches.end()))
				continue;

			Batch& batch = itFoundBatch->second;
			batch.IsMeshDirty = true;

			const auto itFoundDepthRange = std::find_if(mDepthRanges.begin(), mDepthRanges.end(), [depthRangeId = batch.DepthRangeId](const BatchesInDepthRange& depthRange)
														{ return depthRange.Id == depthRangeId; });

			if(!B3D_ENSURE(itFoundDepthRange != mDepthRanges.end()))
				continue;

			BatchesInDepthRange& depthRange = *itFoundDepthRange;
			bool isGroupChangeRequired = false;

			if((entry.second & DirtyMesh) != 0)
			{
				const u32 renderElementDepth = guiElement->GetDepth() + guiRenderElement.Depth;

				if(renderElementDepth != depthRange.MinDepth)
				{
					if(renderElementDepth < depthRange.MinDepth || (renderElementDepth >= (depthRange.MinDepth + depthRange.DepthRange)) || !guiRenderElement.Material->AllowBatching())
					{
						isGroupChangeRequired = true;
					}
				}
			}

			if(!isGroupChangeRequired && (entry.second & DirtyContent) != 0)
			{
				auto itFoundRenderElement = std::find_if(batch.RenderElements.begin(), batch.RenderElements.end(), [guiElement, elementIndex](const BatchedGUIRenderElement& batchedRenderElement)
														 { return batchedRenderElement.ParentGUIElement == guiElement && batchedRenderElement.RenderElementIndex == elementIndex; });

				if(itFoundRenderElement == batch.RenderElements.end())
				{
					// New element
					isGroupChangeRequired = true;
					continue;
				}

				BatchedMaterial batchedMaterial = CreateBatchedMaterial(*guiElement, elementIndex);
				if(!batch.Material.CanBeMergedWith(batchedMaterial))
				{
					isGroupChangeRequired = true;
				}
			}

			const u32 depthRangeIndex = (u32)(itFoundDepthRange - mDepthRanges.begin());
			if(isGroupChangeRequired)
			{
				Remove(batchedGuiElement, elementIndex, depthRangeIndex);
				Add(batchedGuiElement, elementIndex);

				mBatchesOutOfDateInRenderer = true;
			}
			else
			{
				if((entry.second & DirtyContent) != 0) // If content is dirty we already checked for dirty bounds
					MarkBoundsDirty(batchedGuiElement, batchId);
				else
				{
					// Clip area could have changed even if contents did not change
					Area2I bounds = guiElement->GetAbsoluteClippedArea().To<i32, u32>();
					if(batchedGuiElement.Bounds != bounds)
					{
						MarkBoundsDirty(batchedGuiElement);

						dirtyBounds = true;
						batchedGuiElement.Bounds = bounds;

						MarkBoundsDirty(batchedGuiElement);
					}
				}
			}

			if(dirtyBounds)
				batch.IsBoundsDirty = dirtyBounds;
		}
	}
	
		// TODO - Need to add handling for invisible GUI elements. Those should ideally not be part of the batches at all.

	mDirtyElements.clear();

	// Update dirty bounds
	for(const auto& depthRange : mDepthRanges)
	{
		for(const auto& batchId : depthRange.BatchIds)
		{
			auto itFoundBatch = mBatches.find(batchId);
			if(!B3D_ENSURE(itFoundBatch != mBatches.end()))
				continue;

			Batch& batch = itFoundBatch->second;
			if(!batch.IsBoundsDirty)
				continue;

			batch.Bounds = CalculateBounds(batch);
			batch.IsBoundsDirty = false;
		}
	}

	// Rebuild draw group meshes if needed
	// Note: Ideally we can avoid rebuilding all meshes any rebuild only the changed ones
	if(shouldRebuildMeshes)
		RebuildMeshes();

	// Return data required for updating the renderer
	GUIDrawGroupRenderDataUpdate output;

	for(const auto& depthRange : mDepthRanges)
	{
		for(const auto& batchId : depthRange.BatchIds)
		{
			auto itFoundBatch = mBatches.find(batchId);
			if(!B3D_ENSURE(itFoundBatch != mBatches.end()))
				continue;

			Batch& batch = itFoundBatch->second;
			for(const auto& dirtyRegion : batch.DirtyRegions)
				Area2I::AddUnique(dirtyRegion, output.DirtyRegions);

			batch.DirtyRegions.clear();
		}
	}

	for(const auto& dirtyRegion : mDirtyRegionsForRemovedBatches)
		Area2I::AddUnique(dirtyRegion, output.DirtyRegions);

	mDirtyRegionsForRemovedBatches.clear();

	// Note: If only mesh rebuild happened, we should only update the specific render elements
	// that changed. (Note that in this case the mesh rebuild flag also signals changes to the
	// GUI element texture/tint/etc.)
	if(mBatchesOutOfDateInRenderer || shouldRebuildMeshes)
	{
		for(const auto& depthRange : mDepthRanges)
		{
			for(const auto& batchId : depthRange.BatchIds)
			{
				auto itFoundBatch = mBatches.find(batchId);
				if(!B3D_ENSURE(itFoundBatch != mBatches.end()))
					continue;

				Batch& batch = itFoundBatch->second;
				output.Batches.push_back(GetRenderData(batch));
			}
		}

		// Register elements that depend on render textures (currently these always correspond to input bridged elements)
		TInlineArray<std::pair<const GUIInteractable*, SPtr<const RenderTarget>>, 4> bridgedElements;
		GetGUIManager().GetBridgedElements(mWidget, bridgedElements);

		for(auto& entry : bridgedElements)
		{
			auto* element = const_cast<GUIInteractable*>(entry.first);
			auto found = mElements.find(element);

			if(found == mElements.end())
				continue;

			const SPtr<const RenderTarget>& target = entry.second;
			for(auto& batchId : found->second.BatchPerRenderElement)
			{
				for(auto& batchRenderData : output.Batches)
				{
					if(batchRenderData.Id != batchId)
						continue;

					batchRenderData.RenderTargetElements.emplace_back(GUIRenderTargetRenderData(B3DGetRenderProxy(target), element->GetAbsoluteClippedArea().To<i32, u32>()));
				}
			}
		}
	}

	mBatchesOutOfDateInRenderer = false;
	return output;
}

void GUIMeshBatches::MarkContentDirty(GUIRenderable* guiElement)
{
	mDirtyElements[guiElement] |= DirtyContent;
}

void GUIMeshBatches::MarkMeshDirty(GUIRenderable* guiElement)
{
	mDirtyElements[guiElement] |= DirtyMesh;
}

void GUIMeshBatches::MarkBoundsDirty(const BatchedGUIElement& element)
{
	u32 previousBatchId = ~0u;
	for(const u32 batchId : element.BatchPerRenderElement)
	{
		// Usually render elements will be part of the same draw group, so exit early as an optimization
		if(previousBatchId == batchId)
			continue;

		auto itFoundBatch = mBatches.find(batchId);
		if(!B3D_ENSURE(itFoundBatch != mBatches.end()))
			continue;

		Batch& batch = itFoundBatch->second;

		Area2I::AddUnique(element.Bounds, batch.DirtyRegions);
		previousBatchId = batchId;
	}
}

void GUIMeshBatches::MarkBoundsDirty(const BatchedGUIElement& element, u32 batchId)
{
	auto itFoundBatch = mBatches.find(batchId);
	if(!B3D_ENSURE(itFoundBatch != mBatches.end()))
		return;

	Batch& batch = itFoundBatch->second;
	Area2I::AddUnique(element.Bounds, batch.DirtyRegions);
}

void GUIMeshBatches::RebuildMesh(Batch& batch)
{
	FrameAllocatorScope frameScope;

	batch.IndexCount = 0;
	batch.VertexCount = 0;

	for(const auto& batchedGuiRenderElement : batch.RenderElements)
	{
		const GUIRenderable* const guiElement = batchedGuiRenderElement.ParentGUIElement;
		B3D_ASSERT(guiElement != nullptr);

		// Culled elements shouldn't have been part of the batch in the first place. We should do the same for explicitly hidden elements.
		B3D_ENSURE(!guiElement->IsCulled());

		if(guiElement->IsHidden())
			continue;

		const TInlineArray<GUIRenderElement, 4>& guiRenderElements = guiElement->GetRenderElements();
		const GUIRenderElement& guiRenderElement = guiRenderElements[batchedGuiRenderElement.RenderElementIndex];

		batch.VertexCount += guiRenderElement.VertexCount;
		batch.IndexCount += guiRenderElement.IndexCount;
	}

	batch.Mesh = nullptr;

	if(batch.IndexCount == 0 || batch.VertexCount == 0)
		return;

	const SPtr<VertexDescription> vertexDescription = batch.Material.MeshType == GUIMeshType::Triangle ? GetGUITriangleMeshDesc() : GetGUILineMeshDesc();
	const SPtr<MeshData> meshData = MeshData::Create(batch.VertexCount, batch.IndexCount, vertexDescription);
	u8* positionData = meshData->GetElementData(VES_POSITION);
	u8* uvData = vertexDescription->HasElement(VES_TEXCOORD) ? meshData->GetElementData(VES_TEXCOORD) : nullptr;
	u32* indexData = meshData->GetIndices32();

	DataRange positions(positionData, batch.VertexCount, vertexDescription->GetVertexStride());
	DataRange uv(uvData, batch.VertexCount, vertexDescription->GetVertexStride());
	DataRange indices(indexData, batch.IndexCount, meshData->GetIndexElementSize());

	u32 vertexOffset = 0;
	u32 indexOffset = 0;

	for(const auto& batchedGuiRenderElement : batch.RenderElements)
	{
		const GUIRenderable* const guiElement = batchedGuiRenderElement.ParentGUIElement;
		B3D_ASSERT(guiElement != nullptr);

		// Culled elements shouldn't have been part of the batch in the first place. We should do the same for explicitly hidden elements.
		B3D_ENSURE(!guiElement->IsCulled());

		if(guiElement->IsHidden())
			continue;

		const TInlineArray<GUIRenderElement, 4>& guiRenderElements = guiElement->GetRenderElements();
		const GUIRenderElement& guiRenderElement = guiRenderElements[batchedGuiRenderElement.RenderElementIndex];

		if(!guiRenderElement.UseNewFillBuffer)
			guiElement->FillBuffer(positionData, indexData, vertexOffset, indexOffset, Vector2I::kZero, batch.VertexCount, batch.IndexCount, batchedGuiRenderElement.RenderElementIndex); // DEPRECATED
		else
			guiElement->GetRenderElementVertexAndIndexData(batchedGuiRenderElement.RenderElementIndex, vertexOffset, indexOffset, positions, uv, indices);

		const u32 indexStart = indexOffset;
		const u32 indexEnd = indexStart + guiRenderElement.IndexCount;

		for(u32 indexIndex = indexStart; indexIndex < indexEnd; indexIndex++)
			indexData[indexIndex] += vertexOffset;

		indexOffset += guiRenderElement.IndexCount;
		vertexOffset += guiRenderElement.VertexCount;
	}

	batch.Mesh = Mesh::CreateShared(meshData, MeshFlag::Static | MeshFlag::UnorderedAccess, batch.Material.MeshType == GUIMeshType::Triangle ? DOT_TRIANGLE_LIST : DOT_LINE_LIST);
	batch.IsMeshDirty = false;
}

void GUIMeshBatches::RebuildMeshes()
{
	for(const auto& depthRange : mDepthRanges)
	{
		for(const auto& batchId : depthRange.BatchIds)
		{
			auto itFoundBatch = mBatches.find(batchId);
			if(!B3D_ENSURE(itFoundBatch != mBatches.end()))
				continue;

			Batch& batch = itFoundBatch->second;
			RebuildMesh(batch);
		}
	}
}

u32 GUIMeshBatches::SplitDepthRange(u32 depthRangeIndex, u32 depth)
{
	BatchesInDepthRange& depthRange = mDepthRanges[depthRangeIndex];
	B3D_ASSERT(depth > depthRange.MinDepth);

	const u32 maximumDepth = depthRange.MinDepth + depthRange.DepthRange;
	depthRange.DepthRange = depth - depthRange.MinDepth;

	BatchesInDepthRange newDepthRange;
	newDepthRange.MinDepth = depth;
	newDepthRange.DepthRange = maximumDepth - newDepthRange.MinDepth;
	newDepthRange.Id = mNextDepthRangeId++;

	for(auto it = depthRange.BatchIds.begin(); it != depthRange.BatchIds.end();)
	{
		const u32 batchId = *it;

		auto foundBatch = mBatches.find(batchId);
		if(!B3D_ENSURE(foundBatch != mBatches.end()))
		{
			++it;
			continue;
		}

		Batch* batch = &foundBatch->second;
		Batch newBatch;

		auto partitionEdge = std::stable_partition(batch->RenderElements.begin(), batch->RenderElements.end(), [depth](const BatchedGUIRenderElement& batchedGuiRenderElement)
			 {
				 return batchedGuiRenderElement.Depth < depth;
			 });

		std::move(partitionEdge, batch->RenderElements.end(), std::back_inserter(newBatch.RenderElements));
		batch->RenderElements.erase(partitionEdge, batch->RenderElements.end());
		batch->IsBoundsDirty = true;
		batch->IsMeshDirty = true;

		const bool isBatchEmpty = batch->RenderElements.empty();

		if(!newBatch.RenderElements.empty())
		{
			newBatch.Id = AllocateBatchId();
			newBatch.DepthRangeId = newDepthRange.Id;
			newBatch.Material = batch->Material;
			newBatch.IsBoundsDirty = true;

			batch = nullptr; // Not safe to access past this point, as we're modifying the map below

			newDepthRange.BatchIds.push_back(newBatch.Id);

			for(const auto& batchedGuiRenderElement : newBatch.RenderElements)
			{
				auto foundGuiElement = mElements.find(batchedGuiRenderElement.ParentGUIElement);
				if(!B3D_ENSURE(foundGuiElement != mElements.end()))
					continue;

				foundGuiElement->second.BatchPerRenderElement[batchedGuiRenderElement.RenderElementIndex] = newBatch.Id;
				Area2I::AddUnique(foundGuiElement->second.Bounds, newBatch.DirtyRegions);
			}

			mBatches[newBatch.Id] = newBatch;
		}

		if(isBatchEmpty)
		{
			FreeBatchId(batchId);
			mBatches.erase(batchId);
			it = depthRange.BatchIds.erase(it);
		}
		else
			++it;
	}

	mDepthRanges.insert(mDepthRanges.begin() + depthRangeIndex + 1, std::move(newDepthRange));
	return depthRangeIndex + 1;
}

bool GUIMeshBatches::CollapseDepthRange(u32 depthRangeIndex)
{
	if(depthRangeIndex < 1)
		return false;

	BatchesInDepthRange& depthRange = mDepthRanges[depthRangeIndex - 1];
	BatchesInDepthRange& nextDepthRange = mDepthRanges[depthRangeIndex];

	// We can only combine depth ranges with one or no batches, so we can ensure draw order
	if(depthRange.BatchIds.size() > 1 || nextDepthRange.BatchIds.size() > 1)
		return false;

	Batch* batch = nullptr;
	Batch* nextBatch = nullptr;

	if(!depthRange.BatchIds.empty())
	{
		auto foundBatch = mBatches.find(depthRange.BatchIds.back());
		if(!B3D_ENSURE(foundBatch != mBatches.end()))
			return false;

		batch = &foundBatch->second;
	}

	if(!nextDepthRange.BatchIds.empty())
	{
		auto foundBatch = mBatches.find(nextDepthRange.BatchIds.back());
		if(!B3D_ENSURE(foundBatch != mBatches.end()))
			return false;

		nextBatch = &foundBatch->second;
	}

	if(batch != nullptr && nextBatch != nullptr)
	{
		if(!batch->Material.CanBeMergedWith(nextBatch->Material))
			return false;

		batch->Material.Merge(nextBatch->Material);

		for(auto& entry : nextBatch->RenderElements)
		{
			auto foundGuiElement = mElements.find(entry.ParentGUIElement);
			if(!B3D_ENSURE(foundGuiElement != mElements.end()))
				continue;

			if(foundGuiElement != mElements.end())
				foundGuiElement->second.BatchPerRenderElement[entry.RenderElementIndex] = batch->Id;

			Area2I::AddUnique(foundGuiElement->second.Bounds, batch->DirtyRegions);
			batch->IsBoundsDirty = true;
			batch->IsMeshDirty = true;
		}

		batch->RenderElements.insert(batch->RenderElements.begin(), nextBatch->RenderElements.begin(), nextBatch->RenderElements.end());

		for(const Area2I& dirtyRegion : nextBatch->DirtyRegions)
			Area2I::AddUnique(dirtyRegion, batch->DirtyRegions);

		mBatches.erase(nextBatch->Id);
	}
	else if(nextBatch != nullptr)
	{
		nextBatch->DepthRangeId = depthRange.Id;
		depthRange.BatchIds.push_back(nextBatch->Id);
	}

	depthRange.DepthRange += nextDepthRange.DepthRange;
	mDepthRanges.erase(mDepthRanges.begin() + depthRangeIndex);

	return true;
}

u32 GUIMeshBatches::AllocateBatchId()
{
	if(mFreeBatchIds.empty())
		return mNextBatchId++;

	const u32 id = mFreeBatchIds.back();
	mFreeBatchIds.pop_back();

	return id;
}

void GUIMeshBatches::FreeBatchId(u32 id)
{
	mFreeBatchIds.push_back(id);
}

GUIBatchRenderData GUIMeshBatches::GetRenderData(const Batch& batch)
{
	const BatchedMaterial& material = batch.Material;

	GUIBatchRenderData batchRenderData;
	batchRenderData.Id = batch.Id;
	batchRenderData.Bounds = batch.Bounds;

	GUIMeshRenderData meshRenderData;
	meshRenderData.Mesh = B3DGetRenderProxy(batch.Mesh);
	meshRenderData.Material = material.SpriteMaterial;
	meshRenderData.MaterialInformation = material.SpriteMaterialInformation;
	meshRenderData.GpuParametersIndex = 0;
	meshRenderData.Bounds = batch.Bounds;

	meshRenderData.SubMesh.IndexOffset = 0;
	meshRenderData.SubMesh.IndexCount = batch.IndexCount;
	meshRenderData.SubMesh.DrawOp = material.MeshType == GUIMeshType::Line ? DOT_LINE_LIST : DOT_TRIANGLE_LIST;

	if(meshRenderData.SubMesh.IndexCount == 0)
		return batchRenderData;

	batchRenderData.Elements.push_back(std::move(meshRenderData));
	return batchRenderData;
}

Area2I GUIMeshBatches::CalculateBounds(Batch& batch)
{
	Area2I bounds = Area2I();
	bool hasBounds = false;

	for(auto& entry : batch.RenderElements)
	{
		// Culled elements shouldn't have been part of the batch in the first place. We should do the same for explicitly hidden elements.
		B3D_ENSURE(!entry.ParentGUIElement->IsCulled());

		if(entry.ParentGUIElement->IsHidden())
			continue;

		Area2I elementBounds = entry.ParentGUIElement->GetAbsoluteClippedArea().To<i32, u32>();
		if(!hasBounds)
		{
			bounds = elementBounds;
			hasBounds = true;
		}
		else
			bounds.Encapsulate(elementBounds);
	}

	return bounds;
}

GUIMeshBatches::BatchedMaterial GUIMeshBatches::CreateBatchedMaterial(const BatchedGUIRenderElement& batchedGuiRenderElement)
{
	GUIRenderable* const guiElement = batchedGuiRenderElement.ParentGUIElement;
	B3D_ASSERT(guiElement != nullptr);

	return CreateBatchedMaterial(*guiElement, batchedGuiRenderElement.RenderElementIndex);
}

GUIMeshBatches::BatchedMaterial GUIMeshBatches::CreateBatchedMaterial(const GUIRenderable& guiElement, u32 renderElementIndex)
{
	const TInlineArray<GUIRenderElement, 4>& guiRenderElements = guiElement.GetRenderElements();
	const GUIRenderElement& guiRenderElement = guiRenderElements[renderElementIndex];

	BatchedMaterial batchedMaterial;
	batchedMaterial.SpriteMaterial = guiRenderElement.Material;
	batchedMaterial.SpriteMaterialInformation = *guiRenderElement.MaterialInformation;
	batchedMaterial.MaterialHash = batchedMaterial.SpriteMaterial->GetMergeHash(batchedMaterial.SpriteMaterialInformation);
	batchedMaterial.IsBatchingAllowed = batchedMaterial.SpriteMaterial->AllowBatching();
	batchedMaterial.MeshType = guiRenderElement.Type;

	return batchedMaterial;
}
