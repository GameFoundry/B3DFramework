//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUISpace.h"

namespace bs
{
	GUIFixedSpace::~GUIFixedSpace()
	{
		if (mParentElement != nullptr)
			mParentElement->_unregisterChildElement(this);
	}

	LayoutSizeRange GUIFixedSpace::_calculateLayoutSizeRange() const
	{ 
		LayoutSizeRange range;
		range.optimal = _getOptimalSize();
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
			mParentElement->_unregisterChildElement(this);
	}

	LayoutSizeRange GUIFlexibleSpace::_calculateLayoutSizeRange() const
	{
		LayoutSizeRange range;
		range.optimal = _getOptimalSize();
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