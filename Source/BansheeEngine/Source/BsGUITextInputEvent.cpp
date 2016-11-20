//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUITextInputEvent.h"

namespace bs
{
	GUITextInputEvent::GUITextInputEvent()
		:mInputChar(0)
	{

	}

	void GUITextInputEvent::setData(UINT32 inputChar)
	{
		mInputChar = inputChar;
	}
}