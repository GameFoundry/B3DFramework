//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIMenu.h"
#include "GUI/BsGUIDropDownMenu.h"

namespace bs
{
	bool GUIMenuItemComparer::operator() (const GUIMenuItem* const& a, const GUIMenuItem* const& b)
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
		: mParent(parent), mIsSeparator(true), mCallback(nullptr), mPriority(priority), mSeqIdx(seqIdx)
	{

	}

	GUIMenuItem::~GUIMenuItem()
	{
		for(auto& child : mChildren)
			bs_delete(child);
	}

	const GUIMenuItem* GUIMenuItem::findChild(const String& name) const
	{
		auto iterFind = std::find_if(begin(mChildren), end(mChildren), [&] (GUIMenuItem* x) { return x->getName() == name; });

		if(iterFind != mChildren.end())
			return *iterFind;

		return nullptr;
	}

	GUIMenuItem* GUIMenuItem::findChild(const String& name)
	{
		auto iterFind = std::find_if(begin(mChildren), end(mChildren), [&] (GUIMenuItem* x) { return x->getName() == name; });

		if(iterFind != mChildren.end())
			return *iterFind;

		return nullptr;
	}

	void GUIMenuItem::removeChild(const String& name)
	{
		auto iterFind = std::find_if(begin(mChildren), end(mChildren), [&] (GUIMenuItem* x) { return x->getName() == name; });

		if(iterFind != mChildren.end())
		{
			bs_delete(*iterFind);
			mChildren.erase(iterFind);
		}
	}

	void GUIMenuItem::removeChild(const GUIMenuItem* item)
	{
		auto iterFind = std::find(begin(mChildren), end(mChildren), item);

		if (iterFind != mChildren.end())
		{
			bs_delete(*iterFind);
			mChildren.erase(iterFind);
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
		return addMenuItemInternal(path, callback, false, priority, key);
	}

	GUIMenuItem* GUIMenu::addSeparator(const String& path, INT32 priority)
	{
		return addMenuItemInternal(path, nullptr, true, priority, ShortcutKey::NONE);
	}

	GUIMenuItem* GUIMenu::addMenuItemInternal(const String& path, std::function<void()> callback, bool isSeparator, 
		INT32 priority, const ShortcutKey& key)
	{
		Vector<String> pathElements = StringUtil::split(path, "/");

		GUIMenuItem* curSubMenu = &mRootElement;
		for(UINT32 i = 0; i < (UINT32)pathElements.size(); i++)
		{
			if(pathElements[i] == "")
				continue;

			const String& pathElem = *(pathElements.begin() + i);
			GUIMenuItem* existingItem = curSubMenu->findChild(pathElem);

			if(existingItem == nullptr)
			{
				bool isLastElem = i == (UINT32)(pathElements.size() - 1);

				if(isLastElem)
					existingItem = bs_new<GUIMenuItem>(curSubMenu, pathElem, callback, priority, mNextIdx++, key);
				else
				{
					existingItem = bs_alloc<GUIMenuItem>();
					existingItem = new (existingItem) GUIMenuItem(curSubMenu, pathElem, nullptr, priority, mNextIdx++, ShortcutKey::NONE);
				}

				curSubMenu->addChild(existingItem);
			}

			curSubMenu = existingItem;
		}

		if(isSeparator)
		{
			GUIMenuItem* separatorItem = bs_new<GUIMenuItem>(curSubMenu, priority, mNextIdx++);
			curSubMenu->addChild(separatorItem);

			return separatorItem;
		}

		return curSubMenu;
	}

	GUIMenuItem* GUIMenu::getMenuItem(const String& path)
	{
		Vector<String> pathElements = StringUtil::split(path, "/");

		GUIMenuItem* curSubMenu = &mRootElement;
		for(UINT32 i = 0; i < (UINT32)pathElements.size(); i++)
		{
			const String& pathElem = *(pathElements.begin() + i);
			GUIMenuItem* existingItem = curSubMenu->findChild(pathElem);

			if(existingItem == nullptr || existingItem->isSeparator())
				return nullptr;

			curSubMenu = existingItem;
		}

		return curSubMenu;
	}

	void GUIMenu::removeMenuItem(const GUIMenuItem* item)
	{
		GUIMenuItem* parent = item->mParent;
		assert(parent != nullptr);

		parent->removeChild(item->getName());
	}

	GUIDropDownData GUIMenu::getDropDownData() const
	{
		return getDropDownDataInternal(mRootElement);
	}

	void GUIMenu::setLocalizedName(const String& menuItemLabel, const HString& localizedName)
	{
		mLocalizedEntryNames[menuItemLabel] = localizedName;
	}

	GUIDropDownData GUIMenu::getDropDownDataInternal(const GUIMenuItem& menu) const
	{
		GUIDropDownData dropDownData;

		for(auto& menuItem : menu.mChildren)
		{
			if(menuItem->isSeparator())
			{
				dropDownData.entries.push_back(GUIDropDownDataEntry::separator());
			}
			else
			{
				if(menuItem->getNumChildren() == 0)
				{
					dropDownData.entries.push_back(GUIDropDownDataEntry::button(menuItem->getName(), 
						menuItem->getCallback(), menuItem->getShortcut().getName()));
				}
				else
				{
					dropDownData.entries.push_back(GUIDropDownDataEntry::subMenu(menuItem->getName(), 
						getDropDownDataInternal(*menuItem)));
				}
			}
		}

		dropDownData.localizedNames = mLocalizedEntryNames;

		return dropDownData;
	}
}