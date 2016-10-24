//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Win32/BsWin32VideoModeInfo.h"
#include "BsException.h"

namespace BansheeEngine
{
	Win32VideoModeInfo::Win32VideoModeInfo()
	{

	}

	Win32VideoOutputInfo::Win32VideoOutputInfo( UINT32 outputIdx)
	{
		
	}

	Win32VideoOutputInfo::~Win32VideoOutputInfo()
	{

	}

	Win32VideoMode::Win32VideoMode(UINT32 width, UINT32 height, float refreshRate, UINT32 outputIdx)
		:VideoMode(width, height, refreshRate, outputIdx)
	{ }
}