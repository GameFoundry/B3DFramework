//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "../../../Engine/Core/Particles/B3DParticleDistribution.h"
#include "B3DScriptNonReflectableWrapper.h"
#include "../../../Engine/Core/Particles/B3DParticleDistribution.h"
#include "../../../Engine/Core/Particles/B3DParticleDistribution.h"
#include "B3DScriptNonReflectableWrapper.h"
#include "../../../Engine/Core/Particles/B3DParticleDistribution.h"
#include "../../../Engine/Utility/Image/B3DColorGradient.h"
#include "../../../Engine/Utility/Image/B3DColor.h"
#include "../../../Engine/Utility/Image/B3DColorGradient.h"
#include "../../../Engine/Core/Particles/B3DParticleDistribution.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptColorDistribution : public TScriptNonReflectableWrapper<TColorDistribution<ColorGradient>, ScriptColorDistribution>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ColorDistribution")

		ScriptColorDistribution(const TShared<TColorDistribution<ColorGradient>>& nativeObject);
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

		ScriptColorHDRDistribution(const TShared<TColorDistribution<ColorGradientHDR>>& nativeObject);
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
