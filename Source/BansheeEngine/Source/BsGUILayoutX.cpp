//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUILayoutX.h"
#include "BsGUIElement.h"
#include "BsGUISpace.h"
#include "BsMath.h"
#include "BsVector2I.h"
#include "BsProfilerCPU.h"

namespace BansheeEngine
{
	GUILayoutX::GUILayoutX(const GUIDimensions& dimensions)
		: GUILayout(dimensions)
	{ }

	LayoutSizeRange GUILayoutX::_calculateLayoutSizeRange() const
	{
		Vector2I optimalSize;
		Vector2I minSize;
		for (auto& child : mChildren)
		{
			if (!child->_isActive())
				continue;

			LayoutSizeRange sizeRange = child->_calculateLayoutSizeRange();

			if (child->_getType() == GUIElementBase::Type::FixedSpace)
				sizeRange.optimal.y = sizeRange.min.y = 0;

			UINT32 paddingX = child->_getPadding().left + child->_getPadding().right;
			UINT32 paddingY = child->_getPadding().top + child->_getPadding().bottom;

			optimalSize.x += sizeRange.optimal.x + paddingX;
			optimalSize.y = std::max((UINT32)optimalSize.y, sizeRange.optimal.y + paddingY);

			minSize.x += sizeRange.min.x + paddingX;
			minSize.y = std::max((UINT32)minSize.y, sizeRange.min.y + paddingY);
		}

		LayoutSizeRange sizeRange = _getDimensions().calculateSizeRange(optimalSize);
		sizeRange.min.x = std::max(sizeRange.min.x, minSize.x);
		sizeRange.min.y = std::max(sizeRange.min.y, minSize.y);

		return sizeRange;
	}

	void GUILayoutX::_updateOptimalLayoutSizes()
	{
		// Update all children first, otherwise we can't determine our own optimal size
		GUIElementBase::_updateOptimalLayoutSizes();

		if(mChildren.size() != mChildSizeRanges.size())
			mChildSizeRanges.resize(mChildren.size());

		Vector2I optimalSize;
		Vector2I minSize;

		UINT32 childIdx = 0;
		for(auto& child : mChildren)
		{
			LayoutSizeRange& childSizeRange = mChildSizeRanges[childIdx];

			if (child->_isActive())
			{
				childSizeRange = child->_getLayoutSizeRange();
				if (child->_getType() == GUIElementBase::Type::FixedSpace)
				{
					childSizeRange.optimal.y = 0;
					childSizeRange.min.y = 0;
				}

				UINT32 paddingX = child->_getPadding().left + child->_getPadding().right;
				UINT32 paddingY = child->_getPadding().top + child->_getPadding().bottom;

				optimalSize.x += childSizeRange.optimal.x + paddingX;
				optimalSize.y = std::max((UINT32)optimalSize.y, childSizeRange.optimal.y + paddingY);

				minSize.x += childSizeRange.min.x + paddingX;
				minSize.y = std::max((UINT32)minSize.y, childSizeRange.min.y + paddingY);
			}
			else
				childSizeRange = LayoutSizeRange();

			childIdx++;
		}

		mSizeRange = _getDimensions().calculateSizeRange(optimalSize);
		mSizeRange.min.x = std::max(mSizeRange.min.x, minSize.x);
		mSizeRange.min.y = std::max(mSizeRange.min.y, minSize.y);
	}

	void GUILayoutX::_getElementAreas(const Rect2I& layoutArea, Rect2I* elementAreas, UINT32 numElements,
		const Vector<LayoutSizeRange>& sizeRanges, const LayoutSizeRange& mySizeRange) const
	{
		assert(mChildren.size() == numElements);

		UINT32 totalOptimalSize = mySizeRange.optimal.x;
		UINT32 totalNonClampedSize = 0;
		UINT32 numNonClampedElements = 0;
		UINT32 numFlexibleSpaces = 0;

		bool* processedElements = nullptr;
		float* elementScaleWeights = nullptr;

		if (mChildren.size() > 0)
		{
			processedElements = bs_stack_alloc<bool>((UINT32)mChildren.size());
			memset(processedElements, 0, mChildren.size() * sizeof(bool));

			elementScaleWeights = bs_stack_alloc<float>((UINT32)mChildren.size());
			memset(elementScaleWeights, 0, mChildren.size() * sizeof(float));
		}

		// Set initial sizes, count number of children per type and mark fixed elements as already processed
		UINT32 childIdx = 0;
		for (auto& child : mChildren)
		{
			elementAreas[childIdx].width = sizeRanges[childIdx].optimal.x;

			if (child->_getType() == GUIElementBase::Type::FixedSpace)
			{
				processedElements[childIdx] = true;
			}
			else if (child->_getType() == GUIElementBase::Type::FlexibleSpace)
			{
				if (child->_isActive())
				{
					numFlexibleSpaces++;
					numNonClampedElements++;
				}
				else
					processedElements[childIdx] = true;
			}
			else
			{
				const GUIDimensions& dimensions = child->_getDimensions();

				if (dimensions.fixedWidth())
					processedElements[childIdx] = true;
				else
				{
					if (elementAreas[childIdx].width > 0)
					{
						numNonClampedElements++;
						totalNonClampedSize += elementAreas[childIdx].width;
					}
					else
						processedElements[childIdx] = true;
				}
			}

			childIdx++;
		}

		// If there is some room left, calculate flexible space sizes (since they will fill up all that extra room)
		if ((UINT32)layoutArea.width > totalOptimalSize)
		{
			UINT32 extraSize = layoutArea.width - totalOptimalSize;
			UINT32 remainingSize = extraSize;

			// Flexible spaces always expand to fill up all unused space
			if (numFlexibleSpaces > 0)
			{
				float avgSize = remainingSize / (float)numFlexibleSpaces;

				childIdx = 0;
				for (auto& child : mChildren)
				{
					if (processedElements[childIdx])
					{
						childIdx++;
						continue;
					}

					UINT32 extraWidth = std::min((UINT32)Math::ceilToInt(avgSize), remainingSize);
					UINT32 elementWidth = elementAreas[childIdx].width + extraWidth;

					// Clamp if needed
					if (child->_getType() == GUIElementBase::Type::FlexibleSpace)
					{
						processedElements[childIdx] = true;
						numNonClampedElements--;
						elementAreas[childIdx].width = elementWidth;

						remainingSize = (UINT32)std::max(0, (INT32)remainingSize - (INT32)extraWidth);
					}

					childIdx++;
				}

				totalOptimalSize = layoutArea.width;
			}
		}

		// Determine weight scale for every element. When scaling elements up/down they will be scaled based on this weight.
		// Weight is to ensure all elements are scaled fairly, so elements that are large will get effected more than smaller elements.
		childIdx = 0;
		float invOptimalSize = 1.0f / totalNonClampedSize;
		UINT32 childCount = (UINT32)mChildren.size();
		for (UINT32 i = 0; i < childCount; i++)
		{
			if (processedElements[childIdx])
			{
				childIdx++;
				continue;
			}

			elementScaleWeights[childIdx] = invOptimalSize * elementAreas[childIdx].width;

			childIdx++;
		}

		// Our optimal size is larger than maximum allowed, so we need to reduce size of some elements
		if (totalOptimalSize > (UINT32)layoutArea.width)
		{
			UINT32 extraSize = totalOptimalSize - layoutArea.width;
			UINT32 remainingSize = extraSize;

			// Iterate until we reduce everything so it fits, while maintaining
			// equal average sizes using the weights we calculated earlier
			while (remainingSize > 0 && numNonClampedElements > 0)
			{
				UINT32 totalRemainingSize = remainingSize;

				childIdx = 0;
				for (auto& child : mChildren)
				{
					if (processedElements[childIdx])
					{
						childIdx++;
						continue;
					}

					float avgSize = totalRemainingSize * elementScaleWeights[childIdx];

					UINT32 extraWidth = std::min((UINT32)Math::ceilToInt(avgSize), remainingSize);
					UINT32 elementWidth = (UINT32)std::max(0, (INT32)elementAreas[childIdx].width - (INT32)extraWidth);

					// Clamp if needed
					switch (child->_getType())
					{
					case GUIElementBase::Type::FlexibleSpace:
						elementAreas[childIdx].width = 0;
						processedElements[childIdx] = true;
						numNonClampedElements--;
						break;
					case GUIElementBase::Type::Element:
					case GUIElementBase::Type::Layout:
					case GUIElementBase::Type::Panel:
					{
						const LayoutSizeRange& childSizeRange = sizeRanges[childIdx];

						if (elementWidth == 0)
						{
							processedElements[childIdx] = true;
							numNonClampedElements--;
						}
						else if (childSizeRange.min.x > 0 && (INT32)elementWidth < childSizeRange.min.x)
						{
							elementWidth = childSizeRange.min.x;

							processedElements[childIdx] = true;
							numNonClampedElements--;
						}

						extraWidth = elementAreas[childIdx].width - elementWidth;
						elementAreas[childIdx].width = elementWidth;
						remainingSize = (UINT32)std::max(0, (INT32)remainingSize - (INT32)extraWidth);
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
			UINT32 extraSize = layoutArea.width - totalOptimalSize;
			UINT32 remainingSize = extraSize;

			// Iterate until we reduce everything so it fits, while maintaining
			// equal average sizes using the weights we calculated earlier
			while (remainingSize > 0 && numNonClampedElements > 0)
			{
				UINT32 totalRemainingSize = remainingSize;

				childIdx = 0;
				for (auto& child : mChildren)
				{
					if (processedElements[childIdx])
					{
						childIdx++;
						continue;
					}

					float avgSize = totalRemainingSize * elementScaleWeights[childIdx];
					UINT32 extraWidth = std::min((UINT32)Math::ceilToInt(avgSize), remainingSize);
					UINT32 elementWidth = elementAreas[childIdx].width + extraWidth;

					// Clamp if needed
					switch (child->_getType())
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

						if (elementWidth == 0)
						{
							processedElements[childIdx] = true;
							numNonClampedElements--;
						}
						else if (childSizeRange.max.x > 0 && (INT32)elementWidth > childSizeRange.max.x)
						{
							elementWidth = childSizeRange.max.x;

							processedElements[childIdx] = true;
							numNonClampedElements--;
						}

						extraWidth = elementWidth - elementAreas[childIdx].width;
						elementAreas[childIdx].width = elementWidth;
						remainingSize = (UINT32)std::max(0, (INT32)remainingSize - (INT32)extraWidth);
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
		UINT32 xOffset = 0;
		childIdx = 0;

		for (auto& child : mChildren)
		{
			UINT32 elemWidth = elementAreas[childIdx].width;
			xOffset += child->_getPadding().left;

			const LayoutSizeRange& sizeRange = sizeRanges[childIdx];
			UINT32 elemHeight = (UINT32)sizeRange.optimal.y;
			const GUIDimensions& dimensions = child->_getDimensions();
			if (!dimensions.fixedHeight())
			{
				elemHeight = layoutArea.height;
				if (sizeRange.min.y > 0 && elemHeight < (UINT32)sizeRange.min.y)
					elemHeight = (UINT32)sizeRange.min.y;

				if (sizeRange.max.y > 0 && elemHeight > (UINT32)sizeRange.max.y)
					elemHeight = (UINT32)sizeRange.max.y;
			}
			elementAreas[childIdx].height = elemHeight;

			if (child->_getType() == GUIElementBase::Type::Element)
			{
				GUIElement* element = static_cast<GUIElement*>(child);

				UINT32 yPadding = element->_getPadding().top + element->_getPadding().bottom;
				INT32 yOffset = Math::ceilToInt(((INT32)layoutArea.height - (INT32)(elemHeight + yPadding)) * 0.5f);
				yOffset = std::max(0, yOffset);

				elementAreas[childIdx].x = layoutArea.x + xOffset;
				elementAreas[childIdx].y = layoutArea.y + yOffset;
			}
			else
			{
				elementAreas[childIdx].x = layoutArea.x + xOffset;
				elementAreas[childIdx].y = layoutArea.y;
			}

			xOffset += elemWidth + child->_getPadding().right;
			childIdx++;
		}

		if (elementScaleWeights != nullptr)
			bs_stack_free(elementScaleWeights);

		if (processedElements != nullptr)
			bs_stack_free(processedElements);
	}

	void GUILayoutX::_updateLayoutInternal(const GUILayoutData& data)
	{
		UINT32 numElements = (UINT32)mChildren.size();
		Rect2I* elementAreas = nullptr;

		if (numElements > 0)
			elementAreas = bs_stack_new<Rect2I>(numElements);

		_getElementAreas(data.area, elementAreas, numElements, mChildSizeRanges, mSizeRange);

		// Now that we have all the areas, actually assign them
		UINT32 childIdx = 0;

		GUILayoutData childData = data;
		for(auto& child : mChildren)
		{
			if (child->_isActive())
			{
				childData.area = elementAreas[childIdx];
				childData.clipRect = childData.area;
				childData.clipRect.clip(data.clipRect);

				child->_setLayoutData(childData);
				child->_updateLayoutInternal(childData);
			}

			childIdx++;
		}

		if(elementAreas != nullptr)
			bs_stack_free(elementAreas);
	}

	GUILayoutX* GUILayoutX::create()
	{
		return bs_new<GUILayoutX>();
	}

	GUILayoutX* GUILayoutX::create(const GUIOptions& options)
	{
		return bs_new<GUILayoutX>(GUIDimensions::create(options));
	}
}