//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsEditorUtility.h"
#include "BsSceneObject.h"
#include "BsCRenderable.h"

namespace BansheeEngine
{
	AABox EditorUtility::calculateBounds(const HSceneObject& object)
	{
		Vector<HSceneObject> objects = { object };

		return calculateBounds(objects);
	}

	AABox EditorUtility::calculateBounds(const Vector<HSceneObject>& objects)
	{
		if (objects.size() == 0)
			return AABox(Vector3::ZERO, Vector3::ZERO);

		AABox bounds = AABox(Vector3::INF, -Vector3::INF);
		bool gotOneMesh = false;

		for (auto& object : objects)
		{
			AABox meshBounds;
			if (calculateMeshBounds(object, meshBounds))
			{
				bounds.merge(meshBounds);
				gotOneMesh = true;
			}
		}

		if (!gotOneMesh)
		{
			for (auto& object : objects)
			{
				if (object.isDestroyed())
					continue;

				bounds.merge(object->getWorldPosition());
				gotOneMesh = true;
			}
		}

		if(gotOneMesh)
			return bounds;

		return AABox(Vector3::ZERO, Vector3::ZERO);
	}

	Vector3 EditorUtility::calculateCenter(const Vector<HSceneObject>& objects)
	{
		if (objects.size() == 0)
			return Vector3::ZERO;

		Vector3 center = Vector3::ZERO;
		UINT32 count = 0;

		for (auto& object : objects)
		{
			AABox meshBounds;
			if (calculateMeshBounds(object, meshBounds))
			{
				if (meshBounds.getSize() == Vector3::INF)
					center += object->getWorldPosition();
				else
					center += meshBounds.getCenter();

				count++;
			}
		}

		if (count == 0)
		{
			for (auto& object : objects)
			{
				if (object.isDestroyed())
					continue;

				center += object->getWorldPosition();
				count++;
			}
		}

		if (count > 0)
			return center / (float)count;

		return Vector3::ZERO;
	}

	bool EditorUtility::calculateMeshBounds(const HSceneObject& object, AABox& bounds)
	{
		bounds = AABox(Vector3::ZERO, Vector3::ZERO);
		if (object.isDestroyed())
			return false;

		bool foundOne = false;
		const Vector<HComponent>& components = object->getComponents();
		for (auto& component : components)
		{
			Bounds curBounds;
			if (component->calculateBounds(curBounds))
			{
				if (!foundOne)
				{
					bounds = curBounds.getBox();
					foundOne = true;
				}
				else
					bounds.merge(curBounds.getBox());
			}
			else
			{
				if (!foundOne)
					bounds = curBounds.getBox();
			}
		}

		return foundOne;
	}

	EditorUtility::SceneObjProxy EditorUtility::createProxy(const HSceneObject& sceneObject)
	{
		struct TempData
		{
			TempData(SceneObjProxy& proxy, const HSceneObject& so)
				:proxy(proxy), obj(so)
			{ }

			SceneObjProxy& proxy;
			HSceneObject obj;
		};

		SceneObjProxy rootProxy;

		Stack<TempData> todo;
		todo.push(TempData(rootProxy, sceneObject));

		while (!todo.empty())
		{
			TempData data = todo.top();
			todo.pop();

			data.proxy.instanceData = data.obj->_getInstanceData();

			const Vector<HComponent>& components = data.obj->getComponents();
			for (auto& component : components)
				data.proxy.componentInstanceData.push_back(component->_getInstanceData());

			UINT32 numChildren = data.obj->getNumChildren();
			data.proxy.children.resize(numChildren);
			for (UINT32 i = 0; i < numChildren; i++)
			{
				todo.push(TempData(data.proxy.children[i], data.obj->getChild(i)));
			}
		}

		return rootProxy;
	}

	void EditorUtility::restoreIds(const HSceneObject& restored, SceneObjProxy& proxy)
	{
		// Note: This method relies that all restored GameObject handles pointing to the
		// same object also have the same shared handle data (Since I only update instance
		// data on a single handle I know exists, and expect all others will be updated
		// by that as well).

		struct TempData
		{
			TempData(SceneObjProxy& proxy, const HSceneObject& restoredObj)
				:proxy(proxy), restoredObj(restoredObj)
			{ }

			SceneObjProxy& proxy;
			HSceneObject restoredObj;
		};

		Stack<TempData> todo;
		todo.push(TempData(proxy, restored));

		while (!todo.empty())
		{
			TempData data = todo.top();
			todo.pop();

			data.restoredObj->_setInstanceData(data.proxy.instanceData);

			// Find components that are still active and swap the old ones with restored ones,
			// keep any other as is.
			const Vector<HComponent>& restoredComponents = data.restoredObj->getComponents();

			UINT32 idx = 0;
			for (auto& restoredComponent : restoredComponents)
			{
				restoredComponent->_setInstanceData(data.proxy.componentInstanceData[idx]);

				SPtr<GameObject> restoredPtr = std::static_pointer_cast<GameObject>(restoredComponent.getInternalPtr());
				HComponent restoredComponentCopy = restoredComponent; // To remove const
				restoredComponentCopy._setHandleData(restoredPtr);

				idx++;
			}

			// Find children that are still active and swap the old ones with restored ones,
			// keep any other as is
			UINT32 restoredNumChildren = data.restoredObj->getNumChildren();
			for (UINT32 i = 0; i < restoredNumChildren; i++)
			{
				todo.push(TempData(data.proxy.children[i], data.restoredObj->getChild(i)));
			}
		}
	}
}