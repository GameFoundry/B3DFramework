//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfUtility/Image/BsColorGradient.h"
#include "Image/BsColor.h"
#include "../../../Foundation/bsfUtility/Math/BsRandom.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"

namespace bs
{
	struct ColorDistribution;
	class ColorDistributionEx;

	class BS_SCR_BE_EXPORT ScriptColorDistribution : public ScriptObject<ScriptColorDistribution>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ColorDistribution")

		ScriptColorDistribution(MonoObject* managedInstance, const SPtr<ColorDistribution>& value);

		SPtr<ColorDistribution> getInternal() const { return mInternal; }
		static MonoObject* create(const SPtr<ColorDistribution>& value);

	private:
		SPtr<ColorDistribution> mInternal;

		static void Internal_ColorDistribution(MonoObject* managedInstance);
		static void Internal_ColorDistribution0(MonoObject* managedInstance, Color* color);
		static void Internal_ColorDistribution1(MonoObject* managedInstance, Color* minColor, Color* maxColor);
		static void Internal_ColorDistribution2(MonoObject* managedInstance, MonoObject* gradient);
		static void Internal_ColorDistribution3(MonoObject* managedInstance, MonoObject* minGradient, MonoObject* maxGradient);
		static PropertyDistributionType Internal_getType(ScriptColorDistribution* thisPtr);
		static void Internal_getMinConstant(ScriptColorDistribution* thisPtr, Color* __output);
		static void Internal_getMaxConstant(ScriptColorDistribution* thisPtr, Color* __output);
		static MonoObject* Internal_getMinGradient(ScriptColorDistribution* thisPtr);
		static MonoObject* Internal_getMaxGradient(ScriptColorDistribution* thisPtr);
		static void Internal_evaluate(ScriptColorDistribution* thisPtr, float t, float factor, Color* __output);
		static void Internal_evaluate0(ScriptColorDistribution* thisPtr, float t, MonoObject** factor, Color* __output);
	};
}
