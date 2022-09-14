//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUISpace.h"

namespace bs
{
	GUIFixedSpace::~GUIFixedSpace()
	{
		if (mParentElement != nullptr)
			mParentElement->UnregisterChildElementInternal(this);
	}

	LayoutSizeRange GUIFixedSpace::CalculateLayoutSizeRangeInternal() const
	{
		LayoutSizeRange range;
		range.optimal = GetOptimalSizeInternal();
		range.min = range.optimal;
		range.max = range.optimal;

		return range;
	}

	GUIFixedSpace* GUIFixedSpace::create(UINT32 size)
	{
		return bs_new<GUIFixedSpace>(size);
	}

	void GUIFixedSpace::destroy(GUIFixedSpace* space)
	{
		bs_delete(space);
	}

	GUIFlexibleSpace::~GUIFlexibleSpace()
	{
		if (mParentElement != nullptr)
			mParentElement->UnregisterChildElementInternal(this);
	}

	LayoutSizeRange GUIFlexibleSpace::CalculateLayoutSizeRangeInternal() const
	{
		LayoutSizeRange range;
		range.optimal = GetOptimalSizeInternal();
		range.min = range.optimal;
		range.max = range.optimal;

		return range;
	}

	GUIFlexibleSpace* GUIFlexibleSpace::create()
	{
		return bs_new<GUIFlexibleSpace>();
	}

	void GUIFlexibleSpace::destroy(GUIFlexibleSpace* space)
	{
		bs_delete(space);
	}
}
