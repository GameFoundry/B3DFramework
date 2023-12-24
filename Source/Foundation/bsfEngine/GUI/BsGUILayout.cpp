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
	class B3D_EXPORT GUILayoutRTTI : public RTTIType<GUILayout, GUIElementBase, GUILayoutRTTI>
	{
	public:
		const String& GetRttiName()
		{
			static String name = "GUILayout";
			return name;
		}

		u32 GetRttiId() { return TID_GUILayout; }

		SPtr<IReflectable> NewRttiObject() { return nullptr; }
	};
} // namespace bs

/** @} */
/** @endcond */

GUILayout::GUILayout(const GUISizeConstraints& dimensions)
	: GUIElementBase(dimensions)
{}

void GUILayout::AddElement(GUIElementBase* element)
{
	if(!element->IsPendingDestroy())
		RegisterChildElement(element);
}

void GUILayout::RemoveElement(GUIElementBase* element)
{
	UnregisterChildElement(element);
}

void GUILayout::InsertElement(u32 idx, GUIElementBase* element)
{
	if(idx > (u32)mChildren.size())
		B3D_EXCEPT(InvalidParametersException, "Index out of range: " + ToString(idx) + ". Valid range: 0 .. " + ToString((u32)mChildren.size()));

	if(element->IsPendingDestroy())
		return;

	GUIElementBase* parentElement = element->GetParent();
	if(parentElement != nullptr)
	{
		parentElement->UnregisterChildElement(element);
	}

	element->SetParent(this);
	mChildren.Insert(mChildren.begin() + idx, element);

	element->SetActiveRecursive(IsActive());
	element->SetVisibleRecursive(IsVisible());
	element->SetDisabledRecursive(IsDisabled());

	MarkLayoutAsDirty();
}

void GUILayout::Clear()
{
	DestroyChildElements();
}

void GUILayout::RemoveElementAt(u32 idx)
{
	if(idx >= (u32)mChildren.size())
		B3D_EXCEPT(InvalidParametersException, "Index out of range: " + ToString(idx) + ". Valid range: 0 .. " + ToString((u32)mChildren.size()));

	GUIElementBase* child = mChildren[idx];
	mChildren.erase(mChildren.begin() + idx);

	child->SetParent(nullptr);

	MarkLayoutAsDirty();
}

RTTITypeBase* GUILayout::GetRttiStatic()
{
	return GUILayoutRTTI::Instance();
}

RTTITypeBase* GUILayout::GetRtti() const
{
	return GetRttiStatic();
}
