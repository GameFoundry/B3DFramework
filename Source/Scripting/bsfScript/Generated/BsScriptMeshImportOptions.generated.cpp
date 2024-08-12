//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptMeshImportOptions.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptAnimationSplitInfo.generated.h"
#include "BsScriptImportedAnimationEvents.generated.h"
#include "BsScriptMeshImportOptions.generated.h"

namespace bs
{
#if !B3D_IS_ENGINE
	ScriptMeshImportOptions::ScriptMeshImportOptions(MonoObject* managedInstance, const SPtr<MeshImportOptions>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptMeshImportOptions::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetCpuCached", (void*)&ScriptMeshImportOptions::InternalGetCpuCached);
		metaData.ScriptClass->AddInternalCall("Internal_SetCpuCached", (void*)&ScriptMeshImportOptions::InternalSetCpuCached);
		metaData.ScriptClass->AddInternalCall("Internal_GetImportNormals", (void*)&ScriptMeshImportOptions::InternalGetImportNormals);
		metaData.ScriptClass->AddInternalCall("Internal_SetImportNormals", (void*)&ScriptMeshImportOptions::InternalSetImportNormals);
		metaData.ScriptClass->AddInternalCall("Internal_GetImportTangents", (void*)&ScriptMeshImportOptions::InternalGetImportTangents);
		metaData.ScriptClass->AddInternalCall("Internal_SetImportTangents", (void*)&ScriptMeshImportOptions::InternalSetImportTangents);
		metaData.ScriptClass->AddInternalCall("Internal_GetImportBlendShapes", (void*)&ScriptMeshImportOptions::InternalGetImportBlendShapes);
		metaData.ScriptClass->AddInternalCall("Internal_SetImportBlendShapes", (void*)&ScriptMeshImportOptions::InternalSetImportBlendShapes);
		metaData.ScriptClass->AddInternalCall("Internal_GetImportSkin", (void*)&ScriptMeshImportOptions::InternalGetImportSkin);
		metaData.ScriptClass->AddInternalCall("Internal_SetImportSkin", (void*)&ScriptMeshImportOptions::InternalSetImportSkin);
		metaData.ScriptClass->AddInternalCall("Internal_GetImportAnimation", (void*)&ScriptMeshImportOptions::InternalGetImportAnimation);
		metaData.ScriptClass->AddInternalCall("Internal_SetImportAnimation", (void*)&ScriptMeshImportOptions::InternalSetImportAnimation);
		metaData.ScriptClass->AddInternalCall("Internal_GetReduceKeyFrames", (void*)&ScriptMeshImportOptions::InternalGetReduceKeyFrames);
		metaData.ScriptClass->AddInternalCall("Internal_SetReduceKeyFrames", (void*)&ScriptMeshImportOptions::InternalSetReduceKeyFrames);
		metaData.ScriptClass->AddInternalCall("Internal_GetImportRootMotion", (void*)&ScriptMeshImportOptions::InternalGetImportRootMotion);
		metaData.ScriptClass->AddInternalCall("Internal_SetImportRootMotion", (void*)&ScriptMeshImportOptions::InternalSetImportRootMotion);
		metaData.ScriptClass->AddInternalCall("Internal_GetImportScale", (void*)&ScriptMeshImportOptions::InternalGetImportScale);
		metaData.ScriptClass->AddInternalCall("Internal_SetImportScale", (void*)&ScriptMeshImportOptions::InternalSetImportScale);
		metaData.ScriptClass->AddInternalCall("Internal_GetCollisionMeshType", (void*)&ScriptMeshImportOptions::InternalGetCollisionMeshType);
		metaData.ScriptClass->AddInternalCall("Internal_SetCollisionMeshType", (void*)&ScriptMeshImportOptions::InternalSetCollisionMeshType);
		metaData.ScriptClass->AddInternalCall("Internal_GetAnimationSplits", (void*)&ScriptMeshImportOptions::InternalGetAnimationSplits);
		metaData.ScriptClass->AddInternalCall("Internal_SetAnimationSplits", (void*)&ScriptMeshImportOptions::InternalSetAnimationSplits);
		metaData.ScriptClass->AddInternalCall("Internal_GetAnimationEvents", (void*)&ScriptMeshImportOptions::InternalGetAnimationEvents);
		metaData.ScriptClass->AddInternalCall("Internal_SetAnimationEvents", (void*)&ScriptMeshImportOptions::InternalSetAnimationEvents);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptMeshImportOptions::InternalCreate);

	}

	MonoObject* ScriptMeshImportOptions::Create(const SPtr<MeshImportOptions>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptMeshImportOptions>()) ScriptMeshImportOptions(managedInstance, value);
		return managedInstance;
	}
	void ScriptMeshImportOptions::InternalCreate(MonoObject* managedInstance)
	{
		SPtr<MeshImportOptions> instance = MeshImportOptions::Create();
		new (B3DAllocate<ScriptMeshImportOptions>())ScriptMeshImportOptions(managedInstance, instance);
	}
	bool ScriptMeshImportOptions::InternalGetCpuCached(ScriptMeshImportOptions* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->CpuCached;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMeshImportOptions::InternalSetCpuCached(ScriptMeshImportOptions* thisPtr, bool value)
	{
		thisPtr->GetInternal()->CpuCached = value;
	}

	bool ScriptMeshImportOptions::InternalGetImportNormals(ScriptMeshImportOptions* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->ImportNormals;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMeshImportOptions::InternalSetImportNormals(ScriptMeshImportOptions* thisPtr, bool value)
	{
		thisPtr->GetInternal()->ImportNormals = value;
	}

	bool ScriptMeshImportOptions::InternalGetImportTangents(ScriptMeshImportOptions* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->ImportTangents;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMeshImportOptions::InternalSetImportTangents(ScriptMeshImportOptions* thisPtr, bool value)
	{
		thisPtr->GetInternal()->ImportTangents = value;
	}

	bool ScriptMeshImportOptions::InternalGetImportBlendShapes(ScriptMeshImportOptions* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->ImportBlendShapes;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMeshImportOptions::InternalSetImportBlendShapes(ScriptMeshImportOptions* thisPtr, bool value)
	{
		thisPtr->GetInternal()->ImportBlendShapes = value;
	}

	bool ScriptMeshImportOptions::InternalGetImportSkin(ScriptMeshImportOptions* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->ImportSkin;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMeshImportOptions::InternalSetImportSkin(ScriptMeshImportOptions* thisPtr, bool value)
	{
		thisPtr->GetInternal()->ImportSkin = value;
	}

	bool ScriptMeshImportOptions::InternalGetImportAnimation(ScriptMeshImportOptions* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->ImportAnimation;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMeshImportOptions::InternalSetImportAnimation(ScriptMeshImportOptions* thisPtr, bool value)
	{
		thisPtr->GetInternal()->ImportAnimation = value;
	}

	bool ScriptMeshImportOptions::InternalGetReduceKeyFrames(ScriptMeshImportOptions* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->ReduceKeyFrames;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMeshImportOptions::InternalSetReduceKeyFrames(ScriptMeshImportOptions* thisPtr, bool value)
	{
		thisPtr->GetInternal()->ReduceKeyFrames = value;
	}

	bool ScriptMeshImportOptions::InternalGetImportRootMotion(ScriptMeshImportOptions* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->ImportRootMotion;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMeshImportOptions::InternalSetImportRootMotion(ScriptMeshImportOptions* thisPtr, bool value)
	{
		thisPtr->GetInternal()->ImportRootMotion = value;
	}

	float ScriptMeshImportOptions::InternalGetImportScale(ScriptMeshImportOptions* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->ImportScale;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMeshImportOptions::InternalSetImportScale(ScriptMeshImportOptions* thisPtr, float value)
	{
		thisPtr->GetInternal()->ImportScale = value;
	}

	CollisionMeshType ScriptMeshImportOptions::InternalGetCollisionMeshType(ScriptMeshImportOptions* thisPtr)
	{
		CollisionMeshType tmp__output;
		tmp__output = thisPtr->GetInternal()->CollisionMeshType;

		CollisionMeshType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMeshImportOptions::InternalSetCollisionMeshType(ScriptMeshImportOptions* thisPtr, CollisionMeshType value)
	{
		thisPtr->GetInternal()->CollisionMeshType = value;
	}

	MonoArray* ScriptMeshImportOptions::InternalGetAnimationSplits(ScriptMeshImportOptions* thisPtr)
	{
		Vector<AnimationSplitInfo> nativeArray__output;
		nativeArray__output = thisPtr->GetInternal()->AnimationSplits;

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptAnimationSplitInfo>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			SPtr<AnimationSplitInfo> arrayElementPointer__output = B3DMakeShared<AnimationSplitInfo>();
			*arrayElementPointer__output = nativeArray__output[elementIndex];
			MonoObject* arrayElement__output;
			arrayElement__output = ScriptAnimationSplitInfo::Create(arrayElementPointer__output);
			scriptArray__output.Set(elementIndex, arrayElement__output);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptMeshImportOptions::InternalSetAnimationSplits(ScriptMeshImportOptions* thisPtr, MonoArray* value)
	{
		Vector<AnimationSplitInfo> nativeArrayvalue;
		if(value != nullptr)
		{
			ScriptArray scriptArrayvalue(value);
			nativeArrayvalue.resize(scriptArrayvalue.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvalue.Size(); elementIndex++)
			{
				ScriptAnimationSplitInfo* scriptObjectWrappervalue;
				scriptObjectWrappervalue = ScriptAnimationSplitInfo::ToNative(scriptArrayvalue.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrappervalue != nullptr)
				{
					SPtr<AnimationSplitInfo> arrayElementPointervalue = scriptObjectWrappervalue->GetInternal();
					if(arrayElementPointervalue)
						nativeArrayvalue[elementIndex] = *arrayElementPointervalue;
				}
			}

		}
		thisPtr->GetInternal()->AnimationSplits = nativeArrayvalue;
	}

	MonoArray* ScriptMeshImportOptions::InternalGetAnimationEvents(ScriptMeshImportOptions* thisPtr)
	{
		Vector<ImportedAnimationEvents> nativeArray__output;
		nativeArray__output = thisPtr->GetInternal()->AnimationEvents;

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptImportedAnimationEvents>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			SPtr<ImportedAnimationEvents> arrayElementPointer__output = B3DMakeShared<ImportedAnimationEvents>();
			*arrayElementPointer__output = nativeArray__output[elementIndex];
			MonoObject* arrayElement__output;
			arrayElement__output = ScriptImportedAnimationEvents::Create(arrayElementPointer__output);
			scriptArray__output.Set(elementIndex, arrayElement__output);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptMeshImportOptions::InternalSetAnimationEvents(ScriptMeshImportOptions* thisPtr, MonoArray* value)
	{
		Vector<ImportedAnimationEvents> nativeArrayvalue;
		if(value != nullptr)
		{
			ScriptArray scriptArrayvalue(value);
			nativeArrayvalue.resize(scriptArrayvalue.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvalue.Size(); elementIndex++)
			{
				ScriptImportedAnimationEvents* scriptObjectWrappervalue;
				scriptObjectWrappervalue = ScriptImportedAnimationEvents::ToNative(scriptArrayvalue.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrappervalue != nullptr)
				{
					SPtr<ImportedAnimationEvents> arrayElementPointervalue = scriptObjectWrappervalue->GetInternal();
					if(arrayElementPointervalue)
						nativeArrayvalue[elementIndex] = *arrayElementPointervalue;
				}
			}

		}
		thisPtr->GetInternal()->AnimationEvents = nativeArrayvalue;
	}
#endif
}
