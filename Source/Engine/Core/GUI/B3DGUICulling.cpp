//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUILayout.h"
#include "GUI/B3DGUIInteractable.h"
#include "Reflection/B3DRTTIType.h"

using namespace b3d;

GUICulling::GUICulling(const TArrayView<GUIElement*>& elements, float maximumQuadTreeSize)
	: mMaximumQuadTreeSize(maximumQuadTreeSize)
{
	RebuildQuadTree(elements);
}

void GUICulling::RebuildQuadTree(const TArrayView<GUIElement*>& elements)
{
	mQuadTree = B3DMakeUnique<GUIElementQuadTree>(Vector2(-mMaximumQuadTreeSize * 0.5f, -mMaximumQuadTreeSize * 0.5f), mMaximumQuadTreeSize);

	for(const auto& child : elements)
		mQuadTree->AddElement(child);
}

void GUICulling::UpdateVisibleElements(const Area2& bounds)
{
	GUIElementQuadTree::AreaIntersectIterator areaIterator(*mQuadTree, bounds);
	while(areaIterator.MoveNext())
	{
		GUIElement* const element = areaIterator.GetElement();

		// Element was culled, but is no longer culled
		if(element->IsCulled())
		{
			GUIElementCullInformation cullInformation;
			cullInformation.LastVisibleQueryIndex = mCullingQueryIndex;

			mNonCulledElements.insert(std::make_pair(element, cullInformation));
			element->SetCulled(false);
		}
		// Was previously visible, mark as still visible
		else
		{
			if(auto found = mNonCulledElements.find(element); B3D_ENSURE(found != mNonCulledElements.end()))
				found->second.LastVisibleQueryIndex = mCullingQueryIndex;
		}
	}

	// Find entries that were previously not culled but are now culled, populate visible elements
	mVisibleElements.Clear();
	for(auto it = mNonCulledElements.begin(); it != mNonCulledElements.end();)
	{
		if(it->second.LastVisibleQueryIndex == mCullingQueryIndex)
		{
			mVisibleElements.Add(it->first);

			++it;
			continue;
		}

		it->first->SetCulled(true);
		it = mNonCulledElements.erase(it);
	}

	mCullingQueryIndex++;
}

void GUICulling::ClearElements()
{
	GUIElementQuadTree::TreeIterator treeIterator(*mQuadTree);
	while(treeIterator.MoveNext())
	{
		GUIElement* const element = treeIterator.GetElement();

		const SpatialTreeElementId& quadTreeId = element->GetQuadTreeId();
		if(B3D_ENSURE(quadTreeId.IsValid()))
		{
			element->SetQuadTreeId(SpatialTreeElementId());
			element->SetCulled(false);
		}
	}

	mNonCulledElements.clear();
	mVisibleElements.Clear();
}

void GUICulling::RegisterElement(GUIElement* element)
{
	B3D_ENSURE(!element->GetQuadTreeId().IsValid());
	mQuadTree->AddElement(element);

	// All new elements default to culled, but we're guaranteed to run a new culling pass due to MarkLayoutAsDirty before next render, so this will be updated
	element->SetCulled(true);
}

void GUICulling::UnregisterElement(GUIElement* element)
{
	const SpatialTreeElementId& quadTreeId = element->GetQuadTreeId();
	if(B3D_ENSURE(quadTreeId.IsValid()))
	{
		mQuadTree->RemoveElement(quadTreeId);
		element->SetQuadTreeId(SpatialTreeElementId());
	}

	if(element->IsCulled())
	{
		element->SetCulled(false);
	}
	else
	{
		B3D_ENSURE(mNonCulledElements.erase(element) == 1);

		if(auto found = std::find(mVisibleElements.Begin(), mVisibleElements.End(), element); B3D_ENSURE(found != mVisibleElements.End()))
			mVisibleElements.SwapAndErase(found);
	}
}
