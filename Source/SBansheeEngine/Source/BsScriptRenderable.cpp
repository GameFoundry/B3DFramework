//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptRenderable.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "BsApplication.h"
#include "BsRenderable.h"
#include "BsScriptSceneObject.h"
#include "BsSceneObject.h"
#include "BsSceneManager.h"
#include "BsScriptMesh.h"
#include "BsScriptAnimation.h"
#include "BsScriptMaterial.h"

namespace BansheeEngine
{
	ScriptRenderable::ScriptRenderable(MonoObject* managedInstance, const HSceneObject& parentSO)
		:ScriptObject(managedInstance), mRenderable(nullptr), mLastUpdateHash(0)
	{
		mRenderable = Renderable::create();
		gSceneManager()._registerRenderable(mRenderable, parentSO);
	}

	ScriptRenderable::~ScriptRenderable()
	{ }

	void ScriptRenderable::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_Create", &ScriptRenderable::internal_Create);
		metaData.scriptClass->addInternalCall("Internal_SetAnimation", &ScriptRenderable::internal_SetAnimation);
		metaData.scriptClass->addInternalCall("Internal_UpdateTransform", &ScriptRenderable::internal_UpdateTransform);
		metaData.scriptClass->addInternalCall("Internal_SetMesh", &ScriptRenderable::internal_SetMesh);
		metaData.scriptClass->addInternalCall("Internal_GetBounds", &ScriptRenderable::internal_GetBounds);
		metaData.scriptClass->addInternalCall("Internal_GetLayers", &ScriptRenderable::internal_GetLayers);
		metaData.scriptClass->addInternalCall("Internal_SetLayers", &ScriptRenderable::internal_SetLayers);
		metaData.scriptClass->addInternalCall("Internal_SetMaterial", &ScriptRenderable::internal_SetMaterial);
		metaData.scriptClass->addInternalCall("Internal_SetMaterials", &ScriptRenderable::internal_SetMaterials);
		metaData.scriptClass->addInternalCall("Internal_OnDestroy", &ScriptRenderable::internal_OnDestroy);
	}

	void ScriptRenderable::updateTransform(const HSceneObject& parent)
	{
		UINT32 curHash = parent->getTransformHash();
		if (curHash != mLastUpdateHash)
		{
			Matrix4 transformNoScale = Matrix4::TRS(parent->getWorldPosition(), parent->getWorldRotation(), Vector3::ONE);
			mRenderable->setTransform(parent->getWorldTfrm(), transformNoScale);

			mLastUpdateHash = curHash;
		}

		if (parent->getActive() != mRenderable->getIsActive())
		{
			mRenderable->setIsActive(parent->getActive());
		}
	}

	void ScriptRenderable::internal_Create(MonoObject* instance, ScriptSceneObject* parentSO)
	{
		HSceneObject so;
		if (parentSO != nullptr)
			so = parentSO->getNativeHandle();

		new (bs_alloc<ScriptRenderable>()) ScriptRenderable(instance, so);
	}

	void ScriptRenderable::internal_SetAnimation(ScriptRenderable* thisPtr, ScriptAnimation* animation)
	{
		SPtr<Animation> anim;
		if (animation != nullptr)
			anim = animation->getInternal();

		thisPtr->getInternal()->setAnimation(anim);
	}

	void ScriptRenderable::internal_UpdateTransform(ScriptRenderable* thisPtr, ScriptSceneObject* parent)
	{
		HSceneObject parentSO = parent->getNativeSceneObject();

		thisPtr->updateTransform(parentSO);
	}

	void ScriptRenderable::internal_SetMesh(ScriptRenderable* thisPtr, ScriptMesh* mesh)
	{
		HMesh nativeMesh;
		if (mesh != nullptr)
			nativeMesh = mesh->getHandle();

		thisPtr->getInternal()->setMesh(nativeMesh);
	}

	void ScriptRenderable::internal_GetBounds(ScriptRenderable* thisPtr, ScriptSceneObject* parent, AABox* box, Sphere* sphere)
	{
		HSceneObject parentSO = parent->getNativeSceneObject();
		thisPtr->updateTransform(parentSO);

		Bounds bounds = thisPtr->getInternal()->getBounds();

		*box = bounds.getBox();
		*sphere = bounds.getSphere();
	}

	UINT64 ScriptRenderable::internal_GetLayers(ScriptRenderable* thisPtr)
	{
		return thisPtr->getInternal()->getLayer();
	}

	void ScriptRenderable::internal_SetLayers(ScriptRenderable* thisPtr, UINT64 layers)
	{
		thisPtr->getInternal()->setLayer(layers);
	}

	void ScriptRenderable::internal_SetMaterials(ScriptRenderable* thisPtr, MonoArray* materials)
	{
		if (materials != nullptr)
		{
			ScriptArray scriptMaterials(materials);

			Vector<HMaterial> nativeMaterials(scriptMaterials.size());
			for (UINT32 i = 0; i < scriptMaterials.size(); i++)
			{
				MonoObject* monoMaterial = scriptMaterials.get<MonoObject*>(i);
				ScriptMaterial* scriptMaterial = ScriptMaterial::toNative(monoMaterial);

				if (scriptMaterial != nullptr)
					nativeMaterials[i] = scriptMaterial->getHandle();
			}

			thisPtr->getInternal()->setMaterials(nativeMaterials);
		}
		else
		{
			thisPtr->getInternal()->setMaterials({});
		}
	}

	void ScriptRenderable::internal_SetMaterial(ScriptRenderable* thisPtr, ScriptMaterial* material, int index)
	{
		HMaterial nativeMaterial;
		if (material != nullptr)
			nativeMaterial = material->getHandle();

		thisPtr->getInternal()->setMaterial(index, nativeMaterial);
	}

	void ScriptRenderable::internal_OnDestroy(ScriptRenderable* thisPtr)
	{
		thisPtr->destroy();
	}

	void ScriptRenderable::destroy()
	{
		if (mRenderable->isDestroyed())
			return;

		gSceneManager()._unregisterRenderable(mRenderable);
		mRenderable->destroy();
	}

	void ScriptRenderable::_onManagedInstanceDeleted()
	{
		destroy();

		ScriptObject::_onManagedInstanceDeleted();
	}
}