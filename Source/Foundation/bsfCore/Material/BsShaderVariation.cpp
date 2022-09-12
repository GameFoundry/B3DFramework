//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsShaderVariation.h"
#include "Private/RTTI/BsShaderVariationRTTI.h"

namespace bs
{
	void ShaderDefines::Set(const String& name, float value)
	{
		mDefines[name] = toString(value);
	}

	void ShaderDefines::Set(const String& name, INT32 value)
	{
		mDefines[name] = toString(value);
	}

	void ShaderDefines::Set(const String& name, UINT32 value)
	{
		mDefines[name] = toString(value);
	}

	void ShaderDefines::Set(const String& name, const String& value)
	{
		mDefines[name] = value;
	}

	const ShaderVariation ShaderVariation::EMPTY;

	ShaderVariation::ShaderVariation(const SmallVector<Param, 4>& params)
	{
		for (auto& entry : params)
			mParams[entry.name] = entry;
	}

	INT32 ShaderVariation::GetInt(const StringID& name)
	{
		auto iterFind = mParams.Find(name);
		if (iterFind == mParams.End())
			return 0;
		else
			return iterFind->second.i;
	}

	UINT32 ShaderVariation::GetUInt(const StringID& name)
	{
		auto iterFind = mParams.Find(name);
		if (iterFind == mParams.End())
			return 0;
		else
			return iterFind->second.ui;
	}

	float ShaderVariation::GetFloat(const StringID& name)
	{
		auto iterFind = mParams.Find(name);
		if (iterFind == mParams.End())
			return 0.0f;
		else
			return iterFind->second.f;
	}

	bool ShaderVariation::GetBool(const StringID& name)
	{
		auto iterFind = mParams.Find(name);
		if (iterFind == mParams.End())
			return false;
		else
			return iterFind->second.i > 0 ? true : false;
	}

	void ShaderVariation::SetInt(const StringID& name, INT32 value)
	{
		addParam(Param(name, value));
	}

	void ShaderVariation::SetUInt(const StringID& name, UINT32 value)
	{
		addParam(Param(name, value));
	}

	void ShaderVariation::SetFloat(const StringID& name, float value)
	{
		addParam(Param(name, value));
	}

	void ShaderVariation::SetBool(const StringID& name, bool value)
	{
		addParam(Param(name, value));
	}

	Vector<String> ShaderVariation::GetParamNames() const
	{
		Vector<String> params;
		params.Reserve(mParams.size());

		for(auto& entry : mParams)
			params.push_back(entry.first);

		return params;
	}

	ShaderDefines ShaderVariation::GetDefines() const
	{
		ShaderDefines defines;
		for (auto& entry : mParams)
		{
			switch (entry.second.type)
			{
			case Int:
			case Bool:
				defines.Set(entry.first.c_str(), entry.second.i);
				break;
			case UInt:
				defines.Set(entry.first.c_str(), entry.second.ui);
				break;
			case Float:
				defines.Set(entry.first.c_str(), entry.second.f);
				break;
			}
		}

		return defines;
	}

	bool ShaderVariation::Matches(const ShaderVariation& other, bool exact) const
	{
		for(auto& entry : other.mParams)
		{
			const auto iterFind = mParams.Find(entry.first);
			if(iterFind == mParams.End())
				return false;

			if(entry.second.i != iterFind->second.i)
				return false;
		}

		if(exact)
		{
			for (auto& entry : mParams)
			{
				const auto iterFind = other.mParams.Find(entry.first);
				if (iterFind == other.mParams.End())
					return false;

				if (entry.second.i != iterFind->second.i)
					return false;
			}
		}

		return true;
	}

	bool ShaderVariation::operator==(const ShaderVariation& rhs) const
	{
		return Matches(rhs, true);
	}

	void ShaderVariations::Add(const ShaderVariation& variation)
	{
		variation.mIdx = mNextIdx++;

		mVariations.Add(variation);
	}
	
	UINT32 ShaderVariations::Find(const ShaderVariation& variation) const
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
		return ShaderVariationRTTI::Instance();
	}

	RTTITypeBase* ShaderVariation::getRTTI() const
	{
		return ShaderVariation::GetRTTIStatic();
	}

	// This is here to solve a linking issue on Clang 7. The destructor apparently either doesn't get implicitly
	// instantiated. This means external libraries linking with bsf, using the same SmallVector template parameters will
	// trigger an undefined reference linker error. And why doesn't the library instantiate it itself? Don't know, either
	// a Clang issue or maybe even some part of the standard.
	template SmallVector<ShaderVariation::Param, 4>::~SmallVector();
}
