//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUITreeView.h"
#include "BsGUILayout.h"
#include "BsGUITexture.h"
#include "BsGUIButton.h"
#include "BsGUILabel.h"
#include "BsGUISpace.h"
#include "BsCGUIWidget.h"
#include "BsGUIToggle.h"
#include "BsGUITreeViewEditBox.h"
#include "BsGUIMouseEvent.h"
#include "BsGUISkin.h"
#include "BsGUICommandEvent.h"
#include "BsGUIVirtualButtonEvent.h"
#include "BsGUIScrollArea.h"
#include "BsDragAndDropManager.h"
#include "BsTime.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	const UINT32 GUITreeView::ELEMENT_EXTRA_SPACING = 3;
	const UINT32 GUITreeView::INDENT_SIZE = 10;
	const UINT32 GUITreeView::INITIAL_INDENT_OFFSET = 16;
	const UINT32 GUITreeView::DRAG_MIN_DISTANCE = 3;
	const float GUITreeView::AUTO_EXPAND_DELAY_SEC = 0.5f;
	const float GUITreeView::SCROLL_AREA_HEIGHT_PCT = 0.1f;
	const UINT32 GUITreeView::SCROLL_SPEED_PX_PER_SEC = 100;
	const Color GUITreeView::CUT_COLOR = Color(1.0f, 1.0f, 1.0f, 0.3f);
	const Color GUITreeView::DISABLED_COLOR = Color(1.0f, 1.0f, 1.0f, 0.6f);

	GUITreeView::TreeElement::TreeElement()
		: mParent(nullptr), mFoldoutBtn(nullptr), mElement(nullptr), mSortedIdx(0), mIsExpanded(false), mIsSelected(false)
		, mIsHighlighted(false), mIsVisible(true), mIsCut(false), mIsDisabled(false)
	{ }

	GUITreeView::TreeElement::~TreeElement()
	{
		for(auto& child : mChildren)
			bs_delete(child);

		if(mFoldoutBtn != nullptr)
			GUIElement::destroy(mFoldoutBtn);

		if(mElement != nullptr)
			GUIElement::destroy(mElement);

		mChildren.clear();
	}

	bool GUITreeView::TreeElement::isParentRec(TreeElement* element) const
	{
		TreeElement* curParent = mParent;
		while(curParent != nullptr)
		{
			if(curParent == element)
				return true;

			curParent = curParent->mParent;
		}

		return false;
	}

	GUITreeView::TreeElement* GUITreeView::InteractableElement::getTreeElement() const
	{
		if(!isTreeElement())
			return nullptr;

		UINT32 sortedIdx = (index - 1) / 2;

		auto findIter = std::find_if(parent->mChildren.begin(), parent->mChildren.end(),
			[&](const TreeElement* x) { return x->mSortedIdx == sortedIdx; });

		if(findIter != parent->mChildren.end())
			return *findIter;

		return nullptr;
	}

	GUITreeView::GUITreeView(const String& backgroundStyle, const String& elementBtnStyle, 
		const String& foldoutBtnStyle, const String& selectionBackgroundStyle, const String& highlightBackgroundStyle, 
		const String& editBoxStyle, const String& dragHighlightStyle, const String& dragSepHighlightStyle, const GUIDimensions& dimensions)
		: GUIElementContainer(dimensions), mBackgroundStyle(backgroundStyle), mElementBtnStyle(elementBtnStyle)
		, mFoldoutBtnStyle(foldoutBtnStyle), mSelectionBackgroundStyle(selectionBackgroundStyle)
		, mHighlightBackgroundStyle(highlightBackgroundStyle), mEditBoxStyle(editBoxStyle)
		, mDragHighlightStyle(dragHighlightStyle), mDragSepHighlightStyle(dragSepHighlightStyle), mIsElementSelected(false)
		, mIsElementHighlighted(false), mEditElement(nullptr), mNameEditBox(nullptr), mDragInProgress(false)
		, mDragHighlight(nullptr), mDragSepHighlight(nullptr), mScrollState(ScrollState::None), mLastScrollTime(0.0f)
		, mMouseOverDragElement(nullptr), mMouseOverDragElementTime(0.0f)
	{
		if(mBackgroundStyle == StringUtil::BLANK)
			mBackgroundStyle = "TreeViewBackground";

		if(mElementBtnStyle == StringUtil::BLANK)
			mElementBtnStyle = "TreeViewElementBtn";

		if(mFoldoutBtnStyle == StringUtil::BLANK)
			mFoldoutBtnStyle = "Expand";

		if(mSelectionBackgroundStyle == StringUtil::BLANK)
			mSelectionBackgroundStyle = "TreeViewSelectionBackground";

		if (mHighlightBackgroundStyle == StringUtil::BLANK)
			mHighlightBackgroundStyle = "TreeViewHighlightBackground";

		if(mEditBoxStyle == StringUtil::BLANK)
			mEditBoxStyle = "TreeViewEditBox";

		if(mDragHighlightStyle == StringUtil::BLANK)
			mDragHighlightStyle = "TreeViewElementHighlight";

		if(mDragSepHighlightStyle == StringUtil::BLANK)
			mDragSepHighlightStyle = "TreeViewElementSepHighlight";

		mBackgroundImage = GUITexture::create(mBackgroundStyle);
		mNameEditBox = GUITreeViewEditBox::create(mEditBoxStyle);
		mNameEditBox->setVisible(false);

		mNameEditBox->onInputConfirmed.connect(std::bind(&GUITreeView::onEditAccepted, this));
		mNameEditBox->onInputCanceled.connect(std::bind(&GUITreeView::onEditCanceled, this));
		mNameEditBox->onFocusLost.connect(std::bind(&GUITreeView::onEditFocusLost, this));

		mDragHighlight = GUITexture::create(mDragHighlightStyle);
		mDragSepHighlight = GUITexture::create(mDragSepHighlightStyle);

		mDragHighlight->setVisible(false);
		mDragSepHighlight->setVisible(false);

		mDragHighlight->_setElementDepth(2);
		mDragSepHighlight->_setElementDepth(2);

		_registerChildElement(mBackgroundImage);
		_registerChildElement(mNameEditBox);
		_registerChildElement(mDragHighlight);
		_registerChildElement(mDragSepHighlight);
	}

	GUITreeView::~GUITreeView()
	{

	}

	void GUITreeView::_update()
	{
		// Attempt to auto-expand elements we are dragging over
		if(acceptDragAndDrop())
		{
			const InteractableElement* element = findElementUnderCoord(mDragPosition);
			temporarilyExpandElement(element);
		}

		// NOTE - Instead of iterating through every visible element and comparing it with internal values,
		// I might just want to add callbacks to SceneManager that notify me of any changes and then only perform
		// update if anything is actually dirty

		updateTreeElementHierarchy();

		// Attempt to scroll if needed
		if(mScrollState != ScrollState::None)
		{
			GUIScrollArea* scrollArea = findParentScrollArea();
			if(scrollArea != nullptr)
			{
				float curTime = gTime().getTime();
				float timeDiff = curTime - mLastScrollTime;
				float secondsPerPixel = 1.0f / SCROLL_SPEED_PX_PER_SEC;

				switch(mScrollState)
				{
				case ScrollState::TransitioningUp:
					mScrollState = ScrollState::Up;
					mLastScrollTime = curTime;
					break;
				case ScrollState::TransitioningDown:
					mScrollState = ScrollState::Down;
					mLastScrollTime = curTime;
					break;
				case ScrollState::Up:
					{
						UINT32 scrollAmount = (UINT32)Math::floorToInt(timeDiff / secondsPerPixel);
						mLastScrollTime += scrollAmount * secondsPerPixel;

						scrollArea->scrollUpPx(scrollAmount);
					}
					break;
				case ScrollState::Down:
					{
						UINT32 scrollAmount = (UINT32)Math::floorToInt(timeDiff / secondsPerPixel);
						mLastScrollTime += scrollAmount * secondsPerPixel;

						scrollArea->scrollDownPx(scrollAmount);
					}
					break;
				case ScrollState::None:
					break;
				}
			}
		}
	}

	bool GUITreeView::_mouseEvent(const GUIMouseEvent& event)
	{
		if(event.getType() == GUIMouseEventType::MouseUp)
		{
			if(DragAndDropManager::instance().isDragInProgress())
				return false;

			const InteractableElement* element = findElementUnderCoord(event.getPosition());
			TreeElement* treeElement = nullptr;

			if(element != nullptr && element->isTreeElement())
			{
				treeElement = element->getTreeElement();
			}

			if (treeElement != nullptr)
			{
				bool onFoldout = false;
				if (treeElement->mFoldoutBtn != nullptr)
					onFoldout = treeElement->mFoldoutBtn->_getClippedBounds().contains(event.getPosition());

				bool onEditElement = false;
				if (mEditElement != nullptr)
					onEditElement = treeElement == mEditElement;

				if (!onFoldout && !onEditElement)
				{
					if (event.getButton() == GUIMouseButton::Left)
					{
						if (event.isCtrlDown())
						{
							selectElement(treeElement);
						}
						else if (event.isShiftDown())
						{
							if (isSelectionActive())
							{
								TreeElement* selectionRoot = mSelectedElements[0].element;
								unselectAll();

								auto iterStartFind = std::find_if(mVisibleElements.begin(), mVisibleElements.end(),
									[&](const InteractableElement& x) { return x.parent == selectionRoot->mParent; });

								bool foundStart = false;
								bool foundEnd = false;
								for (; iterStartFind != mVisibleElements.end(); ++iterStartFind)
								{
									if (!iterStartFind->isTreeElement())
										continue;

									TreeElement* curElem = iterStartFind->getTreeElement();
									if (curElem == selectionRoot)
									{
										foundStart = true;
										break;
									}
								}

								auto iterEndFind = std::find_if(mVisibleElements.begin(), mVisibleElements.end(),
									[&](const InteractableElement& x) { return &x == element; });

								if (iterEndFind != mVisibleElements.end())
									foundEnd = true;

								if (foundStart && foundEnd)
								{
									if (iterStartFind < iterEndFind)
									{
										for (; iterStartFind != (iterEndFind + 1); ++iterStartFind)
										{
											if (iterStartFind->isTreeElement())
												selectElement(iterStartFind->getTreeElement());
										}
									}
									else if (iterEndFind < iterStartFind)
									{
										for (; iterEndFind != (iterStartFind + 1); ++iterEndFind)
										{
											if (iterEndFind->isTreeElement())
												selectElement(iterEndFind->getTreeElement());
										}
									}
									else
										selectElement(treeElement);
								}

								if (!foundStart || !foundEnd)
									selectElement(treeElement);
							}
							else
							{
								selectElement(treeElement);
							}
						}
						else
						{
							bool doRename = false;
							if (isSelectionActive())
							{
								for (auto& selectedElem : mSelectedElements)
								{
									if (selectedElem.element == treeElement)
									{
										doRename = true;
										break;
									}
								}
							}

							unselectAll();
							selectElement(treeElement);

							if (doRename)
								renameSelected();
						}

						_markLayoutAsDirty();

						return true;
					}
					else if (event.getButton() == GUIMouseButton::Right)
					{
						unselectAll();
						selectElement(treeElement);
						_markLayoutAsDirty();

						return true;
					}
				}
			}
			else
			{
				unselectAll();

				return true;
			}
		}
		else if(event.getType() == GUIMouseEventType::MouseDragStart)
		{
			clearPing();

			mDragStartPosition = event.getPosition();
		}
		else if(event.getType() == GUIMouseEventType::MouseDrag)
		{
			UINT32 dist = mDragStartPosition.manhattanDist(event.getPosition());

			if(!DragAndDropManager::instance().isDragInProgress())
			{
				if(dist > DRAG_MIN_DISTANCE && mEditElement == nullptr)
				{
					const InteractableElement* element = findElementUnderCoord(mDragStartPosition);
					Vector<TreeElement*> draggedElements;

					if(element != nullptr && element->isTreeElement())
					{
						// If multiple elements are selected, drag all of them
						TreeElement* treeElement = element->getTreeElement();
						auto iterFind = std::find_if(mSelectedElements.begin(), mSelectedElements.end(), 
							[&] (const SelectedElement& x) { return x.element == treeElement; });

						if (iterFind != mSelectedElements.end())
						{
							for (auto& entry : mSelectedElements)
								draggedElements.push_back(entry.element);
						}
						else
							draggedElements.push_back(treeElement);
					}

					dragAndDropStart(draggedElements);

					mDragPosition = event.getPosition();
					mDragInProgress = true;
					mScrollState = ScrollState::None;
					_markLayoutAsDirty();
				}
			}
		}
		else if(event.getType() == GUIMouseEventType::MouseDragAndDropDragged)
		{
			if(acceptDragAndDrop())
			{
				clearPing();

				mDragPosition = event.getPosition();
				mDragInProgress = true;
				_markLayoutAsDirty();

				if(mBottomScrollBounds.contains(mDragPosition))
				{
					if(mScrollState != ScrollState::Down)
						mScrollState = ScrollState::TransitioningDown;
				}
				else if(mTopScrollBounds.contains(mDragPosition))
				{
					if(mScrollState != ScrollState::Up)
						mScrollState = ScrollState::TransitioningUp;
				}
				else
					mScrollState = ScrollState::None;

				return true;
			}
		}
		else if(event.getType() == GUIMouseEventType::MouseDragAndDropDropped)
		{
			if(acceptDragAndDrop())
			{
				const InteractableElement* element = findElementUnderCoord(event.getPosition());

				TreeElement* treeElement = nullptr;
				if(element != nullptr)
				{
					if(element->isTreeElement())
						treeElement = element->getTreeElement();
					else
						treeElement = element->parent;
				}

				dragAndDropEnded(treeElement);
				mDragInProgress = false;
				unselectAll();

				return true;
			}
		}
		else if(event.getType() == GUIMouseEventType::MouseOut)
		{
			mDragInProgress = false;
			_markLayoutAsDirty();
		}
		else if(event.getType() == GUIMouseEventType::MouseDragAndDropLeft)
		{
			mDragInProgress = false;
			_markLayoutAsDirty();
		}

		return false;
	}

	bool GUITreeView::_commandEvent(const GUICommandEvent& ev)
	{
		if(ev.getType() == GUICommandEventType::MoveUp || ev.getType() == GUICommandEventType::SelectUp)
		{
			TreeElement* topMostElement = getTopMostSelectedElement();
			auto topMostIter = std::find_if(mVisibleElements.begin(), mVisibleElements.end(), 
				[&] (const InteractableElement& x) { return x.getTreeElement() == topMostElement; });

			if(topMostIter != mVisibleElements.end() && topMostIter != mVisibleElements.begin())
			{
				do 
				{
					topMostIter--;
				} while (!topMostIter->isTreeElement() && topMostIter != mVisibleElements.begin());

				if(topMostIter->isTreeElement())
				{
					if(ev.getType() == GUICommandEventType::MoveUp)
						unselectAll();

					TreeElement* treeElement = topMostIter->getTreeElement();
					selectElement(treeElement);
					scrollToElement(treeElement, false);
				}
			}
		}
		else if(ev.getType() == GUICommandEventType::MoveDown || ev.getType() == GUICommandEventType::SelectDown)
		{
			TreeElement* bottoMostElement = getBottomMostSelectedElement();
			auto bottomMostIter = std::find_if(mVisibleElements.begin(), mVisibleElements.end(), 
				[&] (const InteractableElement& x) { return x.getTreeElement() == bottoMostElement; });

			if(bottomMostIter != mVisibleElements.end())
			{
				do 
				{
					bottomMostIter++;
				} while (bottomMostIter != mVisibleElements.end() && !bottomMostIter->isTreeElement());

				if(bottomMostIter != mVisibleElements.end() && bottomMostIter->isTreeElement())
				{
					if(ev.getType() == GUICommandEventType::MoveDown)
						unselectAll();

					TreeElement* treeElement = bottomMostIter->getTreeElement();
					selectElement(treeElement);
					scrollToElement(treeElement, false);
				}
			}
		}

		return GUIElementContainer::_commandEvent(ev);
	}

	bool GUITreeView::isSelectionActive() const
	{
		return mIsElementSelected && mSelectedElements.size() > 0;
	}

	void GUITreeView::selectElement(TreeElement* element)
	{
		clearPing();

		auto iterFind = std::find_if(mSelectedElements.begin(), mSelectedElements.end(), 
			[&] (const SelectedElement& x) { return x.element == element; });

		if(iterFind == mSelectedElements.end())
		{
			GUITexture* background = GUITexture::create(mSelectionBackgroundStyle);
			background->_setElementDepth(3);
			_registerChildElement(background);

			element->mIsSelected = true;

			mSelectedElements.push_back(SelectedElement(element, background));
			mIsElementSelected = true;

			selectionChanged();
		}
	}

	void GUITreeView::unselectElement(TreeElement* element)
	{
		clearPing();

		auto iterFind = std::find_if(mSelectedElements.begin(), mSelectedElements.end(), 
			[&] (const SelectedElement& x) { return x.element == element; });

		if(iterFind != mSelectedElements.end())
		{
			iterFind->element->mIsSelected = false;
			GUIElement::destroy(iterFind->background);

			mSelectedElements.erase(iterFind);
			_markLayoutAsDirty();

			selectionChanged();
		}

		mIsElementSelected = mSelectedElements.size() > 0;
	}

	void GUITreeView::unselectAll(bool sendEvent)
	{
		clearPing();

		for(auto& selectedElem : mSelectedElements)
		{
			selectedElem.element->mIsSelected = false;
			GUIElement::destroy(selectedElem.background);
		}

		mSelectedElements.clear();
		mIsElementSelected = false;

		_markLayoutAsDirty();

		if (sendEvent)
			selectionChanged();
	}

	void GUITreeView::renameSelected()
	{
		if (isSelectionActive() && mEditElement == nullptr)
		{
			clearPing();
			enableEdit(mSelectedElements[0].element);
			unselectAll();
		}
	}

	void GUITreeView::deleteSelection()
	{
		if (isSelectionActive())
		{
			auto isChildOf = [&](const TreeElement* parent, const TreeElement* child)
			{
				const TreeElement* elem = child;

				while (elem != nullptr && elem != parent)
					elem = elem->mParent;

				return elem == parent;
			};

			// Ensure we don't unnecessarily try to delete children if their
			// parent is getting deleted anyway
			Vector<TreeElement*> elementsToDelete;
			for (UINT32 i = 0; i < (UINT32)mSelectedElements.size(); i++)
			{
				bool hasDeletedParent = false;
				for (UINT32 j = 0; j < (UINT32)mSelectedElements.size(); j++)
				{
					if (i == j)
						continue;

					if (isChildOf(mSelectedElements[j].element, mSelectedElements[i].element))
					{
						hasDeletedParent = true;
						break;
					}
				}

				if (!hasDeletedParent)
					elementsToDelete.push_back(mSelectedElements[i].element);
			}

			clearPing();
			unselectAll();

			for (auto& elem : elementsToDelete)
				deleteTreeElement(elem);
		}
	}

	void GUITreeView::ping(TreeElement* element)
	{
		clearPing();

		expandToElement(element);
		scrollToElement(element, true);

		GUITexture* background = GUITexture::create(mHighlightBackgroundStyle);
		background->_setElementDepth(2);
		_registerChildElement(background);

		element->mIsHighlighted = true;

		mHighlightedElement.element = element;
		mHighlightedElement.background = background;

		mIsElementHighlighted = true;
		_markLayoutAsDirty();
	}

	void GUITreeView::clearPing()
	{
		if (!mIsElementHighlighted)
			return;

		mHighlightedElement.element->mIsHighlighted = false;
		GUIElement::destroy(mHighlightedElement.background);

		mHighlightedElement.element = nullptr;
		mHighlightedElement.background = nullptr;
		mIsElementHighlighted = false;

		_markLayoutAsDirty();
	}

	void GUITreeView::expandToElement(TreeElement* element)
	{
		if (element->mIsVisible || element->mParent == nullptr)
			return;

		Stack<TreeElement*> todo;

		TreeElement* parent = element->mParent;
		while (parent != nullptr)
		{
			if (!parent->mIsExpanded)
				todo.push(parent);

			if (parent->mIsVisible)
				break;

			parent = parent->mParent;
		}

		while (!todo.empty())
		{
			TreeElement* curElement = todo.top();
			todo.pop();

			expandElement(curElement);
		}
	}

	void GUITreeView::expandElement(TreeElement* element)
	{
		if(element->mIsExpanded)
			return;

		element->mIsExpanded = true;

		if(element->mParent == nullptr || (element->mParent->mIsVisible && element->mParent->mIsExpanded))
		{
			Stack<TreeElement*> todo;
			todo.push(element);

			while(!todo.empty())
			{
				TreeElement* curElem = todo.top();
				todo.pop();

				curElem->mIsVisible = true;
				updateElementGUI(curElem);

				if(curElem->mIsExpanded)
				{
					for(auto& child : curElem->mChildren)
						todo.push(child);
				}
			}
		}
	}

	void GUITreeView::collapseElement(TreeElement* element)
	{
		if(!element->mIsExpanded)
			return;

		element->mIsExpanded = false;
		updateElementGUI(element);

		if(element->mParent == nullptr || (element->mParent->mIsVisible && element->mParent->mIsExpanded))
		{
			Stack<TreeElement*> todo;

			for(auto& child : element->mChildren)
				todo.push(child);

			while(!todo.empty())
			{
				TreeElement* curElem = todo.top();
				todo.pop();

				curElem->mIsVisible = false;

				if(curElem->mIsSelected)
					unselectElement(curElem);

				updateElementGUI(curElem);

				if(curElem->mIsExpanded)
				{
					for(auto& child : curElem->mChildren)
						todo.push(child);
				}
			}
		}
	}

	void GUITreeView::updateElementGUI(TreeElement* element)
	{
		if(element == &getRootElement())
			return;

		if(element->mIsVisible)
		{
			HString name(toWString(element->mName));
			if(element->mElement == nullptr)
			{
				element->mElement = GUILabel::create(name, mElementBtnStyle);
				_registerChildElement(element->mElement);
			}

			if (element->mIsCut)
			{
				Color cutTint = element->mTint;
				cutTint.a = CUT_COLOR.a;

				element->mElement->setTint(cutTint);
			}
			else if(element->mIsDisabled)
			{
				Color disabledTint = element->mTint;
				disabledTint.a = DISABLED_COLOR.a;

				element->mElement->setTint(disabledTint);
			}
			else
				element->mElement->setTint(element->mTint);

			if(element->mChildren.size() > 0)
			{
				if(element->mFoldoutBtn == nullptr)
				{
					element->mFoldoutBtn = GUIToggle::create(GUIContent(HString(L"")), mFoldoutBtnStyle);
					_registerChildElement(element->mFoldoutBtn);

					element->mFoldoutBtn->onToggled.connect(std::bind(&GUITreeView::elementToggled, this, element, _1));

					if(element->mIsExpanded)
						element->mFoldoutBtn->toggleOn();
				}
			}
			else
			{
				if(element->mFoldoutBtn != nullptr)
				{
					GUIElement::destroy(element->mFoldoutBtn);
					element->mFoldoutBtn = nullptr;
				}
			}

			element->mElement->setContent(GUIContent(name));
		}
		else
		{
			if(element->mElement != nullptr)
			{
				GUIElement::destroy(element->mElement);
				element->mElement = nullptr;
			}

			if(element->mFoldoutBtn != nullptr)
			{
				GUIElement::destroy(element->mFoldoutBtn);
				element->mFoldoutBtn = nullptr;
			}

			if(element->mIsSelected && element->mIsExpanded)
				unselectElement(element);
		}

		_markLayoutAsDirty();
	}

	void GUITreeView::elementToggled(TreeElement* element, bool toggled)
	{
		clearPing();

		if(toggled)
			expandElement(element);
		else
			collapseElement(element);
	}

	void GUITreeView::onEditAccepted()
	{
		TreeElement* elem = mEditElement;
		disableEdit(true);
		selectElement(elem);
	}

	void GUITreeView::onEditCanceled()
	{
		if (mEditElement != nullptr)
		{
			TreeElement* elem = mEditElement;
			disableEdit(false);
			selectElement(elem);
		}
	}

	void GUITreeView::onEditFocusLost()
	{
		if (mEditElement != nullptr)
			disableEdit(false);
	}

	void GUITreeView::enableEdit(TreeElement* element)
	{
		assert(mEditElement == nullptr);

		mEditElement = element;
		mNameEditBox->setVisible(true);
		mNameEditBox->setText(toWString(element->mName));
		mNameEditBox->setFocus(true);

		if(element->mElement != nullptr)
			element->mElement->setVisible(false);
	}

	void GUITreeView::disableEdit(bool applyChanges)
	{
		assert(mEditElement != nullptr);

		if(mEditElement->mElement != nullptr)
			mEditElement->mElement->setVisible(true);

		if(applyChanges)
		{
			WString newName = mNameEditBox->getText();
			renameTreeElement(mEditElement, newName);
		}

		mNameEditBox->setFocus(false);
		mNameEditBox->setVisible(false);
		mEditElement = nullptr;
	}

	Vector2I GUITreeView::_getOptimalSize() const
	{
		struct UpdateTreeElement
		{
			UpdateTreeElement(const TreeElement* element, UINT32 indent)
				:element(element), indent(indent)
			{ }

			const TreeElement* element;
			UINT32 indent;
		};

		Vector2I optimalSize;

		if (_getDimensions().fixedWidth() && _getDimensions().fixedHeight())
		{
			optimalSize.x = _getDimensions().minWidth;
			optimalSize.y = _getDimensions().minHeight;
		}
		else
		{
			Stack<UpdateTreeElement> todo;
			todo.push(UpdateTreeElement(&getRootElementConst(), 0));

			while(!todo.empty())
			{
				UpdateTreeElement currentUpdateElement = todo.top();
				const TreeElement* current = currentUpdateElement.element;
				todo.pop();

				INT32 yOffset = 0;
				if(current->mElement != nullptr)
				{
					Vector2I curOptimalSize = current->mElement->_getOptimalSize();
					optimalSize.x = std::max(optimalSize.x, 
						(INT32)(INITIAL_INDENT_OFFSET + curOptimalSize.x + currentUpdateElement.indent * INDENT_SIZE));
					yOffset = curOptimalSize.y + ELEMENT_EXTRA_SPACING;
				}

				optimalSize.y += yOffset;

				for(auto& child : current->mChildren)
				{
					if(!child->mIsVisible)
						continue;

					todo.push(UpdateTreeElement(child, currentUpdateElement.indent + 1));
				}
			}

			if(_getDimensions().fixedWidth())
				optimalSize.x = _getDimensions().minWidth;
			else
			{
				if(_getDimensions().minWidth > 0)
					optimalSize.x = std::max((INT32)_getDimensions().minWidth, optimalSize.x);

				if(_getDimensions().maxWidth > 0)
					optimalSize.x = std::min((INT32)_getDimensions().maxWidth, optimalSize.x);
			}

			if (_getDimensions().fixedHeight())
				optimalSize.y = _getDimensions().minHeight;
			else
			{
				if(_getDimensions().minHeight > 0)
					optimalSize.y = std::max((INT32)_getDimensions().minHeight, optimalSize.y);

				if(_getDimensions().maxHeight > 0)
					optimalSize.y = std::min((INT32)_getDimensions().maxHeight, optimalSize.y);
			}
		}

		return optimalSize;
	}

	void GUITreeView::updateClippedBounds()
	{
		mClippedBounds = mLayoutData.area;
		mClippedBounds.clip(mLayoutData.clipRect);
	}

	void GUITreeView::_updateLayoutInternal(const GUILayoutData& data)
	{
		struct UpdateTreeElement
		{
			UpdateTreeElement(TreeElement* element, UINT32 indent)
				:element(element), indent(indent)
			{ }

			TreeElement* element;
			UINT32 indent;
		};

		mVisibleElements.clear();

		Stack<UpdateTreeElement> todo;
		todo.push(UpdateTreeElement(&getRootElement(), 0));

		// NOTE - Instead of iterating through all elements, try to find those within the clip rect
		// and only iterate through those. Others should somehow be marked in-active (similar to GUIElement::isDisabled()?)

		Vector<TreeElement*> tempOrderedElements;

		Vector2I offset(data.area.x, data.area.y);

		while(!todo.empty())
		{
			UpdateTreeElement currentUpdateElement = todo.top();
			TreeElement* current = currentUpdateElement.element;
			UINT32 indent = currentUpdateElement.indent;
			todo.pop();

			INT32 btnHeight = 0;
			INT32 yOffset = 0;
			if(current->mElement != nullptr)
			{
				Vector2I elementSize = current->mElement->_getOptimalSize();
				btnHeight = elementSize.y;

				mVisibleElements.push_back(InteractableElement(current->mParent, current->mSortedIdx * 2 + 0, Rect2I(data.area.x, offset.y, data.area.width, ELEMENT_EXTRA_SPACING)));
				mVisibleElements.push_back(InteractableElement(current->mParent, current->mSortedIdx * 2 + 1, Rect2I(data.area.x, offset.y + ELEMENT_EXTRA_SPACING, data.area.width, btnHeight)));

				offset.x = data.area.x + INITIAL_INDENT_OFFSET + indent * INDENT_SIZE;
				offset.y += ELEMENT_EXTRA_SPACING;

				GUILayoutData childData = data;
				childData.area.x = offset.x;
				childData.area.y = offset.y;
				childData.area.width = elementSize.x;
				childData.area.height = elementSize.y;

				current->mElement->_setLayoutData(childData);

				yOffset = btnHeight;
			}

			if(current->mFoldoutBtn != nullptr)
			{
				Vector2I elementSize = current->mFoldoutBtn->_getOptimalSize();

				offset.x -= std::min((INT32)INITIAL_INDENT_OFFSET, elementSize.x + 2);

				Vector2I myOffset = offset;
				myOffset.y += 1;

				if(elementSize.y > btnHeight)
				{
					UINT32 diff = elementSize.y - btnHeight;
					float half = diff * 0.5f;
					myOffset.y -= Math::floorToInt(half);
				}

				GUILayoutData childData = data;
				childData.area.x = myOffset.x;
				childData.area.y = myOffset.y;
				childData.area.width = elementSize.x;
				childData.area.height = elementSize.y;

				current->mFoldoutBtn->_setLayoutData(childData);
			}

			offset.y += yOffset;

			tempOrderedElements.resize(current->mChildren.size(), nullptr);
			for(auto& child : current->mChildren)
			{
				tempOrderedElements[child->mSortedIdx] = child;
			}

			for(auto iter = tempOrderedElements.rbegin(); iter != tempOrderedElements.rend(); ++iter)
			{
				TreeElement* child = *iter;

				if(!child->mIsVisible)
					continue;

				todo.push(UpdateTreeElement(child, indent + 1));
			}
		}

		UINT32 remainingHeight = (UINT32)std::max(0, (INT32)data.area.height - (offset.y - data.area.y));

		if(remainingHeight > 0)
			mVisibleElements.push_back(InteractableElement(&getRootElement(), (UINT32)getRootElement().mChildren.size() * 2, Rect2I(data.area.x, offset.y, data.area.width, remainingHeight)));

		for(auto selectedElem : mSelectedElements)
		{
			GUILabel* targetElement = selectedElem.element->mElement;
			if (targetElement == nullptr)
				continue;

			GUILayoutData childData = data;
			childData.area.y = targetElement->_getLayoutData().area.y;
			childData.area.height = targetElement->_getLayoutData().area.height;

			selectedElem.background->_setLayoutData(childData);
		}

		if (mIsElementHighlighted)
		{
			GUILabel* targetElement = mHighlightedElement.element->mElement;
			if (targetElement != nullptr)
			{
				GUILayoutData childData = data;
				childData.area.y = targetElement->_getLayoutData().area.y;
				childData.area.height = targetElement->_getLayoutData().area.height;

				mHighlightedElement.background->_setLayoutData(childData);
			}
		}

		if(mEditElement != nullptr)
		{
			GUILabel* targetElement = mEditElement->mElement;
			if (targetElement != nullptr)
			{
				UINT32 remainingWidth = (UINT32)std::max(0, (((INT32)data.area.width) - (offset.x - data.area.x)));

				GUILayoutData childData = data;
				childData.area = targetElement->_getLayoutData().area;
				childData.area.width = remainingWidth;

				mNameEditBox->_setLayoutData(childData);
			}
		}

		if(mDragInProgress)
		{
			const InteractableElement* interactableElement = findElementUnderCoord(mDragPosition);

			if(interactableElement == nullptr)
			{
				mDragHighlight->setVisible(false);
				mDragSepHighlight->setVisible(false);
			}
			else
			{
				if(interactableElement->isTreeElement())
				{
					mDragSepHighlight->setVisible(false);
					mDragHighlight->setVisible(true);

					GUILayoutData childData = data;
					childData.area = interactableElement->bounds;

					mDragHighlight->_setLayoutData(childData);
				}
				else
				{
					mDragHighlight->setVisible(false);
					mDragSepHighlight->setVisible(true);

					GUILayoutData childData = data;
					childData.area = interactableElement->bounds;

					mDragSepHighlight->_setLayoutData(childData);
				}
			}
		}
		else
		{
			mDragHighlight->setVisible(false);
			mDragSepHighlight->setVisible(false);
		}

		// Update scroll bounds
		UINT32 scrollHeight = (UINT32)Math::roundToInt(data.clipRect.height * SCROLL_AREA_HEIGHT_PCT);

		mTopScrollBounds.x = data.clipRect.x;
		mTopScrollBounds.y = data.clipRect.y;
		mTopScrollBounds.width = data.clipRect.width;
		mTopScrollBounds.height = scrollHeight;

		mBottomScrollBounds.x = data.clipRect.x;
		mBottomScrollBounds.y = data.clipRect.y + data.clipRect.height - scrollHeight;
		mBottomScrollBounds.width = data.clipRect.width;
		mBottomScrollBounds.height = scrollHeight;
	}

	const GUITreeView::InteractableElement* GUITreeView::findElementUnderCoord(const Vector2I& coord) const
	{
		for(auto& element : mVisibleElements)
		{
			if(element.bounds.contains(coord))
			{
				return &element;
			}
		}

		return nullptr;
	}

	GUITreeView::TreeElement* GUITreeView::getTopMostSelectedElement() const
	{
		auto topMostElement = mVisibleElements.end();

		for(auto& selectedElement : mSelectedElements)
		{
			auto iterFind = std::find_if(mVisibleElements.begin(), mVisibleElements.end(), 
				[&] (const InteractableElement& x) { return x.getTreeElement() == selectedElement.element; });

			if(iterFind != mVisibleElements.end())
			{
				if(topMostElement == mVisibleElements.end())
					topMostElement = iterFind;
				else
				{
					if(iterFind->bounds.y < topMostElement->bounds.y)
						topMostElement = iterFind;
				}
			}
		}

		if(topMostElement != mVisibleElements.end())
			return topMostElement->getTreeElement();
		else
			return nullptr;
	}

	GUITreeView::TreeElement* GUITreeView::getBottomMostSelectedElement() const
	{
		auto botMostElement = mVisibleElements.end();

		for(auto& selectedElement : mSelectedElements)
		{
			auto iterFind = std::find_if(mVisibleElements.begin(), mVisibleElements.end(), 
				[&] (const InteractableElement& x) { return x.getTreeElement() == selectedElement.element; });

			if(iterFind != mVisibleElements.end())
			{
				if(botMostElement == mVisibleElements.end())
					botMostElement = iterFind;
				else
				{
					if((iterFind->bounds.y + iterFind->bounds.height) > (botMostElement->bounds.y + botMostElement->bounds.height))
						botMostElement = iterFind;
				}
			}
		}

		if(botMostElement != mVisibleElements.end())
			return botMostElement->getTreeElement();
		else
			return nullptr;
	}

	void GUITreeView::closeTemporarilyExpandedElements()
	{
		temporarilyExpandElement(nullptr);
	}

	void GUITreeView::temporarilyExpandElement(const InteractableElement* mouseOverElement)
	{
		TreeElement* treeElement = nullptr;
		if(mouseOverElement != nullptr && mouseOverElement->isTreeElement())
			treeElement = mouseOverElement->getTreeElement();

		if(treeElement == nullptr || treeElement != mMouseOverDragElement)
		{
			while(!mAutoExpandedElements.empty())
			{
				TreeElement* autoExpandedElement = mAutoExpandedElements.top();

				bool unexpandElement = false;
				if(mouseOverElement != nullptr && mouseOverElement->parent != nullptr)
				{
					if(mouseOverElement->parent != autoExpandedElement && !mouseOverElement->parent->isParentRec(autoExpandedElement))
						unexpandElement = true;
					else
						break;
				}
				else
					unexpandElement = true;

				if(unexpandElement)
				{
					collapseElement(autoExpandedElement);

					if(autoExpandedElement->mFoldoutBtn != nullptr)
						autoExpandedElement->mFoldoutBtn->toggleOff();

					mAutoExpandedElements.pop();
				}
			}

			mMouseOverDragElement = treeElement;
			mMouseOverDragElementTime = gTime().getTime();
		}
		else
		{
			if(mMouseOverDragElement != nullptr && !mMouseOverDragElement->mIsExpanded)
			{
				float timeDiff = gTime().getTime() - mMouseOverDragElementTime;
				if(timeDiff >= AUTO_EXPAND_DELAY_SEC)
				{
					mAutoExpandedElements.push(mMouseOverDragElement);
					expandElement(mMouseOverDragElement);

					if(mMouseOverDragElement->mFoldoutBtn != nullptr)
						mMouseOverDragElement->mFoldoutBtn->toggleOn();
				}
			}
		}
	}

	void GUITreeView::scrollToElement(TreeElement* element, bool center)
	{
		if(element->mElement == nullptr)
			return;

		GUIScrollArea* scrollArea = findParentScrollArea();
		if(scrollArea == nullptr)
			return;

		if(center)
		{
			Rect2I myBounds = _getClippedBounds();
			INT32 clipVertCenter = myBounds.y + (INT32)Math::roundToInt(myBounds.height * 0.5f);
			INT32 elemVertCenter = element->mElement->_getLayoutData().area.y + (INT32)Math::roundToInt(element->mElement->_getLayoutData().area.height * 0.5f);

			if(elemVertCenter > clipVertCenter)
				scrollArea->scrollDownPx(elemVertCenter - clipVertCenter);
			else
				scrollArea->scrollUpPx(clipVertCenter - elemVertCenter);
		}
		else
		{
			Rect2I myBounds = _getClippedBounds();
			INT32 elemVertTop = element->mElement->_getLayoutData().area.y;
			INT32 elemVertBottom = element->mElement->_getLayoutData().area.y + element->mElement->_getLayoutData().area.height;

			INT32 top = myBounds.y;
			INT32 bottom = myBounds.y + myBounds.height;

			if(elemVertTop < top)
				scrollArea->scrollUpPx(top - elemVertTop);
			else if(elemVertBottom > bottom)
				scrollArea->scrollDownPx(elemVertBottom - bottom);
		}
	}

	GUIScrollArea* GUITreeView::findParentScrollArea() const
	{
		GUIElementBase* parent = _getParent();
		while(parent != nullptr)
		{
			if(parent->_getType() == GUIElementBase::Type::Element)
			{
				GUIElement* parentElement = static_cast<GUIElement*>(parent);

				if(parentElement->_getElementType() == GUIElement::ElementType::ScrollArea)
				{
					GUIScrollArea* scrollArea = static_cast<GUIScrollArea*>(parentElement);
					return scrollArea;

				}
			}

			parent = parent->_getParent();
		}

		return nullptr;
	}

	const String& GUITreeView::getGUITypeName()
	{
		static String typeName = "SceneTreeView";
		return typeName;
	}
}