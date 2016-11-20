//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsDropDownAreaPlacement.h"
#include "BsDebug.h"

namespace bs
{
	DropDownAreaPlacement DropDownAreaPlacement::aroundPosition(const Vector2I& position)
	{
		DropDownAreaPlacement instance;
		instance.mType = Type::Position;
		instance.mPosition = position;

		return instance;
	}

	DropDownAreaPlacement DropDownAreaPlacement::aroundBoundsVert(const Rect2I& bounds)
	{
		DropDownAreaPlacement instance;
		instance.mType = Type::BoundsVert;
		instance.mBounds = bounds;

		return instance;
	}

	DropDownAreaPlacement DropDownAreaPlacement::aroundBoundsHorz(const Rect2I& bounds)
	{
		DropDownAreaPlacement instance;
		instance.mType = Type::BoundsHorz;
		instance.mBounds = bounds;

		return instance;
	}

	DropDownAreaPlacement DropDownAreaPlacement::aroundBounds(const Rect2I& bounds)
	{
		DropDownAreaPlacement instance;
		instance.mType = Type::BoundsAll;
		instance.mBounds = bounds;

		return instance;
	}

	Rect2I DropDownAreaPlacement::getOptimalBounds(UINT32 width, UINT32 height, const Rect2I& availableArea, HorzDir& horzDir, VertDir& vertDir) const
	{
		Rect2I output;

		int potentialLeftStart = 0;
		int potentialRightStart = 0;
		int potentialTopStart = 0;
		int potentialBottomStart = 0;

		switch (getType())
		{
		case DropDownAreaPlacement::Type::Position:
			potentialLeftStart = potentialRightStart = getPosition().x;
			potentialTopStart = potentialBottomStart = getPosition().y;
			break;
		case DropDownAreaPlacement::Type::BoundsHorz:
			potentialRightStart = getBounds().x;
			potentialLeftStart = getBounds().x + getBounds().width;
			potentialBottomStart = getBounds().y + getBounds().height;
			potentialTopStart = getBounds().y;
			break;
		case DropDownAreaPlacement::Type::BoundsVert:
			potentialRightStart = getBounds().x + getBounds().width;
			potentialLeftStart = getBounds().x;
			potentialBottomStart = getBounds().y;
			potentialTopStart = getBounds().y + getBounds().height;
			break;
		case DropDownAreaPlacement::Type::BoundsAll:
			potentialRightStart = getBounds().x + getBounds().width;
			potentialLeftStart = getBounds().x;
			potentialBottomStart = getBounds().y + getBounds().height;
			potentialTopStart = getBounds().y;
			break;
		}

		// Determine x position and whether to align to left or right side of the drop down list
		UINT32 availableRightwardWidth = (UINT32)std::max(0, (availableArea.x + availableArea.width) - potentialRightStart);
		UINT32 availableLeftwardWidth = (UINT32)std::max(0, potentialLeftStart - availableArea.x);

		//// Prefer right if possible
		if (width <= availableRightwardWidth)
		{
			output.x = potentialRightStart;
			output.width = width;
			horzDir = HorzDir::Right;
		}
		else
		{
			if (availableRightwardWidth >= availableLeftwardWidth)
			{
				output.x = potentialRightStart;
				output.width = std::min(width, availableRightwardWidth);
				horzDir = HorzDir::Right;
			}
			else
			{
				output.x = potentialLeftStart - std::min(width, availableLeftwardWidth);
				output.width = std::min(width, availableLeftwardWidth);
				horzDir = HorzDir::Left;
			}
		}

		// Determine y position and whether to open upward or downward
		UINT32 availableDownwardHeight = (UINT32)std::max(0, (availableArea.y + availableArea.height) - potentialBottomStart);
		UINT32 availableUpwardHeight = (UINT32)std::max(0, potentialTopStart - availableArea.y);

		//// Prefer down if possible
		if (height <= availableDownwardHeight)
		{
			output.y = potentialBottomStart;
			output.height = height;
			vertDir = VertDir::Down;
		}
		else
		{
			if (availableDownwardHeight >= availableUpwardHeight)
			{
				output.y = potentialBottomStart;
				output.height = std::min(height, availableDownwardHeight);;
				vertDir = VertDir::Down;
			}
			else
			{
				output.y = potentialTopStart - (INT32)std::min(height, availableUpwardHeight);
				output.height = std::min(height, availableUpwardHeight);
				vertDir = VertDir::Up;
			}
		}

		return output;
	}
}