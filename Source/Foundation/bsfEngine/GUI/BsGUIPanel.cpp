//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIPanel.h"
#include "GUI/BsGUIElement.h"
#include "GUI/BsGUISpace.h"
#include "Math/BsMath.h"
#include "Math/BsVector2I.h"
#include "Profiling/BsProfilerCPU.h"

namespace bs
{
	GUIPanel::GUIPanel(INT16 depth, UINT16 depthRangeMin, UINT16 depthRangeMax, const GUIDimensions& dimensions)
		: GUILayout(dimensions), mDepthOffset(depth), mDepthRangeMin(depthRangeMin), mDepthRangeMax(depthRangeMax)
	{ }

	void GUIPanel::setDepthRange(INT16 depth, UINT16 depthRangeMin, UINT16 depthRangeMax)
	{
		mDepthOffset = depth;
		mDepthRangeMin = depthRangeMin;
		mDepthRangeMax = depthRangeMax;

		MarkLayoutAsDirtyInternal();
	}

	LayoutSizeRange GUIPanel::CalculateLayoutSizeRangeInternal() const
	{
		Vector2I optimalSize;
		Vector2I minSize;

		for (auto& child : mChildren)
		{
			if (!child->IsActiveInternal())
				continue;

			LayoutSizeRange sizeRange = child->CalculateLayoutSizeRangeInternal();

			if (child->GetTypeInternal() == GUIElementBase::Type::FixedSpace || child->GetTypeInternal() == GUIElementBase::Type::FlexibleSpace)
			{
				sizeRange.optimal.x = sizeRange.optimal.y = 0;
				sizeRange.min.x = sizeRange.min.y = 0;
			}

			UINT32 paddingX = child->GetPaddingInternal().left + child->GetPaddingInternal().right;
			UINT32 paddingY = child->GetPaddingInternal().top + child->GetPaddingInternal().bottom;

			Vector2I childMax;
			childMax.x = child->GetDimensionsInternal().x + sizeRange.optimal.x + paddingX;
			childMax.y = child->GetDimensionsInternal().y + sizeRange.optimal.y + paddingY;

			optimalSize.x = std::max(optimalSize.x, childMax.x);
			optimalSize.y = std::max(optimalSize.y, childMax.y);

			childMax.x = child->GetDimensionsInternal().x + sizeRange.min.x + paddingX;
			childMax.y = child->GetDimensionsInternal().y + sizeRange.min.y + paddingY;

			minSize.x = std::max(minSize.x, childMax.x);
			minSize.y = std::max(minSize.y, childMax.y);
		}

		LayoutSizeRange sizeRange = GetDimensionsInternal().calculateSizeRange(optimalSize);
		sizeRange.min.x = std::max(sizeRange.min.x, minSize.x);
		sizeRange.min.y = std::max(sizeRange.min.y, minSize.y);

		return sizeRange;
	}

	LayoutSizeRange GUIPanel::GetElementSizeRangeInternal(const GUIElementBase* element) const
	{
		if (element->GetTypeInternal() == GUIElementBase::Type::FixedSpace || element->GetTypeInternal() == GUIElementBase::Type::FlexibleSpace)
		{
			LayoutSizeRange sizeRange = element->GetLayoutSizeRangeInternal();
			sizeRange.optimal.x = 0;
			sizeRange.optimal.y = 0;
			sizeRange.min.x = 0;
			sizeRange.min.y = 0;

			return sizeRange;
		}

		return element->GetLayoutSizeRangeInternal();
	}

	void GUIPanel::UpdateOptimalLayoutSizesInternal()
	{
		// Update all children first, otherwise we can't determine our own optimal size
		GUIElementBase::UpdateOptimalLayoutSizesInternal();

		if (mChildren.size() != mChildSizeRanges.size())
			mChildSizeRanges.resize(mChildren.size());

		Vector2I optimalSize;
		Vector2I minSize;

		UINT32 childIdx = 0;
		for (auto& child : mChildren)
		{
			LayoutSizeRange& childSizeRange = mChildSizeRanges[childIdx];

			if (child->IsActiveInternal())
			{
				childSizeRange = GetElementSizeRangeInternal(child);

				UINT32 paddingX = child->GetPaddingInternal().left + child->GetPaddingInternal().right;
				UINT32 paddingY = child->GetPaddingInternal().top + child->GetPaddingInternal().bottom;

				Vector2I childMax;
				childMax.x = child->GetDimensionsInternal().x + childSizeRange.optimal.x + paddingX;
				childMax.y = child->GetDimensionsInternal().y + childSizeRange.optimal.y + paddingY;

				optimalSize.x = std::max(optimalSize.x, childMax.x);
				optimalSize.y = std::max(optimalSize.y, childMax.y);

				childMax.x = child->GetDimensionsInternal().x + childSizeRange.min.x + paddingX;
				childMax.y = child->GetDimensionsInternal().y + childSizeRange.min.y + paddingY;

				minSize.x = std::max(minSize.x, childMax.x);
				minSize.y = std::max(minSize.y, childMax.y);
			}
			else
				childSizeRange = LayoutSizeRange();

			childIdx++;
		}

		mSizeRange = GetDimensionsInternal().calculateSizeRange(optimalSize);
		mSizeRange.min.x = std::max(mSizeRange.min.x, minSize.x);
		mSizeRange.min.y = std::max(mSizeRange.min.y, minSize.y);
	}

	void GUIPanel::GetElementAreasInternal(const Rect2I& layoutArea, Rect2I* elementAreas, UINT32 numElements,
		const Vector<LayoutSizeRange>& sizeRanges, const LayoutSizeRange& mySizeRange) const
	{
		assert(mChildren.size() == numElements);

		// Panel always uses optimal sizes and explicit positions
		UINT32 childIdx = 0;
		for (auto& child : mChildren)
		{
			elementAreas[childIdx] = GetElementAreaInternal(layoutArea, child, sizeRanges[childIdx]);

			childIdx++;
		}
	}

	Rect2I GUIPanel::GetElementAreaInternal(const Rect2I& layoutArea, const GUIElementBase* element, const LayoutSizeRange& sizeRange) const
	{
		const GUIDimensions& dimensions = element->GetDimensionsInternal();

		Rect2I area;

		area.x = layoutArea.x + dimensions.x;
		area.y = layoutArea.y + dimensions.y;

		if (dimensions.fixedWidth())
			area.width = (UINT32)sizeRange.optimal.x;
		else
		{
			UINT32 modifiedWidth = (UINT32)std::max(0, (INT32)layoutArea.width - dimensions.x);

			if (modifiedWidth > (UINT32)sizeRange.optimal.x)
			{
				if (sizeRange.max.x > 0)
					modifiedWidth = std::min(modifiedWidth, (UINT32)sizeRange.max.x);
			}
			else if (modifiedWidth < (UINT32)sizeRange.optimal.x)
			{
				if (sizeRange.min.x > 0)
					modifiedWidth = std::max(modifiedWidth, (UINT32)sizeRange.min.x);
			}

			area.width = modifiedWidth;
		}

		if (dimensions.fixedHeight())
			area.height = (UINT32)sizeRange.optimal.y;
		else
		{
			UINT32 modifiedHeight = (UINT32)std::max(0, (INT32)layoutArea.height - dimensions.y);

			if (modifiedHeight > (UINT32)sizeRange.optimal.y)
			{
				if (sizeRange.max.y > 0)
					modifiedHeight = std::min(modifiedHeight, (UINT32)sizeRange.max.y);
			}
			else if (modifiedHeight < (UINT32)sizeRange.optimal.y)
			{
				if (sizeRange.min.y > 0)
					modifiedHeight = std::max(modifiedHeight, (UINT32)sizeRange.min.y);
			}

			area.height = modifiedHeight;
		}

		return area;
	}

	void GUIPanel::UpdateDepthRangeInternal(GUILayoutData& data)
	{
		INT32 newPanelDepth = data.getPanelDepth() + mDepthOffset;
		INT32 newPanelDepthRangeMin = newPanelDepth - mDepthRangeMin;
		INT32 newPanelDepthRangeMax = newPanelDepth + mDepthRangeMax;

		INT32* allDepths[3] = { &newPanelDepth, &newPanelDepthRangeMin, &newPanelDepthRangeMax };

		for (auto& depth : allDepths)
		{
			INT32 minValue = std::max((INT32)data.getPanelDepth() - (INT32)data.depthRangeMin, (INT32)std::numeric_limits<INT16>::min());
			*depth = std::max(*depth, minValue);

			INT32 maxValue = std::min((INT32)data.getPanelDepth() + (INT32)data.depthRangeMax, (INT32)std::numeric_limits<INT16>::max());
			*depth = std::min(*depth, maxValue);
		}

		data.setPanelDepth((INT16)newPanelDepth);

		if (mDepthRangeMin != (UINT16)-1 || data.depthRangeMin != (UINT16)-1)
			data.depthRangeMin = (UINT16)(newPanelDepth - newPanelDepthRangeMin);

		if (mDepthRangeMax != (UINT16)-1 || data.depthRangeMax != (UINT16)-1)
			data.depthRangeMax = (UINT16)(newPanelDepthRangeMax - newPanelDepth);
	}

	void GUIPanel::UpdateLayoutInternalInternal(const GUILayoutData& data)
	{
		GUILayoutData childData = data;
		UpdateDepthRangeInternal(childData);

		UINT32 numElements = (UINT32)mChildren.size();
		Rect2I* elementAreas = nullptr;

		if (numElements > 0)
			elementAreas = bs_stack_new<Rect2I>(numElements);

		GetElementAreasInternal(data.area, elementAreas, numElements, mChildSizeRanges, mSizeRange);

		UINT32 childIdx = 0;

		for (auto& child : mChildren)
		{
			if (child->IsActiveInternal())
			{
				childData.area = elementAreas[childIdx];

				UpdateChildLayoutInternal(child, childData);
			}

			childIdx++;
		}

		if (elementAreas != nullptr)
			bs_stack_free(elementAreas);
	}

	void GUIPanel::UpdateChildLayoutInternal(GUIElementBase* element, const GUILayoutData& data)
	{
		GUILayoutData childData = data;

		childData.clipRect = data.area;
		childData.clipRect.clip(data.clipRect);

		element->SetLayoutDataInternal(childData);
		element->UpdateLayoutInternalInternal(childData);
	}

	GUIPanel* GUIPanel::create(INT16 depth, UINT16 depthRangeMin, UINT16 depthRangeMax)
	{
		return bs_new<GUIPanel>(depth, depthRangeMin, depthRangeMax, GUIDimensions::create());
	}

	GUIPanel* GUIPanel::create(const GUIOptions& options)
	{
		return bs_new<GUIPanel>(0, -1, -1, GUIDimensions::create(options));
	}

	GUIPanel* GUIPanel::create(INT16 depth, UINT16 depthRangeMin, UINT16 depthRangeMax, const GUIOptions& options)
	{
		return bs_new<GUIPanel>(depth, depthRangeMin, depthRangeMax, GUIDimensions::create(options));
	}
}
