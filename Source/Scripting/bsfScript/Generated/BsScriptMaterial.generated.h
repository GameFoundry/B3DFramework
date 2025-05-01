//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResourceWrapper.h"
#include "../../../Foundation/bsfUtility/Image/BsColor.h"
#include "Math/BsMatrix4.h"
#include "../../../Foundation/bsfCore/Material/BsShaderVariation.h"
#include "../../../Foundation/bsfUtility/Image/BsColorGradient.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "../../../Foundation/bsfUtility/Math/BsVector2.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3.h"
#include "../../../Foundation/bsfUtility/Math/BsVector4.h"
#include "Math/BsMatrix3.h"

namespace bs { class Material; }
namespace bs { class MaterialEx; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptMaterial : public TScriptResourceWrapper<Material, ScriptMaterial>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Material")

		ScriptMaterial(const TResourceHandle<Material>& nativeObject);
		~ScriptMaterial();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetRef(ScriptMaterial* self);

		static void InternalSetShader(ScriptMaterial* self, MonoObject* shader);
		static void InternalSetVariation(ScriptMaterial* self, MonoObject* variation);
		static MonoObject* InternalClone(ScriptMaterial* self);
		static MonoObject* InternalGetShader(ScriptMaterial* self);
		static MonoObject* InternalGetVariation(ScriptMaterial* self);
		static void InternalSetFloat(ScriptMaterial* self, MonoString* name, float value, uint32_t arrayIdx);
		static void InternalSetFloatCurve(ScriptMaterial* self, MonoString* name, MonoObject* value, uint32_t arrayIdx);
		static void InternalSetColor(ScriptMaterial* self, MonoString* name, Color* value, uint32_t arrayIdx);
		static void InternalSetColorGradient(ScriptMaterial* self, MonoString* name, MonoObject* value, uint32_t arrayIdx);
		static void InternalSetVec2(ScriptMaterial* self, MonoString* name, TVector2<float>* value, uint32_t arrayIdx);
		static void InternalSetVec3(ScriptMaterial* self, MonoString* name, TVector3<float>* value, uint32_t arrayIdx);
		static void InternalSetVec4(ScriptMaterial* self, MonoString* name, TVector4<float>* value, uint32_t arrayIdx);
		static void InternalSetMat3(ScriptMaterial* self, MonoString* name, Matrix3* value, uint32_t arrayIdx);
		static void InternalSetMat4(ScriptMaterial* self, MonoString* name, Matrix4* value, uint32_t arrayIdx);
		static float InternalGetFloat(ScriptMaterial* self, MonoString* name, uint32_t arrayIdx);
		static MonoObject* InternalGetFloatCurve(ScriptMaterial* self, MonoString* name, uint32_t arrayIdx);
		static void InternalGetColor(ScriptMaterial* self, MonoString* name, uint32_t arrayIdx, Color* __output);
		static MonoObject* InternalGetColorGradient(ScriptMaterial* self, MonoString* name, uint32_t arrayIdx);
		static void InternalGetVec2(ScriptMaterial* self, MonoString* name, uint32_t arrayIdx, TVector2<float>* __output);
		static void InternalGetVec3(ScriptMaterial* self, MonoString* name, uint32_t arrayIdx, TVector3<float>* __output);
		static void InternalGetVec4(ScriptMaterial* self, MonoString* name, uint32_t arrayIdx, TVector4<float>* __output);
		static void InternalGetMat3(ScriptMaterial* self, MonoString* name, uint32_t arrayIdx, Matrix3* __output);
		static void InternalGetMat4(ScriptMaterial* self, MonoString* name, uint32_t arrayIdx, Matrix4* __output);
		static bool InternalIsAnimated(ScriptMaterial* self, MonoString* name, uint32_t arrayIdx);
		static void InternalCreate(MonoObject* scriptObject);
		static void InternalCreate0(MonoObject* scriptObject, MonoObject* shader);
		static void InternalSetTexture(ScriptMaterial* self, MonoString* name, MonoObject* value, uint32_t mipLevel, uint32_t numMipLevels, uint32_t arraySlice, uint32_t numArraySlices);
		static MonoObject* InternalGetTexture(ScriptMaterial* self, MonoString* name);
		static void InternalSetSpriteImage(ScriptMaterial* self, MonoString* name, MonoObject* value);
		static MonoObject* InternalGetSpriteImage(ScriptMaterial* self, MonoString* name);
	};
}
