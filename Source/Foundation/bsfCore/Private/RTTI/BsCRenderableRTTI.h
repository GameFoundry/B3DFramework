//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Components/BsCRenderable.h"
#include "Private/RTTI/BsGameObjectRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_CORE_EXPORT CRenderableRTTI : public RTTIType<CRenderable, Component, CRenderableRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mInternal, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "CRenderable";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_CRenderable;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<CRenderable>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
