//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"

namespace bs { struct FilmGrainSettings; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptFilmGrainSettings : public TScriptReflectableWrapper<FilmGrainSettings, ScriptFilmGrainSettings>
	{
	public:
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "FilmGrainSettings")

		ScriptFilmGrainSettings(const SPtr<FilmGrainSettings>& nativeObject, MonoObject* scriptObject);

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
