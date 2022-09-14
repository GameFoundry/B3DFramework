//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUINavGroup.h"
#include "GUI/BsGUIElement.h"
#include "GUI/BsGUIManager.h"

namespace bs
{
	SPtr<GUINavGroup> GUINavGroup::create()
	{
		return bs_shared_ptr_new<GUINavGroup>();
	}

	void GUINavGroup::focusFirst()
	{
		if(mOrderedElements.empty())
			return;

		// Find first element with an explicit index, if one exists
		auto iterStart = mOrderedElements.begin();
		if(iterStart->first != 0)
		{
			iterStart->second->setFocus(true, true);
			return;
		}

		// Otherwise look for top-left element without an explicit index
		focusTopLeft();
	}

	void GUINavGroup::focusNext(GUIElement* anchor)
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
				return focusFirst();

			// If a next element with an explicit index exists, select it
			if(iterFind->first != 0)
			{
				iterFind->second->setFocus(true, true);
				return;
			}

			// Select top-left element with no tab index
			focusTopLeft();
			return;
		}

		// Find next element to focus on
		{
			const Rect2I focusedElemBounds = anchor->GetClippedBoundsInternal();

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
						const Rect2I boundsLHS = lhs->GetClippedBoundsInternal();
						const Rect2I boundsRHS = rhs->GetClippedBoundsInternal();

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
					const bool acceptsKeyFocus = element->getOptionFlags().isSet(GUIElementOption::AcceptsKeyFocus);
					if (!acceptsKeyFocus || !element->IsVisibleInternal() || element->IsDisabledInternal())
						continue;

					const Rect2I elemBounds = element->GetClippedBoundsInternal();
					const bool isFullyClipped = elemBounds.width == 0 || elemBounds.height == 0;

					if (isFullyClipped)
						continue;

					elements.insert(element);
				}

				// Find the row the currently selected element is part of
				auto iterElem = elements.begin();
				auto iterRowStart = iterElem;

				INT32 firstRowY = 0;
				INT32 rowY = 0;
				for(; iterElem != elements.end(); ++iterElem)
				{
					GUIElement* element = *iterElem;

					const Rect2I elemBounds = element->GetClippedBoundsInternal();
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

					const Rect2I elemBounds = element->GetClippedBoundsInternal();
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

						const Rect2I elemBounds = element->GetClippedBoundsInternal();
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
					nextElement->setFocus(true, true);
					return;
				}

			}
			bs_frame_clear();

			// No more elements with no tab index. Check elements with positive tab index
			const auto iterAfterUnindexed = unindexedRange.second;
			if(iterAfterUnindexed != mOrderedElements.end())
			{
				iterAfterUnindexed->second->setFocus(true, true);
				return;
			}

			// Reached the end, wrap around
			focusFirst();
		}
	}

	void GUINavGroup::focusTopLeft()
	{
		UINT32 lowestDist = std::numeric_limits<UINT32>::max();
		GUIElement* topLeftElement = nullptr;

		// Grab only elements without an explicit index
		const auto unindexedRange = mOrderedElements.equal_range(0);
		for(auto iter = unindexedRange.first; iter != unindexedRange.second; ++iter)
		{
			GUIElement* element = iter->second;

			// Ignore elements that are hidden, disabled or just don't accept input focus
			const bool acceptsKeyFocus = element->getOptionFlags().isSet(GUIElementOption::AcceptsKeyFocus);
			if(!acceptsKeyFocus || !element->IsVisibleInternal() || element->IsDisabledInternal())
				continue;

			// Ignore elements that have been fully clipped
			const Rect2I elemBounds = element->GetClippedBoundsInternal();
			if(elemBounds.width == 0 || elemBounds.height == 0)
				continue;

			Vector2I elementPos(elemBounds.x, elemBounds.y);

			const UINT32 dist = elementPos.squaredLength();
			if (dist < lowestDist)
			{
				lowestDist = dist;
				topLeftElement = element;
			}
		}

		if (topLeftElement)
			topLeftElement->setFocus(true, true);
	}
	
	void GUINavGroup::registerElement(GUIElement* element, INT32 tabIdx)
	{
		mElements[element] = tabIdx;
		mOrderedElements.insert(std::make_pair(tabIdx, element));
	}

	void GUINavGroup::setIndex(GUIElement* element, INT32 tabIdx)
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

		mOrderedElements.insert(std::make_pair(tabIdx, element));
	}

	void GUINavGroup::unregisterElement(GUIElement* element)
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
