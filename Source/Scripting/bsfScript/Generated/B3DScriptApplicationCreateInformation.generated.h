//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/B3DCoreApplication.h"
#include "../../../Foundation/Core/RenderAPI/B3DRenderWindow.h"
#include "B3DScriptRenderWindowCreateInformation.generated.h"

namespace b3d
{
#if !B3D_IS_ENGINE
	struct __ApplicationCreateInformationInterop
	{
		MonoString* RenderApi;
		MonoString* Renderer;
		MonoString* Physics;
		MonoString* Audio;
		MonoString* Input;
		bool PhysicsCooking;
		bool AsyncAnimation;
		__RenderWindowCreateInformationInterop PrimaryWindow;
		MonoArray* Importers;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptApplicationCreateInformation : public TScriptTypeDefinition<ScriptApplicationCreateInformation>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ApplicationCreateInformation")

		static MonoObject* Box(const __ApplicationCreateInformationInterop& value);
		static __ApplicationCreateInformationInterop Unbox(MonoObject* value);
		static ApplicationCreateInformation FromInterop(const __ApplicationCreateInformationInterop& value);
		static __ApplicationCreateInformationInterop ToInterop(const ApplicationCreateInformation& value);

	private:
		ScriptApplicationCreateInformation();

	};
#endif
}
