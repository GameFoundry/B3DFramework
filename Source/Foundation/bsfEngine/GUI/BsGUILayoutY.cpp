//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUILayoutY.h"
#include "GUI/BsGUIInteractable.h"
#include "GUI/BsGUISpace.h"
#include "Math/BsMath.h"
#include "Math/BsVector2I.h"
#include "Reflection/BsRTTIType.h"

using namespace bs;

GUILayoutY::GUILayoutY(const GUISizeConstraints& sizeConstraints)
	: GUILayout(sizeConstraints)
{}

void GUILayoutY::UpdateOptimalLayoutSizes()
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
				childSizeRange.Optimal.X = 0;
				childSizeRange.Min.X = 0;
			}

			const u32 marginsX = child->GetMargins().Left + child->GetMargins().Right;
			const u32 marginsY = child->GetMargins().Top + child->GetMargins().Bottom;

			optimalSize.Y += childSizeRange.Optimal.Y + marginsY;
			optimalSize.X = std::max((u32)optimalSize.X, childSizeRange.Optimal.X + marginsX);

			minSize.Y += childSizeRange.Min.Y + marginsY;
			minSize.X = std::max((u32)minSize.X, childSizeRange.Min.X + marginsX);
		}
		else
			childSizeRange = GUIConstrainedSize();

		childIdx++;
	}

	mConstrainedSize = GetSizeConstraints().CalculateConstrainedSize(optimalSize);
	mConstrainedSize.Min.X = std::max(mConstrainedSize.Min.X, minSize.X);
	mConstrainedSize.Min.Y = std::max(mConstrainedSize.Min.Y, minSize.Y);
}

void GUILayoutY::GetChildRelativeLayoutAreas(const Size2UI& layoutSize, Vector2I* outElementPositions, Size2UI* outElementSizes, u32 elementCount, const Vector<GUIConstrainedSize>& sizeRanges, const GUIConstrainedSize& mySizeRange) const
{
	B3D_ASSERT(mChildren.size() == elementCount);

	u32 totalOptimalSize = mySizeRange.Optimal.Y;
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
		outElementSizes[childIndex].Height = sizeRanges[childIndex].Optimal.Y;

		const GUISizeConstraints& sizeConstraints = child->GetSizeConstraints();
		if(sizeConstraints.IsHeightFixed() || !child->IsActive())
			processedElements[childIndex] = true;
		else if(sizeConstraints.IsHeightExpanding())
		{
			expandingElementCount++;
			flexibleElementCount++;
			weightedNonClampedSize += (float)outElementSizes[childIndex].Height * sizeConstraints.FlexibleHeightWeight;
		}
		else
		{
			if(outElementSizes[childIndex].Height > 0)
			{
				flexibleElementCount++;
				weightedNonClampedSize += (float)outElementSizes[childIndex].Height * sizeConstraints.FlexibleHeightWeight;
			}
			else
				processedElements[childIndex] = true;
		}

		childIndex++;
	}

	// If there is some room left, calculate flexible space sizes (since they will fill up all that extra room)
	if((u32)layoutSize.Height > totalOptimalSize)
	{
		u32 extraSize = layoutSize.Height - totalOptimalSize;
		u32 remainingSize = extraSize;

		// Expanding elements expand to fill up all unused space
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

				u32 extraHeight = std::min((u32)Math::CeilToInt(avgSize), remainingSize);
				u32 elementHeight = outElementSizes[childIndex].Height + extraHeight;

				// Clamp if needed
				if(child->GetSizeConstraints().IsHeightExpanding())
				{
					processedElements[childIndex] = true;
					flexibleElementCount--;
					outElementSizes[childIndex].Height = elementHeight;

					remainingSize = (u32)std::max(0, (i32)remainingSize - (i32)extraHeight);
				}

				childIndex++;
			}

			totalOptimalSize = layoutSize.Height;
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

		elementScaleWeights[childIndex] = inverseWeightedNonClampedSize * (float)outElementSizes[childIndex].Height * mChildren[childIndex]->GetSizeConstraints().FlexibleHeightWeight;

		childIndex++;
	}

	// Our optimal size is larger than maximum allowed, so we need to reduce size of some elements
	if(totalOptimalSize > (u32)layoutSize.Height)
	{
		u32 remainingExcessSize = totalOptimalSize - layoutSize.Height;

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

				const u32 elementExcessHeight = std::min((u32)Math::CeilToInt(averageSize), remainingExcessSize);
				u32 elementHeight = (u32)std::max(0, (i32)outElementSizes[childIndex].Height - (i32)elementExcessHeight);

				// Clamp if needed
				const GUIConstrainedSize& childSizeRange = sizeRanges[childIndex];

				if(elementHeight == 0)
				{
					processedElements[childIndex] = true;
					flexibleElementCount--;
				}
				else if(childSizeRange.Min.Y > 0 && (i32)elementHeight < childSizeRange.Min.Y)
				{
					elementHeight = childSizeRange.Min.Y;

					processedElements[childIndex] = true;
					flexibleElementCount--;
				}

				const u32 reducedHeight = outElementSizes[childIndex].Height - elementHeight;
				outElementSizes[childIndex].Height = elementHeight;
				remainingExcessSize = (u32)std::max(0, (i32)remainingExcessSize - (i32)reducedHeight);

				childIndex++;
			}
		}
	}
	else // We are smaller than the allowed maximum, so try to expand some elements
	{
		u32 remainingExtraSize = layoutSize.Height - totalOptimalSize;

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

				const u32 elementExtraHeight = std::min((u32)Math::CeilToInt(averageSize), remainingExtraSize);
				u32 elementHeight = outElementSizes[childIndex].Height + elementExtraHeight;

				// Clamp if needed
				const GUIConstrainedSize& childSizeRange = sizeRanges[childIndex];

				if(elementHeight == 0)
				{
					processedElements[childIndex] = true;
					flexibleElementCount--;
				}
				else if(childSizeRange.Max.Y > 0 && (i32)elementHeight > childSizeRange.Max.Y)
				{
					elementHeight = childSizeRange.Max.Y;

					processedElements[childIndex] = true;
					flexibleElementCount--;
				}

				const u32 increasedHeight = elementHeight - outElementSizes[childIndex].Height;
				outElementSizes[childIndex].Height = elementHeight;
				remainingExtraSize = (u32)std::max(0, (i32)remainingExtraSize - (i32)increasedHeight);

				childIndex++;
			}
		}
	}

	if(elementScaleWeights != nullptr)
		B3DStackFree(elementScaleWeights);

	if(processedElements != nullptr)
		B3DStackFree(processedElements);

	// Compute offsets and width
	u32 yOffset = 0;
	childIndex = 0;

	for(auto& child : mChildren)
	{
		u32 elemHeight = outElementSizes[childIndex].Height;
		yOffset += child->GetMargins().Top;

		const GUIConstrainedSize& sizeRange = sizeRanges[childIndex];
		u32 elemWidth = (u32)sizeRanges[childIndex].Optimal.X;
		const GUISizeConstraints& dimensions = child->GetSizeConstraints();
		if(!dimensions.IsWidthFixed())
		{
			elemWidth = layoutSize.Width;
			if(sizeRange.Min.X > 0 && elemWidth < (u32)sizeRange.Min.X)
				elemWidth = (u32)sizeRange.Min.X;

			if(sizeRange.Max.X > 0 && elemWidth > (u32)sizeRange.Max.X)
				elemWidth = (u32)sizeRange.Max.X;
		}

		outElementSizes[childIndex].Width = elemWidth;

		if(GUIInteractable* const element = B3DRTTICast<GUIInteractable>(child))
		{
			u32 xPadding = element->GetMargins().Left + element->GetMargins().Right;
			i32 xOffset = Math::CeilToInt((i32)(layoutSize.Width - (i32)(elemWidth + xPadding)) * 0.5f);
			xOffset = std::max(0, xOffset);

			outElementPositions[childIndex].X = xOffset;
			outElementPositions[childIndex].Y = yOffset;
		}
		else
		{
			outElementPositions[childIndex].X = 0;
			outElementPositions[childIndex].Y = yOffset;
		}

		yOffset += elemHeight + child->GetMargins().Bottom;
		childIndex++;
	}
}

void GUILayoutY::UpdateLayoutForChildren()
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

GUILayoutY* GUILayoutY::Create()
{
	return B3DNew<GUILayoutY>();
}

GUILayoutY* GUILayoutY::Create(const GUIOptions& options)
{
	return B3DNew<GUILayoutY>(GUISizeConstraints::Create(options));
}
