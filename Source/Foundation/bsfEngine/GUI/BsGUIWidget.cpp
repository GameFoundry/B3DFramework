//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIWidget.h"
#include "GUI/BsGUIManager.h"
#include "GUI/BsGUISkin.h"
#include "GUI/BsGUILabel.h"
#include "GUI/BsGUIPanel.h"
#include "GUI/BsGUINavGroup.h"
#include "Math/BsVector2I.h"
#include "Mesh/BsMesh.h"
#include "Components/BsCCamera.h"
#include "RenderAPI/BsViewport.h"
#include "Scene/BsSceneObject.h"
#include "Resources/BsBuiltinResources.h"
#include "RenderAPI/BsVertexDescription.h"
#include "Image/BsSpriteTexture.h"
#include "RenderAPI/BsRenderTexture.h"

using namespace bs;

SPtr<VertexDescription> GetGUITriangleMeshDesc()
{
	static SPtr<VertexDescription> sDesc;

	if(!sDesc)
	{
		SmallVector<VertexElement, 8> vertexElements;
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
		SmallVector<VertexElement, 8> vertexElements;
		vertexElements.Add(VertexElement(VET_FLOAT2, VES_POSITION));

		sDesc = B3DMakeShared<VertexDescription>(vertexElements);
	}

	return sDesc;
}

GUIMeshBatches::GUIMeshBatches(GUIWidget* parentWidget)
	: mWidget(parentWidget)
{
	BatchesInDepthRange mainDrawGroup;
	mainDrawGroup.MinDepth = 0;
	mainDrawGroup.DepthRange = std::numeric_limits<u32>::max();
	mainDrawGroup.Id = mNextDepthRangeId++;

	mDepthRanges.push_back(mainDrawGroup);
}

void GUIMeshBatches::Add(GUIElement* guiElement)
{
	const SmallVector<GUIRenderElement, 4>& guiRenderElements = guiElement->GetRenderElements();

	BatchedGUIElement& batchedGuiElement = mElements[guiElement];
	batchedGuiElement.GUIElement = guiElement;
	batchedGuiElement.Bounds = guiElement->GetClippedBounds();
	batchedGuiElement.BatchPerRenderElement.Resize(guiRenderElements.Size(), ~0u);

	for(u32 renderElementIndex = 0; renderElementIndex < (u32)guiRenderElements.Size(); renderElementIndex++)
		Add(batchedGuiElement, renderElementIndex);

	mBatchesOutOfDateInRenderer = true;
}

void GUIMeshBatches::Add(BatchedGUIElement& batchedGuiElement, u32 renderElementIndex)
{
	GUIElement* const guiElement = batchedGuiElement.GUIElement;
	const SmallVector<GUIRenderElement, 4>& renderElements = guiElement->GetRenderElements();

	const GUIRenderElement& guiRenderElement = renderElements[renderElementIndex];
	const u32 renderElementDepth = guiElement->GetDepth() + guiRenderElement.Depth;

	// Depth ranges are sorted by MinDepth
	for(u32 depthRangeIndex = 0; depthRangeIndex < (u32)mDepthRanges.size(); depthRangeIndex++)
	{
		if(renderElementDepth < mDepthRanges[depthRangeIndex].MinDepth || renderElementDepth >= (mDepthRanges[depthRangeIndex].MinDepth + mDepthRanges[depthRangeIndex].DepthRange))
			continue;

		Add(batchedGuiElement, renderElementIndex, depthRangeIndex);
		break;
	}
}

void GUIMeshBatches::Add(BatchedGUIElement& batchedGuiElement, u32 renderElementIndex, u32 depthRangeIndex)
{
	GUIElement* const guiElement = batchedGuiElement.GUIElement;
	const SmallVector<GUIRenderElement, 4>& guiRenderElements = guiElement->GetRenderElements();

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
				bool isNextElementFound = false;

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
						isNextElementFound = true;

						// Early out
						if(minimumDepth == renderElementDepth)
							break;
					}
				}

				if(isNextElementFound)
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

	foundBatch->RenderElements.push_back(batchedGuiRenderElement);
	batchedGuiElement.BatchPerRenderElement[batchedGuiRenderElement.RenderElementIndex] = foundBatch->Id;

	B3D_ASSERT(batchedGuiRenderElement.ParentGUIElement != nullptr);

	const Rect2I bounds = batchedGuiRenderElement.ParentGUIElement->GetClippedBounds();
	foundBatch->Bounds.Encapsulate(bounds);

	MarkBoundsDirty(batchedGuiElement, foundBatch->Id);

	return foundBatch;
}

void GUIMeshBatches::Remove(GUIElement* guiElement)
{
	auto found = mElements.find(guiElement);
	if(found == mElements.end())
		return;

	for(u32 renderElementIndex = 0; renderElementIndex < found->second.BatchPerRenderElement.Size(); renderElementIndex++)
		Remove(found->second, renderElementIndex);

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

	GUIElement* const guiElement = batchedGuiElement.GUIElement;
	BatchesInDepthRange& depthRange = mDepthRanges[depthRangeIndex];

	bool hasFoundElementToRemove = false;
	Batch& batch = itFoundBatch->second;
	for(auto itRenderElement = batch.RenderElements.begin(); itRenderElement != batch.RenderElements.end();)
	{
		if(itRenderElement->ParentGUIElement == guiElement && itRenderElement->RenderElementIndex == renderElementIndex)
		{
			batch.IsBoundsDirty = true;

			Rect2I::AddUnique(batchedGuiElement.Bounds, batch.DirtyRegions);

			itRenderElement = batch.RenderElements.erase(itRenderElement);
			hasFoundElementToRemove = true;
			break;
		}
		else
		{
			++itRenderElement;
		}
	}

	B3D_ASSERT(hasFoundElementToRemove);

	if(batch.RenderElements.empty())
	{
		for(auto itBatchId = depthRange.BatchIds.begin(); itBatchId != depthRange.BatchIds.end();)
		{
			if(*itBatchId == batchId)
			{
				for(const auto& dirtyRegion : batch.DirtyRegions)
					Rect2I::AddUnique(dirtyRegion, mDirtyRegionsForRemovedBatches);

				FreeBatchId(batch.Id);

				mBatches.erase(itFoundBatch);
				itBatchId = depthRange.BatchIds.erase(itBatchId);

				break;
			}
			else
				++itBatchId;
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
		GUIElement* const guiElement = entry.first;

		auto itFoundElement = mElements.find(guiElement);
		if(itFoundElement == mElements.end())
			continue;

		shouldRebuildMeshes = true;

		const SmallVector<GUIRenderElement, 4>& guiRenderElements = guiElement->GetRenderElements();
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
			Rect2I bounds = guiElement->GetClippedBounds();
			if(batchedGuiElement.Bounds != bounds)
			{
				MarkBoundsDirty(batchedGuiElement);

				dirtyBounds = true;
				batchedGuiElement.Bounds = bounds;

				MarkBoundsDirty(batchedGuiElement);
			}
		}

		for(u32 renderElementIndex = 0; renderElementIndex < (u32)guiRenderElements.size(); renderElementIndex++)
		{
			const GUIRenderElement& guiRenderElement = guiRenderElements[renderElementIndex];
			const u32 batchId = batchedGuiElement.BatchPerRenderElement[renderElementIndex];

			B3D_ASSERT(batchId != ~0u);

			auto itFoundBatch = mBatches.find(batchId);
			if(!B3D_ENSURE(itFoundBatch != mBatches.end()))
				continue;

			Batch& batch = itFoundBatch->second;

			const auto itFoundDepthRange = std::find_if(mDepthRanges.begin(), mDepthRanges.end(), [depthRangeId = batch.DepthRangeId](const BatchesInDepthRange& depthRange)
														{ return depthRange.Id == depthRangeId; });

			if(!B3D_ENSURE(itFoundDepthRange != mDepthRanges.end()))
				continue;

			BatchesInDepthRange& depthRange = *itFoundDepthRange;
			if(dirtyBounds)
				batch.IsBoundsDirty = dirtyBounds;

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
				auto itFoundRenderElement = std::find_if(batch.RenderElements.begin(), batch.RenderElements.end(), [guiElement, renderElementIndex](const BatchedGUIRenderElement& batchedRenderElement)
														 { return batchedRenderElement.ParentGUIElement == guiElement && batchedRenderElement.RenderElementIndex == renderElementIndex; });

				if(itFoundRenderElement == batch.RenderElements.end())
				{
					// New element
					isGroupChangeRequired = true;
					continue;
				}

				BatchedMaterial batchedMaterial = CreateBatchedMaterial(*guiElement, renderElementIndex);
				if(!batch.Material.CanBeMergedWith(batchedMaterial))
				{
					isGroupChangeRequired = true;
				}
			}

			const u32 depthRangeIndex = (u32)(itFoundDepthRange - mDepthRanges.begin());
			if(isGroupChangeRequired)
			{
				Remove(batchedGuiElement, renderElementIndex, depthRangeIndex);
				Add(batchedGuiElement, renderElementIndex);

				mBatchesOutOfDateInRenderer = true;
			}
			else
			{
				MarkBoundsDirty(batchedGuiElement, batchId);
			}
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
	output.TriangleMesh = mTriangleMesh ? mTriangleMesh->GetCore() : nullptr;
	output.LineMesh = mLineMesh ? mLineMesh->GetCore() : nullptr;

	for(const auto& depthRange : mDepthRanges)
	{
		for(const auto& batchId : depthRange.BatchIds)
		{
			auto itFoundBatch = mBatches.find(batchId);
			if(!B3D_ENSURE(itFoundBatch != mBatches.end()))
				continue;

			Batch& batch = itFoundBatch->second;
			for(const auto& dirtyRegion : batch.DirtyRegions)
				Rect2I::AddUnique(dirtyRegion, output.DirtyRegions);

			batch.DirtyRegions.clear();
		}
	}

	for(const auto& dirtyRegion : mDirtyRegionsForRemovedBatches)
		Rect2I::AddUnique(dirtyRegion, output.DirtyRegions);

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
				output.NewBatches.push_back(GetRenderData(batch));
			}
		}

		// Register elements that depend on render textures (currently these always correspond to input bridged elements)
		SmallVector<std::pair<const GUIElement*, SPtr<const RenderTarget>>, 4> bridgedElements;
		GetGUIManager().GetBridgedElements(mWidget, bridgedElements);

		for(auto& entry : bridgedElements)
		{
			auto* element = const_cast<GUIElement*>(entry.first);
			auto iterFind = mElements.find(element);

			B3D_ASSERT(iterFind != mElements.end());
			if(iterFind == mElements.end())
				continue;

			const SPtr<const RenderTarget>& target = entry.second;
			for(auto& batchId : iterFind->second.BatchPerRenderElement)
			{
				for(auto& batchRenderData : output.NewBatches)
				{
					if(batchRenderData.Id != batchId)
						continue;

					batchRenderData.RenderTargetElements.emplace_back(GUIRenderTargetRenderData(target->GetCore(), element->GetClippedBounds()));
				}
			}
		}
	}

	mBatchesOutOfDateInRenderer = false;
	return output;
}

void GUIMeshBatches::MarkContentDirty(GUIElement* guiElement)
{
	mDirtyElements[guiElement] |= DirtyContent;
}

void GUIMeshBatches::MarkMeshDirty(GUIElement* guiElement)
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

		Rect2I::AddUnique(element.Bounds, batch.DirtyRegions);
		previousBatchId = batchId;
	}
}

void GUIMeshBatches::MarkBoundsDirty(const BatchedGUIElement& element, u32 batchId)
{
	auto itFoundBatch = mBatches.find(batchId);
	if(!B3D_ENSURE(itFoundBatch != mBatches.end()))
		return;

	Batch& batch = itFoundBatch->second;
	Rect2I::AddUnique(element.Bounds, batch.DirtyRegions);
}

void GUIMeshBatches::RebuildMeshes()
{
	FrameScope frameScope;

	// Make a list of all GUI elements, sorted from farthest to nearest (highest depth to lowest)
	auto fnCompareRenderElementDepth = [](const BatchedGUIRenderElement& a, const BatchedGUIRenderElement& b)
	{
		// Compare pointers just to differentiate between two elements with the same depth, their order doesn't really matter, but std::set requires all elements to be unique
		return (a.Depth > b.Depth) ||
			(a.Depth == b.Depth && a.ParentGUIElement > b.ParentGUIElement) ||
			(a.Depth == b.Depth && a.ParentGUIElement == b.ParentGUIElement && a.RenderElementIndex > b.RenderElementIndex);
	};

	for(const auto& depthRange : mDepthRanges)
	{
		for(const auto& batchId : depthRange.BatchIds)
		{
			auto itFoundBatch = mBatches.find(batchId);
			if(!B3D_ENSURE(itFoundBatch != mBatches.end()))
				continue;

			Batch& batch = itFoundBatch->second;
			batch.IndexCount = 0;
			batch.VertexCount = 0;

			// TODO - This should be done when I add the element to the batch
			std::sort(batch.RenderElements.begin(), batch.RenderElements.end(), fnCompareRenderElementDepth);

			for(const auto& batchedGuiRenderElement : batch.RenderElements)
			{
				const GUIElement* const guiElement = batchedGuiRenderElement.ParentGUIElement;
				B3D_ASSERT(guiElement != nullptr);

				if(!guiElement->IsVisible())
					continue;

				const SmallVector<GUIRenderElement, 4>& guiRenderElements = guiElement->GetRenderElements();
				const GUIRenderElement& guiRenderElement = guiRenderElements[batchedGuiRenderElement.RenderElementIndex];

				batch.VertexCount += guiRenderElement.NumVertices;
				batch.IndexCount += guiRenderElement.NumIndices;
			}
		}
	}

	mTriangleMesh = nullptr;
	mLineMesh = nullptr;

	u32 totalNumIndices[2] = { 0, 0 };
	u32 totalNumVertices[2] = { 0, 0 };

	for(const auto& depthRange : mDepthRanges)
	{
		for(const auto& batchId : depthRange.BatchIds)
		{
			auto itFoundBatch = mBatches.find(batchId);
			if(!B3D_ENSURE(itFoundBatch != mBatches.end()))
				continue;

			Batch& batch = itFoundBatch->second;

			const u32 meshTypeIndex = batch.Material.MeshType == GUIMeshType::Triangle ? 0 : 1;
			totalNumIndices[meshTypeIndex] += batch.IndexCount;
			totalNumVertices[meshTypeIndex] += batch.VertexCount;
		}
	}

	SPtr<MeshData> meshData[2];
	SPtr<VertexDescription> vertexDesc[2] = { GetGUITriangleMeshDesc(), GetGUILineMeshDesc() };

	u8* vertices[2] = { nullptr, nullptr };
	u32* indices[2] = { nullptr, nullptr };

	for(u32 i = 0; i < 2; i++)
	{
		if(totalNumVertices[i] > 0 && totalNumIndices[i] > 0)
		{
			meshData[i] = MeshData::Create(totalNumVertices[i], totalNumIndices[i], vertexDesc[i]);

			vertices[i] = meshData[i]->GetElementData(VES_POSITION);
			indices[i] = meshData[i]->GetIndices32();
		}
	}

	u32 vertexOffset[2] = { 0, 0 };
	u32 indexOffset[2] = { 0, 0 };

	for(const auto& depthRange : mDepthRanges)
	{
		for(const auto& batchId : depthRange.BatchIds)
		{
			auto itFoundBatch = mBatches.find(batchId);
			if(!B3D_ENSURE(itFoundBatch != mBatches.end()))
				continue;

			Batch& batch = itFoundBatch->second;
			const u32 meshTypeIndex = batch.Material.MeshType == GUIMeshType::Triangle ? 0 : 1;

			batch.IndexOffset = indexOffset[meshTypeIndex];

			const Vector2I groupOffset = Vector2I::kZero;
			for(const auto& batchedGuiRenderElement : batch.RenderElements)
			{
				const GUIElement* const guiElement = batchedGuiRenderElement.ParentGUIElement;
				B3D_ASSERT(guiElement != nullptr);

				if(!guiElement->IsVisible())
					continue;

				const SmallVector<GUIRenderElement, 4>& guiRenderElements = guiElement->GetRenderElements();

				guiElement->FillBuffer(
					vertices[meshTypeIndex], indices[meshTypeIndex],
					vertexOffset[meshTypeIndex], indexOffset[meshTypeIndex], groupOffset,
					totalNumVertices[meshTypeIndex], totalNumIndices[meshTypeIndex],
					batchedGuiRenderElement.RenderElementIndex);

				const GUIRenderElement& guiRenderElement = guiRenderElements[batchedGuiRenderElement.RenderElementIndex];
				const u32 indexStart = indexOffset[meshTypeIndex];
				const u32 indexEnd = indexStart + guiRenderElement.NumIndices;

				for(u32 j = indexStart; j < indexEnd; j++)
					indices[meshTypeIndex][j] += vertexOffset[meshTypeIndex];

				indexOffset[meshTypeIndex] += guiRenderElement.NumIndices;
				vertexOffset[meshTypeIndex] += guiRenderElement.NumVertices;
			}
		}
	}

	if(meshData[0])
		mTriangleMesh = Mesh::CreateShared(meshData[0], MU_STATIC, DOT_TRIANGLE_LIST);

	if(meshData[1])
		mLineMesh = Mesh::CreateShared(meshData[1], MU_STATIC, DOT_LINE_LIST);
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

	for(auto itBatchId = depthRange.BatchIds.begin(); itBatchId != depthRange.BatchIds.end();)
	{
		const u32 batchId = *itBatchId;

		auto foundBatch = mBatches.find(batchId);
		if(!B3D_ENSURE(foundBatch != mBatches.end()))
		{
			++itBatchId;
			continue;
		}

		Batch& batch = foundBatch->second;
		Batch newBatch;

		auto itPartitionEdge = std::partition(batch.RenderElements.begin(), batch.RenderElements.end(), [depth](const BatchedGUIRenderElement& batchedGuiRenderElement)
			 {
				 const u32 renderElementDepth = batchedGuiRenderElement.ParentGUIElement->GetDepth() + batchedGuiRenderElement.ParentGUIElement->GetRenderElements()[batchedGuiRenderElement.RenderElementIndex].Depth;
				 return renderElementDepth < depth;
			 });

		std::move(itPartitionEdge, batch.RenderElements.end(), std::back_inserter(newBatch.RenderElements));
		batch.RenderElements.erase(itPartitionEdge, batch.RenderElements.end());
		batch.IsBoundsDirty = true;

		if(!newBatch.RenderElements.empty())
		{
			newBatch.Id = AllocateBatchId();
			newBatch.DepthRangeId = newDepthRange.Id;
			newBatch.Material = batch.Material;
			newBatch.IsBoundsDirty = true;

			mBatches[newBatch.Id] = newBatch;
			newDepthRange.BatchIds.push_back(newBatch.Id);

			for(const auto& batchedGuiRenderElement : newBatch.RenderElements)
			{
				auto itFoundGuiElement =  mElements.find(batchedGuiRenderElement.ParentGUIElement);
				if(!B3D_ENSURE(itFoundGuiElement != mElements.end()))
					continue;

				itFoundGuiElement->second.BatchPerRenderElement[batchedGuiRenderElement.RenderElementIndex] = newBatch.Id;
				Rect2I::AddUnique(itFoundGuiElement->second.Bounds, newBatch.DirtyRegions);
			}
		}

		if(batch.RenderElements.empty())
		{
			FreeBatchId(batch.Id);
			mBatches.erase(batchId);
			itBatchId = depthRange.BatchIds.erase(itBatchId);
		}
		else
			++itBatchId;
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

			Rect2I::AddUnique(foundGuiElement->second.Bounds, batch->DirtyRegions);
			batch->IsBoundsDirty = true;
		}

		std::move(nextBatch->RenderElements.begin(), nextBatch->RenderElements.end(), std::back_inserter(batch->RenderElements));

		for(const Rect2I& dirtyRegion : nextBatch->DirtyRegions)
			Rect2I::AddUnique(dirtyRegion, batch->DirtyRegions);

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
	meshRenderData.Material = material.SpriteMaterial;
	meshRenderData.MaterialInformation = material.SpriteMaterialInformation;
	meshRenderData.IsLine = material.MeshType == GUIMeshType::Line;
	meshRenderData.UniformBufferIndex = 0;
	meshRenderData.Bounds = batch.Bounds;

	meshRenderData.SubMesh.IndexOffset = batch.IndexOffset;
	meshRenderData.SubMesh.IndexCount = batch.IndexCount;
	meshRenderData.SubMesh.DrawOp = material.MeshType == GUIMeshType::Line ? DOT_LINE_LIST : DOT_TRIANGLE_LIST;

	if(meshRenderData.SubMesh.IndexCount == 0)
		return batchRenderData;

	if(meshRenderData.Material->AllowBatching())
		batchRenderData.CachedElements.push_back(std::move(meshRenderData));
	else
		batchRenderData.NonCachedElements.push_back(std::move(meshRenderData));

	return batchRenderData;
}

Rect2I GUIMeshBatches::CalculateBounds(Batch& batch)
{
	Rect2I bounds = Rect2I();
	bool boundsSet = false;

	for(auto& entry : batch.RenderElements)
	{
		if(!entry.ParentGUIElement->IsVisible())
			continue;

		Rect2I elementBounds = entry.ParentGUIElement->GetClippedBounds();
		if(!boundsSet)
		{
			bounds = elementBounds;
			boundsSet = true;
		}
		else
			bounds.Encapsulate(elementBounds);
	}

	return bounds;
}

GUIMeshBatches::BatchedMaterial GUIMeshBatches::CreateBatchedMaterial(const BatchedGUIRenderElement& batchedGuiRenderElement)
{
	GUIElement* const guiElement = batchedGuiRenderElement.ParentGUIElement;
	B3D_ASSERT(guiElement != nullptr);

	return CreateBatchedMaterial(*guiElement, batchedGuiRenderElement.RenderElementIndex);
}

GUIMeshBatches::BatchedMaterial GUIMeshBatches::CreateBatchedMaterial(const GUIElement& guiElement, u32 renderElementIndex)
{
	const SmallVector<GUIRenderElement, 4>& guiRenderElements = guiElement.GetRenderElements();
	const GUIRenderElement& guiRenderElement = guiRenderElements[renderElementIndex];

	BatchedMaterial batchedMaterial;
	batchedMaterial.SpriteMaterial = guiRenderElement.Material;
	batchedMaterial.SpriteMaterialInformation = *guiRenderElement.MatInfo;
	batchedMaterial.MaterialHash = batchedMaterial.SpriteMaterial->GetMergeHash(batchedMaterial.SpriteMaterialInformation);
	batchedMaterial.IsBatchingAllowed = batchedMaterial.SpriteMaterial->AllowBatching();
	batchedMaterial.MeshType = guiRenderElement.Type;

	return batchedMaterial;
}


GUIWidget::GUIWidget(const SPtr<Camera>& camera)
	: mCamera(camera), mDrawGroups(this)
{
	Construct(camera);
}

GUIWidget::GUIWidget(const HCamera& camera)
	: mCamera(camera->GetCameraInternal()), mDrawGroups(this)
{
	Construct(mCamera);
}

void GUIWidget::Construct(const SPtr<Camera>& camera)
{
	if(mCamera != nullptr)
	{
		SPtr<RenderTarget> target = mCamera->GetViewport()->GetTarget();

		if(target != nullptr)
			mCachedRTId = target->GetInternalId();
	}

	mDefaultNavGroup = GUINavGroup::Create();

	GUIManager::Instance().RegisterWidget(this);

	mPanel = GUIPanel::Create();
	mPanel->ChangeParentWidget(this);
	UpdateRootPanel();
}

GUIWidget::~GUIWidget()
{
	DestroyInternal();
}

SPtr<GUIWidget> GUIWidget::Create(const SPtr<Camera>& camera)
{
	return B3DMakeSharedFromExisting(new(B3DAllocate<GUIWidget>()) GUIWidget(camera));
}

SPtr<GUIWidget> GUIWidget::Create(const HCamera& camera)
{
	return B3DMakeSharedFromExisting(new(B3DAllocate<GUIWidget>()) GUIWidget(camera));
}

void GUIWidget::DestroyInternal()
{
	if(mPanel != nullptr)
	{
		GUILayout::Destroy(mPanel);
		mPanel = nullptr;
	}

	if(mCamera != nullptr)
	{
		GUIManager::Instance().UnregisterWidget(this);
		mCamera = nullptr;
	}

	mElements.clear();
	mDirtyContents.clear();
}

void GUIWidget::SetDepth(u8 depth)
{
	mDepth = depth;
	mWidgetIsDirty = true;

	UpdateRootPanel();
}

Viewport* GUIWidget::GetTarget() const
{
	if(mCamera != nullptr)
		return mCamera->GetViewport().get();

	return nullptr;
}

void GUIWidget::UpdateTransformInternal(const HSceneObject& parent)
{
	// If the widgets parent scene object moved, we need to mark it as dirty
	// as the GUIManager batching relies on object positions, so it needs to be updated.
	const float diffEpsilon = 0.0001f;

	const Transform& tfrm = parent->GetTransform();
	Vector3 position = tfrm.GetPosition();
	Quaternion rotation = tfrm.GetRotation();
	Vector3 scale = tfrm.GetScale();

	if(!mWidgetIsDirty)
	{
		if(!Math::ApproxEquals(mPosition, position, diffEpsilon))
			mWidgetIsDirty = true;
		else
		{
			if(!Math::ApproxEquals(mRotation, rotation, diffEpsilon))
				mWidgetIsDirty = true;
			else
			{
				if(Math::ApproxEquals(mScale, scale))
					mWidgetIsDirty = true;
			}
		}
	}

	mPosition = position;
	mRotation = rotation;
	mScale = scale;
	mTransform = parent->GetWorldMatrix();
}

void GUIWidget::UpdateRTInternal()
{
	SPtr<RenderTarget> rt;
	u64 newRTId = 0;
	if(mCamera != nullptr)
	{
		rt = mCamera->GetViewport()->GetTarget();
		if(rt != nullptr)
			newRTId = rt->GetInternalId();
	}

	if(mCachedRTId != newRTId)
	{
		mCachedRTId = newRTId;
		UpdateRootPanel();
	}
}

void GUIWidget::UpdateLayoutInternal()
{
	// Check if render target size changed and update if needed
	// Note: Purposely not relying to the RenderTarget::onResized callback, as it will trigger /before/ Input events.
	// These events might trigger a resize, meaning the size would be delayed one frame, resulting in a visual artifact
	// where the GUI doesn't match the target size.
	Viewport* target = GetTarget();
	if(target != nullptr)
	{
		Rect2I area = target->GetPixelArea();
		u32 width = area.Width;
		u32 height = area.Height;

		const Rect2I& panelArea = mPanel->GetLayoutDataInternal().Area;
		if(panelArea.Width != width || panelArea.Height != height)
		{
			UpdateRootPanel();
			OnOwnerTargetResized();
		}
	}

	B3DMarkAllocatorFrame();

	// Determine dirty contents and layouts
	FrameStack<GUIElementBase*> todo;
	todo.push(mPanel);

	while(!todo.empty())
	{
		GUIElementBase* currentElem = todo.top();
		todo.pop();

		if(currentElem->IsDirty())
		{
			GUIElementBase* updateParent = currentElem->GetUpdateParentInternal();
			B3D_ASSERT(updateParent != nullptr || currentElem == mPanel);

			if(updateParent != nullptr)
				UpdateLayoutInternal(updateParent);
			else // Must be root panel
				UpdateLayoutInternal(mPanel);
		}
		else
		{
			u32 numChildren = currentElem->GetChildCount();
			for(u32 i = 0; i < numChildren; i++)
				todo.push(currentElem->GetChild(i));
		}
	}

	B3DClearAllocatorFrame();
}

void GUIWidget::UpdateLayoutInternal(GUIElementBase* elem)
{
	GUIElementBase* parent = elem->GetParent();
	bool isPanelOptimized = parent != nullptr && parent->GetTypeInternal() == GUIElementBase::Type::Panel;

	GUIElementBase* updateParent = nullptr;

	if(isPanelOptimized)
		updateParent = parent;
	else
		updateParent = elem;

	// For GUIPanel we can do a an optimization and update only the element in question instead
	// of all the children
	if(isPanelOptimized)
	{
		GUIPanel* panel = static_cast<GUIPanel*>(updateParent);

		GUIElementBase* dirtyElement = elem;
		dirtyElement->UpdateOptimalLayoutSizes();

		LayoutSizeRange elementSizeRange = panel->GetElementSizeRangeInternal(dirtyElement);
		Rect2I elementArea = panel->GetElementAreaInternal(panel->GetLayoutDataInternal().Area, dirtyElement, elementSizeRange);

		GUILayoutData childLayoutData = panel->GetLayoutDataInternal();
		panel->UpdateDepthRangeInternal(childLayoutData);
		childLayoutData.Area = elementArea;

		panel->UpdateChildLayoutInternal(dirtyElement, childLayoutData);
	}
	else
	{
		GUILayoutData childLayoutData = updateParent->GetLayoutDataInternal();
		updateParent->UpdateLayout(childLayoutData);
	}

	// Mark dirty contents
	B3DMarkAllocatorFrame();
	{
		FrameStack<GUIElementBase*> todo;
		todo.push(elem);

		while(!todo.empty())
		{
			GUIElementBase* currentElem = todo.top();
			todo.pop();

			MarkContentDirtyInternal(currentElem);
			currentElem->MarkAsClean();

			u32 numChildren = currentElem->GetChildCount();
			for(u32 i = 0; i < numChildren; i++)
				todo.push(currentElem->GetChild(i));
		}
	}
	B3DClearAllocatorFrame();
}

void GUIWidget::RegisterElementInternal(GUIElementBase* elem)
{
	B3D_ASSERT(elem != nullptr && !elem->IsDestroyed());

	if(elem->GetTypeInternal() == GUIElementBase::Type::Element)
	{
		mElements.push_back(static_cast<GUIElement*>(elem));
		mWidgetIsDirty = true;

		// Find a draw group
		auto guiElem = static_cast<GUIElement*>(elem);
		mDrawGroups.Add(guiElem);
		mDrawGroups.MarkContentDirty(guiElem);
	}
}

void GUIWidget::UnregisterElementInternal(GUIElementBase* elem)
{
	B3D_ASSERT(elem != nullptr);

	auto iterFind = std::find(begin(mElements), end(mElements), elem);

	if(iterFind != mElements.end())
	{
		mElements.erase(iterFind);
		mWidgetIsDirty = true;
	}

	if(elem->GetTypeInternal() == GUIElementBase::Type::Element)
	{
		mDirtyContents.erase(static_cast<GUIElement*>(elem));

		auto guiElem = static_cast<GUIElement*>(elem);
		mDrawGroups.Remove(guiElem);
	}
}

void GUIWidget::MarkMeshDirtyInternal(GUIElementBase* elem)
{
	mWidgetIsDirty = true;

	if(elem->GetTypeInternal() == GUIElementBase::Type::Element)
		mDrawGroups.MarkMeshDirty(static_cast<GUIElement*>(elem));
}

void GUIWidget::MarkContentDirtyInternal(GUIElementBase* elem)
{
	if(elem->GetTypeInternal() == GUIElementBase::Type::Element)
	{
		auto guiElement = static_cast<GUIElement*>(elem);

		mDirtyContents.insert(guiElement);
		mDrawGroups.MarkContentDirty(guiElement);
	}
}

void GUIWidget::SetSkin(const HGUISkin& skin)
{
	mSkin = skin;

	for(auto& element : mElements)
		element->RefreshStyle();
}

const GUISkin& GUIWidget::GetSkin() const
{
	if(mSkin.IsLoaded())
		return *mSkin;
	else
		return *BuiltinResources::Instance().GetGuiSkin();
}

void GUIWidget::SetCamera(const SPtr<Camera>& camera)
{
	SPtr<Camera> newCamera = camera;
	if(newCamera != nullptr)
	{
		if(newCamera->GetViewport()->GetTarget() == nullptr)
			newCamera = nullptr;
	}

	if(mCamera == newCamera)
		return;

	GUIManager::Instance().UnregisterWidget(this);
	mCamera = newCamera;
	GUIManager::Instance().RegisterWidget(this);

	UpdateRootPanel();
}

void GUIWidget::SetIsActive(bool active)
{
	mIsActive = active;
}

GUIDrawGroupRenderDataUpdate GUIWidget::RebuildDirtyRenderData()
{
	if(!mIsActive)
		return GUIDrawGroupRenderDataUpdate();

	const bool dirty = mWidgetIsDirty || !mDirtyContents.empty();

	if(dirty)
	{
		mWidgetIsDirty = false;

		// Update render contents recursively because updates can cause child GUI elements to become dirty
		while(!mDirtyContents.empty())
		{
			mDirtyContentsTemp.swap(mDirtyContents);

			for(auto& dirtyElement : mDirtyContentsTemp)
				dirtyElement->UpdateRenderElements();

			mDirtyContentsTemp.clear();
		}

		UpdateBounds();
	}

	return mDrawGroups.RebuildDirty(dirty);
}

bool GUIWidget::InBounds(const Vector2I& position) const
{
	Viewport* target = GetTarget();
	if(target == nullptr)
		return false;

	// Technically GUI widget bounds can be larger than the viewport, so make sure we clip to viewport first
	if(!target->GetPixelArea().Contains(position))
		return false;

	Vector3 vecPos((float)position.X, (float)position.Y, 0.0f);
	vecPos = mTransform.Inverse().MultiplyAffine(vecPos);

	Vector2I localPos(Math::RoundToI32(vecPos.X), Math::RoundToI32(vecPos.Y));
	return mBounds.Contains(localPos);
}

void GUIWidget::UpdateBounds() const
{
	if(!mElements.empty())
		mBounds = mElements[0]->GetClippedBounds();

	for(auto& elem : mElements)
	{
		Rect2I elemBounds = elem->GetClippedBounds();
		mBounds.Encapsulate(elemBounds);
	}
}

void GUIWidget::OwnerWindowFocusChanged()
{
	OnOwnerWindowFocusChanged();
}

void GUIWidget::UpdateRootPanel()
{
	Viewport* target = GetTarget();
	if(target == nullptr)
		return;

	Rect2I area = target->GetPixelArea();
	u32 width = area.Width;
	u32 height = area.Height;

	GUILayoutData layoutData;
	layoutData.Area.Width = width;
	layoutData.Area.Height = height;
	layoutData.ClipRect = Rect2I(0, 0, width, height);
	layoutData.SetWidgetDepth(mDepth);

	mPanel->SetWidth(width);
	mPanel->SetHeight(height);

	mPanel->SetLayoutData(layoutData);
	mPanel->MarkLayoutAsDirty();
}
