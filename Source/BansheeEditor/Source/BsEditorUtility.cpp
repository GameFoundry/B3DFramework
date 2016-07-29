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
		bool gotOneMesh = false;
		UINT32 count = 0;

		for (auto& object : objects)
		{
			AABox meshBounds;
			if (calculateMeshBounds(object, meshBounds))
			{
				count++;
				if (meshBounds.getSize() == Vector3::INF)
					center += object->getWorldPosition();
				else
					center += meshBounds.getCenter();

				gotOneMesh = true;
			}
		}

		if (!gotOneMesh)
		{
			for (auto& object : objects)
			{
				if (object.isDestroyed())
					continue;

				center += object->getWorldPosition();
				count++;
				gotOneMesh = true;
			}
		}

		if (gotOneMesh)
			return center / count;

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
}