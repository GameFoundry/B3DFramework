//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUILayoutY.h"
#include "BsGUIElement.h"
#include "BsGUISpace.h"
#include "BsMath.h"
#include "BsVector2I.h"

namespace bs
{
	GUILayoutY::GUILayoutY(const GUIDimensions& dimensions)
		: GUILayout(dimensions)
	{ }

	LayoutSizeRange GUILayoutY::_calculateLayoutSizeRange() const
	{
		Vector2I optimalSize;
		Vector2I minSize;

		for (auto& child : mChildren)
		{
			if (!child->_isActive())
				continue;

			LayoutSizeRange sizeRange = child->_calculateLayoutSizeRange();
			
			if (child->_getType() == GUIElementBase::Type::FixedSpace)
				sizeRange.optimal.x = sizeRange.min.x = 0;

			UINT32 paddingX = child->_getPadding().left + child->_getPadding().right;
			UINT32 paddingY = child->_getPadding().top + child->_getPadding().bottom;

			optimalSize.y += sizeRange.optimal.y + paddingY;
			optimalSize.x = std::max((UINT32)optimalSize.x, sizeRange.optimal.x + paddingX);

			minSize.y += sizeRange.min.y + paddingY;
			minSize.x = std::max((UINT32)minSize.x, sizeRange.min.x + paddingX);
		}

		LayoutSizeRange sizeRange = _getDimensions().calculateSizeRange(optimalSize);
		sizeRange.min.x = std::max(sizeRange.min.x, minSize.x);
		sizeRange.min.y = std::max(sizeRange.min.y, minSize.y);

		return sizeRange;
	}

	void GUILayoutY::_updateOptimalLayoutSizes()
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
					childSizeRange.optimal.x = 0;
					childSizeRange.min.x = 0;
				}

				UINT32 paddingX = child->_getPadding().left + child->_getPadding().right;
				UINT32 paddingY = child->_getPadding().top + child->_getPadding().bottom;

				optimalSize.y += childSizeRange.optimal.y + paddingY;
				optimalSize.x = std::max((UINT32)optimalSize.x, childSizeRange.optimal.x + paddingX);

				minSize.y += childSizeRange.min.y + paddingY;
				minSize.x = std::max((UINT32)minSize.x, childSizeRange.min.x + paddingX);
			}
			else
				childSizeRange = LayoutSizeRange();

			childIdx++;
		}

		mSizeRange = _getDimensions().calculateSizeRange(optimalSize);
		mSizeRange.min.x = std::max(mSizeRange.min.x, minSize.x);
		mSizeRange.min.y = std::max(mSizeRange.min.y, minSize.y);
	}

	void GUILayoutY::_getElementAreas(const Rect2I& layoutArea, Rect2I* elementAreas, UINT32 numElements,
		const Vector<LayoutSizeRange>& sizeRanges, const LayoutSizeRange& mySizeRange) const
	{
		assert(mChildren.size() == numElements);

		UINT32 totalOptimalSize = mySizeRange.optimal.y;
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
			elementAreas[childIdx].height = sizeRanges[childIdx].optimal.y;

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

				if (dimensions.fixedHeight())
					processedElements[childIdx] = true;
				else
				{
					if (elementAreas[childIdx].height > 0)
					{
						numNonClampedElements++;
						totalNonClampedSize += elementAreas[childIdx].height;
					}
					else
						processedElements[childIdx] = true;
				}
			}

			childIdx++;
		}

		// If there is some room left, calculate flexible space sizes (since they will fill up all that extra room)
		if ((UINT32)layoutArea.height > totalOptimalSize)
		{
			UINT32 extraSize = layoutArea.height - totalOptimalSize;
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

					UINT32 extraHeight = std::min((UINT32)Math::ceilToInt(avgSize), remainingSize);
					UINT32 elementHeight = elementAreas[childIdx].height + extraHeight;

					// Clamp if needed
					if (child->_getType() == GUIElementBase::Type::FlexibleSpace)
					{
						processedElements[childIdx] = true;
						numNonClampedElements--;
						elementAreas[childIdx].height = elementHeight;

						remainingSize = (UINT32)std::max(0, (INT32)remainingSize - (INT32)extraHeight);
					}

					childIdx++;
				}

				totalOptimalSize = layoutArea.height;
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

			elementScaleWeights[childIdx] = invOptimalSize * elementAreas[childIdx].height;

			childIdx++;
		}

		// Our optimal size is larger than maximum allowed, so we need to reduce size of some elements
		if (totalOptimalSize > (UINT32)layoutArea.height)
		{
			UINT32 extraSize = totalOptimalSize - layoutArea.height;
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

					UINT32 extraHeight = std::min((UINT32)Math::ceilToInt(avgSize), remainingSize);
					UINT32 elementHeight = (UINT32)std::max(0, (INT32)elementAreas[childIdx].height - (INT32)extraHeight);

					// Clamp if needed
					switch (child->_getType())
					{
					case GUIElementBase::Type::FlexibleSpace:
						elementAreas[childIdx].height = 0;
						processedElements[childIdx] = true;
						numNonClampedElements--;
						break;
					case GUIElementBase::Type::Element:
					case GUIElementBase::Type::Layout:
					case GUIElementBase::Type::Panel:
					{
						const LayoutSizeRange& childSizeRange = sizeRanges[childIdx];

						if (elementHeight == 0)
						{
							processedElements[childIdx] = true;
							numNonClampedElements--;
						}
						else if (childSizeRange.min.y > 0 && (INT32)elementHeight < childSizeRange.min.y)
						{
							elementHeight = childSizeRange.min.y;

							processedElements[childIdx] = true;
							numNonClampedElements--;
						}

						extraHeight = elementAreas[childIdx].height - elementHeight;
						elementAreas[childIdx].height = elementHeight;
						remainingSize = (UINT32)std::max(0, (INT32)remainingSize - (INT32)extraHeight);
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
			UINT32 extraSize = layoutArea.height - totalOptimalSize;
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
					UINT32 extraHeight = std::min((UINT32)Math::ceilToInt(avgSize), remainingSize);
					UINT32 elementHeight = elementAreas[childIdx].height + extraHeight;

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

						if (elementHeight == 0)
						{
							processedElements[childIdx] = true;
							numNonClampedElements--;
						}
						else if (childSizeRange.max.y > 0 && (INT32)elementHeight > childSizeRange.max.y)
						{
							elementHeight = childSizeRange.max.y;

							processedElements[childIdx] = true;
							numNonClampedElements--;
						}

						extraHeight = elementHeight - elementAreas[childIdx].height;
						elementAreas[childIdx].height = elementHeight;
						remainingSize = (UINT32)std::max(0, (INT32)remainingSize - (INT32)extraHeight);
					}
						break;
					case GUIElementBase::Type::FixedSpace:
						break;
					}

					childIdx++;
				}
			}
		}

		if (elementScaleWeights != nullptr)
			bs_stack_free(elementScaleWeights);

		if (processedElements != nullptr)
			bs_stack_free(processedElements);

		// Compute offsets and width
		UINT32 yOffset = 0;
		childIdx = 0;

		for (auto& child : mChildren)
		{
			UINT32 elemHeight = elementAreas[childIdx].height;
			yOffset += child->_getPadding().top;

			const LayoutSizeRange& sizeRange = sizeRanges[childIdx];
			UINT32 elemWidth = (UINT32)sizeRanges[childIdx].optimal.x;
			const GUIDimensions& dimensions = child->_getDimensions();
			if (!dimensions.fixedWidth())
			{
				elemWidth = layoutArea.width;
				if (sizeRange.min.x > 0 && elemWidth < (UINT32)sizeRange.min.x)
					elemWidth = (UINT32)sizeRange.min.x;

				if (sizeRange.max.x > 0 && elemWidth > (UINT32)sizeRange.max.x)
					elemWidth = (UINT32)sizeRange.max.x;
			}

			elementAreas[childIdx].width = elemWidth;

			if (child->_getType() == GUIElementBase::Type::Element)
			{
				GUIElement* element = static_cast<GUIElement*>(child);

				UINT32 xPadding = element->_getPadding().left + element->_getPadding().right;
				INT32 xOffset = Math::ceilToInt((INT32)(layoutArea.width - (INT32)(elemWidth + xPadding)) * 0.5f);
				xOffset = std::max(0, xOffset);

				elementAreas[childIdx].x = layoutArea.x + xOffset;
				elementAreas[childIdx].y = layoutArea.y + yOffset;
			}
			else
			{
				elementAreas[childIdx].x = layoutArea.x;
				elementAreas[childIdx].y = layoutArea.y + yOffset;
			}

			yOffset += elemHeight + child->_getPadding().bottom;
			childIdx++;
		}
	}

	void GUILayoutY::_updateLayoutInternal(const GUILayoutData& data)
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

		if (elementAreas != nullptr)
			bs_stack_free(elementAreas);
	}

	GUILayoutY* GUILayoutY::create()
	{
		return bs_new<GUILayoutY>();
	}

	GUILayoutY* GUILayoutY::create(const GUIOptions& options)
	{
		return bs_new<GUILayoutY>(GUIDimensions::create(options));
	}
}