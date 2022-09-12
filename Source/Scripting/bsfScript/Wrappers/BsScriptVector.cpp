//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptVector.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptVector2::ScriptVector2(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptVector2::InitRuntimeData()
	{ }

	MonoObject* ScriptVector2::box(const Vector2& value)
	{
		// We're casting away const but it's fine since structs are passed by value anyway
		return MonoUtil::Box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	Vector2 ScriptVector2::Unbox(MonoObject* obj)
	{
		return *(Vector2*)MonoUtil::unbox(obj);
	}

	ScriptVector3::ScriptVector3(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptVector3::InitRuntimeData()
	{ }

	MonoObject* ScriptVector3::box(const Vector3& value)
	{
		// We're casting away const but it's fine since structs are passed by value anyway
		return MonoUtil::Box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	Vector3 ScriptVector3::Unbox(MonoObject* obj)
	{
		return *(Vector3*)MonoUtil::unbox(obj);
	}

	ScriptVector4::ScriptVector4(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptVector4::InitRuntimeData()
	{ }

	MonoObject* ScriptVector4::box(const Vector4& value)
	{
		// We're casting away const but it's fine since structs are passed by value anyway
		return MonoUtil::Box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	Vector4 ScriptVector4::Unbox(MonoObject* obj)
	{
		return *(Vector4*)MonoUtil::unbox(obj);
	}
}
