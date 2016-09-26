//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptMorphShapes.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "BsMorphShapes.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	ScriptMorphShapes::ScriptMorphShapes(MonoObject* managedInstance, const SPtr<MorphShapes>& morphShapes)
		:ScriptObject(managedInstance), mMorphShapes(morphShapes)
	{ }

	void ScriptMorphShapes::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetChannels", &ScriptMorphShapes::internal_GetChannels);
	}

	MonoObject* ScriptMorphShapes::create(const SPtr<MorphShapes>& morphShapes)
	{
		MonoObject* instance = metaData.scriptClass->createInstance();

		new (bs_alloc<ScriptMorphShapes>()) ScriptMorphShapes(instance, morphShapes);
		return instance;
	}

	MonoArray* ScriptMorphShapes::internal_GetChannels(ScriptMorphShapes* thisPtr)
	{
		SPtr<MorphShapes> morphShapes = thisPtr->getInternal();

		UINT32 numChannels = morphShapes->getNumChannels();
		ScriptArray scriptArray = ScriptArray::create<ScriptMorphChannel>(numChannels);

		for (UINT32 i = 0; i < numChannels; i++)
		{
			MonoObject* monoChannel = ScriptMorphChannel::toManaged(morphShapes->getChannel(i));
			scriptArray.set(i, monoChannel);
		}

		return scriptArray.getInternal();
	}

	ScriptMorphChannel::ScriptMorphChannel(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptMorphChannel::initRuntimeData()
	{ }

	MonoObject* ScriptMorphChannel::toManaged(const SPtr<MorphChannel>& value)
	{
		MonoString* monoName = MonoUtil::stringToMono(value->getName());

		UINT32 numShapes = value->getNumShapes();
		ScriptArray shapeArray = ScriptArray::create<ScriptMorphShape>(numShapes);
		for(UINT32 i = 0; i < numShapes; i++)
		{
			MonoObject* managedShape = ScriptMorphShape::toManaged(value->getShape(i));
			shapeArray.set(i, managedShape);
		}

		void* params[2] = { monoName, shapeArray.getInternal() };
		return metaData.scriptClass->createInstance("string,MorphShape[]", params);
	}

	ScriptMorphShape::ScriptMorphShape(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptMorphShape::initRuntimeData()
	{ }

	MonoObject* ScriptMorphShape::toManaged(const SPtr<MorphShape>& value)
	{
		MonoString* monoName = MonoUtil::stringToMono(value->getName());
		float weight = value->getWeight();

		void* params[2] = { monoName, &weight };
		return metaData.scriptClass->createInstance("string,single", params);
	}
}