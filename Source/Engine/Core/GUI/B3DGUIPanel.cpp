//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIPanel.h"
#include "GUI/B3DGUIInteractable.h"
#include "GUI/B3DGUISpace.h"
#include "Math/B3DMath.h"
#include "Profiling/B3DProfilerCPU.h"
#include "Reflection/B3DRTTIType.h"

using namespace b3d;

/** @cond RTTI */
/** @addtogroup RTTI-Impl-Engine
 *  @{
 */

namespace b3d
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

		TShared<IReflectable> NewRttiObject() { return nullptr; }
	};
} // namespace b3d

/** @} */
/** @endcond */

GUIPanel::GUIPanel(PrivatelyConstruct, const GUIPanelContent& content, const String& styleClass, const GUISizeConstraints& sizeConstraints)
	: GUILayout(styleClass, sizeConstraints), mDepthOffset(content.Depth), mDepthRangeMin(content.DepthRangeMaximum), mDepthRangeMax(content.DepthRangeMaximum)
{}

const String& GUIPanel::GetGuiTypeName()
{
	static String kName = "GUIPanel";
	return kName;
}

void GUIPanel::SetDepthRange(i16 depth, u16 depthRangeMinimum, u16 depthRangeMaximum)
{
	mDepthOffset = depth;
	mDepthRangeMin = depthRangeMinimum;
	mDepthRangeMax = depthRangeMaximum;

	MarkLayoutAsDirty();
}

GUIConstrainedSizeRange GUIPanel::GetChildConstrainedSizeRange(const GUIElement* element) const
{
	if(element->Is<GUIFixedSpace>() || element->Is<GUIFlexibleSpace>())
	{
		GUIConstrainedSizeRange sizeRange = element->GetConstrainedSizeRange();
		sizeRange.Optimal.Width = 0;
		sizeRange.Optimal.Height = 0;
		sizeRange.Minimum.Width = 0;
		sizeRange.Minimum.Height = 0;

		return sizeRange;
	}

	return element->GetConstrainedSizeRange();
}

void GUIPanel::UpdateOptimalLayoutSizes()
{
	// Update all children first, otherwise we can't determine our own optimal size
	GUIElement::UpdateOptimalLayoutSizes();

	if(mChildren.size() != mChildConstrainedSizeRanges.size())
		mChildConstrainedSizeRanges.resize(mChildren.size());

	GUILogicalSize optimalSize(kZeroTag);
	GUILogicalSize minimumSize(kZeroTag);

	u32 childIndex = 0;
	for(auto& child : mChildren)
	{
		GUIConstrainedSizeRange& childSizeRange = mChildConstrainedSizeRanges[childIndex];

		if(child->IsActive())
		{
			childSizeRange = GetChildConstrainedSizeRange(child);

			const GUILogicalUnit marginsHorizontal = child->GetMargins().Left + child->GetMargins().Right;
			const GUILogicalUnit marginsVertical = child->GetMargins().Top + child->GetMargins().Bottom;

			GUILogicalPoint childMaximum;
			childMaximum.X = child->GetSizeConstraints().ExplicitPosition.X + childSizeRange.Optimal.Width + marginsHorizontal;
			childMaximum.Y = child->GetSizeConstraints().ExplicitPosition.Y + childSizeRange.Optimal.Height + marginsVertical;

			optimalSize.Width = Math::Max(optimalSize.Width, childMaximum.X);
			optimalSize.Height = Math::Max(optimalSize.Height, childMaximum.Y);

			childMaximum.X = child->GetSizeConstraints().ExplicitPosition.X + childSizeRange.Minimum.Width + marginsHorizontal;
			childMaximum.Y = child->GetSizeConstraints().ExplicitPosition.Y + childSizeRange.Minimum.Height + marginsVertical;

			minimumSize.Width = Math::Max(minimumSize.Width, childMaximum.X);
			minimumSize.Height = Math::Max(minimumSize.Height, childMaximum.Y);
		}
		else
			childSizeRange = GUIConstrainedSizeRange();

		childIndex++;
	}

	mConstrainedSizeRange = GetSizeConstraints().CalculateConstrainedSizeRange(optimalSize);
	mConstrainedSizeRange.Minimum.Width = std::max(mConstrainedSizeRange.Minimum.Width, minimumSize.Width);
	mConstrainedSizeRange.Minimum.Height = std::max(mConstrainedSizeRange.Minimum.Height, minimumSize.Height);
}

void GUIPanel::GetChildRelativeLayoutAreas(const GUILogicalSize& layoutSize, GUILogicalPoint* outElementPositions, GUILogicalSize* outElementSizes, u32 elementCount, const Vector<GUIConstrainedSizeRange>& sizeRanges) const
{
	B3D_ASSERT(mChildren.size() == elementCount);

	// Panel always uses optimal sizes and explicit positions
	u32 childIndex = 0;
	for(auto& child : mChildren)
	{
		const GUILogicalArea childElementArea = CalculateRelativeElementArea(layoutSize, child, sizeRanges[childIndex]);

		outElementPositions[childIndex] = GUILogicalPoint(childElementArea.X, childElementArea.Y);
		outElementSizes[childIndex] = GUILogicalSize(childElementArea.Width, childElementArea.Height);

		childIndex++;
	}
}

GUILogicalArea GUIPanel::CalculateRelativeElementArea(const GUILogicalSize& layoutSize, const GUIElement* element, const GUIConstrainedSizeRange& sizeRange) const
{
	const GUISizeConstraints& sizeConstraints = element->GetSizeConstraints();

	const GUILogicalSize adjustedSize(
		Math::Max(layoutSize.Width - sizeConstraints.ExplicitPosition.X, 0),
		Math::Max(layoutSize.Height - sizeConstraints.ExplicitPosition.Y, 0));

	GUILogicalArea area;
	area.SetPosition(sizeConstraints.ExplicitPosition);
	area.SetSize(sizeRange.CalculateSizeConstrainedByParentSize(sizeConstraints, adjustedSize));

	return area;
}

void GUIPanel::UpdateDepthRangeInternal(GUILayoutData& data)
{
	i32 newPanelDepth = data.GetPanelDepth() + mDepthOffset;
	i32 newPanelDepthRangeMinimum = newPanelDepth - mDepthRangeMin;
	i32 newPanelDepthRangeMaximum = newPanelDepth + mDepthRangeMax;

	i32* allDepths[3] = { &newPanelDepth, &newPanelDepthRangeMinimum, &newPanelDepthRangeMaximum };

	for(auto& depth : allDepths)
	{
		i32 minimumValue = std::max((i32)data.GetPanelDepth() - (i32)data.DepthRangeMin, (i32)std::numeric_limits<i16>::min());
		*depth = std::max(*depth, minimumValue);

		i32 maximumValue = std::min((i32)data.GetPanelDepth() + (i32)data.DepthRangeMax, (i32)std::numeric_limits<i16>::max());
		*depth = std::min(*depth, maximumValue);
	}

	data.SetPanelDepth((i16)newPanelDepth);

	if(mDepthRangeMin != (u16)-1 || data.DepthRangeMin != (u16)-1)
		data.DepthRangeMin = (u16)(newPanelDepth - newPanelDepthRangeMinimum);

	if(mDepthRangeMax != (u16)-1 || data.DepthRangeMax != (u16)-1)
		data.DepthRangeMax = (u16)(newPanelDepthRangeMaximum - newPanelDepth);
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

	GetChildRelativeLayoutAreas(mLayoutData.Size, elementPositions, elementSizes, elementCount, mChildConstrainedSizeRanges);

	u32 childIndex = 0;

	for(auto& child : mChildren)
	{
		if(child->IsActive())
		{
			childData.RelativePosition = elementPositions[childIndex];
			childData.Size = elementSizes[childIndex];

			child->SetLayoutData(childData);
			child->UpdateLayoutForChildren();
		}

		childIndex++;
	}

	if(elementSizes != nullptr)
		B3DStackFree(elementSizes);

	if(elementPositions != nullptr)
		B3DStackFree(elementPositions);

	if(mCulling != nullptr)
		mCulling->RebuildQuadTree(mChildren);
}

RTTIType* GUIPanel::GetRttiStatic()
{
	return GUIPanelRTTI::Instance();
}

RTTIType* GUIPanel::GetRtti() const
{
	return GetRttiStatic();
}
