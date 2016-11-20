//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsShaderDefines.h"

namespace bs
{
	void ShaderDefines::set(const String& name, float value)
	{
		mDefines[name] = toString(value);
	}

	void ShaderDefines::set(const String& name, INT32 value)
	{
		mDefines[name] = toString(value);
	}

	void ShaderDefines::set(const String& name, UINT32 value)
	{
		mDefines[name] = toString(value);
	}

	void ShaderDefines::set(const String& name, const String& value)
	{
		mDefines[name] = value;
	}
}