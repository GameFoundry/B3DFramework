//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIMenu.h"
#include "GUI/BsGUIDropDownMenu.h"

using namespace b3d;

bool GUIMenuItemComparer::operator()(const GUIMenuItem* const& a, const GUIMenuItem* const& b) const
{
	return a->mPriority > b->mPriority || (a->mPriority == b->mPriority && a->mSeqIdx < b->mSeqIdx);
}

GUIMenuItem::GUIMenuItem(GUIMenuItem* parent, const String& name, std::function<void()> callback, i32 priority, u32 seqIdx, const ShortcutKey& key)
	: mParent(parent), mIsSeparator(false), mName(name), mCallback(callback), mPriority(priority), mShortcut(key), mSeqIdx(seqIdx)
{
}

GUIMenuItem::GUIMenuItem(GUIMenuItem* parent, i32 priority, u32 seqIdx)
	: mParent(parent), mIsSeparator(true), mCallback(nullptr), mPriority(priority), mSeqIdx(seqIdx)
{
}

GUIMenuItem::~GUIMenuItem()
{
	for(auto& child : mChildren)
		B3DDelete(child);
}

const GUIMenuItem* GUIMenuItem::FindChild(const String& name) const
{
	auto iterFind = std::find_if(begin(mChildren), end(mChildren), [&](GUIMenuItem* x)
								 { return x->GetName() == name; });

	if(iterFind != mChildren.end())
		return *iterFind;

	return nullptr;
}

GUIMenuItem* GUIMenuItem::FindChild(const String& name)
{
	auto iterFind = std::find_if(begin(mChildren), end(mChildren), [&](GUIMenuItem* x)
								 { return x->GetName() == name; });

	if(iterFind != mChildren.end())
		return *iterFind;

	return nullptr;
}

void GUIMenuItem::RemoveChild(const String& name)
{
	auto iterFind = std::find_if(begin(mChildren), end(mChildren), [&](GUIMenuItem* x)
								 { return x->GetName() == name; });

	if(iterFind != mChildren.end())
	{
		B3DDelete(*iterFind);
		mChildren.erase(iterFind);
	}
}

void GUIMenuItem::RemoveChild(const GUIMenuItem* item)
{
	auto iterFind = std::find(begin(mChildren), end(mChildren), item);

	if(iterFind != mChildren.end())
	{
		B3DDelete(*iterFind);
		mChildren.erase(iterFind);
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
	for(u32 i = 0; i < (u32)pathElements.size(); i++)
	{
		if(pathElements[i] == "")
			continue;

		const String& pathElem = *(pathElements.begin() + i);
		GUIMenuItem* existingItem = curSubMenu->FindChild(pathElem);

		if(existingItem == nullptr)
		{
			bool isLastElem = i == (u32)(pathElements.size() - 1);

			if(isLastElem)
				existingItem = B3DNew<GUIMenuItem>(curSubMenu, pathElem, callback, priority, mNextIdx++, key);
			else
			{
				existingItem = B3DAllocate<GUIMenuItem>();
				existingItem = new(existingItem) GUIMenuItem(curSubMenu, pathElem, nullptr, priority, mNextIdx++, ShortcutKey::kNone);
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
	for(u32 i = 0; i < (u32)pathElements.size(); i++)
	{
		const String& pathElem = *(pathElements.begin() + i);
		GUIMenuItem* existingItem = curSubMenu->FindChild(pathElem);

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
