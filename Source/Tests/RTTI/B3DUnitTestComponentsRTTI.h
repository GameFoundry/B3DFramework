//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once
#include "RTTI/B3DStringRTTI.h"
#include "Scene/B3DSceneObject.h"

namespace b3d
{
	class UnitTestComponentARTTI : public TRTTIType<UnitTestComponentA, Component, UnitTestComponentARTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(SceneObjectReference, 0)
			B3D_RTTI_MEMBER(ComponentReference, 1)
			B3D_RTTI_MEMBER(StringValue, 2)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "UnitTestComponentA";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_UnitTestComponentA;
		}

		TShared<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<UnitTestComponentA>();
		}
	};

	class UnitTestComponentBRTTI : public TRTTIType<UnitTestComponentB, Component, UnitTestComponentBRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(SceneObjectReference, 0)
			B3D_RTTI_MEMBER(StringValue, 1)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "UnitTestComponentB";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_UnitTestComponentB;
		}

		TShared<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<UnitTestComponentB>();
		}
	};
} // namespace b3d
