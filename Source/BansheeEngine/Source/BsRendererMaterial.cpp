//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRendererMaterial.h"

namespace bs { namespace ct
{
	ShaderVariation::ShaderVariation(const SmallVector<Param, 4>& params)
	{
		for (auto& entry : params)
			mParams[entry.name] = entry;
	}

	INT32 ShaderVariation::getInt(const String& name)
	{
		auto iterFind = mParams.find(name);
		if (iterFind == mParams.end())
			return 0;
		else
			return iterFind->second.i;
	}

	UINT32 ShaderVariation::getUInt(const String& name)
	{
		auto iterFind = mParams.find(name);
		if (iterFind == mParams.end())
			return 0;
		else
			return iterFind->second.ui;
	}

	float ShaderVariation::getFloat(const String& name)
	{
		auto iterFind = mParams.find(name);
		if (iterFind == mParams.end())
			return 0.0f;
		else
			return iterFind->second.f;
	}

	bool ShaderVariation::getBool(const String& name)
	{
		auto iterFind = mParams.find(name);
		if (iterFind == mParams.end())
			return false;
		else
			return iterFind->second.i > 0 ? true : false;
	}

	ShaderDefines ShaderVariation::getDefines() const
	{
		ShaderDefines defines;
		for (auto& entry : mParams)
		{
			switch (entry.second.type)
			{
			case Int:
			case Bool:
				defines.set(entry.first, entry.second.i);
				break;
			case UInt:
				defines.set(entry.first, entry.second.ui);
				break;
			case Float:
				defines.set(entry.first, entry.second.f);
				break;
			}
		}

		return defines;
	}

	void ShaderVariations::add(ShaderVariation& variation)
	{
		variation.mIdx = mNextIdx++;

		mVariations.push_back(variation);
	}
}}