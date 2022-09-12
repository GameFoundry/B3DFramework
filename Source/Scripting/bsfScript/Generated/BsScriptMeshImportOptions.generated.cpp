//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
#if !BS_IS_BANSHEE3D
	ScriptMeshImportOptions::ScriptMeshImportOptions(MonoObject* managedInstance, const SPtr<MeshImportOptions>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptMeshImportOptions::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_getcpuCached", (void*)&ScriptMeshImportOptions::Internal_getcpuCached);
		metaData.scriptClass->AddInternalCall("Internal_setcpuCached", (void*)&ScriptMeshImportOptions::Internal_setcpuCached);
		metaData.scriptClass->AddInternalCall("Internal_getimportNormals", (void*)&ScriptMeshImportOptions::Internal_getimportNormals);
		metaData.scriptClass->AddInternalCall("Internal_setimportNormals", (void*)&ScriptMeshImportOptions::Internal_setimportNormals);
		metaData.scriptClass->AddInternalCall("Internal_getimportTangents", (void*)&ScriptMeshImportOptions::Internal_getimportTangents);
		metaData.scriptClass->AddInternalCall("Internal_setimportTangents", (void*)&ScriptMeshImportOptions::Internal_setimportTangents);
		metaData.scriptClass->AddInternalCall("Internal_getimportBlendShapes", (void*)&ScriptMeshImportOptions::Internal_getimportBlendShapes);
		metaData.scriptClass->AddInternalCall("Internal_setimportBlendShapes", (void*)&ScriptMeshImportOptions::Internal_setimportBlendShapes);
		metaData.scriptClass->AddInternalCall("Internal_getimportSkin", (void*)&ScriptMeshImportOptions::Internal_getimportSkin);
		metaData.scriptClass->AddInternalCall("Internal_setimportSkin", (void*)&ScriptMeshImportOptions::Internal_setimportSkin);
		metaData.scriptClass->AddInternalCall("Internal_getimportAnimation", (void*)&ScriptMeshImportOptions::Internal_getimportAnimation);
		metaData.scriptClass->AddInternalCall("Internal_setimportAnimation", (void*)&ScriptMeshImportOptions::Internal_setimportAnimation);
		metaData.scriptClass->AddInternalCall("Internal_getreduceKeyFrames", (void*)&ScriptMeshImportOptions::Internal_getreduceKeyFrames);
		metaData.scriptClass->AddInternalCall("Internal_setreduceKeyFrames", (void*)&ScriptMeshImportOptions::Internal_setreduceKeyFrames);
		metaData.scriptClass->AddInternalCall("Internal_getimportRootMotion", (void*)&ScriptMeshImportOptions::Internal_getimportRootMotion);
		metaData.scriptClass->AddInternalCall("Internal_setimportRootMotion", (void*)&ScriptMeshImportOptions::Internal_setimportRootMotion);
		metaData.scriptClass->AddInternalCall("Internal_getimportScale", (void*)&ScriptMeshImportOptions::Internal_getimportScale);
		metaData.scriptClass->AddInternalCall("Internal_setimportScale", (void*)&ScriptMeshImportOptions::Internal_setimportScale);
		metaData.scriptClass->AddInternalCall("Internal_getcollisionMeshType", (void*)&ScriptMeshImportOptions::Internal_getcollisionMeshType);
		metaData.scriptClass->AddInternalCall("Internal_setcollisionMeshType", (void*)&ScriptMeshImportOptions::Internal_setcollisionMeshType);
		metaData.scriptClass->AddInternalCall("Internal_getanimationSplits", (void*)&ScriptMeshImportOptions::Internal_getanimationSplits);
		metaData.scriptClass->AddInternalCall("Internal_setanimationSplits", (void*)&ScriptMeshImportOptions::Internal_setanimationSplits);
		metaData.scriptClass->AddInternalCall("Internal_getanimationEvents", (void*)&ScriptMeshImportOptions::Internal_getanimationEvents);
		metaData.scriptClass->AddInternalCall("Internal_setanimationEvents", (void*)&ScriptMeshImportOptions::Internal_setanimationEvents);
		metaData.scriptClass->AddInternalCall("Internal_create", (void*)&ScriptMeshImportOptions::Internal_create);

	}

	MonoObject* ScriptMeshImportOptions::create(const SPtr<MeshImportOptions>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->CreateInstance("bool", ctorParams);
		new (bs_alloc<ScriptMeshImportOptions>()) ScriptMeshImportOptions(managedInstance, value);
		return managedInstance;
	}
	void ScriptMeshImportOptions::Internal_create(MonoObject* managedInstance)
	{
		SPtr<MeshImportOptions> instance = MeshImportOptions::create();
		new (bs_alloc<ScriptMeshImportOptions>())ScriptMeshImportOptions(managedInstance, instance);
	}
	bool ScriptMeshImportOptions::Internal_getcpuCached(ScriptMeshImportOptions* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->cpuCached;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMeshImportOptions::Internal_setcpuCached(ScriptMeshImportOptions* thisPtr, bool value)
	{
		thisPtr->GetInternal()->cpuCached = value;
	}

	bool ScriptMeshImportOptions::Internal_getimportNormals(ScriptMeshImportOptions* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->importNormals;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMeshImportOptions::Internal_setimportNormals(ScriptMeshImportOptions* thisPtr, bool value)
	{
		thisPtr->GetInternal()->importNormals = value;
	}

	bool ScriptMeshImportOptions::Internal_getimportTangents(ScriptMeshImportOptions* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->importTangents;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMeshImportOptions::Internal_setimportTangents(ScriptMeshImportOptions* thisPtr, bool value)
	{
		thisPtr->GetInternal()->importTangents = value;
	}

	bool ScriptMeshImportOptions::Internal_getimportBlendShapes(ScriptMeshImportOptions* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->importBlendShapes;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMeshImportOptions::Internal_setimportBlendShapes(ScriptMeshImportOptions* thisPtr, bool value)
	{
		thisPtr->GetInternal()->importBlendShapes = value;
	}

	bool ScriptMeshImportOptions::Internal_getimportSkin(ScriptMeshImportOptions* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->importSkin;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMeshImportOptions::Internal_setimportSkin(ScriptMeshImportOptions* thisPtr, bool value)
	{
		thisPtr->GetInternal()->importSkin = value;
	}

	bool ScriptMeshImportOptions::Internal_getimportAnimation(ScriptMeshImportOptions* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->importAnimation;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMeshImportOptions::Internal_setimportAnimation(ScriptMeshImportOptions* thisPtr, bool value)
	{
		thisPtr->GetInternal()->importAnimation = value;
	}

	bool ScriptMeshImportOptions::Internal_getreduceKeyFrames(ScriptMeshImportOptions* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->reduceKeyFrames;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMeshImportOptions::Internal_setreduceKeyFrames(ScriptMeshImportOptions* thisPtr, bool value)
	{
		thisPtr->GetInternal()->reduceKeyFrames = value;
	}

	bool ScriptMeshImportOptions::Internal_getimportRootMotion(ScriptMeshImportOptions* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->importRootMotion;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMeshImportOptions::Internal_setimportRootMotion(ScriptMeshImportOptions* thisPtr, bool value)
	{
		thisPtr->GetInternal()->importRootMotion = value;
	}

	float ScriptMeshImportOptions::Internal_getimportScale(ScriptMeshImportOptions* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->importScale;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMeshImportOptions::Internal_setimportScale(ScriptMeshImportOptions* thisPtr, float value)
	{
		thisPtr->GetInternal()->importScale = value;
	}

	CollisionMeshType ScriptMeshImportOptions::Internal_getcollisionMeshType(ScriptMeshImportOptions* thisPtr)
	{
		CollisionMeshType tmp__output;
		tmp__output = thisPtr->GetInternal()->collisionMeshType;

		CollisionMeshType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMeshImportOptions::Internal_setcollisionMeshType(ScriptMeshImportOptions* thisPtr, CollisionMeshType value)
	{
		thisPtr->GetInternal()->collisionMeshType = value;
	}

	MonoArray* ScriptMeshImportOptions::Internal_getanimationSplits(ScriptMeshImportOptions* thisPtr)
	{
		Vector<AnimationSplitInfo> vec__output;
		vec__output = thisPtr->GetInternal()->animationSplits;

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptAnimationSplitInfo>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			SPtr<AnimationSplitInfo> arrayElemPtr__output = bs_shared_ptr_new<AnimationSplitInfo>();
			*arrayElemPtr__output = vec__output[i];
			MonoObject* arrayElem__output;
			arrayElem__output = ScriptAnimationSplitInfo::create(arrayElemPtr__output);
			array__output.Set(i, arrayElem__output);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptMeshImportOptions::Internal_setanimationSplits(ScriptMeshImportOptions* thisPtr, MonoArray* value)
	{
		Vector<AnimationSplitInfo> vecvalue;
		if(value != nullptr)
		{
			ScriptArray Arrayvalue(value);
			vecvalue.Resize(arrayvalue.size());
			for(int i = 0; i < (int)arrayvalue.size(); i++)
			{
				ScriptAnimationSplitInfo* scriptvalue;
				scriptvalue = ScriptAnimationSplitInfo::toNative(arrayvalue.get<MonoObject*>(i));
				if(scriptvalue != nullptr)
				{
					SPtr<AnimationSplitInfo> arrayElemPtrvalue = scriptvalue->GetInternal();
					if(arrayElemPtrvalue)
						vecvalue[i] = *arrayElemPtrvalue;
				}
			}

		}
		thisPtr->GetInternal()->animationSplits = vecvalue;
	}

	MonoArray* ScriptMeshImportOptions::Internal_getanimationEvents(ScriptMeshImportOptions* thisPtr)
	{
		Vector<ImportedAnimationEvents> vec__output;
		vec__output = thisPtr->GetInternal()->animationEvents;

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptImportedAnimationEvents>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			SPtr<ImportedAnimationEvents> arrayElemPtr__output = bs_shared_ptr_new<ImportedAnimationEvents>();
			*arrayElemPtr__output = vec__output[i];
			MonoObject* arrayElem__output;
			arrayElem__output = ScriptImportedAnimationEvents::create(arrayElemPtr__output);
			array__output.Set(i, arrayElem__output);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptMeshImportOptions::Internal_setanimationEvents(ScriptMeshImportOptions* thisPtr, MonoArray* value)
	{
		Vector<ImportedAnimationEvents> vecvalue;
		if(value != nullptr)
		{
			ScriptArray Arrayvalue(value);
			vecvalue.Resize(arrayvalue.size());
			for(int i = 0; i < (int)arrayvalue.size(); i++)
			{
				ScriptImportedAnimationEvents* scriptvalue;
				scriptvalue = ScriptImportedAnimationEvents::toNative(arrayvalue.get<MonoObject*>(i));
				if(scriptvalue != nullptr)
				{
					SPtr<ImportedAnimationEvents> arrayElemPtrvalue = scriptvalue->GetInternal();
					if(arrayElemPtrvalue)
						vecvalue[i] = *arrayElemPtrvalue;
				}
			}

		}
		thisPtr->GetInternal()->animationEvents = vecvalue;
	}
#endif
}
