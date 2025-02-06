//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIPanel.h"
#include "GUI/BsGUIInteractable.h"
#include "GUI/BsGUISpace.h"
#include "Math/BsMath.h"
#include "Profiling/BsProfilerCPU.h"
#include "Reflection/BsRTTIType.h"

using namespace bs;

/** @cond RTTI */
/** @addtogroup RTTI-Impl-Engine
 *  @{
 */

namespace bs
{
	class B3D_EXPORT GUIPanelRTTI : public TRTTIType<GUIPanel, GUILayout, GUIPanelRTTI>
	{
	public:
		const String& GetRttiName()
		{
			static String name = "GUIPanel";
			return name;
		}

		u32 GetRttiId() const override { return TID_GUIPanel; }

		SPtr<IReflectable> NewRttiObject() { return nullptr; }
	};
} // namespace bs

/** @} */
/** @endcond */

GUIPanel::GUIPanel(i16 depth, u16 depthRangeMin, u16 depthRangeMax, const GUISizeConstraints& dimensions)
	: GUILayout(dimensions), mDepthOffset(depth), mDepthRangeMin(depthRangeMin), mDepthRangeMax(depthRangeMax)
{}

void GUIPanel::SetDepthRange(i16 depth, u16 depthRangeMin, u16 depthRangeMax)
{
	mDepthOffset = depth;
	mDepthRangeMin = depthRangeMin;
	mDepthRangeMax = depthRangeMax;

	MarkLayoutAsDirty();
}

GUIConstrainedSize GUIPanel::GetChildElementSizeRange(const GUIElement* element) const
{
	if(element->GetType() == GUIElement::Type::FixedSpace || element->GetType() == GUIElement::Type::FlexibleSpace)
	{
		GUIConstrainedSize sizeRange = element->GetConstrainedSize();
		sizeRange.Optimal.X = 0;
		sizeRange.Optimal.Y = 0;
		sizeRange.Min.X = 0;
		sizeRange.Min.Y = 0;

		return sizeRange;
	}

	return element->GetConstrainedSize();
}

void GUIPanel::UpdateOptimalLayoutSizes()
{
	// Update all children first, otherwise we can't determine our own optimal size
	GUIElement::UpdateOptimalLayoutSizes();

	if(mChildren.size() != mChildrenConstrainedSizes.size())
		mChildrenConstrainedSizes.resize(mChildren.size());

	Vector2I optimalSize(BsZero);
	Vector2I minSize(BsZero);

	u32 childIdx = 0;
	for(auto& child : mChildren)
	{
		GUIConstrainedSize& childSizeRange = mChildrenConstrainedSizes[childIdx];

		if(child->IsActive())
		{
			childSizeRange = GetChildElementSizeRange(child);

			const u32 marginsX = child->GetMargins().Left + child->GetMargins().Right;
			const u32 marginsY = child->GetMargins().Top + child->GetMargins().Bottom;

			Vector2I childMax;
			childMax.X = child->GetSizeConstraints().X + childSizeRange.Optimal.X + marginsX;
			childMax.Y = child->GetSizeConstraints().Y + childSizeRange.Optimal.Y + marginsY;

			optimalSize.X = std::max(optimalSize.X, childMax.X);
			optimalSize.Y = std::max(optimalSize.Y, childMax.Y);

			childMax.X = child->GetSizeConstraints().X + childSizeRange.Min.X + marginsX;
			childMax.Y = child->GetSizeConstraints().Y + childSizeRange.Min.Y + marginsY;

			minSize.X = std::max(minSize.X, childMax.X);
			minSize.Y = std::max(minSize.Y, childMax.Y);
		}
		else
			childSizeRange = GUIConstrainedSize();

		childIdx++;
	}

	mConstrainedSize = GetSizeConstraints().CalculateConstrainedSize(optimalSize);
	mConstrainedSize.Min.X = std::max(mConstrainedSize.Min.X, minSize.X);
	mConstrainedSize.Min.Y = std::max(mConstrainedSize.Min.Y, minSize.Y);
}

void GUIPanel::GetChildRelativeLayoutAreas(const Size2UI& layoutSize, Vector2I* outElementPositions, Size2UI* outElementSizes, u32 elementCount, const Vector<GUIConstrainedSize>& sizeRanges) const
{
	B3D_ASSERT(mChildren.size() == elementCount);

	// Panel always uses optimal sizes and explicit positions
	u32 childIdx = 0;
	for(auto& child : mChildren)
	{
		const Rect2I childElementArea = CalculateRelativeElementArea(layoutSize, child, sizeRanges[childIdx]);

		outElementPositions[childIdx] = Vector2I(childElementArea.X, childElementArea.Y);
		outElementSizes[childIdx] = Size2UI(childElementArea.Width, childElementArea.Height);

		childIdx++;
	}
}

Rect2I GUIPanel::CalculateRelativeElementArea(const Size2UI& layoutSize, const GUIElement* element, const GUIConstrainedSize& sizeRange) const
{
	const GUISizeConstraints& dimensions = element->GetSizeConstraints();

	Rect2I area;

	area.X = dimensions.X;
	area.Y = dimensions.Y;

	if(dimensions.IsWidthFixed())
		area.Width = (u32)sizeRange.Optimal.X;
	else
	{
		u32 modifiedWidth = (u32)std::max(0, (i32)layoutSize.Width - dimensions.X);

		if(modifiedWidth > (u32)sizeRange.Optimal.X)
		{
			if(sizeRange.Max.X > 0)
				modifiedWidth = std::min(modifiedWidth, (u32)sizeRange.Max.X);
		}
		else if(modifiedWidth < (u32)sizeRange.Optimal.X)
		{
			if(sizeRange.Min.X > 0)
				modifiedWidth = std::max(modifiedWidth, (u32)sizeRange.Min.X);
		}

		area.Width = modifiedWidth;
	}

	if(dimensions.IsHeightFixed())
		area.Height = (u32)sizeRange.Optimal.Y;
	else
	{
		u32 modifiedHeight = (u32)std::max(0, (i32)layoutSize.Height - dimensions.Y);

		if(modifiedHeight > (u32)sizeRange.Optimal.Y)
		{
			if(sizeRange.Max.Y > 0)
				modifiedHeight = std::min(modifiedHeight, (u32)sizeRange.Max.Y);
		}
		else if(modifiedHeight < (u32)sizeRange.Optimal.Y)
		{
			if(sizeRange.Min.Y > 0)
				modifiedHeight = std::max(modifiedHeight, (u32)sizeRange.Min.Y);
		}

		area.Height = modifiedHeight;
	}

	return area;
}

void GUIPanel::UpdateDepthRangeInternal(GUILayoutData& data)
{
	i32 newPanelDepth = data.GetPanelDepth() + mDepthOffset;
	i32 newPanelDepthRangeMin = newPanelDepth - mDepthRangeMin;
	i32 newPanelDepthRangeMax = newPanelDepth + mDepthRangeMax;

	i32* allDepths[3] = { &newPanelDepth, &newPanelDepthRangeMin, &newPanelDepthRangeMax };

	for(auto& depth : allDepths)
	{
		i32 minValue = std::max((i32)data.GetPanelDepth() - (i32)data.DepthRangeMin, (i32)std::numeric_limits<i16>::min());
		*depth = std::max(*depth, minValue);

		i32 maxValue = std::min((i32)data.GetPanelDepth() + (i32)data.DepthRangeMax, (i32)std::numeric_limits<i16>::max());
		*depth = std::min(*depth, maxValue);
	}

	data.SetPanelDepth((i16)newPanelDepth);

	if(mDepthRangeMin != (u16)-1 || data.DepthRangeMin != (u16)-1)
		data.DepthRangeMin = (u16)(newPanelDepth - newPanelDepthRangeMin);

	if(mDepthRangeMax != (u16)-1 || data.DepthRangeMax != (u16)-1)
		data.DepthRangeMax = (u16)(newPanelDepthRangeMax - newPanelDepth);
}

void GUIPanel::UpdateLayoutForChildren()
{
	GUILayoutData childData = mLayoutData;
	UpdateDepthRangeInternal(childData);

	u32 elementCount = (u32)mChildren.size();
	Vector2I* elementPositions = nullptr;
	Size2UI* elementSizes = nullptr;

	if(elementCount > 0)
	{
		elementPositions = B3DStackNew<Vector2I>(elementCount);
		elementSizes = B3DStackNew<Size2UI>(elementCount);
	}

	GetChildRelativeLayoutAreas(mLayoutData.Size, elementPositions, elementSizes, elementCount, mChildrenConstrainedSizes);

	u32 childIdx = 0;

	for(auto& child : mChildren)
	{
		if(child->IsActive())
		{
			childData.RelativePosition = elementPositions[childIdx];
			childData.Size = elementSizes[childIdx];

			child->SetLayoutData(childData);
			child->UpdateLayoutForChildren();
		}

		childIdx++;
	}

	if(elementSizes != nullptr)
		B3DStackFree(elementSizes);

	if(elementPositions != nullptr)
		B3DStackFree(elementPositions);

	if(mIsCullingEnabled)
		RebuildQuadTree();
}

GUIPanel* GUIPanel::Create(i16 depth, u16 depthRangeMin, u16 depthRangeMax)
{
	return B3DNew<GUIPanel>(depth, depthRangeMin, depthRangeMax, GUISizeConstraints::Create());
}

GUIPanel* GUIPanel::Create(const GUIOptions& options)
{
	return B3DNew<GUIPanel>(0, -1, -1, GUISizeConstraints::Create(options));
}

GUIPanel* GUIPanel::Create(i16 depth, u16 depthRangeMin, u16 depthRangeMax, const GUIOptions& options)
{
	return B3DNew<GUIPanel>(depth, depthRangeMin, depthRangeMax, GUISizeConstraints::Create(options));
}

RTTIType* GUIPanel::GetRttiStatic()
{
	return GUIPanelRTTI::Instance();
}

RTTIType* GUIPanel::GetRtti() const
{
	return GetRttiStatic();
}
