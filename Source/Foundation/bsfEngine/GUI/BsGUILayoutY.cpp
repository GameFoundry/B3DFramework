//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUILayoutY.h"
#include "GUI/BsGUIElement.h"
#include "GUI/BsGUISpace.h"
#include "Math/BsMath.h"
#include "Math/BsVector2I.h"

using namespace bs;

GUILayoutY::GUILayoutY(const GUIDimensions& dimensions)
	: GUILayout(dimensions)
{}

LayoutSizeRange GUILayoutY::CalculateLayoutSizeRangeInternal() const
{
	Vector2I optimalSize;
	Vector2I minSize;

	for(auto& child : mChildren)
	{
		if(!child->IsActiveInternal())
			continue;

		LayoutSizeRange sizeRange = child->CalculateLayoutSizeRangeInternal();

		if(child->GetTypeInternal() == GUIElementBase::Type::FixedSpace)
			sizeRange.Optimal.X = sizeRange.Min.X = 0;

		u32 paddingX = child->GetPaddingInternal().Left + child->GetPaddingInternal().Right;
		u32 paddingY = child->GetPaddingInternal().Top + child->GetPaddingInternal().Bottom;

		optimalSize.Y += sizeRange.Optimal.Y + paddingY;
		optimalSize.X = std::max((u32)optimalSize.X, sizeRange.Optimal.X + paddingX);

		minSize.Y += sizeRange.Min.Y + paddingY;
		minSize.X = std::max((u32)minSize.X, sizeRange.Min.X + paddingX);
	}

	LayoutSizeRange sizeRange = GetDimensionsInternal().CalculateSizeRange(optimalSize);
	sizeRange.Min.X = std::max(sizeRange.Min.X, minSize.X);
	sizeRange.Min.Y = std::max(sizeRange.Min.Y, minSize.Y);

	return sizeRange;
}

void GUILayoutY::UpdateOptimalLayoutSizesInternal()
{
	// Update all children first, otherwise we can't determine our own optimal size
	GUIElementBase::UpdateOptimalLayoutSizesInternal();

	if(mChildren.size() != mChildSizeRanges.size())
		mChildSizeRanges.resize(mChildren.size());

	Vector2I optimalSize;
	Vector2I minSize;

	u32 childIdx = 0;
	for(auto& child : mChildren)
	{
		LayoutSizeRange& childSizeRange = mChildSizeRanges[childIdx];

		if(child->IsActiveInternal())
		{
			childSizeRange = child->GetLayoutSizeRangeInternal();
			if(child->GetTypeInternal() == GUIElementBase::Type::FixedSpace)
			{
				childSizeRange.Optimal.X = 0;
				childSizeRange.Min.X = 0;
			}

			u32 paddingX = child->GetPaddingInternal().Left + child->GetPaddingInternal().Right;
			u32 paddingY = child->GetPaddingInternal().Top + child->GetPaddingInternal().Bottom;

			optimalSize.Y += childSizeRange.Optimal.Y + paddingY;
			optimalSize.X = std::max((u32)optimalSize.X, childSizeRange.Optimal.X + paddingX);

			minSize.Y += childSizeRange.Min.Y + paddingY;
			minSize.X = std::max((u32)minSize.X, childSizeRange.Min.X + paddingX);
		}
		else
			childSizeRange = LayoutSizeRange();

		childIdx++;
	}

	mSizeRange = GetDimensionsInternal().CalculateSizeRange(optimalSize);
	mSizeRange.Min.X = std::max(mSizeRange.Min.X, minSize.X);
	mSizeRange.Min.Y = std::max(mSizeRange.Min.Y, minSize.Y);
}

void GUILayoutY::GetElementAreasInternal(const Rect2I& layoutArea, Rect2I* elementAreas, u32 numElements, const Vector<LayoutSizeRange>& sizeRanges, const LayoutSizeRange& mySizeRange) const
{
	assert(mChildren.size() == numElements);

	u32 totalOptimalSize = mySizeRange.Optimal.Y;
	u32 totalNonClampedSize = 0;
	u32 numNonClampedElements = 0;
	u32 numFlexibleSpaces = 0;

	bool* processedElements = nullptr;
	float* elementScaleWeights = nullptr;

	if(mChildren.size() > 0)
	{
		processedElements = bs_stack_alloc<bool>((u32)mChildren.size());
		memset(processedElements, 0, mChildren.size() * sizeof(bool));

		elementScaleWeights = bs_stack_alloc<float>((u32)mChildren.size());
		memset(elementScaleWeights, 0, mChildren.size() * sizeof(float));
	}

	// Set initial sizes, count number of children per type and mark fixed elements as already processed
	u32 childIdx = 0;
	for(auto& child : mChildren)
	{
		elementAreas[childIdx].Height = sizeRanges[childIdx].Optimal.Y;

		if(child->GetTypeInternal() == GUIElementBase::Type::FixedSpace)
		{
			processedElements[childIdx] = true;
		}
		else if(child->GetTypeInternal() == GUIElementBase::Type::FlexibleSpace)
		{
			if(child->IsActiveInternal())
			{
				numFlexibleSpaces++;
				numNonClampedElements++;
			}
			else
				processedElements[childIdx] = true;
		}
		else
		{
			const GUIDimensions& dimensions = child->GetDimensionsInternal();

			if(dimensions.FixedHeight())
				processedElements[childIdx] = true;
			else
			{
				if(elementAreas[childIdx].Height > 0)
				{
					numNonClampedElements++;
					totalNonClampedSize += elementAreas[childIdx].Height;
				}
				else
					processedElements[childIdx] = true;
			}
		}

		childIdx++;
	}

	// If there is some room left, calculate flexible space sizes (since they will fill up all that extra room)
	if((u32)layoutArea.Height > totalOptimalSize)
	{
		u32 extraSize = layoutArea.Height - totalOptimalSize;
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
				u32 elementHeight = elementAreas[childIdx].Height + extraHeight;

				// Clamp if needed
				if(child->GetTypeInternal() == GUIElementBase::Type::FlexibleSpace)
				{
					processedElements[childIdx] = true;
					numNonClampedElements--;
					elementAreas[childIdx].Height = elementHeight;

					remainingSize = (u32)std::max(0, (i32)remainingSize - (i32)extraHeight);
				}

				childIdx++;
			}

			totalOptimalSize = layoutArea.Height;
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

		elementScaleWeights[childIdx] = invOptimalSize * elementAreas[childIdx].Height;

		childIdx++;
	}

	// Our optimal size is larger than maximum allowed, so we need to reduce size of some elements
	if(totalOptimalSize > (u32)layoutArea.Height)
	{
		u32 extraSize = totalOptimalSize - layoutArea.Height;
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
				u32 elementHeight = (u32)std::max(0, (i32)elementAreas[childIdx].Height - (i32)extraHeight);

				// Clamp if needed
				switch(child->GetTypeInternal())
				{
				case GUIElementBase::Type::FlexibleSpace:
					elementAreas[childIdx].Height = 0;
					processedElements[childIdx] = true;
					numNonClampedElements--;
					break;
				case GUIElementBase::Type::Element:
				case GUIElementBase::Type::Layout:
				case GUIElementBase::Type::Panel:
					{
						const LayoutSizeRange& childSizeRange = sizeRanges[childIdx];

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

						extraHeight = elementAreas[childIdx].Height - elementHeight;
						elementAreas[childIdx].Height = elementHeight;
						remainingSize = (u32)std::max(0, (i32)remainingSize - (i32)extraHeight);
					}
					break;
				case GUIElementBase::Type::FixedSpace:
					break;
				}

				childIdx++;
			}
		}
	}
	else // We are smaller than the allowed maximum, so try to expand some elements
	{
		u32 extraSize = layoutArea.Height - totalOptimalSize;
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
				u32 elementHeight = elementAreas[childIdx].Height + extraHeight;

				// Clamp if needed
				switch(child->GetTypeInternal())
				{
				case GUIElementBase::Type::FlexibleSpace:
					processedElements[childIdx] = true;
					numNonClampedElements--;
					break;
				case GUIElementBase::Type::Element:
				case GUIElementBase::Type::Layout:
				case GUIElementBase::Type::Panel:
					{
						const LayoutSizeRange& childSizeRange = sizeRanges[childIdx];

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

						extraHeight = elementHeight - elementAreas[childIdx].Height;
						elementAreas[childIdx].Height = elementHeight;
						remainingSize = (u32)std::max(0, (i32)remainingSize - (i32)extraHeight);
					}
					break;
				case GUIElementBase::Type::FixedSpace:
					break;
				}

				childIdx++;
			}
		}
	}

	if(elementScaleWeights != nullptr)
		bs_stack_free(elementScaleWeights);

	if(processedElements != nullptr)
		bs_stack_free(processedElements);

	// Compute offsets and width
	u32 yOffset = 0;
	childIdx = 0;

	for(auto& child : mChildren)
	{
		u32 elemHeight = elementAreas[childIdx].Height;
		yOffset += child->GetPaddingInternal().Top;

		const LayoutSizeRange& sizeRange = sizeRanges[childIdx];
		u32 elemWidth = (u32)sizeRanges[childIdx].Optimal.X;
		const GUIDimensions& dimensions = child->GetDimensionsInternal();
		if(!dimensions.FixedWidth())
		{
			elemWidth = layoutArea.Width;
			if(sizeRange.Min.X > 0 && elemWidth < (u32)sizeRange.Min.X)
				elemWidth = (u32)sizeRange.Min.X;

			if(sizeRange.Max.X > 0 && elemWidth > (u32)sizeRange.Max.X)
				elemWidth = (u32)sizeRange.Max.X;
		}

		elementAreas[childIdx].Width = elemWidth;

		if(child->GetTypeInternal() == GUIElementBase::Type::Element)
		{
			GUIElement* element = static_cast<GUIElement*>(child);

			u32 xPadding = element->GetPaddingInternal().Left + element->GetPaddingInternal().Right;
			i32 xOffset = Math::CeilToInt((i32)(layoutArea.Width - (i32)(elemWidth + xPadding)) * 0.5f);
			xOffset = std::max(0, xOffset);

			elementAreas[childIdx].X = layoutArea.X + xOffset;
			elementAreas[childIdx].Y = layoutArea.Y + yOffset;
		}
		else
		{
			elementAreas[childIdx].X = layoutArea.X;
			elementAreas[childIdx].Y = layoutArea.Y + yOffset;
		}

		yOffset += elemHeight + child->GetPaddingInternal().Bottom;
		childIdx++;
	}
}

void GUILayoutY::UpdateLayoutInternalInternal(const GUILayoutData& data)
{
	u32 numElements = (u32)mChildren.size();
	Rect2I* elementAreas = nullptr;

	if(numElements > 0)
		elementAreas = bs_stack_new<Rect2I>(numElements);

	GetElementAreasInternal(data.Area, elementAreas, numElements, mChildSizeRanges, mSizeRange);

	// Now that we have all the areas, actually assign them
	u32 childIdx = 0;

	GUILayoutData childData = data;
	for(auto& child : mChildren)
	{
		if(child->IsActiveInternal())
		{
			childData.Area = elementAreas[childIdx];
			childData.ClipRect = childData.Area;
			childData.ClipRect.Clip(data.ClipRect);

			child->SetLayoutDataInternal(childData);
			child->UpdateLayoutInternalInternal(childData);
		}

		childIdx++;
	}

	if(elementAreas != nullptr)
		bs_stack_free(elementAreas);
}

GUILayoutY* GUILayoutY::Create()
{
	return bs_new<GUILayoutY>();
}

GUILayoutY* GUILayoutY::Create(const GUIOptions& options)
{
	return bs_new<GUILayoutY>(GUIDimensions::Create(options));
}
