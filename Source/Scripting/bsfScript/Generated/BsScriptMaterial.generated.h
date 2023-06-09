//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "Image/BsColor.h"
#include "Math/BsMatrix4.h"
#include "../../../Foundation/bsfCore/Material/BsShaderVariation.h"
#include "../../../Foundation/bsfUtility/Image/BsColorGradient.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "Math/BsVector2.h"
#include "Math/BsVector3.h"
#include "Math/BsVector4.h"
#include "Math/BsMatrix3.h"

namespace bs { class Material; }
namespace bs { class MaterialEx; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptMaterial : public TScriptResource<ScriptMaterial, Material>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Material")

		ScriptMaterial(MonoObject* managedInstance, const ResourceHandle<Material>& value);

		static MonoObject* CreateInstance();

	private:
		static MonoObject* InternalGetRef(ScriptMaterial* thisPtr);

		static void InternalSetShader(ScriptMaterial* thisPtr, MonoObject* shader);
		static void InternalSetVariation(ScriptMaterial* thisPtr, MonoObject* variation);
		static MonoObject* InternalClone(ScriptMaterial* thisPtr);
		static MonoObject* InternalGetShader(ScriptMaterial* thisPtr);
		static MonoObject* InternalGetVariation(ScriptMaterial* thisPtr);
		static void InternalSetFloat(ScriptMaterial* thisPtr, MonoString* name, float value, uint32_t arrayIdx);
		static void InternalSetFloatCurve(ScriptMaterial* thisPtr, MonoString* name, MonoObject* value, uint32_t arrayIdx);
		static void InternalSetColor(ScriptMaterial* thisPtr, MonoString* name, Color* value, uint32_t arrayIdx);
		static void InternalSetColorGradient(ScriptMaterial* thisPtr, MonoString* name, MonoObject* value, uint32_t arrayIdx);
		static void InternalSetVec2(ScriptMaterial* thisPtr, MonoString* name, TVector2<float>* value, uint32_t arrayIdx);
		static void InternalSetVec3(ScriptMaterial* thisPtr, MonoString* name, TVector3<float>* value, uint32_t arrayIdx);
		static void InternalSetVec4(ScriptMaterial* thisPtr, MonoString* name, TVector4<float>* value, uint32_t arrayIdx);
		static void InternalSetMat3(ScriptMaterial* thisPtr, MonoString* name, Matrix3* value, uint32_t arrayIdx);
		static void InternalSetMat4(ScriptMaterial* thisPtr, MonoString* name, Matrix4* value, uint32_t arrayIdx);
		static float InternalGetFloat(ScriptMaterial* thisPtr, MonoString* name, uint32_t arrayIdx);
		static MonoObject* InternalGetFloatCurve(ScriptMaterial* thisPtr, MonoString* name, uint32_t arrayIdx);
		static void InternalGetColor(ScriptMaterial* thisPtr, MonoString* name, uint32_t arrayIdx, Color* __output);
		static MonoObject* InternalGetColorGradient(ScriptMaterial* thisPtr, MonoString* name, uint32_t arrayIdx);
		static void InternalGetVec2(ScriptMaterial* thisPtr, MonoString* name, uint32_t arrayIdx, TVector2<float>* __output);
		static void InternalGetVec3(ScriptMaterial* thisPtr, MonoString* name, uint32_t arrayIdx, TVector3<float>* __output);
		static void InternalGetVec4(ScriptMaterial* thisPtr, MonoString* name, uint32_t arrayIdx, TVector4<float>* __output);
		static void InternalGetMat3(ScriptMaterial* thisPtr, MonoString* name, uint32_t arrayIdx, Matrix3* __output);
		static void InternalGetMat4(ScriptMaterial* thisPtr, MonoString* name, uint32_t arrayIdx, Matrix4* __output);
		static bool InternalIsAnimated(ScriptMaterial* thisPtr, MonoString* name, uint32_t arrayIdx);
		static void InternalCreate(MonoObject* managedInstance);
		static void InternalCreate0(MonoObject* managedInstance, MonoObject* shader);
		static void InternalSetTexture(ScriptMaterial* thisPtr, MonoString* name, MonoObject* value, uint32_t mipLevel, uint32_t numMipLevels, uint32_t arraySlice, uint32_t numArraySlices);
		static MonoObject* InternalGetTexture(ScriptMaterial* thisPtr, MonoString* name);
		static void InternalSetSpriteTexture(ScriptMaterial* thisPtr, MonoString* name, MonoObject* value);
		static MonoObject* InternalGetSpriteTexture(ScriptMaterial* thisPtr, MonoString* name);
	};
}
