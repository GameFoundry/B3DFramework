//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUINavGroup.h"
#include "GUI/B3DGUIInteractable.h"
#include "GUI/B3DGUIManager.h"

using namespace b3d;

TShared<GUINavGroup> GUINavGroup::Create()
{
	return B3DMakeShared<GUINavGroup>();
}

void GUINavGroup::FocusFirst()
{
	if(mOrderedElements.empty())
		return;

	// Find first element with an explicit index, if one exists
	auto it = mOrderedElements.begin();
	if(it->first != 0)
	{
		it->second->SetFocus(true, true);
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

	const i32 tabIndex = mElements[anchor];

	// Find next element using the explicit index
	if(tabIndex != 0)
	{
		auto found = mOrderedElements.lower_bound(tabIndex);
		while(found->second != anchor)
			++found;

		++found;

		// Reached the end, wrap around
		if(found == mOrderedElements.end())
			return FocusFirst();

		// If a next element with an explicit index exists, select it
		if(found->first != 0)
		{
			found->second->SetFocus(true, true);
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
			for(auto it = unindexedRange.first; it != unindexedRange.second; ++it)
			{
				GUIInteractable* element = it->second;
				const bool acceptsKeyFocus = element->GetOptionFlags().IsSet(GUIElementOption::AcceptsKeyFocus);
				if(!acceptsKeyFocus || element->IsHidden() || element->IsDisabled())
					continue;

				const GUIPhysicalArea elementBounds = element->GetAbsoluteClippedArea();
				const bool isFullyClipped = elementBounds.Width == 0 || elementBounds.Height == 0;

				if(isFullyClipped)
					continue;

				elements.insert(element);
			}

			// Find the row the currently selected element is part of
			auto elementIt = elements.begin();
			auto rowStartIt = elementIt;

			GUIPhysicalUnit firstRowY = 0;
			GUIPhysicalUnit rowY = 0;
			for(; elementIt != elements.end(); ++elementIt)
			{
				GUIInteractable* element = *elementIt;

				const GUIPhysicalArea elementBounds = element->GetAbsoluteClippedArea();
				if(elementIt == elements.begin())
				{
					firstRowY = elementBounds.Y;
					rowY = elementBounds.Y;
				}
				else
				{
					const GUIPhysicalUnit yDiff = elementBounds.Y - rowY;

					// New row
					if(yDiff >= kRowHeight)
					{
						rowStartIt = elementIt;
						rowY = elementBounds.Y;
					}
				}

				if(element == anchor)
					break;
			}

			const bool foundRow = elementIt != elements.end();
			if(!foundRow)
				rowY = firstRowY;

			// Try to find the next element in the current row (to the right of the current one)
			GUIInteractable* nextElement = nullptr;
			GUIPhysicalUnit nearestX = std::numeric_limits<i32>::max();
			elementIt = rowStartIt;
			for(; elementIt != elements.end(); ++elementIt)
			{
				GUIInteractable* element = *elementIt;
				if(element == anchor)
					continue;

				const GUIPhysicalArea elementBounds = element->GetAbsoluteClippedArea();
				const GUIPhysicalUnit yDiff = elementBounds.Y - rowY;

				// New row
				if(yDiff >= kRowHeight)
				{
					rowY = elementBounds.Y;
					break;
				}

				// Note: We're purposely ignoring elements at the same exact position, as the tab focus would then just
				// ping-pong between the two elements, and we'd have to keep a list of previously visited elements in
				// order to avoid the issue.
				if(elementBounds.X > focusedElemBounds.X)
				{
					const GUIPhysicalUnit xDiff = elementBounds.X - focusedElemBounds.X;
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
				for(; elementIt != elements.end(); ++elementIt)
				{
					GUIInteractable* element = *elementIt;

					const GUIPhysicalArea elementBounds = element->GetAbsoluteClippedArea();
					const GUIPhysicalUnit yDiff = elementBounds.Y - rowY;

					// New row
					if(yDiff >= kRowHeight)
						break;

					if(elementBounds.X < nearestX)
					{
						nearestX = elementBounds.X;
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
		const auto afterUnindexedIt = unindexedRange.second;
		if(afterUnindexedIt != mOrderedElements.end())
		{
			afterUnindexedIt->second->SetFocus(true, true);
			return;
		}

		// Reached the end, wrap around
		FocusFirst();
	}
}

void GUINavGroup::FocusTopLeft()
{
	GUIPhysicalUnit lowestDistance = std::numeric_limits<i32>::max();
	GUIInteractable* topLeftElement = nullptr;

	// Grab only elements without an explicit index
	const auto unindexedRange = mOrderedElements.equal_range(0);
	for(auto it = unindexedRange.first; it != unindexedRange.second; ++it)
	{
		GUIInteractable* element = it->second;

		// Ignore elements that are hidden, disabled or just don't accept input focus
		const bool acceptsKeyFocus = element->GetOptionFlags().IsSet(GUIElementOption::AcceptsKeyFocus);
		if(!acceptsKeyFocus || element->IsHidden() || element->IsDisabled())
			continue;

		// Ignore elements that have been fully clipped
		const GUIPhysicalArea elementBounds = element->GetAbsoluteClippedArea();
		if(elementBounds.Width == 0 || elementBounds.Height == 0)
			continue;

		GUIPhysicalPoint elementPosition = elementBounds.GetPosition();

		const GUIPhysicalUnit distance = elementPosition.SquaredLength();
		if(distance < lowestDistance)
		{
			lowestDistance = distance;
			topLeftElement = element;
		}
	}

	if(topLeftElement)
		topLeftElement->SetFocus(true, true);
}

void GUINavGroup::RegisterElement(GUIInteractable* element, i32 tabIndex)
{
	mElements[element] = tabIndex;
	mOrderedElements.insert(std::make_pair(tabIndex, element));
}

void GUINavGroup::SetIndex(GUIInteractable* element, i32 tabIndex)
{
	const auto found = mElements.find(element);
	B3D_ASSERT(found != mElements.end());

	const i32 existingTabIndex = found->second;
	mElements[element] = tabIndex;

	const auto range = mOrderedElements.equal_range(existingTabIndex);
	for(auto it = range.first; it != range.second; ++it)
	{
		if(it->second == element)
		{
			mOrderedElements.erase(it);
			break;
		}
	}

	mOrderedElements.insert(std::make_pair(tabIndex, element));
}

void GUINavGroup::UnregisterElement(GUIInteractable* element)
{
	const auto found = mElements.find(element);
	if(found == mElements.end())
		return;

	const i32 existingTabIndex = found->second;
	const auto range = mOrderedElements.equal_range(existingTabIndex);
	for(auto it = range.first; it != range.second; ++it)
	{
		if(it->second == element)
		{
			mOrderedElements.erase(it);
			break;
		}
	}

	mElements.erase(element);
}
