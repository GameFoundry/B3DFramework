//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsShaderVariation.h"
#include "Private/RTTI/BsShaderVariationRTTI.h"

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

	const ShaderVariation ShaderVariation::EMPTY;

	ShaderVariation::ShaderVariation(const SmallVector<Param, 4>& params)
	{
		for (auto& entry : params)
			mParams[entry.name] = entry;
	}

	INT32 ShaderVariation::getInt(const StringID& name)
	{
		auto iterFind = mParams.find(name);
		if (iterFind == mParams.end())
			return 0;
		else
			return iterFind->second.i;
	}

	UINT32 ShaderVariation::getUInt(const StringID& name)
	{
		auto iterFind = mParams.find(name);
		if (iterFind == mParams.end())
			return 0;
		else
			return iterFind->second.ui;
	}

	float ShaderVariation::getFloat(const StringID& name)
	{
		auto iterFind = mParams.find(name);
		if (iterFind == mParams.end())
			return 0.0f;
		else
			return iterFind->second.f;
	}

	bool ShaderVariation::getBool(const StringID& name)
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
				defines.set(entry.first.c_str(), entry.second.i);
				break;
			case UInt:
				defines.set(entry.first.c_str(), entry.second.ui);
				break;
			case Float:
				defines.set(entry.first.c_str(), entry.second.f);
				break;
			}
		}

		return defines;
	}

	bool ShaderVariation::matches(const ShaderVariation& other, bool exact) const
	{
		for(auto& entry : other.mParams)
		{
			const auto iterFind = mParams.find(entry.first);
			if(iterFind == mParams.end())
				return false;

			if(entry.second.i != iterFind->second.i)
				return false;
		}

		if(exact)
		{
			for (auto& entry : mParams)
			{
				const auto iterFind = other.mParams.find(entry.first);
				if (iterFind == other.mParams.end())
					return false;

				if (entry.second.i != iterFind->second.i)
					return false;
			}
		}

		return true;
	}

	bool ShaderVariation::operator==(const ShaderVariation& rhs) const
	{
		return matches(rhs, true);
	}

	void ShaderVariations::add(const ShaderVariation& variation)
	{
		variation.mIdx = mNextIdx++;

		mVariations.add(variation);
	}
	
	UINT32 ShaderVariations::find(const ShaderVariation& variation) const
	{
		UINT32 idx = 0;
		for(auto& entry : mVariations)
		{
			if(entry == variation)
				return idx;

			idx++;
		}

		return (UINT32)-1;
	}
	
	RTTITypeBase* ShaderVariation::getRTTIStatic()
	{
		return ShaderVariationRTTI::instance();
	}

	RTTITypeBase* ShaderVariation::getRTTI() const
	{
		return ShaderVariation::getRTTIStatic();
	}

}