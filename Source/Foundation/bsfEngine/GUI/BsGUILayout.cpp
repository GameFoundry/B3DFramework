//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUILayout.h"
#include "GUI/BsGUIInteractable.h"
#include "Reflection/BsRTTIType.h"

using namespace bs;

/** @cond RTTI */
/** @addtogroup RTTI-Impl-Engine
 *  @{
 */

namespace bs
{
	class B3D_EXPORT GUILayoutRTTI : public TRTTIType<GUILayout, GUIElement, GUILayoutRTTI>
	{
	public:
		const String& GetRttiName()
		{
			static String name = "GUILayout";
			return name;
		}

		u32 GetRttiId() const override { return TID_GUILayout; }

		SPtr<IReflectable> NewRttiObject() { return nullptr; }
	};
} // namespace bs

/** @} */
/** @endcond */

GUILayout::GUILayout(const GUISizeConstraints& dimensions)
	: GUIElement(dimensions)
{}

void GUILayout::AddElement(GUIElement* element)
{
	if(element->IsPendingDestroy())
		return;

	RegisterChildElement(element);
}

void GUILayout::InsertElement(u32 idx, GUIElement* element)
{
	if(idx > (u32)mChildren.size())
		B3D_EXCEPT(InvalidParametersException, "Index out of range: " + ToString(idx) + ". Valid range: 0 .. " + ToString((u32)mChildren.size()));

	if(element->IsPendingDestroy())
		return;

	GUIElement* parentElement = element->GetParent();
	if(parentElement != nullptr)
	{
		parentElement->UnregisterChildElement(element);
	}

	element->SetParent(this);
	mChildren.Insert(mChildren.begin() + idx, element);

	element->SetActiveRecursive(IsActive());
	element->SetHiddenRecursive(IsHidden());
	element->SetDisabledRecursive(IsDisabled());

	if(mIsCullingEnabled)
		RegisterElementForCulling(element);

	MarkLayoutAsDirty();
}

void GUILayout::RemoveElement(GUIElement* element)
{
	UnregisterChildElement(element);
}

void GUILayout::RemoveElementAt(u32 idx)
{
	if(idx >= (u32)mChildren.size())
		B3D_EXCEPT(InvalidParametersException, "Index out of range: " + ToString(idx) + ". Valid range: 0 .. " + ToString((u32)mChildren.size()));

	GUIElement* child = mChildren[idx];
	mChildren.erase(mChildren.begin() + idx);

	child->SetParent(nullptr);

	if(mIsCullingEnabled)
		UnregisterElementFromCulling(child);

	MarkLayoutAsDirty();
}

void GUILayout::Clear()
{
	if(mIsCullingEnabled)
	{
		ClearElementCullInformation();
		mQuadTree = B3DMakeUnique<GUIElementQuadTree>(Vector2(-kMaximumQuadtreeSize * 0.5f, -kMaximumQuadtreeSize * 0.5f), kMaximumQuadtreeSize);
	}

	DestroyChildElements();
}

void GUILayout::SetEnableCulling(bool enable)
{
	if(mIsCullingEnabled == enable)
		return;
	
	mIsCullingEnabled = enable;

	if(enable)
		RebuildQuadTree();
	else
	{
		ClearElementCullInformation();
		mQuadTree = nullptr;
	}
}

void GUILayout::RebuildQuadTree()
{
	B3D_ENSURE(mIsCullingEnabled);

	mQuadTree = B3DMakeUnique<GUIElementQuadTree>(Vector2(-kMaximumQuadtreeSize * 0.5f, -kMaximumQuadtreeSize * 0.5f), kMaximumQuadtreeSize);

	for(const auto& child : mChildren)
		mQuadTree->AddElement(child);
}

void GUILayout::ClearElementCullInformation()
{
	for(const auto& element : mChildren)
	{
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

void GUILayout::RegisterElementForCulling(GUIElement* element)
{
	B3D_ENSURE(!element->GetQuadTreeId().IsValid());
	mQuadTree->AddElement(element);

	// All new elements default to culled, but we're guaranteed to run a new culling pass due to MarkLayoutAsDirty before next render, so this will be updated
	element->SetCulled(true);
}

void GUILayout::UnregisterElementFromCulling(GUIElement* element)
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

void GUILayout::RegisterChildElement(GUIElement* element)
{
	Super::RegisterChildElement(element);

	if(mIsCullingEnabled)
		RegisterElementForCulling(element);
}

void GUILayout::UnregisterChildElement(GUIElement* element)
{
	if(mIsCullingEnabled)
		UnregisterElementFromCulling(element);

	Super::UnregisterChildElement(element);
}

void GUILayout::UpdateAbsoluteCoordinatesForChildren()
{
	if(mIsCullingEnabled)
	{
		Rect2 relativeClippedArea = mAbsoluteClippedArea.ToRect2();
		relativeClippedArea.X -= (float)mAbsolutePosition.X;
		relativeClippedArea.Y -= (float)mAbsolutePosition.Y;

		// TODO - Need to undo scale

		GUIElementQuadTree::AreaIntersectIterator areaIterator(*mQuadTree, relativeClippedArea);
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

		for(auto& visibleChild : mVisibleElements)
		{
			visibleChild->UpdateAbsoluteCoordinates(mIntermediateAbsolutePosition, mAbsoluteScale, mIntermediateAbsoluteClippedArea);
			visibleChild->UpdateAbsoluteCoordinatesForChildren();
		}
		
		mCullingQueryIndex++;
	}
	else
	{
		for(auto& child : mChildren)
		{
			child->UpdateAbsoluteCoordinates(mIntermediateAbsolutePosition, mAbsoluteScale, mIntermediateAbsoluteClippedArea);
			child->UpdateAbsoluteCoordinatesForChildren();
		}
	}
}

RTTIType* GUILayout::GetRttiStatic()
{
	return GUILayoutRTTI::Instance();
}

RTTIType* GUILayout::GetRtti() const
{
	return GetRttiStatic();
}
