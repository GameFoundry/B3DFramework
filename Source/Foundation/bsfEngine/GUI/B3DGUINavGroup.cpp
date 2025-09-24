//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUINavGroup.h"
#include "GUI/BsGUIInteractable.h"
#include "GUI/BsGUIManager.h"

using namespace b3d;

SPtr<GUINavGroup> GUINavGroup::Create()
{
	return B3DMakeShared<GUINavGroup>();
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
	FocusTopLeft();
}

void GUINavGroup::FocusNext(GUIInteractable* anchor)
{
	// Nothing currently in focus
	if(!anchor)
	{
		FocusFirst();
		return;
	}

	const i32 tabIdx = mElements[anchor];

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
		FocusTopLeft();
		return;
	}

	// Find next element to focus on
	{
		const GUIPhysicalArea focusedElemBounds = anchor->GetAbsoluteClippedArea();

		// We look for the element to the right of the current element, within some Y range (a 'row').
		//// We search by rows in order to make the navigation perceptually nicer. Sometimes elements appear to be
		//// in the same row, but might be off by a few pixels, in which case the simpler approach would 'jump'
		//// over an element.
		constexpr static i32 kRowHeight = 5;

		const auto unindexedRange = mOrderedElements.equal_range(0);
		B3DMarkAllocatorFrame();
		{
			struct YCompare
			{
				bool operator()(const GUIInteractable* lhs, const GUIInteractable* rhs) const
				{
					const GUIPhysicalArea boundsLHS = lhs->GetAbsoluteClippedArea();
					const GUIPhysicalArea boundsRHS = rhs->GetAbsoluteClippedArea();

					if(boundsLHS.Y != boundsRHS.Y)
						return boundsLHS.Y < boundsRHS.Y;

					return lhs < rhs;
				}
			};

			// Build a list of relevant elements, ordered by height
			FrameSet<GUIInteractable*, YCompare> elements;
			for(auto iter = unindexedRange.first; iter != unindexedRange.second; ++iter)
			{
				GUIInteractable* element = iter->second;
				const bool acceptsKeyFocus = element->GetOptionFlags().IsSet(GUIElementOption::AcceptsKeyFocus);
				if(!acceptsKeyFocus || element->IsHidden() || element->IsDisabled())
					continue;

				const GUIPhysicalArea elemBounds = element->GetAbsoluteClippedArea();
				const bool isFullyClipped = elemBounds.Width == 0 || elemBounds.Height == 0;

				if(isFullyClipped)
					continue;

				elements.insert(element);
			}

			// Find the row the currently selected element is part of
			auto iterElem = elements.begin();
			auto iterRowStart = iterElem;

			GUIPhysicalUnit firstRowY = 0;
			GUIPhysicalUnit rowY = 0;
			for(; iterElem != elements.end(); ++iterElem)
			{
				GUIInteractable* element = *iterElem;

				const GUIPhysicalArea elemBounds = element->GetAbsoluteClippedArea();
				if(iterElem == elements.begin())
				{
					firstRowY = elemBounds.Y;
					rowY = elemBounds.Y;
				}
				else
				{
					const GUIPhysicalUnit yDiff = elemBounds.Y - rowY;

					// New row
					if(yDiff >= kRowHeight)
					{
						iterRowStart = iterElem;
						rowY = elemBounds.Y;
					}
				}

				if(element == anchor)
					break;
			}

			const bool foundRow = iterElem != elements.end();
			if(!foundRow)
				rowY = firstRowY;

			// Try to find the next element in the current row (to the right of the current one)
			GUIInteractable* nextElement = nullptr;
			GUIPhysicalUnit nearestX = std::numeric_limits<i32>::max();
			iterElem = iterRowStart;
			for(; iterElem != elements.end(); ++iterElem)
			{
				GUIInteractable* element = *iterElem;
				if(element == anchor)
					continue;

				const GUIPhysicalArea elemBounds = element->GetAbsoluteClippedArea();
				const GUIPhysicalUnit yDiff = elemBounds.Y - rowY;

				// New row
				if(yDiff >= kRowHeight)
				{
					rowY = elemBounds.Y;
					break;
				}

				// Note: We're purposely ignoring elements at the same exact position, as the tab focus would then just
				// ping-pong between the two elements, and we'd have to keep a list of previously visited elements in
				// order to avoid the issue.
				if(elemBounds.X > focusedElemBounds.X)
				{
					const GUIPhysicalUnit xDiff = elemBounds.X - focusedElemBounds.X;
					if(xDiff < nearestX)
					{
						nearestX = xDiff;
						nextElement = element;
					}
				}
			}

			// If no element in the current row, find the left-most element in the next row
			if(!nextElement)
			{
				nearestX = std::numeric_limits<i32>::max();
				for(; iterElem != elements.end(); ++iterElem)
				{
					GUIInteractable* element = *iterElem;

					const GUIPhysicalArea elemBounds = element->GetAbsoluteClippedArea();
					const GUIPhysicalUnit yDiff = elemBounds.Y - rowY;

					// New row
					if(yDiff >= kRowHeight)
						break;

					if(elemBounds.X < nearestX)
					{
						nearestX = elemBounds.X;
						nextElement = element;
					}
				}
			}

			if(nextElement)
			{
				nextElement->SetFocus(true, true);
				return;
			}
		}
		B3DClearAllocatorFrame();

		// No more elements with no tab index. Check elements with positive tab index
		const auto iterAfterUnindexed = unindexedRange.second;
		if(iterAfterUnindexed != mOrderedElements.end())
		{
			iterAfterUnindexed->second->SetFocus(true, true);
			return;
		}

		// Reached the end, wrap around
		FocusFirst();
	}
}

void GUINavGroup::FocusTopLeft()
{
	GUIPhysicalUnit lowestDist = std::numeric_limits<i32>::max();
	GUIInteractable* topLeftElement = nullptr;

	// Grab only elements without an explicit index
	const auto unindexedRange = mOrderedElements.equal_range(0);
	for(auto iter = unindexedRange.first; iter != unindexedRange.second; ++iter)
	{
		GUIInteractable* element = iter->second;

		// Ignore elements that are hidden, disabled or just don't accept input focus
		const bool acceptsKeyFocus = element->GetOptionFlags().IsSet(GUIElementOption::AcceptsKeyFocus);
		if(!acceptsKeyFocus || element->IsHidden() || element->IsDisabled())
			continue;

		// Ignore elements that have been fully clipped
		const GUIPhysicalArea elemBounds = element->GetAbsoluteClippedArea();
		if(elemBounds.Width == 0 || elemBounds.Height == 0)
			continue;

		GUIPhysicalPoint elementPos = elemBounds.GetPosition();

		const GUIPhysicalUnit dist = elementPos.SquaredLength();
		if(dist < lowestDist)
		{
			lowestDist = dist;
			topLeftElement = element;
		}
	}

	if(topLeftElement)
		topLeftElement->SetFocus(true, true);
}

void GUINavGroup::RegisterElement(GUIInteractable* element, i32 tabIdx)
{
	mElements[element] = tabIdx;
	mOrderedElements.insert(std::make_pair(tabIdx, element));
}

void GUINavGroup::SetIndex(GUIInteractable* element, i32 tabIdx)
{
	const auto iterFind = mElements.find(element);
	B3D_ASSERT(iterFind != mElements.end());

	const i32 existingTabIdx = iterFind->second;
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

void GUINavGroup::UnregisterElement(GUIInteractable* element)
{
	const auto iterFind = mElements.find(element);
	if(iterFind == mElements.end())
		return;

	const i32 existingTabIdx = iterFind->second;
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
