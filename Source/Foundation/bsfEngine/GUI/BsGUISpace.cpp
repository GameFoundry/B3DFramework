//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUISpace.h"

using namespace bs;

GUIFixedSpace::~GUIFixedSpace()
{
	if(mParentElement != nullptr)
		mParentElement->UnregisterChildElementInternal(this);
}

LayoutSizeRange GUIFixedSpace::CalculateLayoutSizeRangeInternal() const
{
	LayoutSizeRange range;
	range.Optimal = GetOptimalSizeInternal();
	range.Min = range.Optimal;
	range.Max = range.Optimal;

	return range;
}

GUIFixedSpace* GUIFixedSpace::Create(u32 size)
{
	return bs_new<GUIFixedSpace>(size);
}

void GUIFixedSpace::Destroy(GUIFixedSpace* space)
{
	bs_delete(space);
}

GUIFlexibleSpace::~GUIFlexibleSpace()
{
	if(mParentElement != nullptr)
		mParentElement->UnregisterChildElementInternal(this);
}

LayoutSizeRange GUIFlexibleSpace::CalculateLayoutSizeRangeInternal() const
{
	LayoutSizeRange range;
	range.Optimal = GetOptimalSizeInternal();
	range.Min = range.Optimal;
	range.Max = range.Optimal;

	return range;
}

GUIFlexibleSpace* GUIFlexibleSpace::Create()
{
	return bs_new<GUIFlexibleSpace>();
}

void GUIFlexibleSpace::Destroy(GUIFlexibleSpace* space)
{
	bs_delete(space);
}
