//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
	class BS_SCR_BE_EXPORT ScriptTColorDistributionColorGradient : public ScriptObject<ScriptTColorDistributionColorGradient>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ColorDistribution")

		ScriptTColorDistributionColorGradient(MonoObject* managedInstance, const SPtr<TColorDistribution<ColorGradient>>& value);

		SPtr<TColorDistribution<ColorGradient>> getInternal() const { return mInternal; }
		static MonoObject* create(const SPtr<TColorDistribution<ColorGradient>>& value);

	private:
		SPtr<TColorDistribution<ColorGradient>> mInternal;

		static void Internal_TColorDistribution(MonoObject* managedInstance);
		static void Internal_TColorDistribution0(MonoObject* managedInstance, Color* color);
		static void Internal_TColorDistribution1(MonoObject* managedInstance, Color* minColor, Color* maxColor);
		static void Internal_TColorDistribution2(MonoObject* managedInstance, MonoObject* gradient);
		static void Internal_TColorDistribution3(MonoObject* managedInstance, MonoObject* minGradient, MonoObject* maxGradient);
		static PropertyDistributionType Internal_getType(ScriptTColorDistributionColorGradient* thisPtr);
		static void Internal_getMinConstant(ScriptTColorDistributionColorGradient* thisPtr, Color* __output);
		static void Internal_getMaxConstant(ScriptTColorDistributionColorGradient* thisPtr, Color* __output);
		static MonoObject* Internal_getMinGradient(ScriptTColorDistributionColorGradient* thisPtr);
		static MonoObject* Internal_getMaxGradient(ScriptTColorDistributionColorGradient* thisPtr);
	};

	class BS_SCR_BE_EXPORT ScriptTColorDistributionColorGradientHDR : public ScriptObject<ScriptTColorDistributionColorGradientHDR>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ColorHDRDistribution")

		ScriptTColorDistributionColorGradientHDR(MonoObject* managedInstance, const SPtr<TColorDistribution<ColorGradientHDR>>& value);

		SPtr<TColorDistribution<ColorGradientHDR>> getInternal() const { return mInternal; }
		static MonoObject* create(const SPtr<TColorDistribution<ColorGradientHDR>>& value);

	private:
		SPtr<TColorDistribution<ColorGradientHDR>> mInternal;

		static void Internal_TColorDistribution(MonoObject* managedInstance);
		static void Internal_TColorDistribution0(MonoObject* managedInstance, Color* color);
		static void Internal_TColorDistribution1(MonoObject* managedInstance, Color* minColor, Color* maxColor);
		static void Internal_TColorDistribution2(MonoObject* managedInstance, MonoObject* gradient);
		static void Internal_TColorDistribution3(MonoObject* managedInstance, MonoObject* minGradient, MonoObject* maxGradient);
		static PropertyDistributionType Internal_getType(ScriptTColorDistributionColorGradientHDR* thisPtr);
		static void Internal_getMinConstant(ScriptTColorDistributionColorGradientHDR* thisPtr, Color* __output);
		static void Internal_getMaxConstant(ScriptTColorDistributionColorGradientHDR* thisPtr, Color* __output);
		static MonoObject* Internal_getMinGradient(ScriptTColorDistributionColorGradientHDR* thisPtr);
		static MonoObject* Internal_getMaxGradient(ScriptTColorDistributionColorGradientHDR* thisPtr);
	};
}
