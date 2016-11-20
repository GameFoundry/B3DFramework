//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsEditorWidgetLayout.h"
#include "BsEditorWidgetLayoutRTTI.h"

namespace bs
{
	EditorWidgetLayout::Entry::Entry()
		:isDocked(true), x(0), y(0), width(0), height(0)
	{ }

	EditorWidgetLayout::Entry::~Entry()
	{ }

	EditorWidgetLayout::EditorWidgetLayout(const SPtr<DockManagerLayout>& dockLayout)
		:mDockLayout(dockLayout), mMaximized(false)
	{ }

	EditorWidgetLayout::EditorWidgetLayout(const PrivatelyConstruct& dummy)
		: mMaximized(false)
	{ }

	/************************************************************************/
	/* 								RTTI		                     		*/
	/************************************************************************/

	RTTITypeBase* EditorWidgetLayout::getRTTIStatic()
	{
		return EditorWidgetLayoutRTTI::instance();
	}

	RTTITypeBase* EditorWidgetLayout::getRTTI() const
	{
		return EditorWidgetLayout::getRTTIStatic();
	}
}