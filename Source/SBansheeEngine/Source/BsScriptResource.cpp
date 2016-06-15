//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptResource.h"
#include "BsScriptResourceManager.h"
#include "BsResource.h"
#include "BsMonoUtil.h"
#include "BsScriptTexture2D.h"
#include "BsScriptTexture3D.h"
#include "BsScriptTextureCube.h"
#include "BsScriptSpriteTexture.h"
#include "BsScriptMaterial.h"
#include "BsScriptMesh.h"
#include "BsScriptFont.h"
#include "BsScriptShader.h"
#include "BsScriptShaderInclude.h"
#include "BsScriptPlainText.h"
#include "BsScriptScriptCode.h"
#include "BsScriptStringTable.h"
#include "BsScriptGUISkin.h"
#include "BsScriptPhysicsMesh.h"
#include "BsScriptPhysicsMaterial.h"
#include "BsScriptAudioClip.h"
#include "BsScriptPrefab.h"
#include "BsScriptAnimationClip.h"

namespace BansheeEngine
{
	ScriptResourceBase::ScriptResourceBase(MonoObject* instance)
		:PersistentScriptObjectBase(instance), mRefreshInProgress(false)
	{ }

	ScriptObjectBackup ScriptResourceBase::beginRefresh()
	{
		mRefreshInProgress = true;

		return PersistentScriptObjectBase::beginRefresh();
	}

	void ScriptResourceBase::endRefresh(const ScriptObjectBackup& backupData)
	{
		mRefreshInProgress = false;

		PersistentScriptObjectBase::endRefresh(backupData);
	}

	void ScriptResourceBase::destroy()
	{
		mManagedInstance = nullptr;

		if (!mRefreshInProgress)
			ScriptResourceManager::instance().destroyScriptResource(this);
	}

	void ScriptResource::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetName", &ScriptResource::internal_getName);
		metaData.scriptClass->addInternalCall("Internal_GetUUID", &ScriptResource::internal_getUUID);
		metaData.scriptClass->addInternalCall("Internal_Release", &ScriptResource::internal_release);
	}

	MonoClass* ScriptResource::getClassFromTypeId(UINT32 typeId)
	{
		switch (typeId)
		{
		case TID_Texture:
			// TODO - Need to distinguish from 2D/3D/Cube
			return ScriptTexture2D::getMetaData()->scriptClass;
		case TID_SpriteTexture:
			return ScriptSpriteTexture::getMetaData()->scriptClass;
		case TID_Font:
			return ScriptFont::getMetaData()->scriptClass;
		case TID_PlainText:
			return ScriptPlainText::getMetaData()->scriptClass;
		case TID_ScriptCode:
			return ScriptScriptCode::getMetaData()->scriptClass;
		case TID_Shader:
			return ScriptShader::getMetaData()->scriptClass;
		case TID_ShaderInclude:
			return ScriptShaderInclude::getMetaData()->scriptClass;
		case TID_Material:
			return ScriptMaterial::getMetaData()->scriptClass;
		case TID_Mesh:
			return ScriptMesh::getMetaData()->scriptClass;
		case TID_Prefab:
			return ScriptPrefab::getMetaData()->scriptClass;
		case TID_StringTable:
			return ScriptStringTable::getMetaData()->scriptClass;
		case TID_GUISkin:
			return ScriptGUISkin::getMetaData()->scriptClass;
		case TID_PhysicsMaterial:
			return ScriptPhysicsMaterial::getMetaData()->scriptClass;
		case TID_PhysicsMesh:
			return ScriptPhysicsMesh::getMetaData()->scriptClass;
		case TID_AudioClip:
			return ScriptAudioClip::getMetaData()->scriptClass;
		case TID_AnimationClip:
			return ScriptAnimationClip::getMetaData()->scriptClass;
		}

		return nullptr;
	}

	ScriptResourceType ScriptResource::getTypeFromTypeId(UINT32 typeId)
	{
		switch (typeId)
		{
		case TID_Texture:
			return ScriptResourceType::Texture;
		case TID_SpriteTexture:
			return ScriptResourceType::SpriteTexture;
		case TID_Mesh:
			return ScriptResourceType::Mesh;
		case TID_Shader:
			return ScriptResourceType::Shader;
		case TID_ShaderInclude:
			return ScriptResourceType::ShaderInclude;
		case TID_Material:
			return ScriptResourceType::Material;
		case TID_Font:
			return ScriptResourceType::Font;
		case TID_Prefab:
			return ScriptResourceType::Prefab;
		case TID_StringTable:
			return ScriptResourceType::StringTable;
		case TID_PlainText:
			return ScriptResourceType::PlainText;
		case TID_ScriptCode:
			return ScriptResourceType::ScriptCode;
		case TID_GUISkin:
			return ScriptResourceType::GUISkin;
		case TID_PhysicsMaterial:
			return ScriptResourceType::PhysicsMaterial;
		case TID_PhysicsMesh:
			return ScriptResourceType::PhysicsMesh;
		case TID_AudioClip:
			return ScriptResourceType::AudioClip;
		case TID_AnimationClip:
			return ScriptResourceType::AnimationClip;
		}

		return ScriptResourceType::Undefined;
	}

	UINT32 ScriptResource::getTypeIdFromType(ScriptResourceType type)
	{
		switch (type)
		{
		case ScriptResourceType::Texture:
			return TID_Texture;
		case ScriptResourceType::SpriteTexture:
			return TID_SpriteTexture;
		case ScriptResourceType::Mesh:
			return TID_Mesh;
		case ScriptResourceType::Shader:
			return TID_Shader;
		case ScriptResourceType::ShaderInclude:
			return TID_ShaderInclude;
		case ScriptResourceType::Font:
			return TID_Font;
		case ScriptResourceType::Material:
			return TID_Material;
		case ScriptResourceType::Prefab:
			return TID_Prefab;
		case ScriptResourceType::StringTable:
			return TID_StringTable;
		case ScriptResourceType::PlainText:
			return TID_PlainText;
		case ScriptResourceType::ScriptCode:
			return TID_ScriptCode;
		case ScriptResourceType::GUISkin:
			return TID_GUISkin;
		case ScriptResourceType::PhysicsMaterial:
			return TID_PhysicsMaterial;
		case ScriptResourceType::PhysicsMesh:
			return TID_PhysicsMesh;
		case ScriptResourceType::AudioClip:
			return TID_AudioClip;
		case ScriptResourceType::AnimationClip:
			return TID_AnimationClip;
		default:
			break;
		}

		return 0;
	}

	MonoString* ScriptResource::internal_getName(ScriptResourceBase* nativeInstance)
	{
		return MonoUtil::wstringToMono(nativeInstance->getGenericHandle()->getName());
	}

	MonoString* ScriptResource::internal_getUUID(ScriptResourceBase* nativeInstance)
	{
		return MonoUtil::stringToMono(nativeInstance->getGenericHandle().getUUID());
	}

	void ScriptResource::internal_release(ScriptResourceBase* nativeInstance)
	{
		nativeInstance->getGenericHandle().release();
	}
}