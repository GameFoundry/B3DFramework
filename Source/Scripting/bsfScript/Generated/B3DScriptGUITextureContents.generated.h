//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Engine/GUI/B3DGUITexture.h"
#include "../../../Foundation/Engine/Utility/B3DEnums.h"

namespace b3d
{
	struct __GUITextureContentsInterop
	{
		MonoObject* Image;
		TextureScaleMode ScaleMode;
		bool IsTransparent;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUITextureContents : public TScriptTypeDefinition<ScriptGUITextureContents>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUITextureContents")

		static MonoObject* Box(const __GUITextureContentsInterop& value);
		static __GUITextureContentsInterop Unbox(MonoObject* value);
		static GUITextureContents FromInterop(const __GUITextureContentsInterop& value);
		static __GUITextureContentsInterop ToInterop(const GUITextureContents& value);

	private:
		ScriptGUITextureContents();

	};
}
