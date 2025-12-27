//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIMenu.h"
#include "GUI/B3DGUIDropDownMenu.h"

using namespace b3d;

bool GUIMenuItemComparer::operator()(const GUIMenuItem* const& a, const GUIMenuItem* const& b) const
{
	return a->mPriority > b->mPriority || (a->mPriority == b->mPriority && a->mSeqIdx < b->mSeqIdx);
}

GUIMenuItem::GUIMenuItem(GUIMenuItem* parent, const String& name, std::function<void()> callback, i32 priority, u32 sequentialIndex, const ShortcutKey& key)
	: mParent(parent), mIsSeparator(false), mName(name), mCallback(callback), mPriority(priority), mShortcut(key), mSeqIdx(sequentialIndex)
{
}

GUIMenuItem::GUIMenuItem(GUIMenuItem* parent, i32 priority, u32 sequentialIndex)
	: mParent(parent), mIsSeparator(true), mCallback(nullptr), mPriority(priority), mSeqIdx(sequentialIndex)
{
}

GUIMenuItem::~GUIMenuItem()
{
	for(auto& child : mChildren)
		B3DDelete(child);
}

const GUIMenuItem* GUIMenuItem::FindChild(const String& name) const
{
	auto found = std::find_if(begin(mChildren), end(mChildren), [&](GUIMenuItem* x)
								 { return x->GetName() == name; });

	if(found != mChildren.end())
		return *found;

	return nullptr;
}

GUIMenuItem* GUIMenuItem::FindChild(const String& name)
{
	auto found = std::find_if(begin(mChildren), end(mChildren), [&](GUIMenuItem* x)
								 { return x->GetName() == name; });

	if(found != mChildren.end())
		return *found;

	return nullptr;
}

void GUIMenuItem::RemoveChild(const String& name)
{
	auto found = std::find_if(begin(mChildren), end(mChildren), [&](GUIMenuItem* x)
								 { return x->GetName() == name; });

	if(found != mChildren.end())
	{
		B3DDelete(*found);
		mChildren.erase(found);
	}
}

void GUIMenuItem::RemoveChild(const GUIMenuItem* item)
{
	auto found = std::find(begin(mChildren), end(mChildren), item);

	if(found != mChildren.end())
	{
		B3DDelete(*found);
		mChildren.erase(found);
	}
}

GUIMenu::GUIMenu()
	: mRootElement(nullptr, "", nullptr, 0, 0, ShortcutKey::kNone), mNextIdx(0)
{
}

GUIMenu::~GUIMenu()
{
}

GUIMenuItem* GUIMenu::AddMenuItem(const String& path, std::function<void()> callback, i32 priority, const ShortcutKey& key)
{
	return AddMenuItemInternal(path, callback, false, priority, key);
}

GUIMenuItem* GUIMenu::AddSeparator(const String& path, i32 priority)
{
	return AddMenuItemInternal(path, nullptr, true, priority, ShortcutKey::kNone);
}

GUIMenuItem* GUIMenu::AddMenuItemInternal(const String& path, std::function<void()> callback, bool isSeparator, i32 priority, const ShortcutKey& key)
{
	Vector<String> pathElements = StringUtil::Split(path, "/");

	GUIMenuItem* curSubMenu = &mRootElement;
	for(u32 elementIndex = 0; elementIndex < (u32)pathElements.size(); elementIndex++)
	{
		if(pathElements[elementIndex] == "")
			continue;

		const String& pathElement = *(pathElements.begin() + elementIndex);
		GUIMenuItem* existingItem = curSubMenu->FindChild(pathElement);

		if(existingItem == nullptr)
		{
			bool isLastElement = elementIndex == (u32)(pathElements.size() - 1);

			if(isLastElement)
				existingItem = B3DNew<GUIMenuItem>(curSubMenu, pathElement, callback, priority, mNextIdx++, key);
			else
			{
				existingItem = B3DAllocate<GUIMenuItem>();
				existingItem = new(existingItem) GUIMenuItem(curSubMenu, pathElement, nullptr, priority, mNextIdx++, ShortcutKey::kNone);
			}

			curSubMenu->AddChild(existingItem);
		}

		curSubMenu = existingItem;
	}

	if(isSeparator)
	{
		GUIMenuItem* separatorItem = B3DNew<GUIMenuItem>(curSubMenu, priority, mNextIdx++);
		curSubMenu->AddChild(separatorItem);

		return separatorItem;
	}

	return curSubMenu;
}

GUIMenuItem* GUIMenu::GetMenuItem(const String& path)
{
	Vector<String> pathElements = StringUtil::Split(path, "/");

	GUIMenuItem* curSubMenu = &mRootElement;
	for(u32 elementIndex = 0; elementIndex < (u32)pathElements.size(); elementIndex++)
	{
		const String& pathElement = *(pathElements.begin() + elementIndex);
		GUIMenuItem* existingItem = curSubMenu->FindChild(pathElement);

		if(existingItem == nullptr || existingItem->IsSeparator())
			return nullptr;

		curSubMenu = existingItem;
	}

	return curSubMenu;
}

void GUIMenu::RemoveMenuItem(const GUIMenuItem* item)
{
	GUIMenuItem* parent = item->mParent;
	B3D_ASSERT(parent != nullptr);

	parent->RemoveChild(item->GetName());
}

GUIDropDownData GUIMenu::GetDropDownData() const
{
	return GetDropDownDataInternal(mRootElement);
}

void GUIMenu::SetLocalizedName(const String& menuItemLabel, const HString& localizedName)
{
	mLocalizedEntryNames[menuItemLabel] = localizedName;
}

GUIDropDownData GUIMenu::GetDropDownDataInternal(const GUIMenuItem& menu) const
{
	GUIDropDownData dropDownData;

	for(auto& menuItem : menu.mChildren)
	{
		if(menuItem->IsSeparator())
		{
			dropDownData.Entries.push_back(GUIDropDownDataEntry::Separator());
		}
		else
		{
			if(menuItem->GetNumChildren() == 0)
			{
				dropDownData.Entries.push_back(GUIDropDownDataEntry::Button(menuItem->GetName(), menuItem->GetCallback(), menuItem->GetShortcut().GetName()));
			}
			else
			{
				dropDownData.Entries.push_back(GUIDropDownDataEntry::SubMenu(menuItem->GetName(), GetDropDownDataInternal(*menuItem)));
			}
		}
	}

	dropDownData.LocalizedNames = mLocalizedEntryNames;

	return dropDownData;
}
