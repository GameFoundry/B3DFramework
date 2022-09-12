//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIMenu.h"
#include "GUI/BsGUIDropDownMenu.h"

namespace bs
{
	bool GUIMenuItemComparer::Operator() (const GUIMenuItem* const& a, const GUIMenuItem* const& b) const
	{
		return a->mPriority > b->mPriority || (a->mPriority == b->mPriority && a->mSeqIdx < b->mSeqIdx);
	}

	GUIMenuItem::GUIMenuItem(GUIMenuItem* parent, const String& name, std::function<void()> callback,
		INT32 priority, UINT32 seqIdx, const ShortcutKey& key)
		:mParent(parent), mIsSeparator(false), mName(name), mCallback(callback), mPriority(priority),
		mShortcut(key), mSeqIdx(seqIdx)
	{

	}

	GUIMenuItem::GUIMenuItem(GUIMenuItem* parent, INT32 priority, UINT32 seqIdx)
		: MParent(parent), mIsSeparator(true), mCallback(nullptr), mPriority(priority), mSeqIdx(seqIdx)
	{

	}

	GUIMenuItem::~GUIMenuItem()
	{
		for(auto& child : mChildren)
			bs_delete(child);
	}

	const GUIMenuItem* GUIMenuItem::findChild(const String& name) const
	{
		auto iterFind = std::find_if(begin(mChildren), end(mChildren), [&] (GUIMenuItem* x) { return x->GetName() == name; });

		if(iterFind != mChildren.End())
			return *iterFind;

		return nullptr;
	}

	GUIMenuItem* GUIMenuItem::findChild(const String& name)
	{
		auto iterFind = std::find_if(begin(mChildren), end(mChildren), [&] (GUIMenuItem* x) { return x->GetName() == name; });

		if(iterFind != mChildren.End())
			return *iterFind;

		return nullptr;
	}

	void GUIMenuItem::RemoveChild(const String& name)
	{
		auto iterFind = std::find_if(begin(mChildren), end(mChildren), [&] (GUIMenuItem* x) { return x->GetName() == name; });

		if(iterFind != mChildren.End())
		{
			bs_delete(*iterFind);
			mChildren.Erase(iterFind);
		}
	}

	void GUIMenuItem::RemoveChild(const GUIMenuItem* item)
	{
		auto iterFind = std::find(begin(mChildren), end(mChildren), item);

		if (iterFind != mChildren.End())
		{
			bs_delete(*iterFind);
			mChildren.Erase(iterFind);
		}
	}

	GUIMenu::GUIMenu()
		:mRootElement(nullptr, "", nullptr, 0, 0, ShortcutKey::NONE), mNextIdx(0)
	{

	}

	GUIMenu::~GUIMenu()
	{

	}

	GUIMenuItem* GUIMenu::addMenuItem(const String& path, std::function<void()> callback, INT32 priority, const ShortcutKey& key)
	{
		return AddMenuItemInternal(path, callback, false, priority, key);
	}

	GUIMenuItem* GUIMenu::addSeparator(const String& path, INT32 priority)
	{
		return AddMenuItemInternal(path, nullptr, true, priority, ShortcutKey::NONE);
	}

	GUIMenuItem* GUIMenu::addMenuItemInternal(const String& path, std::function<void()> callback, bool isSeparator,
		INT32 priority, const ShortcutKey& key)
	{
		Vector<String> pathElements = StringUtil::split(path, "/");

		GUIMenuItem* curSubMenu = &mRootElement;
		for(UINT32 i = 0; i < (UINT32)pathElements.Size(); i++)
		{
			if(pathElements[i] == "")
				continue;

			const String& pathElem = *(pathElements.Begin() + i);
			GUIMenuItem* existingItem = curSubMenu->FindChild(pathElem);

			if(existingItem == nullptr)
			{
				bool isLastElem = i == (UINT32)(pathElements.Size() - 1);

				if(isLastElem)
					existingItem = bs_new<GUIMenuItem>(curSubMenu, pathElem, callback, priority, mNextIdx++, key);
				else
				{
					existingItem = bs_alloc<GUIMenuItem>();
					existingItem = new (existingItem) GUIMenuItem(curSubMenu, pathElem, nullptr, priority, mNextIdx++, ShortcutKey::NONE);
				}

				curSubMenu->AddChild(existingItem);
			}

			curSubMenu = existingItem;
		}

		if(isSeparator)
		{
			GUIMenuItem* separatorItem = bs_new<GUIMenuItem>(curSubMenu, priority, mNextIdx++);
			curSubMenu->AddChild(separatorItem);

			return separatorItem;
		}

		return curSubMenu;
	}

	GUIMenuItem* GUIMenu::getMenuItem(const String& path)
	{
		Vector<String> pathElements = StringUtil::split(path, "/");

		GUIMenuItem* curSubMenu = &mRootElement;
		for(UINT32 i = 0; i < (UINT32)pathElements.Size(); i++)
		{
			const String& pathElem = *(pathElements.Begin() + i);
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
		assert(parent != nullptr);

		parent->RemoveChild(item->getName());
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
				dropDownData.entries.push_back(GUIDropDownDataEntry::separator());
			}
			else
			{
				if(menuItem->GetNumChildren() == 0)
				{
					dropDownData.entries.push_back(GUIDropDownDataEntry::button(menuItem->GetName(),
						menuItem->GetCallback(), menuItem->getShortcut().GetName()));
				}
				else
				{
					dropDownData.entries.push_back(GUIDropDownDataEntry::subMenu(menuItem->GetName(),
						getDropDownDataInternal(*menuItem)));
				}
			}
		}

		dropDownData.localizedNames = mLocalizedEntryNames;

		return dropDownData;
	}
}
