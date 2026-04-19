//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once
#include "B3DUtilityPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Serialization/B3DBinaryDelta.h"
#include "RTTI/B3DSerializedObjectRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Utility
	 *  @{
	 */

	class B3D_EXPORT SerializedTupleDeltaRTTI : public TRTTIType<SerializedTupleDelta, ISerialized, SerializedTupleDeltaRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Key, 0)
			B3D_RTTI_MEMBER_CONTAINER(Values, 1)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SerializedTupleDelta";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializedTupleDelta;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<SerializedTupleDelta>();
		}
	};

	class B3D_EXPORT SerializedTupleEntryDeltaRTTI : public TRTTIType<SerializedTupleEntryDelta, IReflectable, SerializedTupleEntryDeltaRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Index, 0)
			B3D_RTTI_MEMBER(Value, 1)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SerializedTupleEntryDelta";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializedTupleEntryDelta;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<SerializedTupleEntryDelta>();
		}
	};

	class B3D_EXPORT SerializedArrayDeltaRTTI : public TRTTIType<SerializedArrayDelta, ISerialized, SerializedArrayDeltaRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(ElementCount, 0)
			B3D_RTTI_MEMBER_CONTAINER(Entries, 1)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SerializedArrayDelta";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializedArrayDelta;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<SerializedArrayDelta>();
		}
	};

	class B3D_EXPORT SerializedArrayEntryDeltaRTTI : public TRTTIType<SerializedArrayEntryDelta, IReflectable, SerializedArrayEntryDeltaRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Index, 0)
			B3D_RTTI_MEMBER(Value, 1)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SerializedArrayEntryDelta";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializedArrayEntryDelta;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<SerializedArrayEntryDelta>();
		}
	};

	class B3D_EXPORT SerializedMapDeltaRTTI : public TRTTIType<SerializedMapDelta, ISerialized, SerializedMapDeltaRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_CONTAINER(Entries, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SerializedMapDelta";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializedMapDelta;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<SerializedMapDelta>();
		}
	};

	class B3D_EXPORT SerializedMapEntryDeltaRTTI : public TRTTIType<SerializedMapEntryDelta, IReflectable, SerializedMapEntryDeltaRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Value, 0)
			B3D_RTTI_MEMBER(IsRemoved, 1)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SerializedMapEntryDelta";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SerializedMapEntryDelta;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<SerializedMapEntryDelta>();
		}
	};
	/** @} */
	/** @endcond */
} // namespace b3d
