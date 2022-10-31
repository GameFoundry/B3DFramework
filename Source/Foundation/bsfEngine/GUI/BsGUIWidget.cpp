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
#include "RenderAPI/BsVertexDataDesc.h"
#include "Image/BsSpriteTexture.h"

using namespace bs;

namespace impl
{
SPtr<VertexDataDesc> GetGUITriangleMeshDesc()
{
	static SPtr<VertexDataDesc> sDesc;

	if(!sDesc)
	{
		sDesc = B3DMakeShared<VertexDataDesc>();
		sDesc->AddVertElem(VET_FLOAT2, VES_POSITION);
		sDesc->AddVertElem(VET_FLOAT2, VES_TEXCOORD);
	}

	return sDesc;
}

SPtr<VertexDataDesc> GetGUILineMeshDesc()
{
	static SPtr<VertexDataDesc> sDesc;

	if(!sDesc)
	{
		sDesc = B3DMakeShared<VertexDataDesc>();
		sDesc->AddVertElem(VET_FLOAT2, VES_POSITION);
	}

	return sDesc;
}
} // namespace impl

GUIDrawGroups::GUIDrawGroups(GUIWidget* parentWidget)
	: mWidget(parentWidget)
{
	GUIDrawGroup mainDrawGroup;
	mainDrawGroup.MinDepth = 0;
	mainDrawGroup.DepthRange = std::numeric_limits<u32>::max();
	mainDrawGroup.Id = mNextDrawGroupId++;

	mDrawGroups.push_back(mainDrawGroup);
}

void GUIDrawGroups::Add(GUIElement* element)
{
	const SmallVector<GUIRenderElement, 4>& renderElements = element->GetRenderElementsInternal();

	GUIGroupElement& groupElement = mElements[element];
	groupElement.Element = element;
	groupElement.Bounds = element->GetClippedBoundsInternal();
	groupElement.Groups.Resize(renderElements.Size());

	for(u32 i = 0; i < renderElements.Size(); i++)
		Add(groupElement, i);

	mGroupsCoreDirty = true;
}

void GUIDrawGroups::Add(GUIGroupElement& groupElement, u32 renderElementIdx)
{
	GUIElement* element = groupElement.Element;
	const SmallVector<GUIRenderElement, 4>& renderElements = element->GetRenderElementsInternal();

	const GUIRenderElement& renderElement = renderElements[renderElementIdx];
	u32 elemDepth = element->GetDepthInternal() + renderElement.Depth;

	// Groups are expected to be sorted by minDepth
	for(u32 j = 0; j < (u32)mDrawGroups.size(); j++)
	{
		if(elemDepth < mDrawGroups[j].MinDepth || elemDepth >= (mDrawGroups[j].MinDepth + mDrawGroups[j].DepthRange))
			continue;

		Add(groupElement, renderElementIdx, j);
		break;
	}
}

void GUIDrawGroups::Add(GUIGroupElement& groupElement, u32 renderElementIdx, u32 groupIdx)
{
	GUIElement* element = groupElement.Element;
	const SmallVector<GUIRenderElement, 4>& renderElements = element->GetRenderElementsInternal();

	const GUIRenderElement& renderElement = renderElements[renderElementIdx];
	u32 elemDepth = element->GetDepthInternal() + renderElement.Depth;

	SpriteMaterial* spriteMaterial = renderElement.Material;
	assert(spriteMaterial != nullptr);

	GUIDrawGroup& group = mDrawGroups[groupIdx];
	if(spriteMaterial->AllowBatching())
	{
		group.CachedElements.push_back(GUIGroupRenderElement(element, renderElementIdx));

		Rect2I bounds = element->GetClippedBoundsInternal();
		group.Bounds.Encapsulate(bounds);
		group.NeedsRedraw = true;
		group.DirtyTexture = true;

		groupElement.Groups[renderElementIdx] = group.Id;
	}
	else
	{
		bool needsSplit = elemDepth != group.MinDepth;
		if(needsSplit)
		{
			GUIDrawGroup& newGroup = Split(groupIdx, elemDepth);

			newGroup.NonCachedElements.push_back(GUIGroupRenderElement(element, renderElementIdx));

			groupElement.Groups[renderElementIdx] = newGroup.Id;
			newGroup.NeedsRedraw = true;
		}
		else
		{
			group.NonCachedElements.push_back(GUIGroupRenderElement(element, renderElementIdx));

			groupElement.Groups[renderElementIdx] = group.Id;
			group.NeedsRedraw = true;
		}
	}
}

void GUIDrawGroups::Remove(GUIElement* element)
{
	auto iterFind = mElements.find(element);
	if(iterFind == mElements.end())
		return;

	for(u32 i = 0; i < iterFind->second.Groups.Size(); i++)
		Remove(iterFind->second, i);

	mElements.erase(element);
	mGroupsCoreDirty = true;
}

void GUIDrawGroups::Remove(GUIGroupElement& groupElement, u32 renderElementIdx)
{
	if(renderElementIdx >= (u32)groupElement.Groups.Size())
		return;

	GUIElement* element = groupElement.Element;
	const SmallVector<GUIRenderElement, 4>& renderElements = element->GetRenderElementsInternal();

	auto iterFind = std::find_if(mDrawGroups.begin(), mDrawGroups.end(), [drawGroupId = groupElement.Groups[renderElementIdx]](const GUIDrawGroup& group)
								 { return group.Id == drawGroupId; });

	assert(iterFind != mDrawGroups.end());
	if(iterFind != mDrawGroups.end())
	{
		u32 idx = (u32)(iterFind - mDrawGroups.begin());
		Remove(groupElement, renderElementIdx, idx);
	}
}

void GUIDrawGroups::Remove(GUIGroupElement& groupElement, u32 renderElementIdx, u32 groupIdx)
{
	GUIElement* element = groupElement.Element;
	const SmallVector<GUIRenderElement, 4>& renderElements = element->GetRenderElementsInternal();
	GUIDrawGroup& group = mDrawGroups[groupIdx];

	for(auto iter = group.CachedElements.begin(); iter != group.CachedElements.end();)
	{
		if(iter->Element == element && iter->RenderElementIdx == renderElementIdx)
		{
			group.DirtyBounds = true;

			iter = group.CachedElements.erase(iter);
		}
		else
			++iter;
	}

	group.NonCachedElements.erase(std::remove_if(group.NonCachedElements.begin(), group.NonCachedElements.end(), [element, renderElementIdx](const GUIGroupRenderElement& x)
												 { return x.Element == element && x.RenderElementIdx == renderElementIdx; }),
								  group.NonCachedElements.end());

	group.NeedsRedraw = true;

	// Purposely not clearing draw group on the GUIRenderElement, as its indices could have changed and it might not
	// match the current index. It's up to external code to handle that.
	groupElement.Groups[renderElementIdx] = -1;

	// Attempt to merge with previous group
	if(group.NonCachedElements.empty() && group.MinDepth > 0)
	{
		assert(groupIdx > 0);

		u32 prevGroupIdx = groupIdx - 1;
		GUIDrawGroup& prevGroup = mDrawGroups[prevGroupIdx];

		prevGroup.DepthRange += group.DepthRange;

		for(auto& entry : group.CachedElements)
		{
			auto iterFind = mElements.find(entry.Element);
			assert(iterFind != mElements.end());
			if(iterFind != mElements.end())
				iterFind->second.Groups[entry.RenderElementIdx] = prevGroup.Id;

			prevGroup.DirtyBounds = true;
		}

		std::move(group.CachedElements.begin(), group.CachedElements.end(), std::back_inserter(prevGroup.CachedElements));
		prevGroup.NeedsRedraw = true;

		mDrawGroups.erase(mDrawGroups.begin() + groupIdx);
	}
}

GUIDrawGroupRenderDataUpdate GUIDrawGroups::RebuildDirty(bool forceRebuildMeshes)
{
	// Update dirty draw groups and mark them for redraw
	bool shouldRebuildMeshes = forceRebuildMeshes;
	for(auto& entry : mDirtyElements)
	{
		GUIElement* element = entry.first;

		auto iterFind = mElements.find(element);
		if(iterFind == mElements.end())
			continue;

		shouldRebuildMeshes = true;

		const SmallVector<GUIRenderElement, 4>& renderElements = element->GetRenderElementsInternal();
		GUIGroupElement& groupElement = iterFind->second;

		bool dirtyBounds = false;
		if((entry.second & DirtyContent) != 0)
		{
			bool renderElementsDirty = groupElement.Groups.Size() != renderElements.Size();

			// If render element count changed, do a full rebuild of the draw group
			if(renderElementsDirty)
			{
				Remove(element);
				Add(element);

				continue;
			}

			// If bounds changed, rebuild the bounds of the draw groups
			Rect2I bounds = element->GetClippedBoundsInternal();
			if(groupElement.Bounds != bounds)
			{
				dirtyBounds = true;
				groupElement.Bounds = bounds;
			}
		}

		for(u32 i = 0; i < renderElements.Size(); i++)
		{
			const GUIRenderElement& renderElement = renderElements[i];
			i32 drawGroupId = groupElement.Groups[i];

			// All render elements draw group IDs should be assigned at this point
			assert(drawGroupId != -1);

			auto iterFind2 = std::find_if(mDrawGroups.begin(), mDrawGroups.end(), [drawGroupId](const GUIDrawGroup& group)
										  { return group.Id == drawGroupId; });

			assert(iterFind2 != mDrawGroups.end());
			if(iterFind2 != mDrawGroups.end())
			{
				GUIDrawGroup& group = *iterFind2;

				if(dirtyBounds)
					group.DirtyBounds = dirtyBounds;

				bool needsGroupChange = false;
				if((entry.second & DirtyMesh) != 0)
				{
					u32 depth = element->GetDepthInternal() + renderElement.Depth;

					// If same as min-depth, no group change is necessary in any case
					if(depth != group.MinDepth)
					{
						// If less than min-depth, group change is always necessary
						if(depth < group.MinDepth)
							needsGroupChange = true;
						// Non-batching elements must be at min-depth, so group change is necessary
						else if(!renderElement.Material->AllowBatching())
							needsGroupChange = true;
						// Batching but outside of the group's depth range, group change is necessary
						else if(depth >= (group.MinDepth + group.DepthRange))
							needsGroupChange = true;
					}
				}

				if(!needsGroupChange && (entry.second & DirtyContent) != 0)
				{
					// Check if the material changed
					if(renderElement.Material->AllowBatching())
					{
						auto iterFind3 = std::find_if(group.CachedElements.begin(), group.CachedElements.end(), [element, i](auto& x)
													  { return x.Element == element && x.RenderElementIdx == i; });
						if(iterFind3 == group.CachedElements.end())
							needsGroupChange = true;
					}
					else
					{
						auto iterFind3 = std::find_if(group.NonCachedElements.begin(), group.NonCachedElements.end(), [element, i](auto& x)
													  { return x.Element == element && x.RenderElementIdx == i; });
						if(iterFind3 == group.NonCachedElements.end())
							needsGroupChange = true;
					}
				}

				if(needsGroupChange)
				{
					u32 groupIdx = (u32)(iterFind2 - mDrawGroups.begin());
					Remove(groupElement, i, groupIdx);
					Add(groupElement, i);

					mGroupsCoreDirty = true;
				}

				group.NeedsRedraw = true;
			}
		}
	}

	mDirtyElements.clear();

	// Update dirty bounds and textures
	for(auto& entry : mDrawGroups)
	{
		if(entry.DirtyBounds)
		{
			Rect2I newBounds = CalculateBounds(entry);
			entry.DirtyTexture = true;
			entry.DirtyBounds = false;

			entry.Bounds = newBounds;
		}

		if(entry.DirtyTexture)
		{
			if(entry.OutputTexture == nullptr ||
			   entry.Bounds.Width != entry.OutputTexture->GetProperties().Width ||
			   entry.Bounds.Height != entry.OutputTexture->GetProperties().Height)
			{
				entry.OutputTexture = nullptr;

				TEXTURE_DESC texDesc;
				texDesc.Width = entry.Bounds.Width;
				texDesc.Height = entry.Bounds.Height;
				texDesc.Format = PF_RGBA8;
				texDesc.Usage = TU_RENDERTARGET;

				HTexture texture = Texture::Create(texDesc);

				RENDER_TEXTURE_DESC rtDesc;
				rtDesc.ColorSurfaces[0].Texture = texture;

				entry.OutputTexture = RenderTexture::Create(rtDesc);
			}

			entry.DirtyTexture = false;
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

	output.GroupDirtyState.reserve(mDrawGroups.size());
	for(auto& entry : mDrawGroups)
	{
		output.GroupDirtyState.push_back(entry.NeedsRedraw);
		entry.NeedsRedraw = false;
	}

	// Note: If only mesh rebuild happened, we should only update the specific render elements
	// that changed. (Note that in this case the mesh rebuild flag also signals changes to the
	// GUI element texture/tint/etc.)
	if(mGroupsCoreDirty || shouldRebuildMeshes)
	{
		output.NewDrawGroups.reserve(mDrawGroups.size());
		for(auto& entry : mDrawGroups)
			output.NewDrawGroups.push_back(GetRenderData(entry));

		// Register elements that depend on render textures (currently these always correspond to input bridged elements)
		SmallVector<std::pair<const GUIElement*, SPtr<const RenderTarget>>, 4> bridgedElements;
		GetGUIManager().GetBridgedElements(mWidget, bridgedElements);

		for(auto& entry : bridgedElements)
		{
			auto* element = const_cast<GUIElement*>(entry.first);
			auto iterFind = mElements.find(element);

			assert(iterFind != mElements.end());
			if(iterFind == mElements.end())
				continue;

			const SPtr<const RenderTarget>& target = entry.second;
			for(auto& groupId : iterFind->second.Groups)
			{
				for(auto& group : output.NewDrawGroups)
				{
					if(group.Id != groupId)
						continue;

					group.RenderTargetElements.push_back(target->GetCore());
				}
			}
		}
	}

	mGroupsCoreDirty = false;
	return output;
}

void GUIDrawGroups::NotifyContentDirty(GUIElement* element)
{
	mDirtyElements[element] |= DirtyContent;
}

void GUIDrawGroups::NotifyMeshDirty(GUIElement* element)
{
	mDirtyElements[element] |= DirtyMesh;
}

void GUIDrawGroups::RebuildMeshes()
{
	struct GUIMaterialGroup
	{
		SpriteMaterial* Material;
		SpriteMaterialInfo MatInfo;
		GUIMeshType MeshType;
		u32 NumVertices;
		u32 NumIndices;
		u32 Depth;
		u32 MinDepth;
		Rect2I Bounds;
		GUIDrawGroup* DrawGroup;
		Vector<GUIGroupRenderElement> Elements;
	};

	struct GUIMaterialGroupSet
	{
		using SortedGroupSet = FrameSet<GUIMaterialGroup, std::function<bool(const GUIMaterialGroup&, const GUIMaterialGroup&)>>;

		u32 NumMeshes = 0;
		u32 NumIndices[2] = { 0, 0 };
		u32 NumVertices[2] = { 0, 0 };

		SortedGroupSet SortedGroups;
	};

	bs_frame_mark();
	{
		// Make a list of all GUI elements, sorted from farthest to nearest (highest depth to lowest)
		auto elemComp = [](const GUIGroupRenderElement& a, const GUIGroupRenderElement& b)
		{
			u32 aDepth = a.Element->GetDepthInternal() + a.Element->GetRenderElementsInternal()[a.RenderElementIdx].Depth;
			u32 bDepth = b.Element->GetDepthInternal() + b.Element->GetRenderElementsInternal()[b.RenderElementIdx].Depth;

			// Compare pointers just to differentiate between two elements with the same depth, their order doesn't really matter, but std::set
			// requires all elements to be unique
			return (aDepth > bDepth) ||
				(aDepth == bDepth && a.Element > b.Element) ||
				(aDepth == bDepth && a.Element == b.Element && a.RenderElementIdx > b.RenderElementIdx);
		};

		FrameVector<GUIMaterialGroupSet> groupSets;
		groupSets.reserve(mDrawGroups.size());

		for(auto& entry : mDrawGroups)
		{
			// Note: If we keep visible elements separate from invisible, plus provide sorting on insert, we could avoid this
			// re-sorting and re-inserting step.
			FrameSet<GUIGroupRenderElement, std::function<bool(const GUIGroupRenderElement&, const GUIGroupRenderElement&)>> allElements(elemComp);
			for(auto& element : entry.CachedElements)
			{
				if(!element.Element->IsVisibleInternal())
					continue;

				allElements.insert(element);
			}

			for(auto& element : entry.NonCachedElements)
			{
				if(!element.Element->IsVisibleInternal())
					continue;

				allElements.insert(element);
			}

			// Group the elements in such a way so that we end up with a smallest amount of
			// meshes, without breaking back to front rendering order
			FrameUnorderedMap<u64, FrameVector<GUIMaterialGroup>> materialGroups;
			for(auto& elem : allElements)
			{
				GUIElement* guiElem = elem.Element;
				u32 renderElemIdx = elem.RenderElementIdx;
				const GUIRenderElement& renderElem = elem.Element->GetRenderElementsInternal()[renderElemIdx];

				u32 elemDepth = guiElem->GetDepthInternal() + renderElem.Depth;
				Rect2I bounds = guiElem->GetClippedBoundsInternal();

				SpriteMaterial* spriteMaterial = renderElem.Material;
				const SpriteMaterialInfo& matInfo = *renderElem.MatInfo;
				assert(spriteMaterial != nullptr);

				u64 hash = spriteMaterial->GetMergeHash(matInfo);
				FrameVector<GUIMaterialGroup>& groupsPerMaterial = materialGroups[hash];

				// Try to find a group this material will fit in:
				//  - Group that has a depth value same or one below elements depth will always be a match
				//  - Otherwise, we search higher depth values as well, but we only use them if no elements in between those depth values
				//    overlap the current elements bounds.
				GUIMaterialGroup* foundGroup = nullptr;

				if(spriteMaterial->AllowBatching())
				{
					for(auto groupIter = groupsPerMaterial.rbegin(); groupIter != groupsPerMaterial.rend(); ++groupIter)
					{
						GUIMaterialGroup& group = *groupIter;
						if(group.Depth == elemDepth)
						{
							foundGroup = &group;
							break;
						}
						else
						{
							u32 startDepth = elemDepth;
							u32 endDepth = group.Depth;

							Rect2I potentialGroupBounds = group.Bounds;
							potentialGroupBounds.Encapsulate(bounds);

							bool foundOverlap = false;
							for(auto& material : materialGroups)
							{
								for(auto& matGroup : material.second)
								{
									if(&matGroup == &group)
										continue;

									if((matGroup.MinDepth >= startDepth && matGroup.MinDepth <= endDepth) || (matGroup.Depth >= startDepth && matGroup.Depth <= endDepth))
									{
										if(matGroup.Bounds.Overlaps(potentialGroupBounds))
										{
											foundOverlap = true;
											break;
										}
									}
								}
							}

							if(!foundOverlap)
							{
								foundGroup = &group;
								break;
							}
						}
					}
				}

				if(foundGroup == nullptr)
				{
					groupsPerMaterial.push_back(GUIMaterialGroup());
					foundGroup = &groupsPerMaterial[groupsPerMaterial.size() - 1];

					foundGroup->Depth = elemDepth;
					foundGroup->MinDepth = elemDepth;
					foundGroup->Bounds = bounds;
					foundGroup->Elements.push_back(GUIGroupRenderElement(guiElem, renderElemIdx));
					foundGroup->MatInfo = matInfo.Clone();
					foundGroup->Material = spriteMaterial;
					foundGroup->NumVertices = renderElem.NumVertices;
					foundGroup->NumIndices = renderElem.NumIndices;
					foundGroup->MeshType = renderElem.Type;
					foundGroup->DrawGroup = &entry;
				}
				else
				{
					foundGroup->Bounds.Encapsulate(bounds);
					foundGroup->Elements.push_back(GUIGroupRenderElement(guiElem, renderElemIdx));
					foundGroup->MinDepth = std::min(foundGroup->MinDepth, elemDepth);

					// It's expected that GUI element doesn't use same material for different mesh types so this should always be true
					assert(renderElem.Type == foundGroup->MeshType);

					// Draw groups are super-set of material groups, so a material group cannot cross a draw group boundary
					assert(foundGroup->DrawGroup == &entry);

					foundGroup->NumVertices += renderElem.NumVertices;
					foundGroup->NumIndices += renderElem.NumIndices;

					spriteMaterial->Merge(foundGroup->MatInfo, matInfo);
				}
			}

			// Make a list of all GUI elements, sorted from farthest to nearest (highest depth to lowest)
			auto groupComp = [](const GUIMaterialGroup& a, const GUIMaterialGroup& b)
			{
				return (a.Depth > b.Depth) || (a.Depth == b.Depth && &a > &b);
				// Compare pointers just to differentiate between two elements with the same depth, their order doesn't really matter, but std::set
				// requires all elements to be unique
			};

			groupSets.push_back(GUIMaterialGroupSet());
			GUIMaterialGroupSet& groupSet = groupSets.back();

			groupSet.SortedGroups = GUIMaterialGroupSet::SortedGroupSet(groupComp);
			for(auto& material : materialGroups)
			{
				for(auto& group : material.second)
				{
					groupSet.SortedGroups.insert(std::move(group));

					u32 typeIdx = (u32)group.MeshType;
					groupSet.NumIndices[typeIdx] += group.NumIndices;
					groupSet.NumVertices[typeIdx] += group.NumVertices;

					groupSet.NumMeshes++;
				}
			}
		}

		mTriangleMesh = nullptr;
		mLineMesh = nullptr;

		u32 totalNumIndices[2] = { 0, 0 };
		u32 totalNumVertices[2] = { 0, 0 };

		for(u32 i = 0; i < (u32)mDrawGroups.size(); i++)
		{
			GUIMaterialGroupSet& set = groupSets[i];
			mDrawGroups[i].Meshes.resize(set.NumMeshes);

			for(u32 j = 0; j < 2; j++)
			{
				totalNumIndices[j] += set.NumIndices[j];
				totalNumVertices[j] += set.NumVertices[j];
			}
		}

		SPtr<MeshData> meshData[2];
		SPtr<VertexDataDesc> vertexDesc[2] = { ::impl::GetGUITriangleMeshDesc(), ::impl::GetGUILineMeshDesc() };

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

		for(u32 i = 0; i < (u32)mDrawGroups.size(); i++)
		{
			GUIMaterialGroupSet& set = groupSets[i];

			// Fill buffers for each group and update their meshes
			u32 meshIdx = 0;

			for(auto& group : set.SortedGroups)
			{
				GUIMesh& guiMesh = mDrawGroups[i].Meshes[meshIdx];
				guiMesh.MatInfo = group.MatInfo;
				guiMesh.Material = group.Material;
				guiMesh.IsLine = group.MeshType == GUIMeshType::Line;

				auto typeIdx = (u32)group.MeshType;
				guiMesh.IndexOffset = indexOffset[typeIdx];

				Vector2I groupOffset(0, 0);
				if(guiMesh.Material->AllowBatching())
					groupOffset = Vector2I(-group.DrawGroup->Bounds.X, -group.DrawGroup->Bounds.Y);

				u32 groupNumIndices = 0;
				for(auto& matElement : group.Elements)
				{
					matElement.Element->FillBuffer(
						vertices[typeIdx], indices[typeIdx],
						vertexOffset[typeIdx], indexOffset[typeIdx], groupOffset,
						totalNumVertices[typeIdx], totalNumIndices[typeIdx],
						matElement.RenderElementIdx);

					const GUIRenderElement& renderElement = matElement.Element->GetRenderElementsInternal()[matElement.RenderElementIdx];

					u32 indexStart = indexOffset[typeIdx];
					u32 indexEnd = indexStart + renderElement.NumIndices;

					for(u32 j = indexStart; j < indexEnd; j++)
						indices[typeIdx][j] += vertexOffset[typeIdx];

					indexOffset[typeIdx] += renderElement.NumIndices;
					vertexOffset[typeIdx] += renderElement.NumVertices;

					groupNumIndices += renderElement.NumIndices;
				}

				guiMesh.IndexCount = groupNumIndices;

				meshIdx++;
			}
		}

		if(meshData[0])
			mTriangleMesh = Mesh::CreatePtrInternal(meshData[0], MU_STATIC, DOT_TRIANGLE_LIST);

		if(meshData[1])
			mLineMesh = Mesh::CreatePtrInternal(meshData[1], MU_STATIC, DOT_LINE_LIST);
	}

	bs_frame_clear();
}

GUIDrawGroups::GUIDrawGroup& GUIDrawGroups::Split(u32 groupIdx, u32 depth)
{
	GUIDrawGroup& group = mDrawGroups[groupIdx];
	assert(depth > group.MinDepth);

	u32 maxDepth = group.MinDepth + group.DepthRange;
	group.DepthRange = depth - group.MinDepth;

	GUIDrawGroup newSplitGroup;
	newSplitGroup.MinDepth = depth;
	newSplitGroup.DepthRange = maxDepth - newSplitGroup.MinDepth;
	newSplitGroup.Id = mNextDrawGroupId++;

	auto it = std::partition(group.CachedElements.begin(), group.CachedElements.end(), [depth](const GUIGroupRenderElement& x)
							 {
				u32 elemDepth = x.Element->GetDepthInternal() + x.Element->GetRenderElementsInternal()[x.RenderElementIdx].Depth;
				return elemDepth < depth; });

	std::move(it, group.CachedElements.end(), std::back_inserter(newSplitGroup.CachedElements));
	group.CachedElements.erase(it, group.CachedElements.end());

	for(auto& entry : newSplitGroup.CachedElements)
	{
		auto iterFind = mElements.find(entry.Element);
		assert(iterFind != mElements.end());
		if(iterFind != mElements.end())
			iterFind->second.Groups[entry.RenderElementIdx] = newSplitGroup.Id;
	}

	group.DirtyBounds = true;
	group.NeedsRedraw = true;
	newSplitGroup.DirtyBounds = true;

	mDrawGroups.insert(mDrawGroups.begin() + groupIdx + 1, std::move(newSplitGroup));
	return mDrawGroups[groupIdx + 1];
}

GUIMeshRenderData GUIDrawGroups::GetRenderData(const GUIMesh& guiMesh)
{
	SPtr<ct::Texture> textureCore;
	if(guiMesh.MatInfo.Texture.IsLoaded())
		textureCore = guiMesh.MatInfo.Texture->GetCore();
	else
		textureCore = nullptr;

	SPtr<ct::SpriteTexture> spriteTextureCore;
	if(guiMesh.MatInfo.SpriteTexture.IsLoaded())
		spriteTextureCore = guiMesh.MatInfo.SpriteTexture->GetCore();
	else
		spriteTextureCore = nullptr;

	GUIMeshRenderData output;
	output.Material = guiMesh.Material;
	output.Texture = textureCore;
	output.SpriteTexture = spriteTextureCore;
	output.Tint = guiMesh.MatInfo.Tint;
	output.IsLine = guiMesh.IsLine;
	output.AnimationStartTime = guiMesh.MatInfo.AnimationStartTime;
	output.AdditionalData = guiMesh.MatInfo.AdditionalData;
	output.BufferIdx = 0;

	output.SubMesh.IndexOffset = guiMesh.IndexOffset;
	output.SubMesh.IndexCount = guiMesh.IndexCount;
	output.SubMesh.DrawOp = guiMesh.IsLine ? DOT_LINE_LIST : DOT_TRIANGLE_LIST;

	return output;
}

GUIDrawGroupRenderData GUIDrawGroups::GetRenderData(const GUIDrawGroup& drawGroup)
{
	GUIDrawGroupRenderData output;
	output.Id = drawGroup.Id;
	output.Destination = drawGroup.OutputTexture->GetCore();
	output.Bounds = drawGroup.Bounds;
	output.RequiresRedraw = true;

	auto numElements = (u32)drawGroup.Meshes.size();
	for(u32 i = 0; i < numElements; i++)
	{
		GUIMeshRenderData meshData = GetRenderData(drawGroup.Meshes[i]);
		if(meshData.SubMesh.IndexCount == 0)
			continue;

		if(meshData.Material->AllowBatching())
			output.CachedElements.push_back(std::move(meshData));
		else
			output.NonCachedElements.push_back(std::move(meshData));
	}

	return output;
}

Rect2I GUIDrawGroups::CalculateBounds(GUIDrawGroup& group)
{
	Rect2I bounds = Rect2I();
	bool boundsSet = false;

	for(auto& entry : group.CachedElements)
	{
		if(!entry.Element->IsVisibleInternal())
			continue;

		Rect2I elementBounds = entry.Element->GetClippedBoundsInternal();
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
	mPanel->ChangeParentWidgetInternal(this);
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

	bs_frame_mark();

	// Determine dirty contents and layouts
	FrameStack<GUIElementBase*> todo;
	todo.push(mPanel);

	while(!todo.empty())
	{
		GUIElementBase* currentElem = todo.top();
		todo.pop();

		if(currentElem->IsDirtyInternal())
		{
			GUIElementBase* updateParent = currentElem->GetUpdateParentInternal();
			assert(updateParent != nullptr || currentElem == mPanel);

			if(updateParent != nullptr)
				UpdateLayoutInternal(updateParent);
			else // Must be root panel
				UpdateLayoutInternal(mPanel);
		}
		else
		{
			u32 numChildren = currentElem->GetNumChildrenInternal();
			for(u32 i = 0; i < numChildren; i++)
				todo.push(currentElem->GetChildInternal(i));
		}
	}

	bs_frame_clear();
}

void GUIWidget::UpdateLayoutInternal(GUIElementBase* elem)
{
	GUIElementBase* parent = elem->GetParentInternal();
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
		dirtyElement->UpdateOptimalLayoutSizesInternal();

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
		updateParent->UpdateLayoutInternal(childLayoutData);
	}

	// Mark dirty contents
	bs_frame_mark();
	{
		FrameStack<GUIElementBase*> todo;
		todo.push(elem);

		while(!todo.empty())
		{
			GUIElementBase* currentElem = todo.top();
			todo.pop();

			MarkContentDirtyInternal(currentElem);
			currentElem->MarkAsCleanInternal();

			u32 numChildren = currentElem->GetNumChildrenInternal();
			for(u32 i = 0; i < numChildren; i++)
				todo.push(currentElem->GetChildInternal(i));
		}
	}
	bs_frame_clear();
}

void GUIWidget::RegisterElementInternal(GUIElementBase* elem)
{
	assert(elem != nullptr && !elem->IsDestroyedInternal());

	if(elem->GetTypeInternal() == GUIElementBase::Type::Element)
	{
		mElements.push_back(static_cast<GUIElement*>(elem));
		mWidgetIsDirty = true;

		// Find a draw group
		auto guiElem = static_cast<GUIElement*>(elem);
		mDrawGroups.Add(guiElem);
		mDrawGroups.NotifyContentDirty(guiElem);
	}
}

void GUIWidget::UnregisterElementInternal(GUIElementBase* elem)
{
	assert(elem != nullptr);

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
		mDrawGroups.NotifyMeshDirty(static_cast<GUIElement*>(elem));
}

void GUIWidget::MarkContentDirtyInternal(GUIElementBase* elem)
{
	if(elem->GetTypeInternal() == GUIElementBase::Type::Element)
	{
		auto guiElement = static_cast<GUIElement*>(elem);

		mDirtyContents.insert(guiElement);
		mDrawGroups.NotifyContentDirty(guiElement);
	}
}

void GUIWidget::SetSkin(const HGUISkin& skin)
{
	mSkin = skin;

	for(auto& element : mElements)
		element->RefreshStyleInternal();
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
				dirtyElement->UpdateRenderElementsInternal();

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

	Vector2I localPos(Math::RoundToInt(vecPos.X), Math::RoundToInt(vecPos.Y));
	return mBounds.Contains(localPos);
}

void GUIWidget::UpdateBounds() const
{
	if(!mElements.empty())
		mBounds = mElements[0]->GetClippedBoundsInternal();

	for(auto& elem : mElements)
	{
		Rect2I elemBounds = elem->GetClippedBoundsInternal();
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

	mPanel->SetLayoutDataInternal(layoutData);
	mPanel->MarkLayoutAsDirtyInternal();
}
