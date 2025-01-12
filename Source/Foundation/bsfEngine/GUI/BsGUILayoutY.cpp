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

GUIConstrainedSize GUILayoutY::CalculateConstrainedSize() const
{
	Vector2I optimalSize;
	Vector2I minSize;

	for(auto& child : mChildren)
	{
		if(!child->IsActive())
			continue;

		GUIConstrainedSize sizeRange = child->CalculateConstrainedSize();

		if(B3DRTTIIsOfType<GUIFixedSpace>(child))
			sizeRange.Optimal.X = sizeRange.Min.X = 0;

		u32 paddingX = child->GetMargins().Left + child->GetMargins().Right;
		u32 paddingY = child->GetMargins().Top + child->GetMargins().Bottom;

		optimalSize.Y += sizeRange.Optimal.Y + paddingY;
		optimalSize.X = std::max((u32)optimalSize.X, sizeRange.Optimal.X + paddingX);

		minSize.Y += sizeRange.Min.Y + paddingY;
		minSize.X = std::max((u32)minSize.X, sizeRange.Min.X + paddingX);
	}

	GUIConstrainedSize sizeRange = GetSizeConstraints().CalculateConstrainedSize(optimalSize);
	sizeRange.Min.X = std::max(sizeRange.Min.X, minSize.X);
	sizeRange.Min.Y = std::max(sizeRange.Min.Y, minSize.Y);

	return sizeRange;
}

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

			u32 paddingX = child->GetMargins().Left + child->GetMargins().Right;
			u32 paddingY = child->GetMargins().Top + child->GetMargins().Bottom;

			optimalSize.Y += childSizeRange.Optimal.Y + paddingY;
			optimalSize.X = std::max((u32)optimalSize.X, childSizeRange.Optimal.X + paddingX);

			minSize.Y += childSizeRange.Min.Y + paddingY;
			minSize.X = std::max((u32)minSize.X, childSizeRange.Min.X + paddingX);
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
	u32 totalNonClampedSize = 0;
	u32 numNonClampedElements = 0;
	u32 numFlexibleSpaces = 0;

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
	u32 childIdx = 0;
	for(auto& child : mChildren)
	{
		outElementSizes[childIdx].Height = sizeRanges[childIdx].Optimal.Y;

		if(B3DRTTIIsOfType<GUIFixedSpace>(child))
		{
			processedElements[childIdx] = true;
		}
		else if(B3DRTTIIsOfType<GUIFlexibleSpace>(child))
		{
			if(child->IsActive())
			{
				numFlexibleSpaces++;
				numNonClampedElements++;
			}
			else
				processedElements[childIdx] = true;
		}
		else
		{
			const GUISizeConstraints& dimensions = child->GetSizeConstraints();

			if(dimensions.IsHeightFixed())
				processedElements[childIdx] = true;
			else
			{
				if(outElementSizes[childIdx].Height > 0)
				{
					numNonClampedElements++;
					totalNonClampedSize += outElementSizes[childIdx].Height;
				}
				else
					processedElements[childIdx] = true;
			}
		}

		childIdx++;
	}

	// If there is some room left, calculate flexible space sizes (since they will fill up all that extra room)
	if((u32)layoutSize.Height > totalOptimalSize)
	{
		u32 extraSize = layoutSize.Height - totalOptimalSize;
		u32 remainingSize = extraSize;

		// Flexible spaces always expand to fill up all unused space
		if(numFlexibleSpaces > 0)
		{
			float avgSize = remainingSize / (float)numFlexibleSpaces;

			childIdx = 0;
			for(auto& child : mChildren)
			{
				if(processedElements[childIdx])
				{
					childIdx++;
					continue;
				}

				u32 extraHeight = std::min((u32)Math::CeilToInt(avgSize), remainingSize);
				u32 elementHeight = outElementSizes[childIdx].Height + extraHeight;

				// Clamp if needed
				if(B3DRTTIIsOfType<GUIFlexibleSpace>(child))
				{
					processedElements[childIdx] = true;
					numNonClampedElements--;
					outElementSizes[childIdx].Height = elementHeight;

					remainingSize = (u32)std::max(0, (i32)remainingSize - (i32)extraHeight);
				}

				childIdx++;
			}

			totalOptimalSize = layoutSize.Height;
		}
	}

	// Determine weight scale for every element. When scaling elements up/down they will be scaled based on this weight.
	// Weight is to ensure all elements are scaled fairly, so elements that are large will get effected more than smaller elements.
	childIdx = 0;
	float invOptimalSize = 1.0f / totalNonClampedSize;
	u32 childCount = (u32)mChildren.size();
	for(u32 i = 0; i < childCount; i++)
	{
		if(processedElements[childIdx])
		{
			childIdx++;
			continue;
		}

		elementScaleWeights[childIdx] = invOptimalSize * outElementSizes[childIdx].Height;

		childIdx++;
	}

	// Our optimal size is larger than maximum allowed, so we need to reduce size of some elements
	if(totalOptimalSize > (u32)layoutSize.Height)
	{
		u32 extraSize = totalOptimalSize - layoutSize.Height;
		u32 remainingSize = extraSize;

		// Iterate until we reduce everything so it fits, while maintaining
		// equal average sizes using the weights we calculated earlier
		while(remainingSize > 0 && numNonClampedElements > 0)
		{
			u32 totalRemainingSize = remainingSize;

			childIdx = 0;
			for(auto& child : mChildren)
			{
				if(processedElements[childIdx])
				{
					childIdx++;
					continue;
				}

				float avgSize = totalRemainingSize * elementScaleWeights[childIdx];

				u32 extraHeight = std::min((u32)Math::CeilToInt(avgSize), remainingSize);
				u32 elementHeight = (u32)std::max(0, (i32)outElementSizes[childIdx].Height - (i32)extraHeight);

				// Clamp if needed
				switch(child->GetRtti()->GetRttiId())
				{
				case TID_GUIFlexibleSpace:
					outElementSizes[childIdx].Height = 0;
					processedElements[childIdx] = true;
					numNonClampedElements--;
					break;
				case TID_GUIFixedSpace:
					break;
				default:
					{
						const GUIConstrainedSize& childSizeRange = sizeRanges[childIdx];

						if(elementHeight == 0)
						{
							processedElements[childIdx] = true;
							numNonClampedElements--;
						}
						else if(childSizeRange.Min.Y > 0 && (i32)elementHeight < childSizeRange.Min.Y)
						{
							elementHeight = childSizeRange.Min.Y;

							processedElements[childIdx] = true;
							numNonClampedElements--;
						}

						extraHeight = outElementSizes[childIdx].Height - elementHeight;
						outElementSizes[childIdx].Height = elementHeight;
						remainingSize = (u32)std::max(0, (i32)remainingSize - (i32)extraHeight);
					}
					break;
				}

				childIdx++;
			}
		}
	}
	else // We are smaller than the allowed maximum, so try to expand some elements
	{
		u32 extraSize = layoutSize.Height - totalOptimalSize;
		u32 remainingSize = extraSize;

		// Iterate until we reduce everything so it fits, while maintaining
		// equal average sizes using the weights we calculated earlier
		while(remainingSize > 0 && numNonClampedElements > 0)
		{
			u32 totalRemainingSize = remainingSize;

			childIdx = 0;
			for(auto& child : mChildren)
			{
				if(processedElements[childIdx])
				{
					childIdx++;
					continue;
				}

				float avgSize = totalRemainingSize * elementScaleWeights[childIdx];
				u32 extraHeight = std::min((u32)Math::CeilToInt(avgSize), remainingSize);
				u32 elementHeight = outElementSizes[childIdx].Height + extraHeight;

				// Clamp if needed
				switch(child->GetRtti()->GetRttiId())
				{
				case TID_GUIFlexibleSpace:
					processedElements[childIdx] = true;
					numNonClampedElements--;
					break;
				case TID_GUIFixedSpace:
					break;
				default:
					{
						const GUIConstrainedSize& childSizeRange = sizeRanges[childIdx];

						if(elementHeight == 0)
						{
							processedElements[childIdx] = true;
							numNonClampedElements--;
						}
						else if(childSizeRange.Max.Y > 0 && (i32)elementHeight > childSizeRange.Max.Y)
						{
							elementHeight = childSizeRange.Max.Y;

							processedElements[childIdx] = true;
							numNonClampedElements--;
						}

						extraHeight = elementHeight - outElementSizes[childIdx].Height;
						outElementSizes[childIdx].Height = elementHeight;
						remainingSize = (u32)std::max(0, (i32)remainingSize - (i32)extraHeight);
					}
					break;
				}

				childIdx++;
			}
		}
	}

	if(elementScaleWeights != nullptr)
		B3DStackFree(elementScaleWeights);

	if(processedElements != nullptr)
		B3DStackFree(processedElements);

	// Compute offsets and width
	u32 yOffset = 0;
	childIdx = 0;

	for(auto& child : mChildren)
	{
		u32 elemHeight = outElementSizes[childIdx].Height;
		yOffset += child->GetMargins().Top;

		const GUIConstrainedSize& sizeRange = sizeRanges[childIdx];
		u32 elemWidth = (u32)sizeRanges[childIdx].Optimal.X;
		const GUISizeConstraints& dimensions = child->GetSizeConstraints();
		if(!dimensions.IsWidthFixed())
		{
			elemWidth = layoutSize.Width;
			if(sizeRange.Min.X > 0 && elemWidth < (u32)sizeRange.Min.X)
				elemWidth = (u32)sizeRange.Min.X;

			if(sizeRange.Max.X > 0 && elemWidth > (u32)sizeRange.Max.X)
				elemWidth = (u32)sizeRange.Max.X;
		}

		outElementSizes[childIdx].Width = elemWidth;

		if(GUIInteractable* const element = B3DRTTICast<GUIInteractable>(child))
		{
			u32 xPadding = element->GetMargins().Left + element->GetMargins().Right;
			i32 xOffset = Math::CeilToInt((i32)(layoutSize.Width - (i32)(elemWidth + xPadding)) * 0.5f);
			xOffset = std::max(0, xOffset);

			outElementPositions[childIdx].X = xOffset;
			outElementPositions[childIdx].Y = yOffset;
		}
		else
		{
			outElementPositions[childIdx].X = 0;
			outElementPositions[childIdx].Y = yOffset;
		}

		yOffset += elemHeight + child->GetMargins().Bottom;
		childIdx++;
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
