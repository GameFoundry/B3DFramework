//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"

namespace bs { struct FilmGrainSettings; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptFilmGrainSettings : public TScriptReflectable<ScriptFilmGrainSettings, FilmGrainSettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "FilmGrainSettings")

		ScriptFilmGrainSettings(MonoObject* managedInstance, const SPtr<FilmGrainSettings>& value);

		static MonoObject* create(const SPtr<FilmGrainSettings>& value);

	private:
		static void Internal_FilmGrainSettings(MonoObject* managedInstance);
		static bool Internal_getenabled(ScriptFilmGrainSettings* thisPtr);
		static void Internal_setenabled(ScriptFilmGrainSettings* thisPtr, bool value);
		static float Internal_getintensity(ScriptFilmGrainSettings* thisPtr);
		static void Internal_setintensity(ScriptFilmGrainSettings* thisPtr, float value);
		static float Internal_getspeed(ScriptFilmGrainSettings* thisPtr);
		static void Internal_setspeed(ScriptFilmGrainSettings* thisPtr, float value);
	};
}
