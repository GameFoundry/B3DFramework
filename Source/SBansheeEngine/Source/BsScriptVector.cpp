//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptVector.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptVector2::ScriptVector2(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptVector2::initRuntimeData()
	{ }

	MonoObject* ScriptVector2::box(const Vector2& value)
	{
		// We're casting away const but it's fine since structs are passed by value anyway
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	Vector2 ScriptVector2::unbox(MonoObject* obj)
	{
		return *(Vector2*)MonoUtil::unbox(obj);
	}

	ScriptVector3::ScriptVector3(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptVector3::initRuntimeData()
	{ }

	MonoObject* ScriptVector3::box(const Vector3& value)
	{
		// We're casting away const but it's fine since structs are passed by value anyway
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	Vector3 ScriptVector3::unbox(MonoObject* obj)
	{
		return *(Vector3*)MonoUtil::unbox(obj);
	}

	ScriptVector4::ScriptVector4(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptVector4::initRuntimeData()
	{ }

	MonoObject* ScriptVector4::box(const Vector4& value)
	{
		// We're casting away const but it's fine since structs are passed by value anyway
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	Vector4 ScriptVector4::unbox(MonoObject* obj)
	{
		return *(Vector4*)MonoUtil::unbox(obj);
	}
}