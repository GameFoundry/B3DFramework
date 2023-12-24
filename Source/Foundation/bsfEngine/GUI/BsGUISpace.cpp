//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUISpace.h"

#include "Reflection/BsRTTIType.h"

using namespace bs;

/** @cond RTTI */
/** @addtogroup RTTI-Impl-Engine
 *  @{
 */

namespace bs
{
	class B3D_EXPORT GUIFixedSpaceRTTI : public RTTIType<GUIFixedSpace, GUIElementBase, GUIFixedSpaceRTTI>
	{
	public:
		const String& GetRttiName()
		{
			static String name = "GUIFixedSpace";
			return name;
		}

		u32 GetRttiId() { return TID_GUIFixedSpace; }

		SPtr<IReflectable> NewRttiObject() { return nullptr; }
	};

	class B3D_EXPORT GUIFlexibleSpaceRTTI : public RTTIType<GUIFlexibleSpace, GUIElementBase, GUIFlexibleSpaceRTTI>
	{
	public:
		const String& GetRttiName()
		{
			static String name = "GUIFlexibleSpace";
			return name;
		}

		u32 GetRttiId() { return TID_GUIFlexibleSpace; }

		SPtr<IReflectable> NewRttiObject() { return nullptr; }
	};
} // namespace bs

/** @} */
/** @endcond */

GUIConstrainedSize GUIFixedSpace::CalculateConstrainedSize() const
{
	GUIConstrainedSize range;
	range.Optimal = CalculateUnconstrainedOptimalSize();
	range.Min = range.Optimal;
	range.Max = range.Optimal;

	return range;
}

GUIFixedSpace* GUIFixedSpace::Create(u32 size)
{
	return B3DNew<GUIFixedSpace>(size);
}

RTTITypeBase* GUIFixedSpace::GetRttiStatic()
{
	return GUIFixedSpaceRTTI::Instance();
}

RTTITypeBase* GUIFixedSpace::GetRtti() const
{
	return GetRttiStatic();
}

GUIConstrainedSize GUIFlexibleSpace::CalculateConstrainedSize() const
{
	GUIConstrainedSize range;
	range.Optimal = CalculateUnconstrainedOptimalSize();
	range.Min = range.Optimal;
	range.Max = range.Optimal;

	return range;
}

GUIFlexibleSpace* GUIFlexibleSpace::Create()
{
	return B3DNew<GUIFlexibleSpace>();
}

RTTITypeBase* GUIFlexibleSpace::GetRttiStatic()
{
	return GUIFlexibleSpaceRTTI::Instance();
}

RTTITypeBase* GUIFlexibleSpace::GetRtti() const
{
	return GetRttiStatic();
}
