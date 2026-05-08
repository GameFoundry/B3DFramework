//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DStringRTTI.h"
#include "RTTI/B3DMathRTTI.h"
#include "Physics/B3DColliderShape.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	template<>
	struct RTTIPlainType<PlaneColliderShapeInformation> : RTTIPlainTypeHelper<PlaneColliderShapeInformation, TID_PlaneColliderShape, 0, 0>
	{
		template <class Processor>
		static void RTTIEnumerateFields(PlaneColliderShapeInformation& object, Processor& processor, u8 version)
		{ }
	};

	template<>
	struct RTTIPlainType<BoxColliderShapeInformation> : RTTIPlainTypeHelper<BoxColliderShapeInformation, TID_BoxColliderShape, 0, 0>
	{
		template <class Processor>
		static void RTTIEnumerateFields(BoxColliderShapeInformation& object, Processor& processor, u8 version)
		{
			processor(object.Extents);
		}
	};

	template<>
	struct RTTIPlainType<SphereColliderShapeInformation> : RTTIPlainTypeHelper<SphereColliderShapeInformation, TID_SphereColliderShape, 0, 0>
	{
		template <class Processor>
		static void RTTIEnumerateFields(SphereColliderShapeInformation& object, Processor& processor, u8 version)
		{
			processor(object.Radius);
		}
	};

	template<>
	struct RTTIPlainType<CapsuleColliderShapeInformation> : RTTIPlainTypeHelper<CapsuleColliderShapeInformation, TID_CapsuleColliderShape, 0, 0>
	{
		template <class Processor>
		static void RTTIEnumerateFields(CapsuleColliderShapeInformation& object, Processor& processor, u8 version)
		{
			processor(object.Radius);
			processor(object.HalfHeight);
		}
	};

	template<>
	struct RTTIPlainType<MeshColliderShapeInformation> : RTTIPlainTypeHelper<MeshColliderShapeInformation, TID_MeshColliderShape, 0, 0>
	{
		template <class Processor>
		static void RTTIEnumerateFields(MeshColliderShapeInformation& object, Processor& processor, u8 version)
		{ }
	};

	template <>
	struct RTTIPlainType<ColliderShapeInformation>
	{
		enum { id = TID_ColliderShapeInformation };
		enum { hasDynamicSize = 1 };

		static constexpr uint8_t kVersion = 0;

		static BitLength ToMemory(const ColliderShapeInformation& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream, compress]()
				{
					BitLength size = 0;
					size += B3DRTTIWrite(kVersion, stream);

					if(std::holds_alternative<PlaneColliderShapeInformation>(data))
					{
						auto underlyingInformation = std::get<PlaneColliderShapeInformation>(data);
						size += B3DRTTIWrite(ColliderShapeType::Plane, stream, compress);
						size += B3DRTTIWrite(underlyingInformation, stream, compress);
					}
					else if(std::holds_alternative<BoxColliderShapeInformation>(data))
					{
						auto underlyingInformation = std::get<BoxColliderShapeInformation>(data);
						size += B3DRTTIWrite(ColliderShapeType::Box, stream, compress);
						size += B3DRTTIWrite(underlyingInformation, stream, compress);
					}
					else if(std::holds_alternative<SphereColliderShapeInformation>(data))
					{
						auto underlyingInformation = std::get<SphereColliderShapeInformation>(data);
						size += B3DRTTIWrite(ColliderShapeType::Sphere, stream, compress);
						size += B3DRTTIWrite(underlyingInformation, stream, compress);
					}
					else if(std::holds_alternative<CapsuleColliderShapeInformation>(data))
					{
						auto underlyingInformation = std::get<CapsuleColliderShapeInformation>(data);
						size += B3DRTTIWrite(ColliderShapeType::Capsule, stream, compress);
						size += B3DRTTIWrite(underlyingInformation, stream, compress);
					}
					else if(std::holds_alternative<MeshColliderShapeInformation>(data))
					{
						auto underlyingInformation = std::get<MeshColliderShapeInformation>(data);
						size += B3DRTTIWrite(ColliderShapeType::Mesh, stream, compress);
						size += B3DRTTIWrite(underlyingInformation, stream, compress);
					}

					return size;
		   });
		}

		static BitLength FromMemory(ColliderShapeInformation& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			uint8_t version;
			B3DRTTIRead(version, stream, compress);
			B3D_ASSERT(version == 0);

			ColliderShapeType colliderShapeType = ColliderShapeType::Plane;
			B3DRTTIRead(colliderShapeType, stream, compress);

			switch(colliderShapeType)
			{
			case ColliderShapeType::Plane:
			{
				PlaneColliderShapeInformation underlyingShapeInformation;
				B3DRTTIRead(underlyingShapeInformation, stream, compress);

				data = underlyingShapeInformation;
				break;
			}
			case ColliderShapeType::Box:
			{
				BoxColliderShapeInformation underlyingShapeInformation;
				B3DRTTIRead(underlyingShapeInformation, stream, compress);

				data = underlyingShapeInformation;
				break;
			}
			case ColliderShapeType::Sphere:
			{
				SphereColliderShapeInformation underlyingShapeInformation;
				B3DRTTIRead(underlyingShapeInformation, stream, compress);

				data = underlyingShapeInformation;
				break;
			}
			case ColliderShapeType::Capsule:
			{
				CapsuleColliderShapeInformation underlyingShapeInformation;
				B3DRTTIRead(underlyingShapeInformation, stream, compress);

				data = underlyingShapeInformation;
				break;
			}
			case ColliderShapeType::Mesh:
			{
				MeshColliderShapeInformation underlyingShapeInformation;
				B3DRTTIRead(underlyingShapeInformation, stream, compress);

				data = underlyingShapeInformation;
				break;
			}
			}

			return size;
		}

		static BitLength GetSize(const ColliderShapeInformation& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			size += B3DRTTISize(kVersion, compress);

			if(std::holds_alternative<PlaneColliderShapeInformation>(data))
			{
				size += B3DRTTISize(ColliderShapeType::Plane, compress);
				size += B3DRTTISize(std::get<PlaneColliderShapeInformation>(data), compress);
			}
			else if(std::holds_alternative<BoxColliderShapeInformation>(data))
			{
				size += B3DRTTISize(ColliderShapeType::Box, compress);
				size += B3DRTTISize(std::get<BoxColliderShapeInformation>(data), compress);
			}
			else if(std::holds_alternative<SphereColliderShapeInformation>(data))
			{
				size += B3DRTTISize(ColliderShapeType::Sphere, compress);
				size += B3DRTTISize(std::get<SphereColliderShapeInformation>(data), compress);
			}
			else if(std::holds_alternative<CapsuleColliderShapeInformation>(data))
			{
				size += B3DRTTISize(ColliderShapeType::Capsule, compress);
				size += B3DRTTISize(std::get<CapsuleColliderShapeInformation>(data), compress);
			}
			else if(std::holds_alternative<MeshColliderShapeInformation>(data))
			{
				size += B3DRTTISize(ColliderShapeType::Mesh, compress);
				size += B3DRTTISize(std::get<MeshColliderShapeInformation>(data), compress);
			}

			B3DRTTIAddHeaderSize(size, compress);
			return size;
		}
	};

	class B3D_EXPORT ColliderShapeRTTI : public TRTTIType<ColliderShape, IReflectable, ColliderShapeRTTI>
	{
		HPhysicsMesh mPhysicsMesh;

		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mPosition, 0)
			B3D_RTTI_MEMBER(mRotation, 1)
			B3D_RTTI_MEMBER(mScale, 2)
			B3D_RTTI_MEMBER(mMaterial, 3)
			B3D_RTTI_MEMBER(mShapeInformation, 4)
			B3D_RTTI_MEMBER(mLayer, 5)
			B3D_RTTI_MEMBER(mMass, 6)
			B3D_RTTI_MEMBER(mRestOffset, 7)
			B3D_RTTI_MEMBER(mContactOffset, 8)
			B3D_RTTI_MEMBER(mCollisionReportMode, 9)
			B3D_RTTI_MEMBER(mContinuousCollisionDetectionEnabled, 10)
			B3D_RTTI_MEMBER(mIsTrigger, 11)
			B3D_RTTI_GENERATED_MEMBER(mPhysicsMesh, 12)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ColliderShape";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ColliderShape;
		}

		TShared<IReflectable> NewRttiObject() override
		{
			return ColliderShape::CreateEmpty();
		}

	protected:
		void OnOperationStarted(ColliderShape& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::ReadBit))
			{
				if(object.GetType() == ColliderShapeType::Mesh)
					mPhysicsMesh = object.GetMeshShapeInformation().Mesh;
			}
		}

		void OnOperationEnded(ColliderShape& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				if(object.GetType() == ColliderShapeType::Mesh)
					object.GetMeshShapeInformation().Mesh = mPhysicsMesh;
			}
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
