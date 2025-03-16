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
		sizeRange.Optimal.Width = 0;
		sizeRange.Optimal.Height = 0;
		sizeRange.Minimum.Width = 0;
		sizeRange.Minimum.Height = 0;

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

	GUILogicalSize optimalSize(BsZero);
	GUILogicalSize minSize(BsZero);

	u32 childIdx = 0;
	for(auto& child : mChildren)
	{
		GUIConstrainedSize& childSizeRange = mChildrenConstrainedSizes[childIdx];

		if(child->IsActive())
		{
			childSizeRange = GetChildElementSizeRange(child);

			const GUILogicalUnit marginsX = child->GetMargins().Left + child->GetMargins().Right;
			const GUILogicalUnit marginsY = child->GetMargins().Top + child->GetMargins().Bottom;

			GUILogicalPoint childMax;
			childMax.X = child->GetSizeConstraints().ExplicitPosition.X + childSizeRange.Optimal.Width + marginsX;
			childMax.Y = child->GetSizeConstraints().ExplicitPosition.Y + childSizeRange.Optimal.Height + marginsY;

			optimalSize.Width = Math::Max(optimalSize.Width, childMax.X);
			optimalSize.Height = Math::Max(optimalSize.Height, childMax.Y);

			childMax.X = child->GetSizeConstraints().ExplicitPosition.X + childSizeRange.Minimum.Width + marginsX;
			childMax.Y = child->GetSizeConstraints().ExplicitPosition.Y + childSizeRange.Minimum.Height + marginsY;

			minSize.Width = Math::Max(minSize.Width, childMax.X);
			minSize.Height = Math::Max(minSize.Height, childMax.Y);
		}
		else
			childSizeRange = GUIConstrainedSize();

		childIdx++;
	}

	mConstrainedSize = GetSizeConstraints().CalculateConstrainedSize(optimalSize);
	mConstrainedSize.Minimum.Width = std::max(mConstrainedSize.Minimum.Width, minSize.Width);
	mConstrainedSize.Minimum.Height = std::max(mConstrainedSize.Minimum.Height, minSize.Height);
}

void GUIPanel::GetChildRelativeLayoutAreas(const GUILogicalSize& layoutSize, GUILogicalPoint* outElementPositions, GUILogicalSize* outElementSizes, u32 elementCount, const Vector<GUIConstrainedSize>& sizeRanges) const
{
	B3D_ASSERT(mChildren.size() == elementCount);

	// Panel always uses optimal sizes and explicit positions
	u32 childIdx = 0;
	for(auto& child : mChildren)
	{
		const GUILogicalArea childElementArea = CalculateRelativeElementArea(layoutSize, child, sizeRanges[childIdx]);

		outElementPositions[childIdx] = GUILogicalPoint(childElementArea.X, childElementArea.Y);
		outElementSizes[childIdx] = GUILogicalSize(childElementArea.Width, childElementArea.Height);

		childIdx++;
	}
}

GUILogicalArea GUIPanel::CalculateRelativeElementArea(const GUILogicalSize& layoutSize, const GUIElement* element, const GUIConstrainedSize& sizeRange) const
{
	const GUISizeConstraints& sizeConstraints = element->GetSizeConstraints();

	GUILogicalArea area;

	area.X = sizeConstraints.ExplicitPosition.X;
	area.Y = sizeConstraints.ExplicitPosition.Y;

	if(sizeConstraints.IsWidthFixed())
		area.Width = sizeRange.Optimal.Width;
	else
	{
		GUILogicalUnit modifiedWidth = Math::Max(layoutSize.Width - sizeConstraints.ExplicitPosition.X, 0);

		if(modifiedWidth > sizeRange.Optimal.Width)
		{
			if(sizeRange.Maximum.Width > 0)
				modifiedWidth = Math::Min(modifiedWidth, sizeRange.Maximum.Width);
		}
		else if(modifiedWidth < sizeRange.Optimal.Width)
		{
			if(sizeRange.Minimum.Width > 0)
				modifiedWidth = Math::Max(modifiedWidth, sizeRange.Minimum.Width);
		}

		area.Width = modifiedWidth;
	}

	if(sizeConstraints.IsHeightFixed())
		area.Height = sizeRange.Optimal.Height;
	else
	{
		GUILogicalUnit modifiedHeight = Math::Max(layoutSize.Height - sizeConstraints.ExplicitPosition.Y, 0);

		if(modifiedHeight > sizeRange.Optimal.Height)
		{
			if(sizeRange.Maximum.Height > 0)
				modifiedHeight = Math::Min(modifiedHeight, sizeRange.Maximum.Height);
		}
		else if(modifiedHeight < sizeRange.Optimal.Height)
		{
			if(sizeRange.Minimum.Height > 0)
				modifiedHeight = Math::Max(modifiedHeight, sizeRange.Minimum.Height);
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
	GUILogicalPoint* elementPositions = nullptr;
	GUILogicalSize* elementSizes = nullptr;

	if(elementCount > 0)
	{
		elementPositions = B3DStackNew<GUILogicalPoint>(elementCount);
		elementSizes = B3DStackNew<GUILogicalSize>(elementCount);
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

	if(mCulling != nullptr)
		mCulling->RebuildQuadTree(mChildren);
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
