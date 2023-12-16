//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUITexture.h"
#include "../../../Foundation/bsfEngine/Utility/BsEnums.h"

namespace bs
{
	struct __GUITextureContentsInterop
	{
		MonoObject* Image;
		TextureScaleMode ScaleMode;
		bool IsTransparent;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUITextureContents : public ScriptObject<ScriptGUITextureContents>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUITextureContents")

		static MonoObject* Box(const __GUITextureContentsInterop& value);
		static __GUITextureContentsInterop Unbox(MonoObject* value);
		static GUITextureContents FromInterop(const __GUITextureContentsInterop& value);
		static __GUITextureContentsInterop ToInterop(const GUITextureContents& value);

	private:
		ScriptGUITextureContents(MonoObject* managedInstance);

	};
}
