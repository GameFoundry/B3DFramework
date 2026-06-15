//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DDropDownAreaPlacement.h"
#include "Debug/B3DDebug.h"

using namespace b3d;

template<typename PositionType, typename SizeType>
TDropDownAreaPlacement<PositionType, SizeType> TDropDownAreaPlacement<PositionType, SizeType>::AroundPosition(const TVector2<PositionType>& position)
{
	TDropDownAreaPlacement instance;
	instance.mType = Type::Position;
	instance.mPosition = position;

	return instance;
}

template<typename PositionType, typename SizeType>
TDropDownAreaPlacement<PositionType, SizeType> TDropDownAreaPlacement<PositionType, SizeType>::AroundBoundsVertical(const TArea2<PositionType, SizeType>& bounds)
{
	TDropDownAreaPlacement instance;
	instance.mType = Type::BoundsVertical;
	instance.mBounds = bounds;

	return instance;
}

template<typename PositionType, typename SizeType>
TDropDownAreaPlacement<PositionType, SizeType> TDropDownAreaPlacement<PositionType, SizeType>::AroundBoundsHorizontal(const TArea2<PositionType, SizeType>& bounds)
{
	TDropDownAreaPlacement instance;
	instance.mType = Type::BoundsHorizontal;
	instance.mBounds = bounds;

	return instance;
}

template<typename PositionType, typename SizeType>
TDropDownAreaPlacement<PositionType, SizeType> TDropDownAreaPlacement<PositionType, SizeType>::AroundBounds(const TArea2<PositionType, SizeType>& bounds)
{
	TDropDownAreaPlacement instance;
	instance.mType = Type::BoundsAll;
	instance.mBounds = bounds;

	return instance;
}

template<typename PositionType, typename SizeType>
TArea2<PositionType, SizeType> TDropDownAreaPlacement<PositionType, SizeType>::GetOptimalBounds(const TSize2<SizeType>& size, const TArea2<PositionType, SizeType>& availableArea, HorizontalDirection& outHorizontalDirection, VerticalDirection& outVerticalDirection) const
{
	TArea2<PositionType, SizeType> output;

	PositionType potentialLeftStart = 0;
	PositionType potentialRightStart = 0;
	PositionType potentialTopStart = 0;
	PositionType potentialBottomStart = 0;

	switch(GetType())
	{
	case TDropDownAreaPlacement::Type::Position:
		potentialLeftStart = potentialRightStart = GetPosition().X;
		potentialTopStart = potentialBottomStart = GetPosition().Y;
		break;
	case TDropDownAreaPlacement::Type::BoundsHorizontal:
		potentialRightStart = GetBounds().X;
		potentialLeftStart = GetBounds().X + (PositionType)GetBounds().Width;
		potentialBottomStart = GetBounds().Y + (PositionType)GetBounds().Height;
		potentialTopStart = GetBounds().Y;
		break;
	case TDropDownAreaPlacement::Type::BoundsVertical:
		potentialRightStart = GetBounds().X + (PositionType)GetBounds().Width;
		potentialLeftStart = GetBounds().X;
		potentialBottomStart = GetBounds().Y;
		potentialTopStart = GetBounds().Y + (PositionType)GetBounds().Height;
		break;
	case TDropDownAreaPlacement::Type::BoundsAll:
		potentialRightStart = GetBounds().X + (PositionType)GetBounds().Width;
		potentialLeftStart = GetBounds().X;
		potentialBottomStart = GetBounds().Y + (PositionType)GetBounds().Height;
		potentialTopStart = GetBounds().Y;
		break;
	}

	// Determine x position and whether to align to left or right side of the drop down list
	SizeType availableRightwardWidth = (SizeType)Math::Max((availableArea.X + (PositionType)availableArea.Width) - potentialRightStart, 0);
	SizeType availableLeftwardWidth = (SizeType)Math::Max(potentialLeftStart - availableArea.X, 0);

	//// Prefer right if possible
	if(size.Width <= availableRightwardWidth)
	{
		output.X = potentialRightStart;
		output.Width = size.Width;
		outHorizontalDirection = HorizontalDirection::Right;
	}
	else
	{
		if(availableRightwardWidth >= availableLeftwardWidth)
		{
			output.X = potentialRightStart;
			output.Width = Math::Min(size.Width, availableRightwardWidth);
			outHorizontalDirection = HorizontalDirection::Right;
		}
		else
		{
			output.X = potentialLeftStart - Math::Min(size.Width, availableLeftwardWidth);
			output.Width = Math::Min(size.Width, availableLeftwardWidth);
			outHorizontalDirection = HorizontalDirection::Left;
		}
	}

	// Determine y position and whether to open upward or downward
	SizeType availableDownwardHeight = (SizeType)Math::Max((availableArea.Y + (PositionType)availableArea.Height) - potentialBottomStart, 0);
	SizeType availableUpwardHeight = (SizeType)Math::Max(potentialTopStart - availableArea.Y, 0);

	//// Prefer down if possible
	if(size.Height <= availableDownwardHeight)
	{
		output.Y = potentialBottomStart;
		output.Height = size.Height;
		outVerticalDirection = VerticalDirection::Down;
	}
	else
	{
		if(availableDownwardHeight >= availableUpwardHeight)
		{
			output.Y = potentialBottomStart;
			output.Height = Math::Min(size.Height, availableDownwardHeight);
			outVerticalDirection = VerticalDirection::Down;
		}
		else
		{
			output.Y = potentialTopStart - (PositionType)Math::Min(size.Height, availableUpwardHeight);
			output.Height = Math::Min(size.Height, availableUpwardHeight);
			outVerticalDirection = VerticalDirection::Up;
		}
	}

	return output;
}

namespace b3d
{
	template class B3D_EXPORT TDropDownAreaPlacement<i32, u32>;
	template class B3D_EXPORT TDropDownAreaPlacement<GUIPhysicalUnit>;
	template class B3D_EXPORT TDropDownAreaPlacement<GUILogicalUnit>;
} // namespace b3d
