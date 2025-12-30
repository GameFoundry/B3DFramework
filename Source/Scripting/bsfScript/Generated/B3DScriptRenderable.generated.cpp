//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptRenderable.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Components/B3DRenderable.h"
#include "B3DScriptResourceManager.h"
#include "Wrappers/B3DScriptRRefBase.h"
#include "../../../Foundation/Core/Mesh/B3DMesh.h"
#include "../../../Foundation/Core/Material/B3DMaterial.h"

namespace b3d
{
	ScriptRenderable::ScriptRenderable(const TGameObjectHandle<Renderable>& nativeObject)
		:TScriptGameObjectWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptRenderable::~ScriptRenderable()
	{
		UnregisterEvents();
	}

	void ScriptRenderable::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBounds", (void*)&ScriptRenderable::InternalGetBounds);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMesh", (void*)&ScriptRenderable::InternalSetMesh);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMesh", (void*)&ScriptRenderable::InternalGetMesh);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMaterial", (void*)&ScriptRenderable::InternalSetMaterial);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMaterial", (void*)&ScriptRenderable::InternalGetMaterial);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMaterial0", (void*)&ScriptRenderable::InternalSetMaterial0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMaterials", (void*)&ScriptRenderable::InternalGetMaterials);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMaterials", (void*)&ScriptRenderable::InternalSetMaterials);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetLayer", (void*)&ScriptRenderable::InternalSetLayer);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetWriteVelocity", (void*)&ScriptRenderable::InternalSetWriteVelocity);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetWriteVelocity", (void*)&ScriptRenderable::InternalGetWriteVelocity);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetCullDistanceFactor", (void*)&ScriptRenderable::InternalSetCullDistanceFactor);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetCullDistanceFactor", (void*)&ScriptRenderable::InternalGetCullDistanceFactor);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLayer", (void*)&ScriptRenderable::InternalGetLayer);

	}

	MonoObject* ScriptRenderable::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptRenderable::InternalGetBounds(ScriptRenderable* self, TBounds<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TBounds<float> tmp__output;
		tmp__output = static_cast<Renderable*>(self->GetNativeObject())->GetBounds();

		*__output = tmp__output;
	}

	void ScriptRenderable::InternalSetMesh(ScriptRenderable* self, MonoObject* mesh)
	{
		if(!self->IsNativeObjectValid())
			return;

		TResourceHandle<Mesh> tmpmesh;
		ScriptRRefBase* scriptObjectWrappermesh;
		scriptObjectWrappermesh = ScriptRRefBase::GetScriptObjectWrapper(mesh);
		if(scriptObjectWrappermesh != nullptr)
			tmpmesh = B3DStaticResourceCast<Mesh>(scriptObjectWrappermesh->GetNativeObject());
		static_cast<Renderable*>(self->GetNativeObject())->SetMesh(tmpmesh);
	}

	MonoObject* ScriptRenderable::InternalGetMesh(ScriptRenderable* self)
	{
		TResourceHandle<Mesh> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Renderable*>(self->GetNativeObject())->GetMesh();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetScriptObject();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptRenderable::InternalSetMaterial(ScriptRenderable* self, uint32_t index, MonoObject* material)
	{
		if(!self->IsNativeObjectValid())
			return;

		TResourceHandle<Material> tmpmaterial;
		ScriptRRefBase* scriptObjectWrappermaterial;
		scriptObjectWrappermaterial = ScriptRRefBase::GetScriptObjectWrapper(material);
		if(scriptObjectWrappermaterial != nullptr)
			tmpmaterial = B3DStaticResourceCast<Material>(scriptObjectWrappermaterial->GetNativeObject());
		static_cast<Renderable*>(self->GetNativeObject())->SetMaterial(index, tmpmaterial);
	}

	MonoObject* ScriptRenderable::InternalGetMaterial(ScriptRenderable* self, uint32_t index)
	{
		TResourceHandle<Material> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Renderable*>(self->GetNativeObject())->GetMaterial(index);

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetScriptObject();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptRenderable::InternalSetMaterial0(ScriptRenderable* self, MonoObject* material)
	{
		if(!self->IsNativeObjectValid())
			return;

		TResourceHandle<Material> tmpmaterial;
		ScriptRRefBase* scriptObjectWrappermaterial;
		scriptObjectWrappermaterial = ScriptRRefBase::GetScriptObjectWrapper(material);
		if(scriptObjectWrappermaterial != nullptr)
			tmpmaterial = B3DStaticResourceCast<Material>(scriptObjectWrappermaterial->GetNativeObject());
		static_cast<Renderable*>(self->GetNativeObject())->SetMaterial(tmpmaterial);
	}

	MonoArray* ScriptRenderable::InternalGetMaterials(ScriptRenderable* self)
	{
		Vector<TResourceHandle<Material>> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<Renderable*>(self->GetNativeObject())->GetMaterials();

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptRRefBase>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			ScriptRRefBase* scriptObjectWrapper__output;
			scriptObjectWrapper__output = ScriptResourceManager::Instance().GetScriptRRef(nativeArray__output[elementIndex]);
			if(scriptObjectWrapper__output != nullptr)
				scriptArray__output.Set(elementIndex, scriptObjectWrapper__output->GetScriptObject());
			else
				scriptArray__output.Set(elementIndex, nullptr);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptRenderable::InternalSetMaterials(ScriptRenderable* self, MonoArray* materials)
	{
		if(!self->IsNativeObjectValid())
			return;

		Vector<TResourceHandle<Material>> nativeArraymaterials;
		if(materials != nullptr)
		{
			ScriptArray scriptArraymaterials(materials);
			nativeArraymaterials.resize(scriptArraymaterials.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArraymaterials.Size(); elementIndex++)
			{
				TResourceHandle<Material> arrayElementPointermaterials;
				ScriptRRefBase* scriptObjectWrappermaterials;
				scriptObjectWrappermaterials = ScriptRRefBase::GetScriptObjectWrapper(scriptArraymaterials.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrappermaterials != nullptr)
				{
					arrayElementPointermaterials = B3DStaticResourceCast<Material>(scriptObjectWrappermaterials->GetNativeObject());
					nativeArraymaterials[elementIndex] = arrayElementPointermaterials;
				}
			}
		}
		static_cast<Renderable*>(self->GetNativeObject())->SetMaterials(nativeArraymaterials);
	}

	void ScriptRenderable::InternalSetLayer(ScriptRenderable* self, uint64_t layer)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Renderable*>(self->GetNativeObject())->SetLayer(layer);
	}

	void ScriptRenderable::InternalSetWriteVelocity(ScriptRenderable* self, bool enable)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Renderable*>(self->GetNativeObject())->SetWriteVelocity(enable);
	}

	bool ScriptRenderable::InternalGetWriteVelocity(ScriptRenderable* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Renderable*>(self->GetNativeObject())->GetWriteVelocity();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderable::InternalSetCullDistanceFactor(ScriptRenderable* self, float factor)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Renderable*>(self->GetNativeObject())->SetCullDistanceFactor(factor);
	}

	float ScriptRenderable::InternalGetCullDistanceFactor(ScriptRenderable* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Renderable*>(self->GetNativeObject())->GetCullDistanceFactor();

		float __output;
		__output = tmp__output;

		return __output;
	}

	uint64_t ScriptRenderable::InternalGetLayer(ScriptRenderable* self)
	{
		uint64_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Renderable*>(self->GetNativeObject())->GetLayer();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}
}
