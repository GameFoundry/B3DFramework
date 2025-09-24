//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Image/B3DSpriteVectorPath.h"
#include "../../../Foundation/Core/Image/B3DSpriteImage.h"
#include "../../../Foundation/Core/VectorGraphics/B3DVectorGraphics.h"
#include "../../../Foundation/Utility/Math/B3DSize2.h"
#include "../../../Foundation/Core/Image/B3DSpriteImage.h"

namespace b3d
{
	struct __SpriteVectorPathCreateInformationInterop
	{
		MonoObject* VectorPath;
		TSize2<int32_t> DefaultSize;
		VectorGraphicsRasterizationScaling ScalingMode;
		SpriteAnimationPlayback AnimationPlayback;
		SpriteSheetGridAnimation Animation;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptSpriteVectorPathCreateInformation : public TScriptTypeDefinition<ScriptSpriteVectorPathCreateInformation>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "SpriteVectorPathCreateInformation")

		static MonoObject* Box(const __SpriteVectorPathCreateInformationInterop& value);
		static __SpriteVectorPathCreateInformationInterop Unbox(MonoObject* value);
		static SpriteVectorPathCreateInformation FromInterop(const __SpriteVectorPathCreateInformationInterop& value);
		static __SpriteVectorPathCreateInformationInterop ToInterop(const SpriteVectorPathCreateInformation& value);

	private:
		ScriptSpriteVectorPathCreateInformation();

	};
}
