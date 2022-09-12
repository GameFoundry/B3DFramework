//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUINavGroup.h"
#include "GUI/BsGUIElement.h"
#include "GUI/BsGUIManager.h"

namespace bs
{
	SPtr<GUINavGroup> GUINavGroup::Create()
	{
		return bs_shared_ptr_new<GUINavGroup>();
	}

	void GUINavGroup::FocusFirst()
	{
		if(mOrderedElements.empty())
			return;

		// Find first element with an explicit index, if one exists
		auto iterStart = mOrderedElements.begin();
		if(iterStart->first != 0)
		{
			iterStart->second->SetFocus(true, true);
			return;
		}

		// Otherwise look for top-left element without an explicit index
		focusTopLeft();
	}

	void GUINavGroup::FocusNext(GUIElement* anchor)
	{
		// Nothing currently in focus
		if(!anchor)
		{
			focusFirst();
			return;
		}

		const INT32 tabIdx = mElements[anchor];

		// Find next element using the explicit index
		if(tabIdx != 0)
		{
			auto iterFind = mOrderedElements.lower_bound(tabIdx);
			while(iterFind->second != anchor)
				++iterFind;

			++iterFind;

			// Reached the end, wrap around
			if(iterFind == mOrderedElements.end())
				return FocusFirst();

			// If a next element with an explicit index exists, select it
			if(iterFind->first != 0)
			{
				iterFind->second->SetFocus(true, true);
				return;
			}

			// Select top-left element with no tab index
			focusTopLeft();
			return;
		}

		// Find next element to focus on
		{
			const Rect2I focusedElemBounds = anchor->_getClippedBounds();

			// We look for the element to the right of the current element, within some Y range (a 'row').
			//// We search by rows in order to make the navigation perceptually nicer. Sometimes elements appear to be
			//// in the same row, but might be off by a few pixels, in which case the simpler approach would 'jump'
			//// over an element.
			constexpr static INT32 ROW_HEIGHT = 5;

			const auto unindexedRange = mOrderedElements.equal_range(0);
			bs_frame_mark();
			{
				struct YCompare
				{
					bool operator()(const GUIElement* lhs, const GUIElement* rhs) const
					{
						const Rect2I boundsLHS = lhs->_getClippedBounds();
						const Rect2I boundsRHS = rhs->_getClippedBounds();

						if(boundsLHS.y != boundsRHS.y)
							return boundsLHS.y < boundsRHS.y;

						return lhs < rhs;
					}
				};

				// Build a list of relevant elements, ordered by height
				FrameSet<GUIElement*, YCompare> elements;
				for (auto iter = unindexedRange.first; iter != unindexedRange.second; ++iter)
				{
					GUIElement* element = iter->second;
					const bool acceptsKeyFocus = element->GetOptionFlags().IsSet(GUIElementOption::AcceptsKeyFocus);
					if (!acceptsKeyFocus || !element->_isVisible() || element->_isDisabled())
						continue;

					const Rect2I elemBounds = element->_getClippedBounds();
					const bool isFullyClipped = elemBounds.width == 0 || elemBounds.height == 0;

					if (isFullyClipped)
						continue;

					elements.Insert(element);
				}

				// Find the row the currently selected element is part of
				auto iterElem = elements.begin();
				auto iterRowStart = iterElem;

				INT32 firstRowY = 0;
				INT32 rowY = 0;
				for(; iterElem != elements.end(); ++iterElem)
				{
					GUIElement* element = *iterElem;

					const Rect2I elemBounds = element->_getClippedBounds();
					if(iterElem == elements.begin())
					{
						firstRowY = elemBounds.y;
						rowY = elemBounds.y;
					}
					else
					{
						const INT32 yDiff = elemBounds.y - rowY;

						// New row
						if (yDiff >= ROW_HEIGHT)
						{
							iterRowStart = iterElem;
							rowY = elemBounds.y;
						}
					}

					if(element == anchor)
						break;
				}

				const bool foundRow = iterElem != elements.end();
				if(!foundRow)
					rowY = firstRowY;

				// Try to find the next element in the current row (to the right of the current one)
				GUIElement* nextElement = nullptr;
				INT32 nearestX = std::numeric_limits<INT32>::max();
				iterElem = iterRowStart;
				for(; iterElem != elements.end(); ++iterElem)
				{
					GUIElement* element = *iterElem;
					if(element == anchor)
						continue;

					const Rect2I elemBounds = element->_getClippedBounds();
					const INT32 yDiff = elemBounds.y - rowY;

					// New row
					if(yDiff >= ROW_HEIGHT)
					{
						rowY = elemBounds.y;
						break;
					}

					// Note: We're purposely ignoring elements at the same exact position, as the tab focus would then just
					// ping-pong between the two elements, and we'd have to keep a list of previously visited elements in
					// order to avoid the issue.
					if(elemBounds.x > focusedElemBounds.x)
					{
						const INT32 xDiff = elemBounds.x - focusedElemBounds.x;
						if (xDiff < nearestX)
						{
							nearestX = xDiff;
							nextElement = element;
						}
					}
				}

				// If no element in the current row, find the left-most element in the next row
				if(!nextElement)
				{
					nearestX = std::numeric_limits<INT32>::max();
					for (; iterElem != elements.end(); ++iterElem)
					{
						GUIElement* element = *iterElem;

						const Rect2I elemBounds = element->_getClippedBounds();
						const INT32 yDiff = elemBounds.y - rowY;

						// New row
						if (yDiff >= ROW_HEIGHT)
							break;

						if (elemBounds.x < nearestX)
						{
							nearestX = elemBounds.x;
							nextElement = element;
						}
					}
				}

				if (nextElement)
				{
					nextElement->SetFocus(true, true);
					return;
				}

			}
			bs_frame_clear();

			// No more elements with no tab index. Check elements with positive tab index
			const auto iterAfterUnindexed = unindexedRange.second;
			if(iterAfterUnindexed != mOrderedElements.end())
			{
				iterAfterUnindexed->second->SetFocus(true, true);
				return;
			}

			// Reached the end, wrap around
			focusFirst();
		}
	}

	void GUINavGroup::FocusTopLeft()
	{
		UINT32 lowestDist = std::numeric_limits<UINT32>::max();
		GUIElement* topLeftElement = nullptr;

		// Grab only elements without an explicit index
		const auto unindexedRange = mOrderedElements.equal_range(0);
		for(auto iter = unindexedRange.first; iter != unindexedRange.second; ++iter)
		{
			GUIElement* element = iter->second;

			// Ignore elements that are hidden, disabled or just don't accept input focus
			const bool acceptsKeyFocus = element->GetOptionFlags().IsSet(GUIElementOption::AcceptsKeyFocus);
			if(!acceptsKeyFocus || !element->_isVisible() || element->_isDisabled())
				continue;

			// Ignore elements that have been fully clipped
			const Rect2I elemBounds = element->_getClippedBounds();
			if(elemBounds.width == 0 || elemBounds.height == 0)
				continue;

			Vector2I ElementPos(elemBounds.x, elemBounds.y);

			const UINT32 dist = elementPos.SquaredLength();
			if (dist < lowestDist)
			{
				lowestDist = dist;
				topLeftElement = element;
			}
		}

		if (topLeftElement)
			topLeftElement->SetFocus(true, true);
	}
	
	void GUINavGroup::RegisterElement(GUIElement* element, INT32 tabIdx)
	{
		mElements[element] = tabIdx;
		mOrderedElements.Insert(std::make_pair(tabIdx, element));
	}

	void GUINavGroup::SetIndex(GUIElement* element, INT32 tabIdx)
	{
		const auto iterFind = mElements.find(element);
		assert(iterFind != mElements.end());

		const INT32 existingTabIdx = iterFind->second;
		mElements[element] = tabIdx;

		const auto iterPair = mOrderedElements.equal_range(existingTabIdx);
		for(auto iter = iterPair.first; iter != iterPair.second; ++iter)
		{
			if(iter->second == element)
			{
				mOrderedElements.erase(iter);
				break;
			}
		}

		mOrderedElements.Insert(std::make_pair(tabIdx, element));
	}

	void GUINavGroup::UnregisterElement(GUIElement* element)
	{
		const auto iterFind = mElements.find(element);
		if(iterFind == mElements.end())
			return;

		const INT32 existingTabIdx = iterFind->second;
		const auto iterPair = mOrderedElements.equal_range(existingTabIdx);
		for(auto iter = iterPair.first; iter != iterPair.second; ++iter)
		{
			if(iter->second == element)
			{
				mOrderedElements.erase(iter);
				break;
			}
		}

		mElements.erase(element);
	}
}
