//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsShaderVariation.h"
#include "Private/RTTI/BsShaderVariationRTTI.h"

using namespace bs;

void ShaderDefines::Set(const String& name, float value)
{
	mDefines[name] = toString(value);
}

void ShaderDefines::Set(const String& name, i32 value)
{
	mDefines[name] = toString(value);
}

void ShaderDefines::Set(const String& name, u32 value)
{
	mDefines[name] = toString(value);
}

void ShaderDefines::Set(const String& name, const String& value)
{
	mDefines[name] = value;
}

const ShaderVariation ShaderVariation::kEmpty;

ShaderVariation::ShaderVariation(const SmallVector<Param, 4>& params)
{
	for(auto& entry : params)
		mParams[entry.Name] = entry;
}

i32 ShaderVariation::GetInt(const StringID& name)
{
	auto iterFind = mParams.find(name);
	if(iterFind == mParams.end())
		return 0;
	else
		return iterFind->second.I;
}

u32 ShaderVariation::GetUInt(const StringID& name)
{
	auto iterFind = mParams.find(name);
	if(iterFind == mParams.end())
		return 0;
	else
		return iterFind->second.Ui;
}

float ShaderVariation::GetFloat(const StringID& name)
{
	auto iterFind = mParams.find(name);
	if(iterFind == mParams.end())
		return 0.0f;
	else
		return iterFind->second.F;
}

bool ShaderVariation::GetBool(const StringID& name)
{
	auto iterFind = mParams.find(name);
	if(iterFind == mParams.end())
		return false;
	else
		return iterFind->second.I > 0 ? true : false;
}

void ShaderVariation::SetInt(const StringID& name, i32 value)
{
	AddParam(Param(name, value));
}

void ShaderVariation::SetUInt(const StringID& name, u32 value)
{
	AddParam(Param(name, value));
}

void ShaderVariation::SetFloat(const StringID& name, float value)
{
	AddParam(Param(name, value));
}

void ShaderVariation::SetBool(const StringID& name, bool value)
{
	AddParam(Param(name, value));
}

Vector<String> ShaderVariation::GetParamNames() const
{
	Vector<String> params;
	params.reserve(mParams.size());

	for(auto& entry : mParams)
		params.push_back(entry.first);

	return params;
}

ShaderDefines ShaderVariation::GetDefines() const
{
	ShaderDefines defines;
	for(auto& entry : mParams)
	{
		switch(entry.second.Type)
		{
		case Int:
		case Bool:
			defines.Set(entry.first.CStr(), entry.second.I);
			break;
		case UInt:
			defines.Set(entry.first.CStr(), entry.second.Ui);
			break;
		case Float:
			defines.Set(entry.first.CStr(), entry.second.F);
			break;
		}
	}

	return defines;
}

bool ShaderVariation::Matches(const ShaderVariation& other, bool exact) const
{
	for(auto& entry : other.mParams)
	{
		const auto iterFind = mParams.find(entry.first);
		if(iterFind == mParams.end())
			return false;

		if(entry.second.I != iterFind->second.I)
			return false;
	}

	if(exact)
	{
		for(auto& entry : mParams)
		{
			const auto iterFind = other.mParams.find(entry.first);
			if(iterFind == other.mParams.end())
				return false;

			if(entry.second.I != iterFind->second.I)
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

u32 ShaderVariations::Find(const ShaderVariation& variation) const
{
	u32 idx = 0;
	for(auto& entry : mVariations)
	{
		if(entry == variation)
			return idx;

		idx++;
	}

	return (u32)-1;
}

RTTITypeBase* ShaderVariation::GetRttiStatic()
{
	return ShaderVariationRTTI::Instance();
}

RTTITypeBase* ShaderVariation::GetRtti() const
{
	return ShaderVariation::GetRttiStatic();
}

// This is here to solve a linking issue on Clang 7. The destructor apparently either doesn't get implicitly
// instantiated. This means external libraries linking with bsf, using the same SmallVector template parameters will
// trigger an undefined reference linker error. And why doesn't the library instantiate it itself? Don't know, either
// a Clang issue or maybe even some part of the standard.
template SmallVector<ShaderVariation::Param, 4>::~SmallVector();
