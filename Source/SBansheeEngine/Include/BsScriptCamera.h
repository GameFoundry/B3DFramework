//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsVector2.h"
#include "BsVector3.h"
#include "BsVector2I.h"
#include "BsRay.h"
#include "BsDegree.h"
#include "BsMatrix4.h"
#include "BsRect2.h"
#include "BsColor.h"
#include "BsPostProcessSettings.h"

namespace BansheeEngine
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for Camera. */
	class BS_SCR_BE_EXPORT ScriptCamera : public ScriptObject<ScriptCamera>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "NativeCamera")

		/**	Returns the wrapped native Camera object. */
		SPtr<Camera> getInternal() const { return mCamera; }

	private:
		ScriptCamera(MonoObject* managedInstance, const HSceneObject& parentSO);
		~ScriptCamera();

		/**	Updates the internal camera handler from the transform of the provided scene object. */
		void updateView(const HSceneObject& parent);

		/**	Destroys the internal camera handler object. */
		void destroy();

		/** @copydoc ScriptObject::_onManagedInstanceDeleted */
		void _onManagedInstanceDeleted() override;

		SPtr<Camera> mCamera;
		UINT32 mLastUpdateHash;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_Create(MonoObject* managedInstance, ScriptSceneObject* parentSO);

		static float internal_GetAspect(ScriptCamera* instance);
		static void internal_SetAspect(ScriptCamera* instance, float value);
		static float internal_GetNearClip(ScriptCamera* instance);
		static void internal_SetNearClip(ScriptCamera* instance, float value);

		static float internal_GetFarClip(ScriptCamera* instance);
		static void internal_SetFarClip(ScriptCamera* instance, float value);

		static void internal_GetFieldOfView(ScriptCamera* instance, Degree* value);
		static void internal_SetFieldOfView(ScriptCamera* instance, Degree* value);

		static void internal_GetViewportRect(ScriptCamera* instance, Rect2* value);
		static void internal_SetViewportRect(ScriptCamera* instance, Rect2* value);

		static UINT32 internal_GetProjectionType(ScriptCamera* instance);
		static void internal_SetProjectionType(ScriptCamera* instance, UINT32 value);

		static float internal_GetOrthographicHeight(ScriptCamera* instance);
		static void internal_SetOrthographicHeight(ScriptCamera* instance, float value);

		static float internal_GetOrthographicWidth(ScriptCamera* instance);

		static void internal_GetClearColor(ScriptCamera* instance, Color* value);
		static void internal_SetClearColor(ScriptCamera* instance, Color* value);

		static float internal_GetDepthClearValue(ScriptCamera* instance);
		static void internal_SetDepthClearValue(ScriptCamera* instance, float value);

		static UINT16 internal_GetStencilClearValue(ScriptCamera* instance);
		static void internal_SetStencilClearValue(ScriptCamera* instance, UINT16 value);

		static UINT32 internal_GetClearFlags(ScriptCamera* instance);
		static void internal_SetClearFlags(ScriptCamera* instance, UINT32 value);

		static int internal_GetPriority(ScriptCamera* instance);
		static void internal_SetPriority(ScriptCamera* instance, int value);

		static bool internal_GetHDR(ScriptCamera* instance);
		static void internal_SetHDR(ScriptCamera* instance, bool value);

		static MonoObject* internal_GetPostProcessSettings(ScriptCamera* instance);
		static void internal_SetPostProcessSettings(ScriptCamera* instance, MonoObject* value);

		static UINT64 internal_GetLayers(ScriptCamera* instance);
		static void internal_SetLayers(ScriptCamera* instance, UINT64 value);

		static void internal_GetProjMatrix(ScriptCamera* instance, Matrix4* value);
		static void internal_GetProjMatrixInv(ScriptCamera* instance, Matrix4* value);

		static void internal_GetViewMatrix(ScriptCamera* instance, Matrix4* value);
		static void internal_GetViewMatrixInv(ScriptCamera* instance, Matrix4* value);

		static int internal_GetWidthPixels(ScriptCamera* instance);
		static int internal_GetHeightPixels(ScriptCamera* instance);

		static void internal_WorldToViewport(ScriptCamera* instance, Vector3* value, Vector2I* output);
		static void internal_WorldToNDC(ScriptCamera* instance, Vector3* value, Vector2* output);
		static void internal_WorldToView(ScriptCamera* instance, Vector3* value, Vector3* output);

		static void internal_ScreenToViewport(ScriptCamera* instance, Vector2I* value, Vector2I* output);

		static void internal_ViewportToWorld(ScriptCamera* instance, Vector2I* value, float depth, Vector3* output);
		static void internal_ViewportToView(ScriptCamera* instance, Vector2I* value, float depth, Vector3* output);
		static void internal_ViewportToNDC(ScriptCamera* instance, Vector2I* value, Vector2* output);

		static void internal_ViewToWorld(ScriptCamera* instance, Vector3* value, Vector3* output);
		static void internal_ViewToViewport(ScriptCamera* instance, Vector3* value, Vector2I* output);
		static void internal_ViewToNDC(ScriptCamera* instance, Vector3* value, Vector2* output);

		static void internal_NDCToWorld(ScriptCamera* instance, Vector2* value, float depth, Vector3* output);
		static void internal_NDCToView(ScriptCamera* instance, Vector2* value, float depth, Vector3* output);
		static void internal_NDCToViewport(ScriptCamera* instance, Vector2* value, Vector2I* output);

		static void internal_ViewportToWorldRay(ScriptCamera* instance, Vector2I* value, Ray* output);
		static void internal_ScreenToWorldRay(ScriptCamera* instance, Vector2I* value, Ray* output);
		
		static void internal_ProjectPoint(ScriptCamera* instance, Vector3* value, Vector3* output);
		static void internal_UnprojectPoint(ScriptCamera* instance, Vector3* value, Vector3* output);

		static void internal_SetRenderTarget(ScriptCamera* instance, ScriptRenderTarget* target);

		static bool internal_GetMain(ScriptCamera* instance);
		static void internal_SetMain(ScriptCamera* instance, bool main);

		static void internal_UpdateView(ScriptCamera* instance, ScriptSceneObject* parent);
		static void internal_OnDestroy(ScriptCamera* instance);
	};

	/** @} */
}