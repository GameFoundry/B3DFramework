//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUISpace.h"

using namespace bs;

GUIFixedSpace::~GUIFixedSpace()
{
	if(mParentElement != nullptr)
		mParentElement->UnregisterChildElement(this);
}

LayoutSizeRange GUIFixedSpace::CalculateLayoutSizeRange() const
{
	LayoutSizeRange range;
	range.Optimal = GetOptimalSize();
	range.Min = range.Optimal;
	range.Max = range.Optimal;

	return range;
}

GUIFixedSpace* GUIFixedSpace::Create(u32 size)
{
	return B3DNew<GUIFixedSpace>(size);
}

void GUIFixedSpace::Destroy(GUIFixedSpace* space)
{
	B3DDelete(space);
}

GUIFlexibleSpace::~GUIFlexibleSpace()
{
	if(mParentElement != nullptr)
		mParentElement->UnregisterChildElement(this);
}

LayoutSizeRange GUIFlexibleSpace::CalculateLayoutSizeRange() const
{
	LayoutSizeRange range;
	range.Optimal = GetOptimalSize();
	range.Min = range.Optimal;
	range.Max = range.Optimal;

	return range;
}

GUIFlexibleSpace* GUIFlexibleSpace::Create()
{
	return B3DNew<GUIFlexibleSpace>();
}

void GUIFlexibleSpace::Destroy(GUIFlexibleSpace* space)
{
	B3DDelete(space);
}
