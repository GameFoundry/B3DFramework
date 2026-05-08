//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIElement.h"
#include "GUI/B3DGUILayout.h"
#include "GUI/B3DGUIPanel.h"
#include "GUI/B3DGUISpace.h"
#include "GUI/B3DGUIInteractable.h"
#include "GUI/B3DGUIWidget.h"
#include "B3DGUIManager.h"
#include "B3DGUIUtility.h"
#include "StyleSheet/B3DGUIStyleSheet.h"
#include "Reflection/B3DRTTIType.h"
#include "Scene/B3DSceneObject.h"

using namespace b3d;

/** @cond RTTI */
/** @addtogroup RTTI-Impl-Engine
 *  @{
 */

namespace b3d
{
	class B3D_EXPORT GUIElementRTTI : public TRTTIType<GUIElement, IReflectable, GUIElementRTTI>
	{
	public:
		const String& GetRttiName()
		{
			static String name = "GUIElement";
			return name;
		}

		u32 GetRttiId() const override { return TID_GUIElement; }

		TShared<IReflectable> NewRttiObject() { return nullptr; }
	};
} // namespace b3d

/** @} */
/** @endcond */

GUIElement::GUIElement(const GUISizeConstraints& sizeConstraints)
	: mSizeConstraints(sizeConstraints)
{}

void GUIElement::SetPosition(const GUILogicalPoint& position)
{
	mSizeConstraints.ExplicitPosition = position;

	// Note: I could call _markMeshAsDirty with a little more work. If parent is layout then this call can be ignored
	// and if it's a panel, we can immediately change the position without a full layout rebuild.
	MarkLayoutAsDirty();
}

void GUIElement::SetSize(const GUILogicalSize& size)
{
	const bool isFixedBefore = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

	mSizeConstraints.Flags |= GUISizeConstraintFlag::FixedWidth | GUISizeConstraintFlag::WidthOverridenAtRuntime | GUISizeConstraintFlag::FixedHeight | GUISizeConstraintFlag::HeightOverridenAtRuntime;
	mSizeConstraints.Flags.Unset(GUISizeConstraintFlag::ExpandingWidth);
	mSizeConstraints.Flags.Unset(GUISizeConstraintFlag::ExpandingHeight);
	mSizeConstraints.MinimumWidth = mSizeConstraints.MaximumWidth = size.Width;
	mSizeConstraints.MinimumHeight = mSizeConstraints.MaximumHeight = size.Height;

	const bool isFixedAfter = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

	if(isFixedBefore != isFixedAfter)
		RefreshLayoutUpdateParentsForChildren();

	MarkLayoutAsDirty();
}

void GUIElement::SetWidth(GUILogicalUnit width)
{
	const bool isFixedBefore = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

	mSizeConstraints.Flags |= GUISizeConstraintFlag::FixedWidth | GUISizeConstraintFlag::WidthOverridenAtRuntime;
	mSizeConstraints.Flags.Unset(GUISizeConstraintFlag::ExpandingWidth);
	mSizeConstraints.MinimumWidth = mSizeConstraints.MaximumWidth = width;

	const bool isFixedAfter = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

	if(isFixedBefore != isFixedAfter)
		RefreshLayoutUpdateParentsForChildren();

	MarkLayoutAsDirty();
}

void GUIElement::SetFlexibleWidth(GUILogicalUnit minWidth, GUILogicalUnit maxWidth)
{
	if(maxWidth < minWidth)
		std::swap(minWidth, maxWidth);

	const bool isFixedBefore = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

	mSizeConstraints.Flags |= GUISizeConstraintFlag::WidthOverridenAtRuntime;
	mSizeConstraints.Flags.Unset(GUISizeConstraintFlag::FixedWidth);
	mSizeConstraints.Flags.Unset(GUISizeConstraintFlag::ExpandingWidth);
	mSizeConstraints.MinimumWidth = minWidth;
	mSizeConstraints.MaximumWidth = maxWidth;

	const bool isFixedAfter = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

	if(isFixedBefore != isFixedAfter)
		RefreshLayoutUpdateParentsForChildren();

	MarkLayoutAsDirty();
}

void GUIElement::SetHeight(GUILogicalUnit height)
{
	const bool isFixedBefore = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

	mSizeConstraints.Flags |= GUISizeConstraintFlag::FixedHeight | GUISizeConstraintFlag::HeightOverridenAtRuntime;
	mSizeConstraints.Flags.Unset(GUISizeConstraintFlag::ExpandingHeight);
	mSizeConstraints.MinimumHeight = mSizeConstraints.MaximumHeight = height;

	const bool isFixedAfter = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

	if(isFixedBefore != isFixedAfter)
		RefreshLayoutUpdateParentsForChildren();

	MarkLayoutAsDirty();
}

void GUIElement::SetFlexibleHeight(GUILogicalUnit minHeight, GUILogicalUnit maxHeight)
{
	if(maxHeight < minHeight)
		std::swap(minHeight, maxHeight);

	const bool isFixedBefore = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

	mSizeConstraints.Flags |= GUISizeConstraintFlag::HeightOverridenAtRuntime;
	mSizeConstraints.Flags.Unset(GUISizeConstraintFlag::FixedHeight);
	mSizeConstraints.Flags.Unset(GUISizeConstraintFlag::ExpandingHeight);
	mSizeConstraints.MinimumHeight = minHeight;
	mSizeConstraints.MaximumHeight = maxHeight;

	const bool isFixedAfter = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

	if(isFixedBefore != isFixedAfter)
		RefreshLayoutUpdateParentsForChildren();

	MarkLayoutAsDirty();
}

void GUIElement::ResetSizeConstraints()
{
	const bool isFixedBefore = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

	mSizeConstraints = GUISizeConstraints::Create();

	const bool isFixedAfter = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

	if(isFixedBefore != isFixedAfter)
		RefreshLayoutUpdateParentsForChildren();

	MarkLayoutAsDirty();
}

void GUIElement::SetScale(float scale)
{
	if(mScale == scale)
		return;

	// Don't allow zero scale
	mScale = Math::Max(scale, 0.01f);
	MarkAbsoluteCoordinatesAsDirty();
}

GUILogicalSize GUIElement::CalculateSizeInLayout() const
{
	if(mLayoutUpdateParent != nullptr && mLayoutUpdateParent->IsLayoutDirty() && mParentWidget != nullptr)
		mParentWidget->UpdateLayout(mLayoutUpdateParent);

	return GetLayoutCalculatedSize();
}

GUILogicalPoint GUIElement::CalculatePositionRelativeTo(GUIElement* relativeTo) const
{
	if(relativeTo == nullptr)
		relativeTo = mPanelParent;

	if(mLayoutUpdateParent != nullptr && mLayoutUpdateParent->IsLayoutDirty() && mParentWidget != nullptr)
		mParentWidget->UpdateLayout(mLayoutUpdateParent);

	if(relativeTo == nullptr)
		return mLayoutData.RelativePosition;

	auto fnGetAccumulatedRelativePosition = [relativeTo](const GUIElement* element, auto&& fnGetAccumulatedRelativePosition) -> GUILogicalPoint
	{
		GUIElement* const parent = element->GetParent();
		if(parent == nullptr || element == relativeTo)
			return GUILogicalPoint::kZero;

		const GUILogicalPoint& parentPosition = fnGetAccumulatedRelativePosition(parent, fnGetAccumulatedRelativePosition);
		return parentPosition + element->GetLayoutData().RelativePosition;
	};

	return fnGetAccumulatedRelativePosition(this, fnGetAccumulatedRelativePosition);
}

GUIPhysicalArea GUIElement::CalculateAbsoluteBoundsRelativeTo(GUIElement* relativeTo)
{
	if(relativeTo == nullptr)
		relativeTo = mPanelParent;

	GUIPhysicalArea anchorBounds;
	if(relativeTo != nullptr)
		anchorBounds = relativeTo->CalculateAbsoluteBounds();

	if(mLayoutUpdateParent != nullptr && mLayoutUpdateParent->IsLayoutDirty() && mParentWidget != nullptr)
		mParentWidget->UpdateLayout(mLayoutUpdateParent);

	GUIPhysicalArea bounds = GetAbsoluteBounds();
	bounds.X -= anchorBounds.X;
	bounds.Y -= anchorBounds.Y;

	return bounds;
}

GUIPhysicalArea GUIElement::GetClippedAreaTranslatedRelativeToParent() const
{
	GUIPhysicalArea localClippedArea = mAbsoluteClippedArea;
	localClippedArea.X -= mAbsolutePosition.X;
	localClippedArea.Y -= mAbsolutePosition.Y;

	return localClippedArea;
}

GUIPhysicalArea GUIElement::CalculateAbsoluteBounds() const
{
	if(mLayoutUpdateParent != nullptr && mLayoutUpdateParent->IsLayoutDirty() && mParentWidget != nullptr)
		mParentWidget->UpdateLayout(mLayoutUpdateParent);

	return GetAbsoluteBounds();
}

GUIPhysicalArea GUIElement::CalculateScreenBounds() const
{
	GUIPhysicalArea area = CalculateAbsoluteBounds();
	if(mParentWidget)
	{
		const Matrix4& widgetTfrm = mParentWidget->SceneObject()->GetTransform().GetMatrix();
		const GUIPhysicalPoint elementPosition = area.GetPosition();

		const Vector4 elementPosition4D = widgetTfrm.MultiplyAffine(Vector4((float)elementPosition.X, (float)elementPosition.Y, 0.0f, 1.0f));
		const GUIPhysicalPoint widgetRelativePosition(Math::RoundToI32(elementPosition4D.X), Math::RoundToI32(elementPosition4D.Y));

		const RenderWindow* parentWindow = GUIManager::Instance().GetWidgetWindow(*mParentWidget);
		if(parentWindow)
		{
			const GUIPhysicalPoint windowPos = parentWindow->WindowToScreenPosition(widgetRelativePosition.To<i32>()).To<GUIPhysicalUnit>();
			area.X = windowPos.X;
			area.Y = windowPos.Y;
		}
		else
		{
			area.X = widgetRelativePosition.X;
			area.Y = widgetRelativePosition.Y;
		}
	}

	return area;
}

GUILogicalPoint GUIElement::WidgetToElementSpace(const GUIPhysicalPoint& point) const
{
	const GUIPhysicalPoint physicalRelativePoint = point - GetAbsolutePosition().To<GUIPhysicalUnit>();
	return GUIUtility::PhysicalToLogical(physicalRelativePoint, GetAbsoluteScale());
}

GUIPhysicalPoint GUIElement::ElementToWidgetSpace(const GUILogicalPoint& point) const
{
	const GUIPhysicalPoint physicalRelativePoint = GUIUtility::LogicalToPhysical(point, GetAbsoluteScale());
	return GetAbsolutePosition() + physicalRelativePoint;
}

GUILogicalArea GUIElement::WidgetToElementSpace(const GUIPhysicalArea& area) const
{
	const GUIPhysicalPoint point = area.GetPosition();
	const GUIPhysicalSize size = area.GetSize();

	return GUILogicalArea(WidgetToElementSpace(point), GUIUtility::PhysicalToLogical(size, GetAbsoluteScale()));
}

GUIPhysicalArea GUIElement::ElementToWidgetSpace(const GUILogicalArea& area) const
{
	const GUILogicalPoint point = area.GetPosition();
	const GUILogicalSize size = area.GetSize();

	return GUIPhysicalArea(ElementToWidgetSpace(point), GUIUtility::LogicalToPhysical(size, GetAbsoluteScale()));
}

void GUIElement::MarkAsClean()
{
	mFlags.Unset(GUIElementInternalStateFlag::LayoutDirty);
	mFlags.Unset(GUIElementInternalStateFlag::AbsoluteCoordinatesDirty);
}

void GUIElement::MarkLayoutAsDirty()
{
	if(!IsHidden())
	{
		GUIElement* const layoutUpdateParent = mLayoutUpdateParent != nullptr ? mLayoutUpdateParent : GetParent();
		bool isLayoutDirtyOld = false;

		if(layoutUpdateParent != nullptr)
		{
			isLayoutDirtyOld = layoutUpdateParent->mFlags.IsSet(GUIElementInternalStateFlag::LayoutDirty);
			layoutUpdateParent->mFlags.Set(GUIElementInternalStateFlag::LayoutDirty);
		}

		if(mParentWidget != nullptr && !isLayoutDirtyOld)
			mParentWidget->MarkLayoutDirty(layoutUpdateParent);
	}
	else
	{
		// If hidden, set the layout dirty flag locally so we know to rebuild the layout once its unhidden
		mFlags.Set(GUIElementInternalStateFlag::LayoutDirty);
	}
}

void GUIElement::MarkAbsoluteCoordinatesAsDirty()
{
	if(IsHidden())
		return;

	const bool areAbsoluteCoordinatesDirtyOld = mFlags.IsSet(GUIElementInternalStateFlag::AbsoluteCoordinatesDirty);
	mFlags.Set(GUIElementInternalStateFlag::AbsoluteCoordinatesDirty);

	if(!IsCulled() && mParentWidget != nullptr && !areAbsoluteCoordinatesDirtyOld)
		mParentWidget->MarkAbsoluteCoordinatesDirty(this);
}

void GUIElement::MarkContentAsDirty()
{
	if(IsHidden())
		return;

	if(!IsCulled() && mParentWidget != nullptr)
		mParentWidget->MarkContentDirty(this);
}

void GUIElement::MarkMeshAsDirty()
{
	if(IsHiddenOrCulled())
		return;

	if(mParentWidget != nullptr)
		mParentWidget->MarkMeshDirty(this);
}

void GUIElement::SetHidden(bool hidden)
{
	// No visibility states matter if object is not active. And we re-apply visibility flags after object is made active.
	if(!IsActive())
		return;

	const bool isCurrentlyHiddenSelf = mFlags.IsSet(GUIElementInternalStateFlag::HiddenSelf);
	if(isCurrentlyHiddenSelf != hidden)
	{
		// If making an element visible make sure to mark layout as dirty, as we didn't track any dirty flags while the element was inactive
		if(hidden)
		{
			mFlags.Set(GUIElementInternalStateFlag::HiddenSelf);
			SetHiddenRecursive(true);
		}
		else
		{
			mFlags.Unset(GUIElementInternalStateFlag::HiddenSelf);

			if(mParent == nullptr || !mParent->IsHidden())
				SetHiddenRecursive(false);
		}
	}
}

void GUIElement::SetHiddenRecursive(bool hidden)
{
	const bool isCurrentlyHidden = mFlags.IsSet(GUIElementInternalStateFlag::Hidden);
	if(isCurrentlyHidden == hidden)
		return;

	if(hidden)
	{
		if(mParentWidget && !IsCulled())
			mParentWidget->NotifyElementVisibilityChanged(this, false);

		mFlags.Set(GUIElementInternalStateFlag::Hidden);

		for(auto& child : mChildren)
			child->SetHiddenRecursive(true);
	}
	else
	{
		const bool isCurrentlyHiddenSelf = mFlags.IsSet(GUIElementInternalStateFlag::HiddenSelf);
		if(!isCurrentlyHiddenSelf)
		{
			mFlags.Unset(GUIElementInternalStateFlag::Hidden);

			if(mParentWidget && !IsCulled())
				mParentWidget->NotifyElementVisibilityChanged(this, true);

			// It's possible the layout was dirtied while object was hidden, make sure to invalidate layout once its visible again
			if(mFlags.IsSet(GUIElementInternalStateFlag::LayoutDirty))
				MarkLayoutAsDirty();

			for(auto& child : mChildren)
				child->SetHiddenRecursive(false);
		}
	}
}

void GUIElement::SetActive(bool active)
{
	static const GUIElementInternalStateFlags kActiveFlags = GUIElementInternalStateFlag::InactiveSelf | GUIElementInternalStateFlag::HiddenSelf;

	bool activeSelf = !mFlags.IsSet(GUIElementInternalStateFlag::InactiveSelf);
	if(activeSelf != active)
	{
		if(!active)
		{
			mFlags |= kActiveFlags;

			// Make inactive before hidden, as layout update will be skipped if hidden
			SetActiveRecursive(false);
			SetHiddenRecursive(true);
		}
		else
		{
			mFlags &= ~kActiveFlags;

			if(mParent != nullptr)
			{
				if(mParent->IsActive())
				{
					if(!mParent->IsHidden())
						SetHiddenRecursive(false);

					SetActiveRecursive(true);
				}
			}
			else
			{
				SetHiddenRecursive(false);
				SetActiveRecursive(true);
			}
		}
	}
}

void GUIElement::SetActiveRecursive(bool active)
{
	bool isActive = !mFlags.IsSet(GUIElementInternalStateFlag::Inactive);
	if(isActive == active)
		return;

	if(!active)
	{
		MarkLayoutAsDirty();

		mFlags.Set(GUIElementInternalStateFlag::Inactive);

		for(auto& child : mChildren)
			child->SetActiveRecursive(false);
	}
	else
	{
		bool childActiveSelf = !mFlags.IsSet(GUIElementInternalStateFlag::InactiveSelf);
		if(childActiveSelf)
		{
			mFlags.Unset(GUIElementInternalStateFlag::Inactive);
			MarkLayoutAsDirty();

			for(auto& child : mChildren)
				child->SetActiveRecursive(true);
		}
	}
}

void GUIElement::SetDisabled(bool disabled)
{
	bool disabledSelf = mFlags.IsSet(GUIElementInternalStateFlag::DisabledSelf);
	if(disabledSelf != disabled)
	{
		if(!disabled)
			mFlags.Unset(GUIElementInternalStateFlag::DisabledSelf);
		else
			mFlags.Set(GUIElementInternalStateFlag::DisabledSelf);

		SetDisabledRecursive(disabled);
	}
}

void GUIElement::SetDisabledRecursive(bool disabled)
{
	bool isDisabled = mFlags.IsSet(GUIElementInternalStateFlag::Disabled);
	if(isDisabled == disabled)
		return;

	if(!disabled)
	{
		bool disabledSelf = mFlags.IsSet(GUIElementInternalStateFlag::DisabledSelf);
		if(!disabledSelf)
		{
			mFlags.Unset(GUIElementInternalStateFlag::Disabled);

			for(auto& child : mChildren)
				child->SetDisabledRecursive(false);
		}
	}
	else
	{
		mFlags.Set(GUIElementInternalStateFlag::Disabled);

		for(auto& child : mChildren)
			child->SetDisabledRecursive(true);
	}

	MarkContentAsDirty();
}

void GUIElement::SetCulled(bool culled)
{
	auto fnSetCulledRecursive = [](GUIElement* element, bool culled, auto&& fnSetCulledRecursive) -> void
	{
		const bool isCurrentlyCulled = element->mFlags.IsSet(GUIElementInternalStateFlag::Culled);
		if(isCurrentlyCulled == culled)
			return;

		if(culled)
		{
			if(element->mParentWidget && !element->IsHidden())
				element->mParentWidget->NotifyElementVisibilityChanged(element, false);

			element->mFlags.Set(GUIElementInternalStateFlag::Culled);

			for(auto& child : element->mChildren)
				fnSetCulledRecursive(child, true, fnSetCulledRecursive);
		}
		else
		{
			const bool isCurrentlyCulledSelf = element->mFlags.IsSet(GUIElementInternalStateFlag::CulledSelf);
			if(!isCurrentlyCulledSelf)
			{
				element->mFlags.Unset(GUIElementInternalStateFlag::Culled);

				if(element->mParentWidget && !element->IsHidden())
					element->mParentWidget->NotifyElementVisibilityChanged(element, true);

				for(auto& child : element->mChildren)
					fnSetCulledRecursive(child, false, fnSetCulledRecursive);
			}
		}
	};

	const bool isCurrentlyCulledSelf = mFlags.IsSet(GUIElementInternalStateFlag::CulledSelf);
	if(isCurrentlyCulledSelf != culled)
	{
		if(culled)
		{
			mFlags.Set(GUIElementInternalStateFlag::CulledSelf);
			fnSetCulledRecursive(this, true, fnSetCulledRecursive);
		}
		else
		{
			mFlags.Unset(GUIElementInternalStateFlag::CulledSelf);

			if(mParent == nullptr || !mParent->IsCulled())
				fnSetCulledRecursive(this, false, fnSetCulledRecursive);
		}
	}
}

void GUIElement::UpdateLayout()
{
	UpdateOptimalLayoutSizes(); // We calculate optimal sizes of all layouts as a pre-processing step, as they are requested often during update layout
	UpdateLayoutForChildren();
	UpdateAbsoluteCoordinatesForChildren();
}

void GUIElement::UpdateLayoutIfDirty()
{
	if(mLayoutUpdateParent != nullptr && mLayoutUpdateParent->IsLayoutDirty() && mParentWidget != nullptr)
		mParentWidget->UpdateLayout(mLayoutUpdateParent);
}

void GUIElement::UpdateOptimalLayoutSizes()
{
	for(auto& child : mChildren)
	{
		child->UpdateOptimalLayoutSizes();
	}
}

void GUIElement::UpdateAbsoluteCoordinates(const GUIPhysicalPointF& parentOrigin, float parentScale, const GUIPhysicalAreaF& parentVisibleArea)
{
	UpdateAbsoluteCoordinatesWithExplicitContentSize(parentOrigin, parentScale, parentVisibleArea, mLayoutData.Size);
}

void GUIElement::UpdateAbsoluteCoordinatesWithExplicitContentSize(const GUIPhysicalPointF& parentOrigin, float parentScale, const GUIPhysicalAreaF& parentVisibleArea, const GUILogicalSize& contentSize)
{
	mAbsoluteScale = mScale * parentScale;

	// Keep intermediates in floating point not to lose precision as we go deeper in the hierarchy. We also need to cache these as layout update
	// can occur from anywhere in the hierarchy.
	mIntermediateAbsolutePosition = (mLayoutData.RelativePosition.To<float>() * parentScale).To<GUIPhysicalUnitF>() + parentOrigin;
	const GUIPhysicalSizeF intermediateAbsoluteSize = (mLayoutData.Size.To<float>() * parentScale).To<GUIPhysicalUnitF>();

	mIntermediateAbsoluteClippedArea = GUIPhysicalAreaF(mIntermediateAbsolutePosition, (contentSize.To<float>() * parentScale).To<GUIPhysicalUnitF>());
	mIntermediateAbsoluteClippedArea.Clip(parentVisibleArea);

	mAbsolutePosition = mIntermediateAbsolutePosition.To<GUIPhysicalUnit>();
	mAbsoluteSize = intermediateAbsoluteSize.To<GUIPhysicalUnit>();
	mAbsoluteClippedArea = mIntermediateAbsoluteClippedArea.To<GUIPhysicalUnit>();

	UpdateAbsoluteCoordinatesForChildren();
}

void GUIElement::UpdateAbsoluteCoordinatesForChildren()
{
	for(auto& child : mChildren)
	{
		child->UpdateAbsoluteCoordinates(mIntermediateAbsolutePosition, mAbsoluteScale, mIntermediateAbsoluteClippedArea);
	}
}

GUIConstrainedSizeRange GUIElement::CalculateConstrainedSizeRange() const
{
	const GUISizeConstraints& sizeConstraints = GetSizeConstraints();
	return sizeConstraints.CalculateConstrainedSizeRange(CalculateUnconstrainedOptimalSize());
}

GUIConstrainedSizeRange GUIElement::GetConstrainedSizeRange() const
{
	return CalculateConstrainedSizeRange();
}

const RectOffset& GUIElement::GetMargins() const
{
	static RectOffset margins;
	return margins;
}

const RectOffset& GUIElement::GetPadding() const
{
	static RectOffset padding;
	return padding;
}

void GUIElement::SetParent(GUIElement* parent)
{
	if(mParent != parent)
	{
		mParent = parent;
		UpdatePanelAndLayoutUpdateParents();

		if(parent != nullptr)
		{
			if(GetParentWidget() != parent->GetParentWidget())
				ChangeParentWidget(parent->GetParentWidget());
		}
		else
			ChangeParentWidget(nullptr);
	}
}

void GUIElement::RegisterChildElement(GUIElement* element)
{
	B3D_ASSERT(!element->IsPendingDestroy());

	GUIElement* parentElement = element->GetParent();
	if(parentElement != nullptr)
	{
		parentElement->UnregisterChildElement(element);
	}

	element->SetParent(this);
	mChildren.Add(element);

	element->SetActiveRecursive(IsActive());
	element->SetHiddenRecursive(IsHidden());
	element->SetDisabledRecursive(IsDisabled());

	// No need to mark ourselves as dirty. If we're part of the element's update chain, this will do it for us.
	element->MarkLayoutAsDirty();
}

void GUIElement::UnregisterChildElement(GUIElement* element)
{
	bool foundElementToRemove = false;
	for(auto it = mChildren.begin(); it != mChildren.end(); ++it)
	{
		GUIElement* child = *it;

		if(child == element)
		{
			element->MarkLayoutAsDirty();

			mChildren.erase(it);
			element->SetParent(nullptr);
			foundElementToRemove = true;

			break;
		}
	}

	B3D_ENSURE(foundElementToRemove);
}

void GUIElement::Destroy()
{
	if(mIsPendingDestroy)
		return;

	if(mParent != nullptr)
		mParent->UnregisterChildElement(this);

	DestroyChildElements();

	mIsPendingDestroy = true;

	GUIManager::Instance().QueueForDestroy(this);
}

void GUIElement::DestroyChildElements()
{
	TInlineArray<GUIElement*, 4> childCopy = mChildren;
	for(auto& child : childCopy)
		child->Destroy();

	B3D_ASSERT(mChildren.Empty());
}

void GUIElement::ChangeParentWidget(GUIWidget* widget)
{
	B3D_ASSERT(!IsPendingDestroy());

	if(mParentWidget != widget)
	{
		if(mParentWidget != nullptr)
			mParentWidget->UnregisterElement(this);

		if(widget != nullptr)
			widget->RegisterElement(this);
	}

	mParentWidget = widget;

	for(auto& child : mChildren)
	{
		child->ChangeParentWidget(widget);
	}

	MarkLayoutAsDirty();
}

void GUIElement::UpdatePanelAndLayoutUpdateParents()
{
	GUIElement* layoutUpdateParent = nullptr;
	if(mParent != nullptr)
	{
		layoutUpdateParent = mParent->FindLayoutUpdateParent();

		// If parent is a panel then we can do an optimization and only update
		// one child instead of all of them, so change parent to that child.
		if(layoutUpdateParent != nullptr && B3DRTTIIsOfType<GUIPanel>(layoutUpdateParent))
		{
			GUIElement* optimizedUpdateParent = this;
			while(optimizedUpdateParent->GetParent() != layoutUpdateParent)
				optimizedUpdateParent = optimizedUpdateParent->GetParent();

			layoutUpdateParent = optimizedUpdateParent;
		}
	}

	GUIPanel* panelParent = nullptr;
	GUIElement* currentParent = mParent;
	while(currentParent != nullptr)
	{
		if(B3DRTTIIsOfType<GUIPanel>(currentParent))
		{
			panelParent = static_cast<GUIPanel*>(currentParent);
			break;
		}

		currentParent = currentParent->mParent;
	}

	SetPanelParent(panelParent);
	SetLayoutUpdateParent(layoutUpdateParent);
}

GUIElement* GUIElement::FindLayoutUpdateParent()
{
	GUIElement* currentElement = this;
	while(currentElement != nullptr)
	{
		const GUISizeConstraints& parentDimensions = currentElement->GetSizeConstraints();
		bool boundsDependOnChildren = !parentDimensions.IsHeightFixed() || !parentDimensions.IsWidthFixed();

		if(!boundsDependOnChildren)
			return currentElement;

		currentElement = currentElement->mParent;
	}

	return nullptr;
}

void GUIElement::RefreshLayoutUpdateParentsForChildren()
{
	GUIElement* updateParent = FindLayoutUpdateParent();

	for(auto& child : mChildren)
	{
		GUIElement* childUpdateParent = updateParent;

		// If parent is a panel then we can do an optimization and only update
		// one child instead of all of them, so change parent to that child.
		if(childUpdateParent != nullptr && B3DRTTIIsOfType<GUIPanel>(childUpdateParent))
		{
			GUIElement* optimizedUpdateParent = child;
			while(optimizedUpdateParent->GetParent() != childUpdateParent)
				optimizedUpdateParent = optimizedUpdateParent->GetParent();

			childUpdateParent = optimizedUpdateParent;
		}

		child->SetLayoutUpdateParent(childUpdateParent);
	}
}

void GUIElement::SetPanelParent(GUIPanel* panelParent)
{
	mPanelParent = panelParent;

	if(B3DRTTIIsOfType<GUIPanel>(this))
		return;

	for(auto& child : mChildren)
		child->SetPanelParent(panelParent);
}

void GUIElement::SetLayoutUpdateParent(GUIElement* layoutUpdateParent)
{
	mLayoutUpdateParent = layoutUpdateParent;

	const GUISizeConstraints& dimensions = GetSizeConstraints();
	bool boundsDependOnChildren = !dimensions.IsHeightFixed() || !dimensions.IsWidthFixed();

	if(!boundsDependOnChildren)
		return;

	for(auto& child : mChildren)
		child->SetLayoutUpdateParent(layoutUpdateParent);
}

RTTIType* GUIElement::GetRttiStatic()
{
	return GUIElementRTTI::Instance();
}

RTTIType* GUIElement::GetRtti() const
{
	return GUIElement::GetRttiStatic();
}
