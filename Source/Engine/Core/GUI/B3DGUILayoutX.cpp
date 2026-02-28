//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUILayoutX.h"
#include "GUI/B3DGUIInteractable.h"
#include "GUI/B3DGUISpace.h"
#include "Math/B3DMath.h"
#include "Profiling/B3DProfilerCPU.h"
#include "Reflection/B3DRTTIType.h"

using namespace b3d;

GUILayoutX::GUILayoutX(PrivatelyConstruct, const String& styleClass, const GUISizeConstraints& sizeConstraints)
	: GUILayout(styleClass, sizeConstraints)
{}

const String& GUILayoutX::GetGuiTypeName()
{
	static String kName = "GUILayoutX";
	return kName;
}

void GUILayoutX::UpdateOptimalLayoutSizes()
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
				childSizeRange.Optimal.Height = 0;
				childSizeRange.Minimum.Height = 0;
			}

			const GUILogicalUnit marginsX = child->GetMargins().Left + child->GetMargins().Right;
			const GUILogicalUnit marginsY = child->GetMargins().Top + child->GetMargins().Bottom;

			optimalSize.Width += childSizeRange.Optimal.Width + marginsX;
			optimalSize.Height = Math::Max(optimalSize.Height, childSizeRange.Optimal.Height + marginsY);

			minSize.Width += childSizeRange.Minimum.Width + marginsX;
			minSize.Height = Math::Max(minSize.Height, childSizeRange.Minimum.Height + marginsY);
		}
		else
			childSizeRange = GUIConstrainedSizeRange();

		childIndex++;
	}

	mConstrainedSizeRange = GetSizeConstraints().CalculateConstrainedSizeRange(optimalSize);
	mConstrainedSizeRange.Minimum.Width = std::max(mConstrainedSizeRange.Minimum.Width, minSize.Width);
	mConstrainedSizeRange.Minimum.Height = std::max(mConstrainedSizeRange.Minimum.Height, minSize.Height);
}

void GUILayoutX::GetChildRelativeLayoutAreas(const GUILogicalSize& layoutSize, GUILogicalPoint* outElementPositions, GUILogicalSize* outElementSizes, u32 elementCount, const Vector<GUIConstrainedSizeRange>& sizeRanges, const GUILogicalSize& myOptimalSize) const
{
	B3D_ASSERT(mChildren.size() == elementCount);

	GUILogicalUnit totalOptimalSize = myOptimalSize.Width;
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
		outElementSizes[childIndex].Width = sizeRanges[childIndex].Optimal.Width;

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
		GUILogicalUnit extraSize = layoutSize.Width - totalOptimalSize;
		GUILogicalUnit remainingSize = extraSize;

		// Flexible spaces always expand to fill up all unused space
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

				GUILogicalUnit extraWidth = Math::Min(GUILogicalUnit(Math::CeilToInt(averageSize)), remainingSize);
				GUILogicalUnit elementWidth = outElementSizes[childIndex].Width + extraWidth;

				// Clamp if needed
				if(child->GetSizeConstraints().IsWidthExpanding())
				{
					processedElements[childIndex] = true;
					flexibleElementCount--;
					outElementSizes[childIndex].Width = elementWidth;

					remainingSize = Math::Max(remainingSize - extraWidth, 0);
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
	for(u32 childLoopIndex = 0; childLoopIndex < childCount; childLoopIndex++)
	{
		if(processedElements[childIndex])
		{
			childIndex++;
			continue;
		}

		elementScaleWeights[childIndex] = inverseWeightedNonClampedSize * (float)outElementSizes[childIndex].Width * mChildren[childIndex]->GetSizeConstraints().FlexibleWidthWeight;

		childIndex++;
	}

	// Our optimal size is larger than maximum allowed, so we need to reduce size of some elements
	if(totalOptimalSize > (i32)layoutSize.Width)
	{
		GUILogicalUnit remainingExcessSize = totalOptimalSize - GUILogicalUnit((i32)layoutSize.Width);

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

				const GUILogicalUnit elementExcessWidth = Math::Min(GUILogicalUnit(Math::CeilToInt(averageSize)), remainingExcessSize);
				GUILogicalUnit elementWidth = Math::Max(outElementSizes[childIndex].Width - elementExcessWidth, 0);

				// Clamp if needed
				const GUIConstrainedSizeRange& childSizeRange = sizeRanges[childIndex];

				if(elementWidth == 0)
				{
					processedElements[childIndex] = true;
					flexibleElementCount--;
				}
				else if(childSizeRange.Minimum.Width > 0 && elementWidth < childSizeRange.Minimum.Width)
				{
					elementWidth = childSizeRange.Minimum.Width;

					processedElements[childIndex] = true;
					flexibleElementCount--;
				}

				const GUILogicalUnit reducedWidth = outElementSizes[childIndex].Width - elementWidth;
				outElementSizes[childIndex].Width = elementWidth;
				remainingExcessSize = Math::Max(remainingExcessSize - reducedWidth, 0);

				childIndex++;
			}
		}
	}
	else // We are smaller than the allowed maximum, so try to expand some elements
	{
		GUILogicalUnit remainingExtraSize = layoutSize.Width - totalOptimalSize;

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

				const GUILogicalUnit elementExtraWidth = Math::Min(GUILogicalUnit(Math::CeilToInt(averageSize)), remainingExtraSize);
				GUILogicalUnit elementWidth = outElementSizes[childIndex].Width + elementExtraWidth;

				// Clamp if needed
				const GUIConstrainedSizeRange& childSizeRange = sizeRanges[childIndex];

				if(elementWidth == 0)
				{
					processedElements[childIndex] = true;
					flexibleElementCount--;
				}
				else if(childSizeRange.Maximum.Width > 0 && elementWidth > childSizeRange.Maximum.Width)
				{
					elementWidth = childSizeRange.Maximum.Width;

					processedElements[childIndex] = true;
					flexibleElementCount--;
				}

				const GUILogicalUnit increasedWidth = elementWidth - outElementSizes[childIndex].Width;
				outElementSizes[childIndex].Width = elementWidth;
				remainingExtraSize = Math::Max(remainingExtraSize - increasedWidth, 0);

				childIndex++;
			}
		}
	}

	if(elementScaleWeights != nullptr)
		B3DStackFree(elementScaleWeights);

	if(processedElements != nullptr)
		B3DStackFree(processedElements);

	// Compute offsets and height
	GUILogicalUnit xOffset = 0;
	childIndex = 0;

	for(auto& child : mChildren)
	{
		GUILogicalUnit elemWidth = outElementSizes[childIndex].Width;
		xOffset += child->GetMargins().Left;

		const GUIConstrainedSizeRange& sizeRange = sizeRanges[childIndex];
		GUILogicalUnit elemHeight = sizeRange.Optimal.Height;
		const GUISizeConstraints& dimensions = child->GetSizeConstraints();
		if(!dimensions.IsHeightFixed())
		{
			elemHeight = layoutSize.Height;
			if(sizeRange.Minimum.Height > 0 && elemHeight < (u32)sizeRange.Minimum.Height)
				elemHeight = sizeRange.Minimum.Height;

			if(sizeRange.Maximum.Height > 0 && elemHeight > (u32)sizeRange.Maximum.Height)
				elemHeight = sizeRange.Maximum.Height;
		}
		outElementSizes[childIndex].Height = elemHeight;

		if(GUIInteractable* const element = B3DRTTICast<GUIInteractable>(child))
		{
			const GUILogicalUnit yPadding = element->GetMargins().Top + element->GetMargins().Bottom;
			GUILogicalUnit yOffset = Math::CeilToInt((float)((i32)layoutSize.Height - (i32)(elemHeight + yPadding)) * 0.5f);
			yOffset = Math::Max(yOffset, 0);

			outElementPositions[childIndex].X = xOffset;
			outElementPositions[childIndex].Y = yOffset;
		}
		else
		{
			outElementPositions[childIndex].X = xOffset;
			outElementPositions[childIndex].Y = 0;
		}

		xOffset += elemWidth + child->GetMargins().Right;
		childIndex++;
	}
}

void GUILayoutX::UpdateLayoutForChildren()
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
