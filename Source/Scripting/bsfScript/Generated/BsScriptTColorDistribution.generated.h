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
	class B3D_SCRIPT_INTEROP_EXPORT ScriptTColorDistributionColorGradient : public ScriptObject<ScriptTColorDistributionColorGradient>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ColorDistribution")

		ScriptTColorDistributionColorGradient(MonoObject* managedInstance, const SPtr<TColorDistribution<ColorGradient>>& value);

		SPtr<TColorDistribution<ColorGradient>> GetInternal() const { return mInternal; }
		static MonoObject* Create(const SPtr<TColorDistribution<ColorGradient>>& value);

	private:
		SPtr<TColorDistribution<ColorGradient>> mInternal;

		static void InternalTColorDistribution(MonoObject* managedInstance);
		static void InternalTColorDistribution0(MonoObject* managedInstance, Color* color);
		static void InternalTColorDistribution1(MonoObject* managedInstance, Color* minColor, Color* maxColor);
		static void InternalTColorDistribution2(MonoObject* managedInstance, MonoObject* gradient);
		static void InternalTColorDistribution3(MonoObject* managedInstance, MonoObject* minGradient, MonoObject* maxGradient);
		static PropertyDistributionType InternalGetType(ScriptTColorDistributionColorGradient* thisPtr);
		static void InternalGetMinConstant(ScriptTColorDistributionColorGradient* thisPtr, Color* __output);
		static void InternalGetMaxConstant(ScriptTColorDistributionColorGradient* thisPtr, Color* __output);
		static MonoObject* InternalGetMinGradient(ScriptTColorDistributionColorGradient* thisPtr);
		static MonoObject* InternalGetMaxGradient(ScriptTColorDistributionColorGradient* thisPtr);
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptTColorDistributionColorGradientHDR : public ScriptObject<ScriptTColorDistributionColorGradientHDR>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ColorHDRDistribution")

		ScriptTColorDistributionColorGradientHDR(MonoObject* managedInstance, const SPtr<TColorDistribution<ColorGradientHDR>>& value);

		SPtr<TColorDistribution<ColorGradientHDR>> GetInternal() const { return mInternal; }
		static MonoObject* Create(const SPtr<TColorDistribution<ColorGradientHDR>>& value);

	private:
		SPtr<TColorDistribution<ColorGradientHDR>> mInternal;

		static void InternalTColorDistribution(MonoObject* managedInstance);
		static void InternalTColorDistribution0(MonoObject* managedInstance, Color* color);
		static void InternalTColorDistribution1(MonoObject* managedInstance, Color* minColor, Color* maxColor);
		static void InternalTColorDistribution2(MonoObject* managedInstance, MonoObject* gradient);
		static void InternalTColorDistribution3(MonoObject* managedInstance, MonoObject* minGradient, MonoObject* maxGradient);
		static PropertyDistributionType InternalGetType(ScriptTColorDistributionColorGradientHDR* thisPtr);
		static void InternalGetMinConstant(ScriptTColorDistributionColorGradientHDR* thisPtr, Color* __output);
		static void InternalGetMaxConstant(ScriptTColorDistributionColorGradientHDR* thisPtr, Color* __output);
		static MonoObject* InternalGetMinGradient(ScriptTColorDistributionColorGradientHDR* thisPtr);
		static MonoObject* InternalGetMaxGradient(ScriptTColorDistributionColorGradientHDR* thisPtr);
	};
}
