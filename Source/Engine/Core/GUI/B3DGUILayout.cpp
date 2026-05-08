//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUILayout.h"
#include "GUI/B3DGUIInteractable.h"
#include "Reflection/B3DRTTIType.h"

using namespace b3d;

/** @cond RTTI */
/** @addtogroup RTTI-Impl-Engine
 *  @{
 */

namespace b3d
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

		TShared<IReflectable> NewRttiObject() { return nullptr; }
	};
} // namespace b3d

/** @} */
/** @endcond */

GUILayout::GUILayout(const String& styleClass, const GUISizeConstraints& dimensions)
	: GUIElement(dimensions)
{}

void GUILayout::AddElement(GUIElement* element)
{
	if(element->IsPendingDestroy())
		return;

	RegisterChildElement(element);
}

void GUILayout::InsertElement(u32 index, GUIElement* element)
{
	if(!B3D_ENSURE_LOG(index <= (u32)mChildren.size(), "Index out of range: {0}. Valid range: 0 .. {1}", index, (u32)mChildren.size()))
		return;

	if(element->IsPendingDestroy())
		return;

	GUIElement* parentElement = element->GetParent();
	if(parentElement != nullptr)
	{
		parentElement->UnregisterChildElement(element);
	}

	element->SetParent(this);
	mChildren.Insert(mChildren.begin() + index, element);

	element->SetActiveRecursive(IsActive());
	element->SetHiddenRecursive(IsHidden());
	element->SetDisabledRecursive(IsDisabled());

	if(mCulling != nullptr)
		mCulling->RegisterElement(element);

	MarkLayoutAsDirty();
}

void GUILayout::RemoveElement(GUIElement* element)
{
	UnregisterChildElement(element);
}

void GUILayout::RemoveElementAt(u32 index)
{
	if(!B3D_ENSURE_LOG(index < (u32)mChildren.size(), "Index out of range: {0}. Valid range: 0 .. {1}", index, (u32)mChildren.size()))
		return;

	GUIElement* child = mChildren[index];
	mChildren.erase(mChildren.begin() + index);

	child->SetParent(nullptr);

	if(mCulling != nullptr)
		mCulling->UnregisterElement(child);

	MarkLayoutAsDirty();
}

void GUILayout::Clear()
{
	if(mCulling != nullptr)
	{
		mCulling->ClearElements();
		mCulling = B3DMakeUnique<GUICulling>(mChildren);
	}

	DestroyChildElements();
}

void GUILayout::SetEnableCulling(bool enable)
{
	const bool isCullingEnabled = mCulling != nullptr;
	if(isCullingEnabled == enable)
		return;
	
	if(enable)
		mCulling = B3DMakeUnique<GUICulling>(mChildren);
	else
	{
		mCulling->ClearElements();
		mCulling = nullptr;
	}
}

void GUILayout::RegisterChildElement(GUIElement* element)
{
	Super::RegisterChildElement(element);

	if(mCulling != nullptr)
		mCulling->RegisterElement(element);
}

void GUILayout::UnregisterChildElement(GUIElement* element)
{
	if(mCulling != nullptr)
		mCulling->UnregisterElement(element);

	Super::UnregisterChildElement(element);
}

void GUILayout::UpdateAbsoluteCoordinates(const GUIPhysicalPointF& parentOrigin, float parentScale, const GUIPhysicalAreaF& parentVisibleArea)
{
	// Account for the fact that the content size may be larger than the calculated layout size (e.g. a scroll area with more elements than it can display)
	const GUILogicalSize optimalContentSize = CalculateConstrainedOptimalSize();
	const GUILogicalSize contentSize(Math::Max(optimalContentSize.Width, mLayoutData.Size.Width), Math::Max(optimalContentSize.Height, mLayoutData.Size.Height));

	UpdateAbsoluteCoordinatesWithExplicitContentSize(parentOrigin, parentScale, parentVisibleArea, contentSize);
}

void GUILayout::UpdateAbsoluteCoordinatesForChildren()
{
	if(mCulling != nullptr)
	{
		const Area2 relativeClippedArea = WidgetToElementSpace(mAbsoluteClippedArea).To<float>();
		mCulling->UpdateVisibleElements(relativeClippedArea);

		for(auto& visibleElement : mCulling->GetVisibleElements())
		{
			visibleElement->UpdateAbsoluteCoordinates(mIntermediateAbsolutePosition, mAbsoluteScale, mIntermediateAbsoluteClippedArea);
			visibleElement->UpdateAbsoluteCoordinatesForChildren();
		}
	}
	else
	{
		for(auto& childElement : mChildren)
		{
			childElement->UpdateAbsoluteCoordinates(mIntermediateAbsolutePosition, mAbsoluteScale, mIntermediateAbsoluteClippedArea);
			childElement->UpdateAbsoluteCoordinatesForChildren();
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
