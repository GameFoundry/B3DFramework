//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptResourceManager.h"
#include "BsMonoManager.h"
#include "BsMonoAssembly.h"
#include "BsMonoClass.h"
#include "BsResources.h"
#include "BsScriptTexture2D.h"
#include "BsScriptTexture3D.h"
#include "BsScriptTextureCube.h"
#include "BsScriptSpriteTexture.h"
#include "BsScriptPlainText.h"
#include "BsScriptScriptCode.h"
#include "BsScriptShader.h"
#include "BsScriptShaderInclude.h"
#include "BsScriptMaterial.h"
#include "BsScriptMesh.h"
#include "BsScriptFont.h"
#include "BsScriptPrefab.h"
#include "BsScriptStringTable.h"
#include "BsScriptGUISkin.h"
#include "BsScriptPhysicsMaterial.h"
#include "BsScriptPhysicsMesh.h"
#include "BsScriptAudioClip.h"
#include "BsScriptAnimationClip.h"
#include "BsScriptManagedResource.h"
#include "BsScriptAssemblyManager.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	ScriptResourceManager::ScriptResourceManager()
	{
		mResourceDestroyedConn = gResources().onResourceDestroyed.connect(std::bind(&ScriptResourceManager::onResourceDestroyed, this, _1));
	}

	ScriptResourceManager::~ScriptResourceManager()
	{
		mResourceDestroyedConn.disconnect();
	}

	template<class RetType, class InType>
	void ScriptResourceManager::createScriptResource(MonoObject* instance, const ResourceHandle<InType>& resourceHandle, RetType** out)
	{
		const String& uuid = resourceHandle.getUUID();
#if BS_DEBUG_MODE
		_throwExceptionIfInvalidOrDuplicate(uuid);
#endif

		RetType* scriptResource = new (bs_alloc<RetType>()) RetType(instance, resourceHandle);
		mScriptResources[uuid] = scriptResource;

		*out = scriptResource;
	}

	template<class RetType, class InType>
	void ScriptResourceManager::getScriptResource(const ResourceHandle<InType>& resourceHandle, RetType** out, bool create)
	{
		String uuid = resourceHandle.getUUID();

		if (!uuid.empty())
		{
			*out = static_cast<RetType*>(getScriptResource(uuid));

			if (*out == nullptr && create)
				createScriptResource(resourceHandle, out);
		}
		else
			*out = nullptr;
	}

	namespace Detail
	{
		template<class RetType, class InType>
		void ScriptResourceManager_createScriptResource(ScriptResourceManager* thisPtr, const ResourceHandle<InType>& resourceHandle, RetType** out)
		{
			MonoObject* monoInstance = RetType::createInstance();

			thisPtr->createScriptResource(monoInstance, resourceHandle, out);
		}

		template<>
		void ScriptResourceManager_createScriptResource(ScriptResourceManager* thisPtr, const ResourceHandle<StringTable>& resourceHandle, ScriptStringTable** out)
		{
			MonoClass* resourceClass = ScriptStringTable::getMetaData()->scriptClass;

			bool dummy = true;
			void* params = { &dummy };

			MonoObject* monoInstance = resourceClass->createInstance(&params, 1);
			thisPtr->createScriptResource(monoInstance, resourceHandle, out);
		}

		template<>
		void ScriptResourceManager_createScriptResource(ScriptResourceManager* thisPtr, const ResourceHandle<Texture>& resourceHandle, ScriptTextureBase** out)
		{
			TextureType type = resourceHandle->getProperties().getTextureType();

			if (type == TEX_TYPE_3D)
				return ScriptResourceManager_createScriptResource(thisPtr, resourceHandle, (ScriptTexture3D**)out);
			else if (type == TEX_TYPE_CUBE_MAP)
				return ScriptResourceManager_createScriptResource(thisPtr, resourceHandle, (ScriptTextureCube**)out);
			else
				return ScriptResourceManager_createScriptResource(thisPtr, resourceHandle, (ScriptTexture2D**)out);
		}

		template<>
		void ScriptResourceManager_createScriptResource(ScriptResourceManager* thisPtr, const HResource& resourceHandle, ScriptResourceBase** out)
		{
#if BS_DEBUG_MODE
			thisPtr->_throwExceptionIfInvalidOrDuplicate(resourceHandle.getUUID());
#endif

			UINT32 resTypeID = resourceHandle->getTypeId();

			switch (resTypeID)
			{
			case TID_Texture:
			{
				HTexture texture = static_resource_cast<Texture>(resourceHandle);
				TextureType type = texture->getProperties().getTextureType();

				if (type == TEX_TYPE_3D)
					return ScriptResourceManager_createScriptResource(thisPtr, texture, (ScriptTexture3D**)out);
				else if (type == TEX_TYPE_CUBE_MAP)
					return ScriptResourceManager_createScriptResource(thisPtr, texture, (ScriptTextureCube**)out);
				else
					return ScriptResourceManager_createScriptResource(thisPtr, texture, (ScriptTexture2D**)out);
			}
			case TID_SpriteTexture:
				return ScriptResourceManager_createScriptResource(thisPtr, static_resource_cast<SpriteTexture>(resourceHandle), (ScriptSpriteTexture**)out);
			case TID_Font:
				return ScriptResourceManager_createScriptResource(thisPtr, static_resource_cast<Font>(resourceHandle), (ScriptFont**)out);
			case TID_PlainText:
				return ScriptResourceManager_createScriptResource(thisPtr, static_resource_cast<PlainText>(resourceHandle), (ScriptPlainText**)out);
			case TID_ScriptCode:
				return ScriptResourceManager_createScriptResource(thisPtr, static_resource_cast<ScriptCode>(resourceHandle), (ScriptScriptCode**)out);
			case TID_Shader:
				return ScriptResourceManager_createScriptResource(thisPtr, static_resource_cast<Shader>(resourceHandle), (ScriptShader**)out);
			case TID_ShaderInclude:
				return ScriptResourceManager_createScriptResource(thisPtr, static_resource_cast<ShaderInclude>(resourceHandle), (ScriptShaderInclude**)out);
			case TID_Prefab:
				return ScriptResourceManager_createScriptResource(thisPtr, static_resource_cast<Prefab>(resourceHandle), (ScriptPrefab**)out);
			case TID_StringTable:
				return ScriptResourceManager_createScriptResource(thisPtr, static_resource_cast<StringTable>(resourceHandle), (ScriptStringTable**)out);
			case TID_Material:
				return ScriptResourceManager_createScriptResource(thisPtr, static_resource_cast<Material>(resourceHandle), (ScriptMaterial**)out);
			case TID_Mesh:
				return ScriptResourceManager_createScriptResource(thisPtr, static_resource_cast<Mesh>(resourceHandle), (ScriptMesh**)out);
			case TID_GUISkin:
				return ScriptResourceManager_createScriptResource(thisPtr, static_resource_cast<GUISkin>(resourceHandle), (ScriptGUISkin**)out);
			case TID_PhysicsMaterial:
				return ScriptResourceManager_createScriptResource(thisPtr, static_resource_cast<PhysicsMaterial>(resourceHandle), (ScriptPhysicsMaterial**)out);
			case TID_PhysicsMesh:
				return ScriptResourceManager_createScriptResource(thisPtr, static_resource_cast<PhysicsMesh>(resourceHandle), (ScriptPhysicsMesh**)out);
			case TID_AudioClip:
				return ScriptResourceManager_createScriptResource(thisPtr, static_resource_cast<AudioClip>(resourceHandle), (ScriptAudioClip**)out);
			case TID_AnimationClip:
				return ScriptResourceManager_createScriptResource(thisPtr, static_resource_cast<AnimationClip>(resourceHandle), (ScriptAnimationClip**)out);
			case TID_ManagedResource:
				BS_EXCEPT(InternalErrorException, "Managed resources must have a managed instance by default, this call is invalid.")
					break;
			default:
				BS_EXCEPT(NotImplementedException, "Attempting to load a resource type that is not supported. Type ID: " + toString(resTypeID));
				break;
			}
		}

		template void ScriptResourceManager_createScriptResource(ScriptResourceManager* thisPtr, const ResourceHandle<Texture>&, ScriptTexture2D**);
		template void ScriptResourceManager_createScriptResource(ScriptResourceManager* thisPtr, const ResourceHandle<Texture>&, ScriptTexture3D**);
		template void ScriptResourceManager_createScriptResource(ScriptResourceManager* thisPtr, const ResourceHandle<Texture>&, ScriptTextureCube**);
		template void ScriptResourceManager_createScriptResource(ScriptResourceManager* thisPtr, const ResourceHandle<Texture>&, ScriptTextureBase**);
		template void ScriptResourceManager_createScriptResource(ScriptResourceManager* thisPtr, const ResourceHandle<SpriteTexture>&, ScriptSpriteTexture**);
		template void ScriptResourceManager_createScriptResource(ScriptResourceManager* thisPtr, const ResourceHandle<Mesh>&, ScriptMesh**);
		template void ScriptResourceManager_createScriptResource(ScriptResourceManager* thisPtr, const ResourceHandle<Material>&, ScriptMaterial**);
		template void ScriptResourceManager_createScriptResource(ScriptResourceManager* thisPtr, const ResourceHandle<Shader>&, ScriptShader**);
		template void ScriptResourceManager_createScriptResource(ScriptResourceManager* thisPtr, const ResourceHandle<ShaderInclude>&, ScriptShaderInclude**);
		template void ScriptResourceManager_createScriptResource(ScriptResourceManager* thisPtr, const ResourceHandle<Prefab>&, ScriptPrefab**);
		template void ScriptResourceManager_createScriptResource(ScriptResourceManager* thisPtr, const ResourceHandle<Font>&, ScriptFont**);
		template void ScriptResourceManager_createScriptResource(ScriptResourceManager* thisPtr, const ResourceHandle<PlainText>&, ScriptPlainText**);
		template void ScriptResourceManager_createScriptResource(ScriptResourceManager* thisPtr, const ResourceHandle<ScriptCode>&, ScriptScriptCode**);
		template void ScriptResourceManager_createScriptResource(ScriptResourceManager* thisPtr, const ResourceHandle<StringTable>&, ScriptStringTable**);
		template void ScriptResourceManager_createScriptResource(ScriptResourceManager* thisPtr, const ResourceHandle<GUISkin>&, ScriptGUISkin**);
		template void ScriptResourceManager_createScriptResource(ScriptResourceManager* thisPtr, const ResourceHandle<PhysicsMesh>&, ScriptPhysicsMesh**);
		template void ScriptResourceManager_createScriptResource(ScriptResourceManager* thisPtr, const ResourceHandle<PhysicsMaterial>&, ScriptPhysicsMaterial**);
		template void ScriptResourceManager_createScriptResource(ScriptResourceManager* thisPtr, const ResourceHandle<AudioClip>&, ScriptAudioClip**);
		template void ScriptResourceManager_createScriptResource(ScriptResourceManager* thisPtr, const ResourceHandle<AnimationClip>&, ScriptAnimationClip**);
	}

	template BS_SCR_BE_EXPORT void ScriptResourceManager::createScriptResource(MonoObject*, const ResourceHandle<Texture>&, ScriptTexture2D**);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::createScriptResource(MonoObject*, const ResourceHandle<Texture>&, ScriptTexture3D**);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::createScriptResource(MonoObject*, const ResourceHandle<Texture>&, ScriptTextureCube**);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::createScriptResource(MonoObject*, const ResourceHandle<SpriteTexture>&, ScriptSpriteTexture**);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::createScriptResource(MonoObject*, const ResourceHandle<Mesh>&, ScriptMesh**);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::createScriptResource(MonoObject*, const ResourceHandle<Material>&, ScriptMaterial**);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::createScriptResource(MonoObject*, const ResourceHandle<Shader>&, ScriptShader**);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::createScriptResource(MonoObject*, const ResourceHandle<ShaderInclude>&, ScriptShaderInclude**);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::createScriptResource(MonoObject*, const ResourceHandle<Prefab>&, ScriptPrefab**);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::createScriptResource(MonoObject*, const ResourceHandle<Font>&, ScriptFont**);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::createScriptResource(MonoObject*, const ResourceHandle<PlainText>&, ScriptPlainText**);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::createScriptResource(MonoObject*, const ResourceHandle<ScriptCode>&, ScriptScriptCode**);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::createScriptResource(MonoObject*, const ResourceHandle<StringTable>&, ScriptStringTable**);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::createScriptResource(MonoObject*, const ResourceHandle<GUISkin>&, ScriptGUISkin**);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::createScriptResource(MonoObject*, const ResourceHandle<PhysicsMesh>&, ScriptPhysicsMesh**);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::createScriptResource(MonoObject*, const ResourceHandle<PhysicsMaterial>&, ScriptPhysicsMaterial**);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::createScriptResource(MonoObject*, const ResourceHandle<AudioClip>&, ScriptAudioClip**);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::createScriptResource(MonoObject*, const ResourceHandle<AnimationClip>&, ScriptAnimationClip**);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::createScriptResource(MonoObject*, const ResourceHandle<ManagedResource>&, ScriptManagedResource**);

	template BS_SCR_BE_EXPORT void ScriptResourceManager::getScriptResource(const ResourceHandle<Texture>& resourceHandle, ScriptTexture2D** out, bool create);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::getScriptResource(const ResourceHandle<Texture>& resourceHandle, ScriptTexture3D** out, bool create);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::getScriptResource(const ResourceHandle<Texture>& resourceHandle, ScriptTextureCube** out, bool create);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::getScriptResource(const ResourceHandle<Texture>& resourceHandle, ScriptTextureBase** out, bool create);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::getScriptResource(const ResourceHandle<SpriteTexture>& resourceHandle, ScriptSpriteTexture** out, bool create);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::getScriptResource(const ResourceHandle<Mesh>& resourceHandle, ScriptMesh** out, bool create);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::getScriptResource(const ResourceHandle<Material>& resourceHandle, ScriptMaterial** out, bool create);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::getScriptResource(const ResourceHandle<Shader>& resourceHandle, ScriptShader** out, bool create);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::getScriptResource(const ResourceHandle<ShaderInclude>& resourceHandle, ScriptShaderInclude** out, bool create);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::getScriptResource(const ResourceHandle<Prefab>& resourceHandle, ScriptPrefab** out, bool create);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::getScriptResource(const ResourceHandle<Font>& resourceHandle, ScriptFont** out, bool create);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::getScriptResource(const ResourceHandle<PlainText>& resourceHandle, ScriptPlainText** out, bool create);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::getScriptResource(const ResourceHandle<ScriptCode>& resourceHandle, ScriptScriptCode** out, bool create);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::getScriptResource(const ResourceHandle<StringTable>& resourceHandle, ScriptStringTable** out, bool create);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::getScriptResource(const ResourceHandle<GUISkin>& resourceHandle, ScriptGUISkin** out, bool create);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::getScriptResource(const ResourceHandle<PhysicsMesh>& resourceHandle, ScriptPhysicsMesh** out, bool create);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::getScriptResource(const ResourceHandle<PhysicsMaterial>& resourceHandle, ScriptPhysicsMaterial** out, bool create);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::getScriptResource(const ResourceHandle<AudioClip>& resourceHandle, ScriptAudioClip** out, bool create);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::getScriptResource(const ResourceHandle<AnimationClip>& resourceHandle, ScriptAnimationClip** out, bool create);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::getScriptResource(const ResourceHandle<ManagedResource>& resourceHandle, ScriptManagedResource** out, bool create);
	template BS_SCR_BE_EXPORT void ScriptResourceManager::getScriptResource(const ResourceHandle<Resource>& resourceHandle, ScriptResourceBase** out, bool create);

	ScriptResourceBase* ScriptResourceManager::getScriptResource(const String& uuid)
	{
		if (uuid == "")
			return nullptr;

		auto findIter = mScriptResources.find(uuid);
		if(findIter != mScriptResources.end())
			return findIter->second;

		return nullptr;
	}

	void ScriptResourceManager::destroyScriptResource(ScriptResourceBase* resource)
	{
		HResource resourceHandle = resource->getGenericHandle();
		const String& uuid = resourceHandle.getUUID();

		if(uuid == "")
			BS_EXCEPT(InvalidParametersException, "Provided resource handle has an undefined resource UUID.");

		(resource)->~ScriptResourceBase();
		MemoryAllocator<GenAlloc>::free(resource);

		mScriptResources.erase(uuid);
	}

	void ScriptResourceManager::onResourceDestroyed(const String& UUID)
	{
		auto findIter = mScriptResources.find(UUID);
		if (findIter != mScriptResources.end())
		{
			findIter->second->notifyResourceDestroyed();
			mScriptResources.erase(findIter);
		}
	}

	void ScriptResourceManager::_throwExceptionIfInvalidOrDuplicate(const String& uuid) const
	{
		if(uuid == "")
			BS_EXCEPT(InvalidParametersException, "Provided resource handle has an undefined resource UUID.");

		auto findIter = mScriptResources.find(uuid);
		if(findIter != mScriptResources.end())
		{
			BS_EXCEPT(InvalidStateException, "Provided resource handle already has a script resource. \
											 Retrieve the existing instance instead of creating a new one.");
		}
	}
}