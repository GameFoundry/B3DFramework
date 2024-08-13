//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "../../../Foundation/bsfUtility/Image/BsColorGradient.h"
#include "Image/BsColor.h"
#include "../../../Foundation/bsfUtility/Image/BsColorGradient.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"

namespace bs { template<class T0> struct TColorDistribution; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptColorDistribution : public ScriptObject<ScriptColorDistribution>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ColorDistribution")

		ScriptColorDistribution(MonoObject* managedInstance, const SPtr<TColorDistribution<ColorGradient>>& value);

		SPtr<TColorDistribution<ColorGradient>> GetInternal() const { return mInternal; }
		static MonoObject* Create(const SPtr<TColorDistribution<ColorGradient>>& value);

	private:
		SPtr<TColorDistribution<ColorGradient>> mInternal;

		static void InternalTColorDistribution(MonoObject* managedInstance);
		static void InternalTColorDistribution0(MonoObject* managedInstance, Color* color);
		static void InternalTColorDistribution1(MonoObject* managedInstance, Color* minColor, Color* maxColor);
		static void InternalTColorDistribution2(MonoObject* managedInstance, MonoObject* gradient);
		static void InternalTColorDistribution3(MonoObject* managedInstance, MonoObject* minGradient, MonoObject* maxGradient);
		static PropertyDistributionType InternalGetType(ScriptColorDistribution* self);
		static void InternalGetMinConstant(ScriptColorDistribution* self, Color* __output);
		static void InternalGetMaxConstant(ScriptColorDistribution* self, Color* __output);
		static MonoObject* InternalGetMinGradient(ScriptColorDistribution* self);
		static MonoObject* InternalGetMaxGradient(ScriptColorDistribution* self);
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptColorHDRDistribution : public ScriptObject<ScriptColorHDRDistribution>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ColorHDRDistribution")

		ScriptColorHDRDistribution(MonoObject* managedInstance, const SPtr<TColorDistribution<ColorGradientHDR>>& value);

		SPtr<TColorDistribution<ColorGradientHDR>> GetInternal() const { return mInternal; }
		static MonoObject* Create(const SPtr<TColorDistribution<ColorGradientHDR>>& value);

	private:
		SPtr<TColorDistribution<ColorGradientHDR>> mInternal;

		static void InternalTColorDistribution(MonoObject* managedInstance);
		static void InternalTColorDistribution0(MonoObject* managedInstance, Color* color);
		static void InternalTColorDistribution1(MonoObject* managedInstance, Color* minColor, Color* maxColor);
		static void InternalTColorDistribution2(MonoObject* managedInstance, MonoObject* gradient);
		static void InternalTColorDistribution3(MonoObject* managedInstance, MonoObject* minGradient, MonoObject* maxGradient);
		static PropertyDistributionType InternalGetType(ScriptColorHDRDistribution* self);
		static void InternalGetMinConstant(ScriptColorHDRDistribution* self, Color* __output);
		static void InternalGetMaxConstant(ScriptColorHDRDistribution* self, Color* __output);
		static MonoObject* InternalGetMinGradient(ScriptColorHDRDistribution* self);
		static MonoObject* InternalGetMaxGradient(ScriptColorHDRDistribution* self);
	};
}
