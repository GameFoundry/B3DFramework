//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUILayoutY.h"
#include "GUI/B3DGUIInteractable.h"
#include "GUI/B3DGUISpace.h"
#include "Math/B3DMath.h"
#include "Reflection/B3DRTTIType.h"

using namespace b3d;

GUILayoutY::GUILayoutY(PrivatelyConstruct, const String& styleClass, const GUISizeConstraints& sizeConstraints)
	: GUILayout(styleClass, sizeConstraints)
{}

const String& GUILayoutY::GetGuiTypeName()
{
	static String kName = "GUILayoutY";
	return kName;
}

void GUILayoutY::UpdateOptimalLayoutSizes()
{
	// Update all children first, otherwise we can't determine our own optimal size
	GUIElement::UpdateOptimalLayoutSizes();

	if(mChildren.size() != mChildConstrainedSizeRanges.size())
		mChildConstrainedSizeRanges.resize(mChildren.size());

	GUILogicalSize optimalSize(kZeroTag);
	GUILogicalSize minSize(kZeroTag);

	u32 childIndex = 0;
	for(auto& child : mChildren)
	{
		GUIConstrainedSizeRange& childSizeRange = mChildConstrainedSizeRanges[childIndex];

		if(child->IsActive())
		{
			childSizeRange = child->GetConstrainedSizeRange();
			if(B3DRTTIIsOfType<GUIFixedSpace>(child))
			{
				childSizeRange.Optimal.Width = 0;
				childSizeRange.Minimum.Width = 0;
			}

			const GUILogicalUnit marginsX = child->GetMargins().Left + child->GetMargins().Right;
			const GUILogicalUnit marginsY = child->GetMargins().Top + child->GetMargins().Bottom;

			optimalSize.Height += childSizeRange.Optimal.Height + marginsY;
			optimalSize.Width = Math::Max(optimalSize.Width, childSizeRange.Optimal.Width + marginsX);

			minSize.Height += childSizeRange.Minimum.Height + marginsY;
			minSize.Width = Math::Max(minSize.Width, childSizeRange.Minimum.Width + marginsX);
		}
		else
			childSizeRange = GUIConstrainedSizeRange();

		childIndex++;
	}

	mConstrainedSizeRange = GetSizeConstraints().CalculateConstrainedSizeRange(optimalSize);
	mConstrainedSizeRange.Minimum.Width = std::max(mConstrainedSizeRange.Minimum.Width, minSize.Width);
	mConstrainedSizeRange.Minimum.Height = std::max(mConstrainedSizeRange.Minimum.Height, minSize.Height);
}

void GUILayoutY::GetChildRelativeLayoutAreas(const GUILogicalSize& layoutSize, GUILogicalPoint* outElementPositions, GUILogicalSize* outElementSizes, u32 elementCount, const Vector<GUIConstrainedSizeRange>& sizeRanges, const GUILogicalSize& myOptimalSize) const
{
	B3D_ASSERT(mChildren.size() == elementCount);

	GUILogicalUnit totalOptimalSize = myOptimalSize.Height;
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
		outElementSizes[childIndex].Height = sizeRanges[childIndex].Optimal.Height;

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
	if(layoutSize.Height > totalOptimalSize)
	{
		GUILogicalUnit extraSize = layoutSize.Height - totalOptimalSize;
		GUILogicalUnit remainingSize = extraSize;

		// Expanding elements expand to fill up all unused space
		if(expandingElementCount > 0)
		{
			const float averageSize = (float)remainingSize / (float)expandingElementCount;

			childIndex = 0;
			for(auto& child : mChildren)
			{
				if(processedElements[childIndex])
				{
					childIndex++;
					continue;
				}

				const GUILogicalUnit extraHeight = Math::Min(remainingSize, GUILogicalUnit(Math::CeilToInt(averageSize)));
				const GUILogicalUnit elementHeight = outElementSizes[childIndex].Height + extraHeight;

				// Clamp if needed
				if(child->GetSizeConstraints().IsHeightExpanding())
				{
					processedElements[childIndex] = true;
					flexibleElementCount--;
					outElementSizes[childIndex].Height = elementHeight;

					remainingSize = Math::Max(remainingSize - extraHeight, 0);
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
	for(u32 childLoopIndex = 0; childLoopIndex < childCount; childLoopIndex++)
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
	if(totalOptimalSize > layoutSize.Height)
	{
		GUILogicalUnit remainingExcessSize = totalOptimalSize - layoutSize.Height;

		// Iterate until we reduce everything so it fits, while maintaining
		// equal average sizes using the weights we calculated earlier
		while(remainingExcessSize > 0 && flexibleElementCount > 0)
		{
			const GUILogicalUnit remainingExcessSizeForIteration = remainingExcessSize;

			childIndex = 0;
			for(auto& child : mChildren)
			{
				if(processedElements[childIndex])
				{
					childIndex++;
					continue;
				}

				const float averageSize = (float)remainingExcessSizeForIteration * elementScaleWeights[childIndex];

				const GUILogicalUnit elementExcessHeight = Math::Min(remainingExcessSize, GUILogicalUnit(Math::CeilToInt(averageSize)));
				GUILogicalUnit elementHeight = Math::Max(outElementSizes[childIndex].Height - elementExcessHeight, 0);

				// Clamp if needed
				const GUIConstrainedSizeRange& childSizeRange = sizeRanges[childIndex];

				if(elementHeight == 0)
				{
					processedElements[childIndex] = true;
					flexibleElementCount--;
				}
				else if(childSizeRange.Minimum.Height > 0 && elementHeight < childSizeRange.Minimum.Height)
				{
					elementHeight = childSizeRange.Minimum.Height;

					processedElements[childIndex] = true;
					flexibleElementCount--;
				}

				const GUILogicalUnit reducedHeight = outElementSizes[childIndex].Height - elementHeight;
				outElementSizes[childIndex].Height = elementHeight;
				remainingExcessSize = Math::Max(remainingExcessSize - reducedHeight, 0);

				childIndex++;
			}
		}
	}
	else // We are smaller than the allowed maximum, so try to expand some elements
	{
		GUILogicalUnit remainingExtraSize = layoutSize.Height - totalOptimalSize;

		// Iterate until we reduce everything so it fits, while maintaining
		// equal average sizes using the weights we calculated earlier
		while(remainingExtraSize > 0 && flexibleElementCount > 0)
		{
			const GUILogicalUnit remainingExtraSizeForIteration = remainingExtraSize;

			childIndex = 0;
			for(auto& child : mChildren)
			{
				if(processedElements[childIndex])
				{
					childIndex++;
					continue;
				}

				const float averageSize = (float)remainingExtraSizeForIteration * elementScaleWeights[childIndex];

				const GUILogicalUnit elementExtraHeight = Math::Min(GUILogicalUnit(Math::CeilToInt(averageSize)), remainingExtraSize);
				GUILogicalUnit elementHeight = outElementSizes[childIndex].Height + elementExtraHeight;

				// Clamp if needed
				const GUIConstrainedSizeRange& childSizeRange = sizeRanges[childIndex];

				if(elementHeight == 0)
				{
					processedElements[childIndex] = true;
					flexibleElementCount--;
				}
				else if(childSizeRange.Maximum.Height > 0 && elementHeight > childSizeRange.Maximum.Height)
				{
					elementHeight = childSizeRange.Maximum.Height;

					processedElements[childIndex] = true;
					flexibleElementCount--;
				}

				const GUILogicalUnit increasedHeight = elementHeight - outElementSizes[childIndex].Height;
				outElementSizes[childIndex].Height = elementHeight;
				remainingExtraSize = Math::Max(remainingExtraSize - increasedHeight, 0);

				childIndex++;
			}
		}
	}

	if(elementScaleWeights != nullptr)
		B3DStackFree(elementScaleWeights);

	if(processedElements != nullptr)
		B3DStackFree(processedElements);

	// Compute offsets and width
	GUILogicalUnit yOffset = 0;
	childIndex = 0;

	for(auto& child : mChildren)
	{
		GUILogicalUnit elemHeight = outElementSizes[childIndex].Height;
		yOffset += child->GetMargins().Top;

		const GUIConstrainedSizeRange& sizeRange = sizeRanges[childIndex];
		GUILogicalUnit elemWidth = sizeRanges[childIndex].Optimal.Width;
		const GUISizeConstraints& dimensions = child->GetSizeConstraints();
		if(!dimensions.IsWidthFixed())
		{
			elemWidth = layoutSize.Width;
			if(sizeRange.Minimum.Width > 0 && elemWidth < sizeRange.Minimum.Width)
				elemWidth = sizeRange.Minimum.Width;

			if(sizeRange.Maximum.Width > 0 && elemWidth > (u32)sizeRange.Maximum.Width)
				elemWidth = sizeRange.Maximum.Width;
		}

		outElementSizes[childIndex].Width = elemWidth;

		if(GUIInteractable* const element = B3DRTTICast<GUIInteractable>(child))
		{
			GUILogicalUnit xPadding = element->GetMargins().Left + element->GetMargins().Right;
			GUILogicalUnit xOffset = Math::CeilToInt((float)((i32)(layoutSize.Width - (i32)(elemWidth + xPadding))) * 0.5f);
			xOffset = Math::Max(xOffset, 0);

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
	GUILogicalPoint* elementPositions = nullptr;
	GUILogicalSize* elementSizes = nullptr;

	if(elementCount > 0)
	{
		elementPositions = B3DStackNew<GUILogicalPoint>(elementCount);
		elementSizes = B3DStackNew<GUILogicalSize>(elementCount);
	}

	GetChildRelativeLayoutAreas(mLayoutData.Size, elementPositions, elementSizes, elementCount, mChildConstrainedSizeRanges, mConstrainedSizeRange.Optimal);

	// Now that we have all the areas, actually assign them
	u32 childIndex = 0;

	GUILayoutData childData = mLayoutData;
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
