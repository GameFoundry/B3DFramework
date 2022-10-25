//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUILayoutX.h"
#include "GUI/BsGUIElement.h"
#include "GUI/BsGUISpace.h"
#include "Math/BsMath.h"
#include "Math/BsVector2I.h"
#include "Profiling/BsProfilerCPU.h"

namespace bs
{
	GUILayoutX::GUILayoutX(const GUIDimensions& dimensions)
		: GUILayout(dimensions)
	{}

	LayoutSizeRange GUILayoutX::CalculateLayoutSizeRangeInternal() const
	{
		Vector2I optimalSize;
		Vector2I minSize;
		for(auto& child : mChildren)
		{
			if(!child->IsActiveInternal())
				continue;

			LayoutSizeRange sizeRange = child->CalculateLayoutSizeRangeInternal();

			if(child->GetTypeInternal() == GUIElementBase::Type::FixedSpace)
				sizeRange.Optimal.Y = sizeRange.Min.Y = 0;

			u32 paddingX = child->GetPaddingInternal().Left + child->GetPaddingInternal().Right;
			u32 paddingY = child->GetPaddingInternal().Top + child->GetPaddingInternal().Bottom;

			optimalSize.X += sizeRange.Optimal.X + paddingX;
			optimalSize.Y = std::max((u32)optimalSize.Y, sizeRange.Optimal.Y + paddingY);

			minSize.X += sizeRange.Min.X + paddingX;
			minSize.Y = std::max((u32)minSize.Y, sizeRange.Min.Y + paddingY);
		}

		LayoutSizeRange sizeRange = GetDimensionsInternal().CalculateSizeRange(optimalSize);
		sizeRange.Min.X = std::max(sizeRange.Min.X, minSize.X);
		sizeRange.Min.Y = std::max(sizeRange.Min.Y, minSize.Y);

		return sizeRange;
	}

	void GUILayoutX::UpdateOptimalLayoutSizesInternal()
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
					childSizeRange.Optimal.Y = 0;
					childSizeRange.Min.Y = 0;
				}

				u32 paddingX = child->GetPaddingInternal().Left + child->GetPaddingInternal().Right;
				u32 paddingY = child->GetPaddingInternal().Top + child->GetPaddingInternal().Bottom;

				optimalSize.X += childSizeRange.Optimal.X + paddingX;
				optimalSize.Y = std::max((u32)optimalSize.Y, childSizeRange.Optimal.Y + paddingY);

				minSize.X += childSizeRange.Min.X + paddingX;
				minSize.Y = std::max((u32)minSize.Y, childSizeRange.Min.Y + paddingY);
			}
			else
				childSizeRange = LayoutSizeRange();

			childIdx++;
		}

		mSizeRange = GetDimensionsInternal().CalculateSizeRange(optimalSize);
		mSizeRange.Min.X = std::max(mSizeRange.Min.X, minSize.X);
		mSizeRange.Min.Y = std::max(mSizeRange.Min.Y, minSize.Y);
	}

	void GUILayoutX::GetElementAreasInternal(const Rect2I& layoutArea, Rect2I* elementAreas, u32 numElements, const Vector<LayoutSizeRange>& sizeRanges, const LayoutSizeRange& mySizeRange) const
	{
		assert(mChildren.size() == numElements);

		u32 totalOptimalSize = mySizeRange.Optimal.X;
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
			elementAreas[childIdx].Width = sizeRanges[childIdx].Optimal.X;

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

				if(dimensions.FixedWidth())
					processedElements[childIdx] = true;
				else
				{
					if(elementAreas[childIdx].Width > 0)
					{
						numNonClampedElements++;
						totalNonClampedSize += elementAreas[childIdx].Width;
					}
					else
						processedElements[childIdx] = true;
				}
			}

			childIdx++;
		}

		// If there is some room left, calculate flexible space sizes (since they will fill up all that extra room)
		if((u32)layoutArea.Width > totalOptimalSize)
		{
			u32 extraSize = layoutArea.Width - totalOptimalSize;
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

					u32 extraWidth = std::min((u32)Math::CeilToInt(avgSize), remainingSize);
					u32 elementWidth = elementAreas[childIdx].Width + extraWidth;

					// Clamp if needed
					if(child->GetTypeInternal() == GUIElementBase::Type::FlexibleSpace)
					{
						processedElements[childIdx] = true;
						numNonClampedElements--;
						elementAreas[childIdx].Width = elementWidth;

						remainingSize = (u32)std::max(0, (i32)remainingSize - (i32)extraWidth);
					}

					childIdx++;
				}

				totalOptimalSize = layoutArea.Width;
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

			elementScaleWeights[childIdx] = invOptimalSize * elementAreas[childIdx].Width;

			childIdx++;
		}

		// Our optimal size is larger than maximum allowed, so we need to reduce size of some elements
		if(totalOptimalSize > (u32)layoutArea.Width)
		{
			u32 extraSize = totalOptimalSize - layoutArea.Width;
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

					u32 extraWidth = std::min((u32)Math::CeilToInt(avgSize), remainingSize);
					u32 elementWidth = (u32)std::max(0, (i32)elementAreas[childIdx].Width - (i32)extraWidth);

					// Clamp if needed
					switch(child->GetTypeInternal())
					{
					case GUIElementBase::Type::FlexibleSpace:
						elementAreas[childIdx].Width = 0;
						processedElements[childIdx] = true;
						numNonClampedElements--;
						break;
					case GUIElementBase::Type::Element:
					case GUIElementBase::Type::Layout:
					case GUIElementBase::Type::Panel:
						{
							const LayoutSizeRange& childSizeRange = sizeRanges[childIdx];

							if(elementWidth == 0)
							{
								processedElements[childIdx] = true;
								numNonClampedElements--;
							}
							else if(childSizeRange.Min.X > 0 && (i32)elementWidth < childSizeRange.Min.X)
							{
								elementWidth = childSizeRange.Min.X;

								processedElements[childIdx] = true;
								numNonClampedElements--;
							}

							extraWidth = elementAreas[childIdx].Width - elementWidth;
							elementAreas[childIdx].Width = elementWidth;
							remainingSize = (u32)std::max(0, (i32)remainingSize - (i32)extraWidth);
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
			u32 extraSize = layoutArea.Width - totalOptimalSize;
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
					u32 extraWidth = std::min((u32)Math::CeilToInt(avgSize), remainingSize);
					u32 elementWidth = elementAreas[childIdx].Width + extraWidth;

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

							if(elementWidth == 0)
							{
								processedElements[childIdx] = true;
								numNonClampedElements--;
							}
							else if(childSizeRange.Max.X > 0 && (i32)elementWidth > childSizeRange.Max.X)
							{
								elementWidth = childSizeRange.Max.X;

								processedElements[childIdx] = true;
								numNonClampedElements--;
							}

							extraWidth = elementWidth - elementAreas[childIdx].Width;
							elementAreas[childIdx].Width = elementWidth;
							remainingSize = (u32)std::max(0, (i32)remainingSize - (i32)extraWidth);
						}
						break;
					case GUIElementBase::Type::FixedSpace:
						break;
					}

					childIdx++;
				}
			}
		}

		// Compute offsets and height
		u32 xOffset = 0;
		childIdx = 0;

		for(auto& child : mChildren)
		{
			u32 elemWidth = elementAreas[childIdx].Width;
			xOffset += child->GetPaddingInternal().Left;

			const LayoutSizeRange& sizeRange = sizeRanges[childIdx];
			u32 elemHeight = (u32)sizeRange.Optimal.Y;
			const GUIDimensions& dimensions = child->GetDimensionsInternal();
			if(!dimensions.FixedHeight())
			{
				elemHeight = layoutArea.Height;
				if(sizeRange.Min.Y > 0 && elemHeight < (u32)sizeRange.Min.Y)
					elemHeight = (u32)sizeRange.Min.Y;

				if(sizeRange.Max.Y > 0 && elemHeight > (u32)sizeRange.Max.Y)
					elemHeight = (u32)sizeRange.Max.Y;
			}
			elementAreas[childIdx].Height = elemHeight;

			if(child->GetTypeInternal() == GUIElementBase::Type::Element)
			{
				GUIElement* element = static_cast<GUIElement*>(child);

				u32 yPadding = element->GetPaddingInternal().Top + element->GetPaddingInternal().Bottom;
				i32 yOffset = Math::CeilToInt(((i32)layoutArea.Height - (i32)(elemHeight + yPadding)) * 0.5f);
				yOffset = std::max(0, yOffset);

				elementAreas[childIdx].X = layoutArea.X + xOffset;
				elementAreas[childIdx].Y = layoutArea.Y + yOffset;
			}
			else
			{
				elementAreas[childIdx].X = layoutArea.X + xOffset;
				elementAreas[childIdx].Y = layoutArea.Y;
			}

			xOffset += elemWidth + child->GetPaddingInternal().Right;
			childIdx++;
		}

		if(elementScaleWeights != nullptr)
			bs_stack_free(elementScaleWeights);

		if(processedElements != nullptr)
			bs_stack_free(processedElements);
	}

	void GUILayoutX::UpdateLayoutInternalInternal(const GUILayoutData& data)
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

	GUILayoutX* GUILayoutX::Create()
	{
		return bs_new<GUILayoutX>();
	}

	GUILayoutX* GUILayoutX::Create(const GUIOptions& options)
	{
		return bs_new<GUILayoutX>(GUIDimensions::Create(options));
	}
} // namespace bs
