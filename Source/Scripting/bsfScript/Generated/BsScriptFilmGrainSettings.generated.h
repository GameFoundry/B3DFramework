//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"

namespace bs { struct FilmGrainSettings; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptFilmGrainSettings : public TScriptReflectable<ScriptFilmGrainSettings, FilmGrainSettings>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "FilmGrainSettings")

		ScriptFilmGrainSettings(MonoObject* managedInstance, const SPtr<FilmGrainSettings>& value);

		static MonoObject* Create(const SPtr<FilmGrainSettings>& value);

	private:
		static void InternalFilmGrainSettings(MonoObject* managedInstance);
		static bool InternalGetEnabled(ScriptFilmGrainSettings* self);
		static void InternalSetEnabled(ScriptFilmGrainSettings* self, bool value);
		static float InternalGetIntensity(ScriptFilmGrainSettings* self);
		static void InternalSetIntensity(ScriptFilmGrainSettings* self, float value);
		static float InternalGetSpeed(ScriptFilmGrainSettings* self);
		static void InternalSetSpeed(ScriptFilmGrainSettings* self, float value);
	};
}
