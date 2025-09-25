//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DShaderVariation.h"
#include "RTTI/B3DShaderVariationRTTI.h"

using namespace b3d;

void ShaderDefines::Set(const String& name, float value)
{
	mDefines[name] = ToString(value);
}

void ShaderDefines::Set(const String& name, i32 value)
{
	mDefines[name] = ToString(value);
}

void ShaderDefines::Set(const String& name, u32 value)
{
	mDefines[name] = ToString(value);
}

void ShaderDefines::Set(const String& name, const String& value)
{
	mDefines[name] = value;
}

const ShaderVariationParameters ShaderVariationParameters::kEmpty {};

ShaderVariationParameters::ShaderVariationParameters(const TInlineArray<ShaderVariationParameter, 4>& params)
	: mParams(params)
{ }

i32 ShaderVariationParameters::GetI32(const StringID& name)
{
	const ShaderVariationParameter* const parameter = FindParameter(name);
	if(parameter == nullptr)
		return 0;

	return parameter->SignedInteger;
}

u32 ShaderVariationParameters::GetUI32(const StringID& name)
{
	const ShaderVariationParameter* const parameter = FindParameter(name);
	if(parameter == nullptr)
		return 0;

	return parameter->UnsignedInteger;
}

float ShaderVariationParameters::GetFloat(const StringID& name)
{
	const ShaderVariationParameter* const parameter = FindParameter(name);
	if(parameter == nullptr)
		return 0.0f;

	return parameter->Float;
}

bool ShaderVariationParameters::GetBool(const StringID& name)
{
	const ShaderVariationParameter* const parameter = FindParameter(name);
	if(parameter == nullptr)
		return false;

	return parameter->SignedInteger > 0 ? true : false;
}

void ShaderVariationParameters::SetI32(const StringID& name, i32 value)
{
	if(ShaderVariationParameter* parameter = FindParameter(name))
	{
		parameter->SignedInteger = value;
		return;
	}

	AddParameter(ShaderVariationParameter(name, value));
}

void ShaderVariationParameters::SetU32(const StringID& name, u32 value)
{
	if(ShaderVariationParameter* parameter = FindParameter(name))
	{
		parameter->UnsignedInteger = value;
		return;
	}

	AddParameter(ShaderVariationParameter(name, value));
}

void ShaderVariationParameters::SetFloat(const StringID& name, float value)
{
	if(ShaderVariationParameter* parameter = FindParameter(name))
	{
		parameter->Float = value;
		return;
	}

	AddParameter(ShaderVariationParameter(name, value));
}

void ShaderVariationParameters::SetBool(const StringID& name, bool value)
{
	if(ShaderVariationParameter* parameter = FindParameter(name))
	{
		parameter->UnsignedInteger = (u32)value;
		return;
	}

	AddParameter(ShaderVariationParameter(name, value));
}

Vector<String> ShaderVariationParameters::GetParameters() const
{
	Vector<String> params;
	params.reserve(mParams.size());

	for(auto& entry : mParams)
		params.push_back(entry.Name);

	return params;
}

String ShaderVariationParameters::CreateVariationName() const
{
	bool isFirst = true;
	StringStream output;
	for(const auto& entry : mParams)
	{
		if(isFirst)
		{
			isFirst = false;
		}
		else
		{
			output << "-";
		}

		output << entry.Name.CStr() << "=";

		switch(entry.Type)
		{
		case ShaderVariationParameterType::Int:
		case ShaderVariationParameterType::Bool:
			output << ToString(entry.SignedInteger);
			break;
		case ShaderVariationParameterType::UInt:
			output << ToString(entry.UnsignedInteger);
			break;
		case ShaderVariationParameterType::Float:
			output << ToString(entry.Float);
			break;
		}
	}

	return output.str();
}

ShaderDefines ShaderVariationParameters::GetDefines() const
{
	ShaderDefines defines;
	for(auto& entry : mParams)
	{
		switch(entry.Type)
		{
		case ShaderVariationParameterType::Int:
		case ShaderVariationParameterType::Bool:
			defines.Set(entry.Name.CStr(), entry.SignedInteger);
			break;
		case ShaderVariationParameterType::UInt:
			defines.Set(entry.Name.CStr(), entry.UnsignedInteger);
			break;
		case ShaderVariationParameterType::Float:
			defines.Set(entry.Name.CStr(), entry.Float);
			break;
		}
	}

	return defines;
}

bool ShaderVariationParameters::Matches(const ShaderVariationParameters& other, bool exact) const
{
	for(auto& entry : other.mParams)
	{
		const ShaderVariationParameter* const foundParameter = FindParameter(entry.Name);
		if(foundParameter == nullptr)
			return false;

		if(entry.SignedInteger != foundParameter->SignedInteger)
			return false;
	}

	if(exact)
	{
		for(auto& entry : mParams)
		{
		const ShaderVariationParameter* const foundParameter = other.FindParameter(entry.Name);
			if(foundParameter == nullptr)
				return false;

			if(entry.SignedInteger != foundParameter->SignedInteger)
				return false;
		}
	}

	return true;
}

void ShaderVariationParameters::AddParameter(const ShaderVariationParameter& param)
{
	if(!B3D_ENSURE(FindParameter(param.Name) == nullptr))
		return;
	
	mParams.Add(param);
}

void ShaderVariationParameters::RemoveParameter(const StringID& paramName)
{
	for(auto it = mParams.begin(); it != mParams.end(); ++it)
	{
		if(it->Name == paramName)
		{
			mParams.erase(it);
			return;
		}
	}
}

const ShaderVariationParameter* ShaderVariationParameters::FindParameter(const StringID& name) const
{
	for(auto& entry : mParams)
	{
		if(entry.Name == name)
			return &entry;
	}

	return nullptr;
}

ShaderVariationParameter* ShaderVariationParameters::FindParameter(const StringID& name)
{
	for(auto& entry : mParams)
	{
		if(entry.Name == name)
			return &entry;
	}

	return nullptr;
}

bool ShaderVariationParameters::operator==(const ShaderVariationParameters& rhs) const
{
	return Matches(rhs, true);
}

void ShaderVariations::Add(const ShaderVariationParameters& variation)
{
	variation.mIndex = mNextIdx++;

	mVariations.Add(variation);
}

u32 ShaderVariations::Find(const ShaderVariationParameters& variation) const
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

RTTIType* ShaderVariationParameters::GetRttiStatic()
{
	return ShaderVariationRTTI::Instance();
}

RTTIType* ShaderVariationParameters::GetRtti() const
{
	return ShaderVariationParameters::GetRttiStatic();
}

// This is here to solve a linking issue on Clang 7. The destructor apparently either doesn't get implicitly
// instantiated. This means external libraries linking with bsf, using the same TArray template parameters will
// trigger an undefined reference linker error. And why doesn't the library instantiate it itself? Don't know, either
// a Clang issue or maybe even some part of the standard.
template TArray<ShaderVariationParameter, InlineContainerAllocator<4>>::~TArray();
