//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once
#include "Scene/B3DComponent.h"

namespace b3d
{
	class UnitTestComponentA : public Component
	{
	public:
		HSceneObject SceneObjectReference;
		HComponent ComponentReference;
		String StringValue;

		/************************************************************************/
		/* 							COMPONENT OVERRIDES                    		*/
		/************************************************************************/

	protected:
		friend class SceneObject;

		UnitTestComponentA(const HSceneObject& parent);

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class UnitTestComponentARTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;

	protected:
		UnitTestComponentA() {} // Serialization only
	};

	class UnitTestComponentB : public Component
	{
	public:
		HSceneObject SceneObjectReference;
		String StringValue;

		/************************************************************************/
		/* 							COMPONENT OVERRIDES                    		*/
		/************************************************************************/

	protected:
		friend class SceneObject;

		UnitTestComponentB(const HSceneObject& parent);

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class UnitTestComponentBRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;

	protected:
		UnitTestComponentB() {} // Serialization only
	};

	using HUnitTestComponentA = TGameObjectHandle<UnitTestComponentA>;
	using HUnitTestComponentB = TGameObjectHandle<UnitTestComponentB>;
} // namespace b3d
