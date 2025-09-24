//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"

namespace b3d { struct FilmGrainSettings; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptFilmGrainSettings : public TScriptReflectableWrapper<FilmGrainSettings, ScriptFilmGrainSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "FilmGrainSettings")

		ScriptFilmGrainSettings(const SPtr<FilmGrainSettings>& nativeObject);
		~ScriptFilmGrainSettings();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalFilmGrainSettings(MonoObject* scriptObject);
		static bool InternalGetEnabled(ScriptFilmGrainSettings* self);
		static void InternalSetEnabled(ScriptFilmGrainSettings* self, bool value);
		static float InternalGetIntensity(ScriptFilmGrainSettings* self);
		static void InternalSetIntensity(ScriptFilmGrainSettings* self, float value);
		static float InternalGetSpeed(ScriptFilmGrainSettings* self);
		static void InternalSetSpeed(ScriptFilmGrainSettings* self, float value);
	};
}
