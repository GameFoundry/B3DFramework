//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUILayoutX.h"
#include "GUI/BsGUIInteractable.h"
#include "GUI/BsGUISpace.h"
#include "Math/BsMath.h"
#include "Math/BsVector2I.h"
#include "Profiling/BsProfilerCPU.h"
#include "Reflection/BsRTTIType.h"

using namespace bs;

GUILayoutX::GUILayoutX(const GUISizeConstraints& dimensions)
	: GUILayout(dimensions)
{}

void GUILayoutX::UpdateOptimalLayoutSizes()
{
	// Update all children first, otherwise we can't determine our own optimal size
	GUIElement::UpdateOptimalLayoutSizes();

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
			childSizeRange = child->GetConstrainedSize();
			if(B3DRTTIIsOfType<GUIFixedSpace>(child))
			{
				childSizeRange.Optimal.Y = 0;
				childSizeRange.Min.Y = 0;
			}

			const u32 marginsX = child->GetMargins().Left + child->GetMargins().Right;
			const u32 marginsY = child->GetMargins().Top + child->GetMargins().Bottom;

			optimalSize.X += childSizeRange.Optimal.X + marginsX;
			optimalSize.Y = std::max((u32)optimalSize.Y, childSizeRange.Optimal.Y + marginsY);

			minSize.X += childSizeRange.Min.X + marginsX;
			minSize.Y = std::max((u32)minSize.Y, childSizeRange.Min.Y + marginsY);
		}
		else
			childSizeRange = GUIConstrainedSize();

		childIdx++;
	}

	mConstrainedSize = GetSizeConstraints().CalculateConstrainedSize(optimalSize);
	mConstrainedSize.Min.X = std::max(mConstrainedSize.Min.X, minSize.X);
	mConstrainedSize.Min.Y = std::max(mConstrainedSize.Min.Y, minSize.Y);
}

void GUILayoutX::GetChildRelativeLayoutAreas(const Size2UI& layoutSize, Vector2I* outElementPositions, Size2UI* outElementSizes, u32 elementCount, const Vector<GUIConstrainedSize>& sizeRanges, const GUIConstrainedSize& mySizeRange) const
{
	B3D_ASSERT(mChildren.size() == elementCount);

	u32 totalOptimalSize = mySizeRange.Optimal.X;
	float weightedNonClampedSize = 0.0f;
	u32 flexibleElementCount = 0;
	u32 expandingElementCount = 0;

	bool* processedElements = nullptr;
	float* elementScaleWeights = nullptr;

	if(mChildren.size() > 0)
	{
		processedElements = B3DStackAllocate<bool>((u32)mChildren.size());
		memset(processedElements, 0, mChildren.size() * sizeof(bool));

		elementScaleWeights = B3DStackAllocate<float>((u32)mChildren.size());
		memset(elementScaleWeights, 0, mChildren.size() * sizeof(float));
	}

	// Set initial sizes, count number of children per type and mark fixed elements as already processed
	u32 childIndex = 0;
	for(auto& child : mChildren)
	{
		outElementSizes[childIndex].Width = sizeRanges[childIndex].Optimal.X;

		const GUISizeConstraints& sizeConstraints = child->GetSizeConstraints();
		if(sizeConstraints.IsWidthFixed() || !child->IsActive())
			processedElements[childIndex] = true;
		else if(sizeConstraints.IsWidthExpanding())
		{
			expandingElementCount++;
			flexibleElementCount++;
			weightedNonClampedSize += (float)outElementSizes[childIndex].Width * sizeConstraints.FlexibleWidthWeight;
		}
		else
		{
			if(outElementSizes[childIndex].Width > 0)
			{
				flexibleElementCount++;
				weightedNonClampedSize += (float)outElementSizes[childIndex].Width * sizeConstraints.FlexibleWidthWeight;
			}
			else
				processedElements[childIndex] = true;
		}

		childIndex++;
	}

	// If there is some room left, calculate flexible space sizes (since they will fill up all that extra room)
	if(layoutSize.Width > totalOptimalSize)
	{
		u32 extraSize = layoutSize.Width - totalOptimalSize;
		u32 remainingSize = extraSize;

		// Flexible spaces always expand to fill up all unused space
		if(expandingElementCount > 0)
		{
			float avgSize = remainingSize / (float)expandingElementCount;

			childIndex = 0;
			for(auto& child : mChildren)
			{
				if(processedElements[childIndex])
				{
					childIndex++;
					continue;
				}

				u32 extraWidth = std::min((u32)Math::CeilToInt(avgSize), remainingSize);
				u32 elementWidth = outElementSizes[childIndex].Width + extraWidth;

				// Clamp if needed
				if(child->GetSizeConstraints().IsWidthExpanding())
				{
					processedElements[childIndex] = true;
					flexibleElementCount--;
					outElementSizes[childIndex].Width = elementWidth;

					remainingSize = (u32)std::max(0, (i32)remainingSize - (i32)extraWidth);
				}

				childIndex++;
			}

			totalOptimalSize = layoutSize.Width;
		}
	}

	// Determine weight scale for every element. When scaling elements up/down they will be scaled based on this weight.
	// Weight is to ensure all elements are scaled fairly, so elements that are large will get effected more than smaller elements.
	childIndex = 0;
	const float inverseWeightedNonClampedSize = 1.0f / weightedNonClampedSize;
	u32 childCount = (u32)mChildren.size();
	for(u32 i = 0; i < childCount; i++)
	{
		if(processedElements[childIndex])
		{
			childIndex++;
			continue;
		}

		elementScaleWeights[childIndex] = inverseWeightedNonClampedSize * outElementSizes[childIndex].Width * mChildren[childIndex]->GetSizeConstraints().FlexibleWidthWeight;

		childIndex++;
	}

	// Our optimal size is larger than maximum allowed, so we need to reduce size of some elements
	if(totalOptimalSize > (u32)layoutSize.Width)
	{
		u32 remainingExcessSize = totalOptimalSize - layoutSize.Width;

		// Iterate until we reduce everything so it fits, while maintaining
		// equal average sizes using the weights we calculated earlier
		while(remainingExcessSize > 0 && flexibleElementCount > 0)
		{
			const u32 remainingExcessSizeForIteration = remainingExcessSize;

			childIndex = 0;
			for(auto& child : mChildren)
			{
				if(processedElements[childIndex])
				{
					childIndex++;
					continue;
				}

				const float averageSize = (float)remainingExcessSizeForIteration * elementScaleWeights[childIndex];

				const u32 elementExcessWidth = std::min((u32)Math::CeilToInt(averageSize), remainingExcessSize);
				u32 elementWidth = (u32)std::max(0, (i32)outElementSizes[childIndex].Width - (i32)elementExcessWidth);

				// Clamp if needed
				const GUIConstrainedSize& childSizeRange = sizeRanges[childIndex];

				if(elementWidth == 0)
				{
					processedElements[childIndex] = true;
					flexibleElementCount--;
				}
				else if(childSizeRange.Min.X > 0 && (i32)elementWidth < childSizeRange.Min.X)
				{
					elementWidth = childSizeRange.Min.X;

					processedElements[childIndex] = true;
					flexibleElementCount--;
				}

				const u32 reducedWidth = outElementSizes[childIndex].Width - elementWidth;
				outElementSizes[childIndex].Width = elementWidth;
				remainingExcessSize = (u32)std::max(0, (i32)remainingExcessSize - (i32)reducedWidth);

				childIndex++;
			}
		}
	}
	else // We are smaller than the allowed maximum, so try to expand some elements
	{
		u32 remainingExtraSize = layoutSize.Width - totalOptimalSize;

		// Iterate until we reduce everything so it fits, while maintaining
		// equal average sizes using the weights we calculated earlier
		while(remainingExtraSize > 0 && flexibleElementCount > 0)
		{
			const u32 remainingExtraSizeForIteration = remainingExtraSize;

			childIndex = 0;
			for(auto& child : mChildren)
			{
				if(processedElements[childIndex])
				{
					childIndex++;
					continue;
				}

				const float averageSize = (float)remainingExtraSizeForIteration * elementScaleWeights[childIndex];

				const u32 elementExtraWidth = std::min((u32)Math::CeilToInt(averageSize), remainingExtraSize);
				u32 elementWidth = outElementSizes[childIndex].Width + elementExtraWidth;

				// Clamp if needed
				const GUIConstrainedSize& childSizeRange = sizeRanges[childIndex];

				if(elementWidth == 0)
				{
					processedElements[childIndex] = true;
					flexibleElementCount--;
				}
				else if(childSizeRange.Max.X > 0 && (i32)elementWidth > childSizeRange.Max.X)
				{
					elementWidth = childSizeRange.Max.X;

					processedElements[childIndex] = true;
					flexibleElementCount--;
				}

				const u32 increasedWidth = elementWidth - outElementSizes[childIndex].Width;
				outElementSizes[childIndex].Width = elementWidth;
				remainingExtraSize = (u32)std::max(0, (i32)remainingExtraSize - (i32)increasedWidth);

				childIndex++;
			}
		}
	}

	if(elementScaleWeights != nullptr)
		B3DStackFree(elementScaleWeights);

	if(processedElements != nullptr)
		B3DStackFree(processedElements);

	// Compute offsets and height
	u32 xOffset = 0;
	childIndex = 0;

	for(auto& child : mChildren)
	{
		u32 elemWidth = outElementSizes[childIndex].Width;
		xOffset += child->GetMargins().Left;

		const GUIConstrainedSize& sizeRange = sizeRanges[childIndex];
		u32 elemHeight = (u32)sizeRange.Optimal.Y;
		const GUISizeConstraints& dimensions = child->GetSizeConstraints();
		if(!dimensions.IsHeightFixed())
		{
			elemHeight = layoutSize.Height;
			if(sizeRange.Min.Y > 0 && elemHeight < (u32)sizeRange.Min.Y)
				elemHeight = (u32)sizeRange.Min.Y;

			if(sizeRange.Max.Y > 0 && elemHeight > (u32)sizeRange.Max.Y)
				elemHeight = (u32)sizeRange.Max.Y;
		}
		outElementSizes[childIndex].Height = elemHeight;

		if(GUIInteractable* const element = B3DRTTICast<GUIInteractable>(child))
		{
			u32 yPadding = element->GetMargins().Top + element->GetMargins().Bottom;
			i32 yOffset = Math::CeilToInt(((i32)layoutSize.Height - (i32)(elemHeight + yPadding)) * 0.5f);
			yOffset = std::max(0, yOffset);

			outElementPositions[childIndex].X = (i32)xOffset;
			outElementPositions[childIndex].Y = yOffset;
		}
		else
		{
			outElementPositions[childIndex].X = (i32)xOffset;
			outElementPositions[childIndex].Y = 0;
		}

		xOffset += elemWidth + child->GetMargins().Right;
		childIndex++;
	}
}

void GUILayoutX::UpdateLayoutForChildren()
{
	const u32 elementCount = (u32)mChildren.size();
	Vector2I* elementPositions = nullptr;
	Size2UI* elementSizes = nullptr;

	if(elementCount > 0)
	{
		elementPositions = B3DStackNew<Vector2I>(elementCount);
		elementSizes = B3DStackNew<Size2UI>(elementCount);
	}

	GetChildRelativeLayoutAreas(mLayoutData.Size, elementPositions, elementSizes, elementCount, mChildrenConstrainedSizes, mConstrainedSize);

	// Now that we have all the areas, actually assign them
	u32 childIdx = 0;

	GUILayoutData childData = mLayoutData;
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
}

GUILayoutX* GUILayoutX::Create()
{
	return B3DNew<GUILayoutX>();
}

GUILayoutX* GUILayoutX::Create(const GUIOptions& options)
{
	return B3DNew<GUILayoutX>(GUISizeConstraints::Create(options));
}
