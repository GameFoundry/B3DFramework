//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIPanel.h"
#include "GUI/BsGUIInteractable.h"
#include "GUI/BsGUISpace.h"
#include "Math/BsMath.h"
#include "Math/BsVector2I.h"
#include "Profiling/BsProfilerCPU.h"
#include "Reflection/BsRTTIType.h"

using namespace bs;

/** @cond RTTI */
/** @addtogroup RTTI-Impl-Engine
 *  @{
 */

namespace bs
{
	class B3D_EXPORT GUIPanelRTTI : public RTTIType<GUIPanel, GUILayout, GUIPanelRTTI>
	{
	public:
		const String& GetRttiName()
		{
			static String name = "GUIPanel";
			return name;
		}

		u32 GetRttiId() { return TID_GUIPanel; }

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

GUIConstrainedSize GUIPanel::CalculateConstrainedSize() const
{
	Vector2I optimalSize;
	Vector2I minSize;

	for(auto& child : mChildren)
	{
		if(!child->IsActive())
			continue;

		GUIConstrainedSize sizeRange = child->CalculateConstrainedSize();

		if(child->GetType() == GUIElementBase::Type::FixedSpace || child->GetType() == GUIElementBase::Type::FlexibleSpace)
		{
			sizeRange.Optimal.X = sizeRange.Optimal.Y = 0;
			sizeRange.Min.X = sizeRange.Min.Y = 0;
		}

		u32 paddingX = child->GetMargins().Left + child->GetMargins().Right;
		u32 paddingY = child->GetMargins().Top + child->GetMargins().Bottom;

		Vector2I childMax;
		childMax.X = child->GetSizeConstraints().X + sizeRange.Optimal.X + paddingX;
		childMax.Y = child->GetSizeConstraints().Y + sizeRange.Optimal.Y + paddingY;

		optimalSize.X = std::max(optimalSize.X, childMax.X);
		optimalSize.Y = std::max(optimalSize.Y, childMax.Y);

		childMax.X = child->GetSizeConstraints().X + sizeRange.Min.X + paddingX;
		childMax.Y = child->GetSizeConstraints().Y + sizeRange.Min.Y + paddingY;

		minSize.X = std::max(minSize.X, childMax.X);
		minSize.Y = std::max(minSize.Y, childMax.Y);
	}

	GUIConstrainedSize sizeRange = GetSizeConstraints().CalculateConstrainedSize(optimalSize);
	sizeRange.Min.X = std::max(sizeRange.Min.X, minSize.X);
	sizeRange.Min.Y = std::max(sizeRange.Min.Y, minSize.Y);

	return sizeRange;
}

GUIConstrainedSize GUIPanel::GetElementSizeRangeInternal(const GUIElementBase* element) const
{
	if(element->GetType() == GUIElementBase::Type::FixedSpace || element->GetType() == GUIElementBase::Type::FlexibleSpace)
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
	GUIElementBase::UpdateOptimalLayoutSizes();

	if(mChildren.size() != mChildrenConstrainedSizes.size())
		mChildrenConstrainedSizes.resize(mChildren.size());

	Vector2I optimalSize;
	Vector2I minSize;

	u32 childIdx = 0;
	for(auto& child : mChildren)
	{
		GUIConstrainedSize& childSizeRange = mChildrenConstrainedSizes[childIdx];

		if(child->IsActive())
		{
			childSizeRange = GetElementSizeRangeInternal(child);

			u32 paddingX = child->GetMargins().Left + child->GetMargins().Right;
			u32 paddingY = child->GetMargins().Top + child->GetMargins().Bottom;

			Vector2I childMax;
			childMax.X = child->GetSizeConstraints().X + childSizeRange.Optimal.X + paddingX;
			childMax.Y = child->GetSizeConstraints().Y + childSizeRange.Optimal.Y + paddingY;

			optimalSize.X = std::max(optimalSize.X, childMax.X);
			optimalSize.Y = std::max(optimalSize.Y, childMax.Y);

			childMax.X = child->GetSizeConstraints().X + childSizeRange.Min.X + paddingX;
			childMax.Y = child->GetSizeConstraints().Y + childSizeRange.Min.Y + paddingY;

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

void GUIPanel::GetChildLayoutAreas(const Rect2I& layoutArea, Rect2I* elementAreas, u32 numElements, const Vector<GUIConstrainedSize>& sizeRanges, const GUIConstrainedSize& mySizeRange) const
{
	B3D_ASSERT(mChildren.size() == numElements);

	// Panel always uses optimal sizes and explicit positions
	u32 childIdx = 0;
	for(auto& child : mChildren)
	{
		elementAreas[childIdx] = GetElementAreaInternal(layoutArea, child, sizeRanges[childIdx]);

		childIdx++;
	}
}

Rect2I GUIPanel::GetElementAreaInternal(const Rect2I& layoutArea, const GUIElementBase* element, const GUIConstrainedSize& sizeRange) const
{
	const GUISizeConstraints& dimensions = element->GetSizeConstraints();

	Rect2I area;

	area.X = layoutArea.X + dimensions.X;
	area.Y = layoutArea.Y + dimensions.Y;

	if(dimensions.IsWidthFixed())
		area.Width = (u32)sizeRange.Optimal.X;
	else
	{
		u32 modifiedWidth = (u32)std::max(0, (i32)layoutArea.Width - dimensions.X);

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
		u32 modifiedHeight = (u32)std::max(0, (i32)layoutArea.Height - dimensions.Y);

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

void GUIPanel::UpdateLayoutRecursive(const GUILayoutData& data)
{
	GUILayoutData childData = data;
	UpdateDepthRangeInternal(childData);

	u32 numElements = (u32)mChildren.size();
	Rect2I* elementAreas = nullptr;

	if(numElements > 0)
		elementAreas = B3DStackNew<Rect2I>(numElements);

	GetChildLayoutAreas(data.Area, elementAreas, numElements, mChildrenConstrainedSizes, mConstrainedSize);

	u32 childIdx = 0;

	for(auto& child : mChildren)
	{
		if(child->IsActive())
		{
			childData.Area = elementAreas[childIdx];

			UpdateChildLayoutInternal(child, childData);
		}

		childIdx++;
	}

	if(elementAreas != nullptr)
		B3DStackFree(elementAreas);
}

void GUIPanel::UpdateChildLayoutInternal(GUIElementBase* element, const GUILayoutData& data)
{
	GUILayoutData childData = data;

	childData.ClipRect = data.Area;
	childData.ClipRect.Clip(data.ClipRect);

	element->SetLayoutData(childData);
	element->UpdateLayoutRecursive(childData);
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

RTTITypeBase* GUIPanel::GetRttiStatic()
{
	return GUIPanelRTTI::Instance();
}

RTTITypeBase* GUIPanel::GetRtti() const
{
	return GetRttiStatic();
}
