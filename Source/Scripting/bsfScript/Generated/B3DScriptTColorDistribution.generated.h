//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptNonReflectableWrapper.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptNonReflectableWrapper.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "../../../Foundation/bsfUtility/Image/BsColorGradient.h"
#include "../../../Foundation/bsfUtility/Image/BsColor.h"
#include "../../../Foundation/bsfUtility/Image/BsColorGradient.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptColorDistribution : public TScriptNonReflectableWrapper<TColorDistribution<ColorGradient>, ScriptColorDistribution>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ColorDistribution")

		ScriptColorDistribution(const SPtr<TColorDistribution<ColorGradient>>& nativeObject);
		~ScriptColorDistribution();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalTColorDistribution(MonoObject* scriptObject);
		static void InternalTColorDistribution0(MonoObject* scriptObject, Color* color);
		static void InternalTColorDistribution1(MonoObject* scriptObject, Color* minColor, Color* maxColor);
		static void InternalTColorDistribution2(MonoObject* scriptObject, MonoObject* gradient);
		static void InternalTColorDistribution3(MonoObject* scriptObject, MonoObject* minGradient, MonoObject* maxGradient);
		static PropertyDistributionType InternalGetType(ScriptColorDistribution* self);
		static void InternalGetMinConstant(ScriptColorDistribution* self, Color* __output);
		static void InternalGetMaxConstant(ScriptColorDistribution* self, Color* __output);
		static MonoObject* InternalGetMinGradient(ScriptColorDistribution* self);
		static MonoObject* InternalGetMaxGradient(ScriptColorDistribution* self);
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptColorHDRDistribution : public TScriptNonReflectableWrapper<TColorDistribution<ColorGradientHDR>, ScriptColorHDRDistribution>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ColorHDRDistribution")

		ScriptColorHDRDistribution(const SPtr<TColorDistribution<ColorGradientHDR>>& nativeObject);
		~ScriptColorHDRDistribution();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalTColorDistribution(MonoObject* scriptObject);
		static void InternalTColorDistribution0(MonoObject* scriptObject, Color* color);
		static void InternalTColorDistribution1(MonoObject* scriptObject, Color* minColor, Color* maxColor);
		static void InternalTColorDistribution2(MonoObject* scriptObject, MonoObject* gradient);
		static void InternalTColorDistribution3(MonoObject* scriptObject, MonoObject* minGradient, MonoObject* maxGradient);
		static PropertyDistributionType InternalGetType(ScriptColorHDRDistribution* self);
		static void InternalGetMinConstant(ScriptColorHDRDistribution* self, Color* __output);
		static void InternalGetMaxConstant(ScriptColorHDRDistribution* self, Color* __output);
		static MonoObject* InternalGetMinGradient(ScriptColorHDRDistribution* self);
		static MonoObject* InternalGetMaxGradient(ScriptColorHDRDistribution* self);
	};
}
