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

namespace bs
{
	namespace impl
	{
		SPtr<VertexDataDesc> gGUITriangleMeshDesc()
		{
			static SPtr<VertexDataDesc> sDesc;

			if(!sDesc)
			{
				sDesc = bs_shared_ptr_new<VertexDataDesc>();
				sDesc->addVertElem(VET_FLOAT2, VES_POSITION);
				sDesc->addVertElem(VET_FLOAT2, VES_TEXCOORD);
			}

			return sDesc;
		}

		SPtr<VertexDataDesc> gGUILineMeshDesc()
		{
			static SPtr<VertexDataDesc> sDesc;

			if(!sDesc)
			{
				sDesc = bs_shared_ptr_new<VertexDataDesc>();
				sDesc->addVertElem(VET_FLOAT2, VES_POSITION);
			}

			return sDesc;
			
		}
	}

	GUIDrawGroups::GUIDrawGroups(GUIWidget* parentWidget)
		:mWidget(parentWidget)
	{
		GUIDrawGroup mainDrawGroup;
		mainDrawGroup.minDepth = 0;
		mainDrawGroup.depthRange = std::numeric_limits<UINT32>::max();
		mainDrawGroup.id = mNextDrawGroupId++;
		
		mDrawGroups.push_back(mainDrawGroup);
	}

	void GUIDrawGroups::add(GUIElement* element)
	{
		const SmallVector<GUIRenderElement, 4> & renderElements = element->_getRenderElements();

		GUIGroupElement& groupElement = mElements[element];
		groupElement.element = element;
		groupElement.bounds = element->_getClippedBounds();
		groupElement.groups.resize(renderElements.size());

		for (UINT32 i = 0; i < renderElements.size(); i++)
			add(groupElement, i);

		mGroupsCoreDirty = true;
	}

	void GUIDrawGroups::add(GUIGroupElement& groupElement, UINT32 renderElementIdx)
	{
		GUIElement* element = groupElement.element;
		const SmallVector<GUIRenderElement, 4> & renderElements = element->_getRenderElements();

		const GUIRenderElement& renderElement = renderElements[renderElementIdx];
		UINT32 elemDepth = element->_getDepth() + renderElement.depth;

		// Groups are expected to be sorted by minDepth
		for (UINT32 j = 0; j < (UINT32)mDrawGroups.size(); j++)
		{
			if (elemDepth < mDrawGroups[j].minDepth || elemDepth >= (mDrawGroups[j].minDepth + mDrawGroups[j].depthRange))
				continue;

			add(groupElement, renderElementIdx, j);
			break;
		}
	}

	void GUIDrawGroups::add(GUIGroupElement& groupElement, UINT32 renderElementIdx, UINT32 groupIdx)
	{
		GUIElement* element = groupElement.element;
		const SmallVector<GUIRenderElement, 4> & renderElements = element->_getRenderElements();

		const GUIRenderElement& renderElement = renderElements[renderElementIdx];
		UINT32 elemDepth = element->_getDepth() + renderElement.depth;

		SpriteMaterial* spriteMaterial = renderElement.material;
		assert(spriteMaterial != nullptr);

		GUIDrawGroup& group = mDrawGroups[groupIdx];
		if (spriteMaterial->allowBatching())
		{
			group.cachedElements.push_back(GUIGroupRenderElement(element, renderElementIdx));

			Rect2I bounds = element->_getClippedBounds();
			group.bounds.encapsulate(bounds);
			group.needsRedraw = true;
			group.dirtyTexture = true;

			groupElement.groups[renderElementIdx] = group.id;
		}
		else
		{
			bool needsSplit = elemDepth != group.minDepth;
			if (needsSplit)
			{
				GUIDrawGroup& newGroup = split(groupIdx, elemDepth);

				newGroup.nonCachedElements.push_back(GUIGroupRenderElement(element, renderElementIdx));

				groupElement.groups[renderElementIdx] = newGroup.id;
				newGroup.needsRedraw = true;
			}
			else
			{
				group.nonCachedElements.push_back(GUIGroupRenderElement(element, renderElementIdx));

				groupElement.groups[renderElementIdx] = group.id;
				group.needsRedraw = true;
			}
		}
	}

	void GUIDrawGroups::remove(GUIElement* element)
	{
		auto iterFind = mElements.find(element);
		if (iterFind == mElements.end())
			return;

		for (UINT32 i = 0; i < iterFind->second.groups.size(); i++)
			remove(iterFind->second, i);

		mElements.erase(element);
		mGroupsCoreDirty = true;
	}

	void GUIDrawGroups::remove(GUIGroupElement& groupElement, UINT32 renderElementIdx)
	{
		if (renderElementIdx >= (UINT32)groupElement.groups.size())
			return;
		
		GUIElement* element = groupElement.element;
		const SmallVector<GUIRenderElement, 4>& renderElements = element->_getRenderElements();

		auto iterFind = std::find_if(mDrawGroups.begin(), mDrawGroups.end(),
			[drawGroupId = groupElement.groups[renderElementIdx]](const GUIDrawGroup& group) { return group.id == drawGroupId; });

		assert(iterFind != mDrawGroups.end());
		if (iterFind != mDrawGroups.end())
		{
			UINT32 idx = (UINT32)(iterFind - mDrawGroups.begin());
			remove(groupElement, renderElementIdx, idx);
		}
	}

	void GUIDrawGroups::remove(GUIGroupElement& groupElement, UINT32 renderElementIdx, UINT32 groupIdx)
	{
		GUIElement* element = groupElement.element;
		const SmallVector<GUIRenderElement, 4>& renderElements = element->_getRenderElements();
		GUIDrawGroup& group = mDrawGroups[groupIdx];

		for(auto iter = group.cachedElements.begin(); iter != group.cachedElements.end();)
		{
			if (iter->element == element && iter->renderElementIdx == renderElementIdx)
			{
				group.dirtyBounds = true;
				
				iter = group.cachedElements.erase(iter);
			}
			else
				++iter;
		}
		
		group.nonCachedElements.erase(std::remove_if(group.nonCachedElements.begin(), group.nonCachedElements.end(),
			[element, renderElementIdx](const GUIGroupRenderElement& x)
			{ return x.element == element && x.renderElementIdx == renderElementIdx; }),
			group.nonCachedElements.end());

		group.needsRedraw = true;

		// Purposely not clearing draw group on the GUIRenderElement, as its indices could have changed and it might not
		// match the current index. It's up to external code to handle that.
		groupElement.groups[renderElementIdx] = -1;

		// Attempt to merge with previous group
		if(group.nonCachedElements.empty() && group.minDepth > 0)
		{
			assert(groupIdx > 0);

			UINT32 prevGroupIdx = groupIdx - 1;
			GUIDrawGroup& prevGroup = mDrawGroups[prevGroupIdx];

			prevGroup.depthRange += group.depthRange;

			for (auto& entry : group.cachedElements)
			{
				auto iterFind = mElements.find(entry.element);
				assert(iterFind != mElements.end());
				if (iterFind != mElements.end())
					iterFind->second.groups[entry.renderElementIdx] = prevGroup.id;

				prevGroup.dirtyBounds = true;
			}
			
			std::move(group.cachedElements.begin(), group.cachedElements.end(), std::back_inserter(prevGroup.cachedElements));
			prevGroup.needsRedraw = true;
			
			mDrawGroups.erase(mDrawGroups.begin() + groupIdx);
		}
	}

	GUIDrawGroupRenderDataUpdate GUIDrawGroups::rebuildDirty(bool forceRebuildMeshes)
	{
		// Update dirty draw groups and mark them for redraw
		bool shouldRebuildMeshes = forceRebuildMeshes;
		for(auto& entry : mDirtyElements)
		{
			GUIElement* element = entry.first;
			
			auto iterFind = mElements.find(element);
			if (iterFind == mElements.end())
				continue;

			shouldRebuildMeshes = true;
			
			const SmallVector<GUIRenderElement, 4> & renderElements = element->_getRenderElements();
			GUIGroupElement& groupElement = iterFind->second;

			bool dirtyBounds = false;
			if ((entry.second & DirtyContent) != 0)
			{
				bool renderElementsDirty = groupElement.groups.size() != renderElements.size();

				// If render element count changed, do a full rebuild of the draw group
				if (renderElementsDirty)
				{
					remove(element);
					add(element);

					continue;
				}

				// If bounds changed, rebuild the bounds of the draw groups
				Rect2I bounds = element->_getClippedBounds();
				if (groupElement.bounds != bounds)
				{
					dirtyBounds = true;
					groupElement.bounds = bounds;
				}
			}
				
			for (UINT32 i = 0; i < renderElements.size(); i++)
			{
				const GUIRenderElement& renderElement = renderElements[i];
				INT32 drawGroupId = groupElement.groups[i];

				// All render elements draw group IDs should be assigned at this point
				assert(drawGroupId != -1);

				auto iterFind2 = std::find_if(mDrawGroups.begin(), mDrawGroups.end(),
					[drawGroupId](const GUIDrawGroup& group) { return group.id == drawGroupId; });

				assert(iterFind2 != mDrawGroups.end());
				if (iterFind2 != mDrawGroups.end())
				{
					GUIDrawGroup& group = *iterFind2;

					if (dirtyBounds)
						group.dirtyBounds = dirtyBounds;

					bool needsGroupChange = false;
					if((entry.second & DirtyMesh) != 0)
					{
						UINT32 depth = element->_getDepth() + renderElement.depth;

						// If same as min-depth, no group change is necessary in any case
						if (depth != group.minDepth)
						{
							// If less than min-depth, group change is always necessary
							if (depth < group.minDepth)
								needsGroupChange = true;
							// Non-batching elements must be at min-depth, so group change is necessary
							else if (!renderElement.material->allowBatching())
								needsGroupChange = true;
							// Batching but outside of the group's depth range, group change is necessary
							else if (depth >= (group.minDepth + group.depthRange))
								needsGroupChange = true;
						}
					}

					if (!needsGroupChange && (entry.second & DirtyContent) != 0)
					{
						// Check if the material changed
						if (renderElement.material->allowBatching())
						{
							auto iterFind3 = std::find_if(group.cachedElements.begin(), group.cachedElements.end(), [element, i](auto& x)
								{ return x.element == element && x.renderElementIdx == i; });
							if (iterFind3 == group.cachedElements.end())
								needsGroupChange = true;
						}
						else
						{
							auto iterFind3 = std::find_if(group.nonCachedElements.begin(), group.nonCachedElements.end(), [element, i](auto& x)
								{ return x.element == element && x.renderElementIdx == i; });
							if (iterFind3 == group.nonCachedElements.end())
								needsGroupChange = true;
						}
					}

					if(needsGroupChange)
					{
						UINT32 groupIdx = (UINT32)(iterFind2 - mDrawGroups.begin());
						remove(groupElement, i, groupIdx);
						add(groupElement, i);

						mGroupsCoreDirty = true;
					}

					group.needsRedraw = true;
				}
			}
		}

		mDirtyElements.clear();
		
		// Update dirty bounds and textures
		for(auto& entry : mDrawGroups)
		{
			if(entry.dirtyBounds)
			{
				Rect2I newBounds = calculateBounds(entry);
				entry.dirtyTexture = true;
				entry.dirtyBounds = false;

				entry.bounds = newBounds;
			}

			if(entry.dirtyTexture)
			{
				if(entry.outputTexture == nullptr ||
					entry.bounds.width != entry.outputTexture->getProperties().width ||
					entry.bounds.height != entry.outputTexture->getProperties().height)
				{
					entry.outputTexture = nullptr;

					TEXTURE_DESC texDesc;
					texDesc.width = entry.bounds.width;
					texDesc.height = entry.bounds.height;
					texDesc.format = PF_RGBA8;
					texDesc.usage = TU_RENDERTARGET;

					HTexture texture = Texture::create(texDesc);

					RENDER_TEXTURE_DESC rtDesc;
					rtDesc.colorSurfaces[0].texture = texture;

					entry.outputTexture = RenderTexture::create(rtDesc);
				}
				
				entry.dirtyTexture = false;
			}
		}

		// Rebuild draw group meshes if needed
		// Note: Ideally we can avoid rebuilding all meshes any rebuild only the changed ones
		if (shouldRebuildMeshes)
			rebuildMeshes();

		// Return data required for updating the renderer
		GUIDrawGroupRenderDataUpdate output;
		output.triangleMesh = mTriangleMesh ? mTriangleMesh->getCore() : nullptr;
		output.lineMesh = mLineMesh ? mLineMesh->getCore() : nullptr;

		output.groupDirtyState.reserve(mDrawGroups.size());
		for (auto& entry : mDrawGroups)
		{
			output.groupDirtyState.push_back(entry.needsRedraw);
			entry.needsRedraw = false;
		}

		// Note: If only mesh rebuild happened, we should only update the specific render elements
		// that changed. (Note that in this case the mesh rebuild flag also signals changes to the
		// GUI element texture/tint/etc.)
		if(mGroupsCoreDirty || shouldRebuildMeshes)
		{
			output.newDrawGroups.reserve(mDrawGroups.size());
			for (auto& entry : mDrawGroups)
				output.newDrawGroups.push_back(getRenderData(entry));

			// Register elements that depend on render textures (currently these always correspond to input bridged elements)
			SmallVector<std::pair<const GUIElement*, SPtr<const RenderTarget>>, 4> bridgedElements;
			gGUIManager().getBridgedElements(mWidget, bridgedElements);

			for (auto& entry : bridgedElements)
			{
				auto* element = const_cast<GUIElement*>(entry.first);
				auto iterFind = mElements.find(element);

				assert(iterFind != mElements.end());
				if (iterFind == mElements.end())
					continue;

				const SPtr<const RenderTarget>& target = entry.second;
				for(auto& groupId : iterFind->second.groups)
				{
					for (auto& group : output.newDrawGroups)
					{
						if (group.id != groupId)
							continue;

						group.renderTargetElements.push_back(target->getCore());
					}
				}
			}
		}
		
		mGroupsCoreDirty = false;
		return output;
	}

	void GUIDrawGroups::notifyContentDirty(GUIElement* element)
	{
		mDirtyElements[element] |= DirtyContent;
	}

	void GUIDrawGroups::notifyMeshDirty(GUIElement* element)
	{
		mDirtyElements[element] |= DirtyMesh;
	}

	void GUIDrawGroups::rebuildMeshes()
	{
		struct GUIMaterialGroup
		{
			SpriteMaterial* material;
			SpriteMaterialInfo matInfo;
			GUIMeshType meshType;
			UINT32 numVertices;
			UINT32 numIndices;
			UINT32 depth;
			UINT32 minDepth;
			Rect2I bounds;
			GUIDrawGroup* drawGroup;
			Vector<GUIGroupRenderElement> elements;
		};

		struct GUIMaterialGroupSet
		{
			using SortedGroupSet = FrameSet<GUIMaterialGroup, std::function<bool(const GUIMaterialGroup&, const GUIMaterialGroup&)>>;
			
			UINT32 numMeshes = 0;
			UINT32 numIndices[2] = { 0, 0 };
			UINT32 numVertices[2] = { 0, 0 };

			SortedGroupSet sortedGroups;
		};
		
		bs_frame_mark();
		{
			// Make a list of all GUI elements, sorted from farthest to nearest (highest depth to lowest)
			auto elemComp = [](const GUIGroupRenderElement& a, const GUIGroupRenderElement& b)
			{
				UINT32 aDepth = a.element->_getDepth() + a.element->_getRenderElements()[a.renderElementIdx].depth;
				UINT32 bDepth = b.element->_getDepth() + b.element->_getRenderElements()[b.renderElementIdx].depth;

				// Compare pointers just to differentiate between two elements with the same depth, their order doesn't really matter, but std::set
				// requires all elements to be unique
				return (aDepth > bDepth) ||
					(aDepth == bDepth && a.element > b.element) ||
					(aDepth == bDepth && a.element == b.element && a.renderElementIdx > b.renderElementIdx);
			};

			FrameVector<GUIMaterialGroupSet> groupSets;
			groupSets.reserve(mDrawGroups.size());

			for (auto& entry : mDrawGroups)
			{
				// Note: If we keep visible elements separate from invisible, plus provide sorting on insert, we could avoid this
				// re-sorting and re-inserting step.
				FrameSet<GUIGroupRenderElement, std::function<bool(const GUIGroupRenderElement&, const GUIGroupRenderElement&)>> allElements(elemComp);
				for (auto& element : entry.cachedElements)
				{
					if (!element.element->_isVisible())
						continue;

					allElements.insert(element);
				}

				for (auto& element : entry.nonCachedElements)
				{
					if (!element.element->_isVisible())
						continue;

					allElements.insert(element);
				}

				// Group the elements in such a way so that we end up with a smallest amount of
				// meshes, without breaking back to front rendering order
				FrameUnorderedMap<UINT64, FrameVector<GUIMaterialGroup>> materialGroups;
				for (auto& elem : allElements)
				{
					GUIElement* guiElem = elem.element;
					UINT32 renderElemIdx = elem.renderElementIdx;
					const GUIRenderElement& renderElem = elem.element->_getRenderElements()[renderElemIdx];

					UINT32 elemDepth = guiElem->_getDepth() + renderElem.depth;
					Rect2I bounds = guiElem->_getClippedBounds();

					SpriteMaterial* spriteMaterial = renderElem.material;
					const SpriteMaterialInfo& matInfo = *renderElem.matInfo;
					assert(spriteMaterial != nullptr);

					UINT64 hash = spriteMaterial->getMergeHash(matInfo);
					FrameVector<GUIMaterialGroup>& groupsPerMaterial = materialGroups[hash];

					// Try to find a group this material will fit in:
					//  - Group that has a depth value same or one below elements depth will always be a match
					//  - Otherwise, we search higher depth values as well, but we only use them if no elements in between those depth values
					//    overlap the current elements bounds.
					GUIMaterialGroup* foundGroup = nullptr;

					if (spriteMaterial->allowBatching())
					{
						for (auto groupIter = groupsPerMaterial.rbegin(); groupIter != groupsPerMaterial.rend(); ++groupIter)
						{
							GUIMaterialGroup& group = *groupIter;
							if (group.depth == elemDepth)
							{
								foundGroup = &group;
								break;
							}
							else
							{
								UINT32 startDepth = elemDepth;
								UINT32 endDepth = group.depth;

								Rect2I potentialGroupBounds = group.bounds;
								potentialGroupBounds.encapsulate(bounds);

								bool foundOverlap = false;
								for (auto& material : materialGroups)
								{
									for (auto& matGroup : material.second)
									{
										if (&matGroup == &group)
											continue;

										if ((matGroup.minDepth >= startDepth && matGroup.minDepth <= endDepth)
											|| (matGroup.depth >= startDepth && matGroup.depth <= endDepth))
										{
											if (matGroup.bounds.overlaps(potentialGroupBounds))
											{
												foundOverlap = true;
												break;
											}
										}
									}
								}

								if (!foundOverlap)
								{
									foundGroup = &group;
									break;
								}
							}
						}
					}

					if (foundGroup == nullptr)
					{
						groupsPerMaterial.push_back(GUIMaterialGroup());
						foundGroup = &groupsPerMaterial[groupsPerMaterial.size() - 1];

						foundGroup->depth = elemDepth;
						foundGroup->minDepth = elemDepth;
						foundGroup->bounds = bounds;
						foundGroup->elements.push_back(GUIGroupRenderElement(guiElem, renderElemIdx));
						foundGroup->matInfo = matInfo.clone();
						foundGroup->material = spriteMaterial;
						foundGroup->numVertices = renderElem.numVertices;
						foundGroup->numIndices = renderElem.numIndices;
						foundGroup->meshType = renderElem.type;
						foundGroup->drawGroup = &entry;
					}
					else
					{
						foundGroup->bounds.encapsulate(bounds);
						foundGroup->elements.push_back(GUIGroupRenderElement(guiElem, renderElemIdx));
						foundGroup->minDepth = std::min(foundGroup->minDepth, elemDepth);

						// It's expected that GUI element doesn't use same material for different mesh types so this should always be true
						assert(renderElem.type == foundGroup->meshType);

						// Draw groups are super-set of material groups, so a material group cannot cross a draw group boundary
						assert(foundGroup->drawGroup == &entry);

						foundGroup->numVertices += renderElem.numVertices;
						foundGroup->numIndices += renderElem.numIndices;

						spriteMaterial->merge(foundGroup->matInfo, matInfo);
					}
				}

				// Make a list of all GUI elements, sorted from farthest to nearest (highest depth to lowest)
				auto groupComp = [](const GUIMaterialGroup& a, const GUIMaterialGroup& b)
				{
					return (a.depth > b.depth) || (a.depth == b.depth && &a > &b);
					// Compare pointers just to differentiate between two elements with the same depth, their order doesn't really matter, but std::set
					// requires all elements to be unique
				};

				groupSets.push_back(GUIMaterialGroupSet());
				GUIMaterialGroupSet& groupSet = groupSets.back();

				groupSet.sortedGroups = GUIMaterialGroupSet::SortedGroupSet(groupComp);
				for (auto& material : materialGroups)
				{
					for (auto& group : material.second)
					{
						groupSet.sortedGroups.insert(std::move(group));

						UINT32 typeIdx = (UINT32)group.meshType;
						groupSet.numIndices[typeIdx] += group.numIndices;
						groupSet.numVertices[typeIdx] += group.numVertices;

						groupSet.numMeshes++;
					}
				}
			}

			mTriangleMesh = nullptr;
			mLineMesh = nullptr;

			UINT32 totalNumIndices[2] = { 0, 0 };
			UINT32 totalNumVertices[2] = { 0, 0 };

			for (UINT32 i = 0; i < (UINT32)mDrawGroups.size(); i++)
			{
				GUIMaterialGroupSet& set = groupSets[i];
				mDrawGroups[i].meshes.resize(set.numMeshes);

				for (UINT32 j = 0; j < 2; j++)
				{
					totalNumIndices[j] += set.numIndices[j];
					totalNumVertices[j] += set.numVertices[j];
				}
			}

			SPtr<MeshData> meshData[2];
			SPtr<VertexDataDesc> vertexDesc[2] = { impl::gGUITriangleMeshDesc(), impl::gGUILineMeshDesc() };

			UINT8* vertices[2] = { nullptr, nullptr };
			UINT32* indices[2] = { nullptr, nullptr };

			for (UINT32 i = 0; i < 2; i++)
			{
				if (totalNumVertices[i] > 0 && totalNumIndices[i] > 0)
				{
					meshData[i] = MeshData::create(totalNumVertices[i], totalNumIndices[i], vertexDesc[i]);

					vertices[i] = meshData[i]->getElementData(VES_POSITION);
					indices[i] = meshData[i]->getIndices32();
				}
			}

			UINT32 vertexOffset[2] = { 0, 0 };
			UINT32 indexOffset[2] = { 0, 0 };

			for (UINT32 i = 0; i < (UINT32)mDrawGroups.size(); i++)
			{
				GUIMaterialGroupSet& set = groupSets[i];

				// Fill buffers for each group and update their meshes
				UINT32 meshIdx = 0;

				for (auto& group : set.sortedGroups)
				{
					GUIMesh& guiMesh = mDrawGroups[i].meshes[meshIdx];
					guiMesh.matInfo = group.matInfo;
					guiMesh.material = group.material;
					guiMesh.isLine = group.meshType == GUIMeshType::Line;

					auto typeIdx = (UINT32)group.meshType;
					guiMesh.indexOffset = indexOffset[typeIdx];

					Vector2I groupOffset(0, 0);
					if(guiMesh.material->allowBatching())
						groupOffset = Vector2I(-group.drawGroup->bounds.x, -group.drawGroup->bounds.y);
					
					UINT32 groupNumIndices = 0;
					for (auto& matElement : group.elements)
					{
						matElement.element->_fillBuffer(
							vertices[typeIdx], indices[typeIdx],
							vertexOffset[typeIdx], indexOffset[typeIdx], groupOffset,
							totalNumVertices[typeIdx], totalNumIndices[typeIdx],
							matElement.renderElementIdx);

						const GUIRenderElement& renderElement = matElement.element->_getRenderElements()[matElement.renderElementIdx];

						UINT32 indexStart = indexOffset[typeIdx];
						UINT32 indexEnd = indexStart + renderElement.numIndices;

						for (UINT32 j = indexStart; j < indexEnd; j++)
							indices[typeIdx][j] += vertexOffset[typeIdx];

						indexOffset[typeIdx] += renderElement.numIndices;
						vertexOffset[typeIdx] += renderElement.numVertices;

						groupNumIndices += renderElement.numIndices;
					}

					guiMesh.indexCount = groupNumIndices;

					meshIdx++;
				}
			}

			if (meshData[0])
				mTriangleMesh = Mesh::_createPtr(meshData[0], MU_STATIC, DOT_TRIANGLE_LIST);

			if (meshData[1])
				mLineMesh = Mesh::_createPtr(meshData[1], MU_STATIC, DOT_LINE_LIST);
		}

		bs_frame_clear();
	}

	GUIDrawGroups::GUIDrawGroup& GUIDrawGroups::split(UINT32 groupIdx, UINT32 depth)
	{
		GUIDrawGroup& group = mDrawGroups[groupIdx];
		assert(depth > group.minDepth);
		
		UINT32 maxDepth = group.minDepth + group.depthRange;
		group.depthRange = depth - group.minDepth;

		GUIDrawGroup newSplitGroup;
		newSplitGroup.minDepth = depth;
		newSplitGroup.depthRange = maxDepth - newSplitGroup.minDepth;
		newSplitGroup.id = mNextDrawGroupId++;

		auto it = std::partition(group.cachedElements.begin(), group.cachedElements.end(),
			[depth](const GUIGroupRenderElement& x)
		{
				UINT32 elemDepth = x.element->_getDepth() + x.element->_getRenderElements()[x.renderElementIdx].depth;
				return elemDepth < depth;
		});

		std::move(it, group.cachedElements.end(), std::back_inserter(newSplitGroup.cachedElements));
		group.cachedElements.erase(it, group.cachedElements.end());

		for (auto& entry : newSplitGroup.cachedElements)
		{
			auto iterFind = mElements.find(entry.element);
			assert(iterFind != mElements.end());
			if (iterFind != mElements.end())
				iterFind->second.groups[entry.renderElementIdx] = newSplitGroup.id;
		}

		group.dirtyBounds = true;
		group.needsRedraw = true;
		newSplitGroup.dirtyBounds = true;

		mDrawGroups.insert(mDrawGroups.begin() + groupIdx + 1, std::move(newSplitGroup));
		return mDrawGroups[groupIdx + 1];
	}

	GUIMeshRenderData GUIDrawGroups::getRenderData(const GUIMesh& guiMesh)
	{
		SPtr<ct::Texture> textureCore;
		if (guiMesh.matInfo.texture.isLoaded())
			textureCore = guiMesh.matInfo.texture->getCore();
		else
			textureCore = nullptr;

		SPtr<ct::SpriteTexture> spriteTextureCore;
		if (guiMesh.matInfo.spriteTexture.isLoaded())
			spriteTextureCore = guiMesh.matInfo.spriteTexture->getCore();
		else
			spriteTextureCore = nullptr;

		GUIMeshRenderData output;
		output.material = guiMesh.material;
		output.texture = textureCore;
		output.spriteTexture = spriteTextureCore;
		output.tint = guiMesh.matInfo.tint;
		output.isLine = guiMesh.isLine;
		output.animationStartTime = guiMesh.matInfo.animationStartTime;
		output.additionalData = guiMesh.matInfo.additionalData;
		output.bufferIdx = 0;

		output.subMesh.indexOffset = guiMesh.indexOffset;
		output.subMesh.indexCount = guiMesh.indexCount;
		output.subMesh.drawOp = guiMesh.isLine ? DOT_LINE_LIST : DOT_TRIANGLE_LIST;

		return output;
	}

	GUIDrawGroupRenderData GUIDrawGroups::getRenderData(const GUIDrawGroup& drawGroup)
	{
		GUIDrawGroupRenderData output;
		output.id = drawGroup.id;
		output.destination = drawGroup.outputTexture->getCore();
		output.bounds = drawGroup.bounds;
		output.requiresRedraw = true;
		
		auto numElements = (UINT32)drawGroup.meshes.size();
		for(UINT32 i = 0; i < numElements; i++)
		{
			GUIMeshRenderData meshData = getRenderData(drawGroup.meshes[i]);
			if (meshData.subMesh.indexCount == 0)
				continue;
			
			if (meshData.material->allowBatching())
				output.cachedElements.push_back(std::move(meshData));
			else
				output.nonCachedElements.push_back(std::move(meshData));
		}
		
		return output;
	}

	Rect2I GUIDrawGroups::calculateBounds(GUIDrawGroup& group)
	{
		Rect2I bounds = Rect2I();
		bool boundsSet = false;

		for(auto& entry : group.cachedElements)
		{
			if (!entry.element->_isVisible())
				continue;
			
			Rect2I elementBounds = entry.element->_getClippedBounds();
			if (!boundsSet)
			{
				bounds = elementBounds;
				boundsSet = true;
			}
			else
				bounds.encapsulate(elementBounds);
		}

		return bounds;
	}

	GUIWidget::GUIWidget(const SPtr<Camera>& camera)
		: mCamera(camera), mDrawGroups(this)
	{
		construct(camera);
	}

	GUIWidget::GUIWidget(const HCamera& camera)
		: mCamera(camera->_getCamera()), mDrawGroups(this)
	{
		construct(mCamera);
	}

	void GUIWidget::construct(const SPtr<Camera>& camera)
	{
		if (mCamera != nullptr)
		{
			SPtr<RenderTarget> target = mCamera->getViewport()->getTarget();

			if (target != nullptr)
				mCachedRTId = target->getInternalID();
		}

		mDefaultNavGroup = GUINavGroup::create();

		GUIManager::instance().registerWidget(this);

		mPanel = GUIPanel::create();
		mPanel->_changeParentWidget(this);
		updateRootPanel();
	}

	GUIWidget::~GUIWidget()
	{
		_destroy();
	}

	SPtr<GUIWidget> GUIWidget::create(const SPtr<Camera>& camera)
	{
		return bs_shared_ptr(new (bs_alloc<GUIWidget>()) GUIWidget(camera));
	}

	SPtr<GUIWidget> GUIWidget::create(const HCamera& camera)
	{
		return bs_shared_ptr(new (bs_alloc<GUIWidget>()) GUIWidget(camera));
	}

	void GUIWidget::_destroy()
	{
		if (mPanel != nullptr)
		{
			GUILayout::destroy(mPanel);
			mPanel = nullptr;
		}

		if (mCamera != nullptr)
		{
			GUIManager::instance().unregisterWidget(this);
			mCamera = nullptr;
		}

		mElements.clear();
		mDirtyContents.clear();
	}

	void GUIWidget::setDepth(UINT8 depth)
	{
		mDepth = depth;
		mWidgetIsDirty = true;

		updateRootPanel();
	}

	Viewport* GUIWidget::getTarget() const
	{
		if(mCamera != nullptr)
			return mCamera->getViewport().get();

		return nullptr;
	}

	void GUIWidget::_updateTransform(const HSceneObject& parent)
	{
		// If the widgets parent scene object moved, we need to mark it as dirty
		// as the GUIManager batching relies on object positions, so it needs to be updated.
		const float diffEpsilon = 0.0001f;

		const Transform& tfrm = parent->getTransform();
		Vector3 position = tfrm.getPosition();
		Quaternion rotation = tfrm.getRotation();
		Vector3 scale = tfrm.getScale();

		if(!mWidgetIsDirty)
		{
			if(!Math::approxEquals(mPosition, position, diffEpsilon))
				mWidgetIsDirty = true;
			else
			{
				if(!Math::approxEquals(mRotation, rotation, diffEpsilon))
					mWidgetIsDirty = true;
				else
				{
					if(Math::approxEquals(mScale, scale))
						mWidgetIsDirty = true;
				}
			}
		}

		mPosition = position;
		mRotation = rotation;
		mScale = scale;
		mTransform = parent->getWorldMatrix();
	}

	void GUIWidget::_updateRT()
	{
		SPtr<RenderTarget> rt;
		UINT64 newRTId = 0;
		if(mCamera != nullptr)
		{
			rt = mCamera->getViewport()->getTarget();
			if (rt != nullptr)
				newRTId = rt->getInternalID();
		}

		if(mCachedRTId != newRTId)
		{
			mCachedRTId = newRTId;
			updateRootPanel();
		}
	}

	void GUIWidget::_updateLayout()
	{
		// Check if render target size changed and update if needed
		// Note: Purposely not relying to the RenderTarget::onResized callback, as it will trigger /before/ Input events.
		// These events might trigger a resize, meaning the size would be delayed one frame, resulting in a visual artifact
		// where the GUI doesn't match the target size.
		Viewport* target = getTarget();
		if (target != nullptr)
		{
			Rect2I area = target->getPixelArea();
			UINT32 width = area.width;
			UINT32 height = area.height;

			const Rect2I& panelArea = mPanel->_getLayoutData().area;
			if(panelArea.width != width || panelArea.height != height)
			{
				updateRootPanel();
				onOwnerTargetResized();
			}
		}

		bs_frame_mark();

		// Determine dirty contents and layouts
		FrameStack<GUIElementBase*> todo;
		todo.push(mPanel);

		while (!todo.empty())
		{
			GUIElementBase* currentElem = todo.top();
			todo.pop();

			if (currentElem->_isDirty())
			{
				GUIElementBase* updateParent = currentElem->_getUpdateParent();
				assert(updateParent != nullptr || currentElem == mPanel);

				if (updateParent != nullptr)
					_updateLayout(updateParent);
				else // Must be root panel
					_updateLayout(mPanel);
			}
			else
			{
				UINT32 numChildren = currentElem->_getNumChildren();
				for (UINT32 i = 0; i < numChildren; i++)
					todo.push(currentElem->_getChild(i));
			}
		}

		bs_frame_clear();
	}

	void GUIWidget::_updateLayout(GUIElementBase* elem)
	{
		GUIElementBase* parent = elem->_getParent();
		bool isPanelOptimized = parent != nullptr && parent->_getType() == GUIElementBase::Type::Panel;

		GUIElementBase* updateParent = nullptr;

		if (isPanelOptimized)
			updateParent = parent;
		else
			updateParent = elem;

		// For GUIPanel we can do a an optimization and update only the element in question instead
		// of all the children
		if (isPanelOptimized)
		{
			GUIPanel* panel = static_cast<GUIPanel*>(updateParent);

			GUIElementBase* dirtyElement = elem;
			dirtyElement->_updateOptimalLayoutSizes();

			LayoutSizeRange elementSizeRange = panel->_getElementSizeRange(dirtyElement);
			Rect2I elementArea = panel->_getElementArea(panel->_getLayoutData().area, dirtyElement, elementSizeRange);

			GUILayoutData childLayoutData = panel->_getLayoutData();
			panel->_updateDepthRange(childLayoutData);
			childLayoutData.area = elementArea;

			panel->_updateChildLayout(dirtyElement, childLayoutData);
		}
		else
		{
			GUILayoutData childLayoutData = updateParent->_getLayoutData();
			updateParent->_updateLayout(childLayoutData);
		}
		
		// Mark dirty contents
		bs_frame_mark();
		{
			FrameStack<GUIElementBase*> todo;
			todo.push(elem);

			while (!todo.empty())
			{
				GUIElementBase* currentElem = todo.top();
				todo.pop();

				_markContentDirty(currentElem);
				currentElem->_markAsClean();

				UINT32 numChildren = currentElem->_getNumChildren();
				for (UINT32 i = 0; i < numChildren; i++)
					todo.push(currentElem->_getChild(i));
			}
		}
		bs_frame_clear();
	}

	void GUIWidget::_registerElement(GUIElementBase* elem)
	{
		assert(elem != nullptr && !elem->_isDestroyed());

		if (elem->_getType() == GUIElementBase::Type::Element)
		{
			mElements.push_back(static_cast<GUIElement*>(elem));
			mWidgetIsDirty = true;

			// Find a draw group
			auto guiElem = static_cast<GUIElement*>(elem);
			mDrawGroups.add(guiElem);
			mDrawGroups.notifyContentDirty(guiElem);
		}
	}

	void GUIWidget::_unregisterElement(GUIElementBase* elem)
	{
		assert(elem != nullptr);

		auto iterFind = std::find(begin(mElements), end(mElements), elem);

		if (iterFind != mElements.end())
		{
			mElements.erase(iterFind);
			mWidgetIsDirty = true;
		}

		if (elem->_getType() == GUIElementBase::Type::Element)
		{
			mDirtyContents.erase(static_cast<GUIElement*>(elem));

			auto guiElem = static_cast<GUIElement*>(elem);
			mDrawGroups.remove(guiElem);
		}
	}

	void GUIWidget::_markMeshDirty(GUIElementBase* elem)
	{
		mWidgetIsDirty = true;

		if (elem->_getType() == GUIElementBase::Type::Element)
			mDrawGroups.notifyMeshDirty(static_cast<GUIElement*>(elem));
	}

	void GUIWidget::_markContentDirty(GUIElementBase* elem)
	{
		if (elem->_getType() == GUIElementBase::Type::Element)
		{
			auto guiElement = static_cast<GUIElement*>(elem);
			
			mDirtyContents.insert(guiElement);
			mDrawGroups.notifyContentDirty(guiElement);
		}
	}

	void GUIWidget::setSkin(const HGUISkin& skin)
	{
		mSkin = skin;

		for(auto& element : mElements)
			element->_refreshStyle();
	}

	const GUISkin& GUIWidget::getSkin() const
	{
		if(mSkin.isLoaded())
			return *mSkin;
		else
			return *BuiltinResources::instance().getGUISkin();
	}

	void GUIWidget::setCamera(const SPtr<Camera>& camera)
	{
		SPtr<Camera> newCamera = camera;
		if(newCamera != nullptr)
		{
			if (newCamera->getViewport()->getTarget() == nullptr)
				newCamera = nullptr;
		}

		if (mCamera == newCamera)
			return;

		GUIManager::instance().unregisterWidget(this);
		mCamera = newCamera;
		GUIManager::instance().registerWidget(this);

		updateRootPanel();
	}

	void GUIWidget::setIsActive(bool active)
	{
		mIsActive = active;
	}

	GUIDrawGroupRenderDataUpdate GUIWidget::rebuildDirtyRenderData()
	{
		if (!mIsActive)
			return GUIDrawGroupRenderDataUpdate();

		const bool dirty = mWidgetIsDirty || !mDirtyContents.empty();

		if(dirty)
		{
			mWidgetIsDirty = false;

			// Update render contents recursively because updates can cause child GUI elements to become dirty
			while(!mDirtyContents.empty())
			{
				mDirtyContentsTemp.swap(mDirtyContents);

				for (auto& dirtyElement : mDirtyContentsTemp)
					dirtyElement->_updateRenderElements();

				mDirtyContentsTemp.clear();
			}

			updateBounds();
		}

		return mDrawGroups.rebuildDirty(dirty);
	}

	bool GUIWidget::inBounds(const Vector2I& position) const
	{
		Viewport* target = getTarget();
		if (target == nullptr)
			return false;

		// Technically GUI widget bounds can be larger than the viewport, so make sure we clip to viewport first
		if(!target->getPixelArea().contains(position))
			return false;

		Vector3 vecPos((float)position.x, (float)position.y, 0.0f);
		vecPos = mTransform.inverse().multiplyAffine(vecPos);

		Vector2I localPos(Math::roundToInt(vecPos.x), Math::roundToInt(vecPos.y));
		return mBounds.contains(localPos);
	}

	void GUIWidget::updateBounds() const
	{
		if(!mElements.empty())
			mBounds = mElements[0]->_getClippedBounds();

		for(auto& elem : mElements)
		{
			Rect2I elemBounds = elem->_getClippedBounds();
			mBounds.encapsulate(elemBounds);
		}
	}

	void GUIWidget::ownerWindowFocusChanged()
	{
		onOwnerWindowFocusChanged();
	}

	void GUIWidget::updateRootPanel()
	{
		Viewport* target = getTarget();
		if (target == nullptr)
			return;

		Rect2I area = target->getPixelArea();
		UINT32 width = area.width;
		UINT32 height = area.height;

		GUILayoutData layoutData;
		layoutData.area.width = width;
		layoutData.area.height = height;
		layoutData.clipRect = Rect2I(0, 0, width, height);
		layoutData.setWidgetDepth(mDepth);

		mPanel->setWidth(width);
		mPanel->setHeight(height);

		mPanel->_setLayoutData(layoutData);
		mPanel->_markLayoutAsDirty();
	}
}
