//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptMeshImportOptions.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptAnimationSplitInfo.generated.h"
#include "B3DScriptImportedAnimationEvents.generated.h"
#include "B3DScriptMeshImportOptions.generated.h"

namespace b3d
{
#if !B3D_IS_ENGINE
	ScriptMeshImportOptions::ScriptMeshImportOptions(const TShared<MeshImportOptions>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptMeshImportOptions::~ScriptMeshImportOptions()
	{
		UnregisterEvents();
	}

	void ScriptMeshImportOptions::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetCpuCached", (void*)&ScriptMeshImportOptions::InternalGetCpuCached);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetCpuCached", (void*)&ScriptMeshImportOptions::InternalSetCpuCached);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetImportNormals", (void*)&ScriptMeshImportOptions::InternalGetImportNormals);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetImportNormals", (void*)&ScriptMeshImportOptions::InternalSetImportNormals);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetImportTangents", (void*)&ScriptMeshImportOptions::InternalGetImportTangents);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetImportTangents", (void*)&ScriptMeshImportOptions::InternalSetImportTangents);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetImportBlendShapes", (void*)&ScriptMeshImportOptions::InternalGetImportBlendShapes);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetImportBlendShapes", (void*)&ScriptMeshImportOptions::InternalSetImportBlendShapes);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetImportSkin", (void*)&ScriptMeshImportOptions::InternalGetImportSkin);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetImportSkin", (void*)&ScriptMeshImportOptions::InternalSetImportSkin);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetImportAnimation", (void*)&ScriptMeshImportOptions::InternalGetImportAnimation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetImportAnimation", (void*)&ScriptMeshImportOptions::InternalSetImportAnimation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetReduceKeyFrames", (void*)&ScriptMeshImportOptions::InternalGetReduceKeyFrames);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetReduceKeyFrames", (void*)&ScriptMeshImportOptions::InternalSetReduceKeyFrames);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetImportRootMotion", (void*)&ScriptMeshImportOptions::InternalGetImportRootMotion);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetImportRootMotion", (void*)&ScriptMeshImportOptions::InternalSetImportRootMotion);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetImportScale", (void*)&ScriptMeshImportOptions::InternalGetImportScale);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetImportScale", (void*)&ScriptMeshImportOptions::InternalSetImportScale);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetCollisionMeshType", (void*)&ScriptMeshImportOptions::InternalGetCollisionMeshType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetCollisionMeshType", (void*)&ScriptMeshImportOptions::InternalSetCollisionMeshType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetAnimationSplits", (void*)&ScriptMeshImportOptions::InternalGetAnimationSplits);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetAnimationSplits", (void*)&ScriptMeshImportOptions::InternalSetAnimationSplits);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetAnimationEvents", (void*)&ScriptMeshImportOptions::InternalGetAnimationEvents);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetAnimationEvents", (void*)&ScriptMeshImportOptions::InternalSetAnimationEvents);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptMeshImportOptions::InternalCreate);

	}

	MonoObject* ScriptMeshImportOptions::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptMeshImportOptions::InternalCreate(MonoObject* scriptObject)
	{
		TShared<MeshImportOptions> nativeObject = MeshImportOptions::Create();
		ScriptObjectWrapper::Create<ScriptMeshImportOptions>(nativeObject, scriptObject);
	}
	bool ScriptMeshImportOptions::InternalGetCpuCached(ScriptMeshImportOptions* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<MeshImportOptions*>(self->GetNativeObject())->CpuCached;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMeshImportOptions::InternalSetCpuCached(ScriptMeshImportOptions* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<MeshImportOptions*>(self->GetNativeObject())->CpuCached = value;
	}

	bool ScriptMeshImportOptions::InternalGetImportNormals(ScriptMeshImportOptions* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<MeshImportOptions*>(self->GetNativeObject())->ImportNormals;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMeshImportOptions::InternalSetImportNormals(ScriptMeshImportOptions* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<MeshImportOptions*>(self->GetNativeObject())->ImportNormals = value;
	}

	bool ScriptMeshImportOptions::InternalGetImportTangents(ScriptMeshImportOptions* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<MeshImportOptions*>(self->GetNativeObject())->ImportTangents;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMeshImportOptions::InternalSetImportTangents(ScriptMeshImportOptions* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<MeshImportOptions*>(self->GetNativeObject())->ImportTangents = value;
	}

	bool ScriptMeshImportOptions::InternalGetImportBlendShapes(ScriptMeshImportOptions* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<MeshImportOptions*>(self->GetNativeObject())->ImportBlendShapes;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMeshImportOptions::InternalSetImportBlendShapes(ScriptMeshImportOptions* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<MeshImportOptions*>(self->GetNativeObject())->ImportBlendShapes = value;
	}

	bool ScriptMeshImportOptions::InternalGetImportSkin(ScriptMeshImportOptions* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<MeshImportOptions*>(self->GetNativeObject())->ImportSkin;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMeshImportOptions::InternalSetImportSkin(ScriptMeshImportOptions* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<MeshImportOptions*>(self->GetNativeObject())->ImportSkin = value;
	}

	bool ScriptMeshImportOptions::InternalGetImportAnimation(ScriptMeshImportOptions* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<MeshImportOptions*>(self->GetNativeObject())->ImportAnimation;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMeshImportOptions::InternalSetImportAnimation(ScriptMeshImportOptions* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<MeshImportOptions*>(self->GetNativeObject())->ImportAnimation = value;
	}

	bool ScriptMeshImportOptions::InternalGetReduceKeyFrames(ScriptMeshImportOptions* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<MeshImportOptions*>(self->GetNativeObject())->ReduceKeyFrames;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMeshImportOptions::InternalSetReduceKeyFrames(ScriptMeshImportOptions* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<MeshImportOptions*>(self->GetNativeObject())->ReduceKeyFrames = value;
	}

	bool ScriptMeshImportOptions::InternalGetImportRootMotion(ScriptMeshImportOptions* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<MeshImportOptions*>(self->GetNativeObject())->ImportRootMotion;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMeshImportOptions::InternalSetImportRootMotion(ScriptMeshImportOptions* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<MeshImportOptions*>(self->GetNativeObject())->ImportRootMotion = value;
	}

	float ScriptMeshImportOptions::InternalGetImportScale(ScriptMeshImportOptions* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<MeshImportOptions*>(self->GetNativeObject())->ImportScale;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMeshImportOptions::InternalSetImportScale(ScriptMeshImportOptions* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<MeshImportOptions*>(self->GetNativeObject())->ImportScale = value;
	}

	CollisionMeshType ScriptMeshImportOptions::InternalGetCollisionMeshType(ScriptMeshImportOptions* self)
	{
		CollisionMeshType tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<MeshImportOptions*>(self->GetNativeObject())->CollisionMeshType;

		CollisionMeshType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMeshImportOptions::InternalSetCollisionMeshType(ScriptMeshImportOptions* self, CollisionMeshType value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<MeshImportOptions*>(self->GetNativeObject())->CollisionMeshType = value;
	}

	MonoArray* ScriptMeshImportOptions::InternalGetAnimationSplits(ScriptMeshImportOptions* self)
	{
		Vector<AnimationSplitInfo> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<MeshImportOptions*>(self->GetNativeObject())->AnimationSplits;

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptAnimationSplitInfo>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			TShared<AnimationSplitInfo> arrayElementPointer__output = B3DMakeShared<AnimationSplitInfo>();
			*arrayElementPointer__output = nativeArray__output[elementIndex];
			MonoObject* arrayElement__output;
			arrayElement__output = ScriptAnimationSplitInfo::GetOrCreateScriptObject(arrayElementPointer__output);
			scriptArray__output.Set(elementIndex, arrayElement__output);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptMeshImportOptions::InternalSetAnimationSplits(ScriptMeshImportOptions* self, MonoArray* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		Vector<AnimationSplitInfo> nativeArrayvalue;
		if(value != nullptr)
		{
			ScriptArray scriptArrayvalue(value);
			nativeArrayvalue.resize(scriptArrayvalue.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvalue.Size(); elementIndex++)
			{
				TShared<AnimationSplitInfo> arrayElementPointervalue;
				ScriptAnimationSplitInfo* scriptObjectWrappervalue;
				scriptObjectWrappervalue = ScriptAnimationSplitInfo::GetScriptObjectWrapper(scriptArrayvalue.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrappervalue != nullptr)
				{
					arrayElementPointervalue = std::static_pointer_cast<AnimationSplitInfo>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
					if(arrayElementPointervalue)
						nativeArrayvalue[elementIndex] = *arrayElementPointervalue;
				}
			}

		}
		static_cast<MeshImportOptions*>(self->GetNativeObject())->AnimationSplits = nativeArrayvalue;
	}

	MonoArray* ScriptMeshImportOptions::InternalGetAnimationEvents(ScriptMeshImportOptions* self)
	{
		Vector<ImportedAnimationEvents> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<MeshImportOptions*>(self->GetNativeObject())->AnimationEvents;

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptImportedAnimationEvents>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			TShared<ImportedAnimationEvents> arrayElementPointer__output = B3DMakeShared<ImportedAnimationEvents>();
			*arrayElementPointer__output = nativeArray__output[elementIndex];
			MonoObject* arrayElement__output;
			arrayElement__output = ScriptImportedAnimationEvents::GetOrCreateScriptObject(arrayElementPointer__output);
			scriptArray__output.Set(elementIndex, arrayElement__output);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptMeshImportOptions::InternalSetAnimationEvents(ScriptMeshImportOptions* self, MonoArray* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		Vector<ImportedAnimationEvents> nativeArrayvalue;
		if(value != nullptr)
		{
			ScriptArray scriptArrayvalue(value);
			nativeArrayvalue.resize(scriptArrayvalue.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvalue.Size(); elementIndex++)
			{
				TShared<ImportedAnimationEvents> arrayElementPointervalue;
				ScriptImportedAnimationEvents* scriptObjectWrappervalue;
				scriptObjectWrappervalue = ScriptImportedAnimationEvents::GetScriptObjectWrapper(scriptArrayvalue.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrappervalue != nullptr)
				{
					arrayElementPointervalue = std::static_pointer_cast<ImportedAnimationEvents>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
					if(arrayElementPointervalue)
						nativeArrayvalue[elementIndex] = *arrayElementPointervalue;
				}
			}

		}
		static_cast<MeshImportOptions*>(self->GetNativeObject())->AnimationEvents = nativeArrayvalue;
	}
#endif
}
