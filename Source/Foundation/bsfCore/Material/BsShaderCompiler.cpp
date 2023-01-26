//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsShaderCompiler.h"

using namespace bs;

SPtr<IShaderCompiler> ShaderCompilers::GetCompiler(const String& language)
{
	auto found = _compilers.find(language);
	if(found != _compilers.end())
		return found->second;

	return nullptr;
}

ShadingLanguageFlag ShaderCompilers::ParseShadingLanguage(const String& name)
{
	if(name == "hlsl")
		return ShadingLanguageFlag::HLSL;

	if(name == "glsl")
		return ShadingLanguageFlag::GLSL;

	if(name == "vksl")
		return ShadingLanguageFlag::VKSL;

	if(name == "mvksl")
		return ShadingLanguageFlag::MSL;

	return ShadingLanguageFlag::Unknown;
}

const char* ShaderCompilers::GetShadingLanguageName(ShadingLanguageFlag language)
{
	switch(language)
	{
	case ShadingLanguageFlag::HLSL: return "hlsl";
	case ShadingLanguageFlag::GLSL: return "glsl";
	case ShadingLanguageFlag::VKSL: return "vksl";
	case ShadingLanguageFlag::MSL: return "mvksl";
	default: return "";
	}
}
