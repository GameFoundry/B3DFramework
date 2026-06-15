//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/B3DDynamicLibrary.h"

using namespace b3d;

DynamicLibrary::DynamicLibrary(String name)
	: mName(std::move(name))
{
	Load();
}

DynamicLibrary::~DynamicLibrary()
{
	Unload();
}
