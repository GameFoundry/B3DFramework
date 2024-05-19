//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Particles/BsParticleSystem.h"
#include "Particles/BsParticleEvolver.h"
#include "RTTI/BsColorGradientRTTI.h"
#include "RTTI/BsMathRTTI.h"
#include "Private/RTTI/BsParticleDistributionRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT ParticleEmitterConeShapeRTTI : public RTTIType<ParticleEmitterConeShape, IReflectable, ParticleEmitterConeShapeRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN_NAMED(type, mInfo.Type, 0)
			B3D_RTTI_MEMBER_PLAIN_NAMED(radius, mInfo.Radius, 1)
			B3D_RTTI_MEMBER_PLAIN_NAMED(angle, mInfo.Angle, 2)
			B3D_RTTI_MEMBER_PLAIN_NAMED(length, mInfo.Length, 3)
			B3D_RTTI_MEMBER_PLAIN_NAMED(thickness, mInfo.Thickness, 4)
			B3D_RTTI_MEMBER_PLAIN_NAMED(arc, mInfo.Arc, 5)
			B3D_RTTI_MEMBER_PLAIN_NAMED(modeType, mInfo.Mode.Type, 6)
			B3D_RTTI_MEMBER_PLAIN_NAMED(modeInterval, mInfo.Mode.Interval, 7)
			B3D_RTTI_MEMBER_PLAIN_NAMED(modeSpeed, mInfo.Mode.Speed, 8)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleEmitterConeShape";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ParticleEmitterConeShape;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ParticleEmitterConeShape>();
		}
	};

	class B3D_CORE_EXPORT ParticleEmitterSphereShapeRTTI : public RTTIType<ParticleEmitterSphereShape, IReflectable, ParticleEmitterSphereShapeRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN_NAMED(radius, mInfo.Radius, 0)
			B3D_RTTI_MEMBER_PLAIN_NAMED(thickness, mInfo.Thickness, 1)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleEmitterSphereShape";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ParticleEmitterSphereShape;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ParticleEmitterSphereShape>();
		}
	};

	class B3D_CORE_EXPORT ParticleEmitterHemisphereShapeRTTI : public RTTIType<ParticleEmitterHemisphereShape, IReflectable, ParticleEmitterHemisphereShapeRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN_NAMED(radius, mInfo.Radius, 0)
			B3D_RTTI_MEMBER_PLAIN_NAMED(thickness, mInfo.Thickness, 1)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleEmitterHemisphereShape";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ParticleEmitterHemisphereShape;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ParticleEmitterHemisphereShape>();
		}
	};

	class B3D_CORE_EXPORT ParticleEmitterBoxShapeRTTI : public RTTIType<ParticleEmitterBoxShape, IReflectable, ParticleEmitterBoxShapeRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN_NAMED(type, mInfo.Type, 0)
			B3D_RTTI_MEMBER_PLAIN_NAMED(extents, mInfo.Extents, 1)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleEmitterBoxShape";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ParticleEmitterBoxShape;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ParticleEmitterBoxShape>();
		}
	};

	class B3D_CORE_EXPORT ParticleEmitterLineShapeRTTI : public RTTIType<ParticleEmitterLineShape, IReflectable, ParticleEmitterLineShapeRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN_NAMED(length, mInfo.Length, 0)
			B3D_RTTI_MEMBER_PLAIN_NAMED(modeType, mInfo.Mode.Type, 1)
			B3D_RTTI_MEMBER_PLAIN_NAMED(modeInterval, mInfo.Mode.Interval, 2)
			B3D_RTTI_MEMBER_PLAIN_NAMED(modeSpeed, mInfo.Mode.Speed, 3)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleEmitterLineShape";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ParticleEmitterLineShape;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ParticleEmitterLineShape>();
		}
	};

	class B3D_CORE_EXPORT ParticleEmitterCircleShapeRTTI : public RTTIType<ParticleEmitterCircleShape, IReflectable, ParticleEmitterCircleShapeRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN_NAMED(radius, mInfo.Radius, 0)
			B3D_RTTI_MEMBER_PLAIN_NAMED(thickness, mInfo.Thickness, 1)
			B3D_RTTI_MEMBER_PLAIN_NAMED(arc, mInfo.Arc, 2)
			B3D_RTTI_MEMBER_PLAIN_NAMED(modeType, mInfo.Mode.Type, 3)
			B3D_RTTI_MEMBER_PLAIN_NAMED(modeInterval, mInfo.Mode.Interval, 4)
			B3D_RTTI_MEMBER_PLAIN_NAMED(modeSpeed, mInfo.Mode.Speed, 5)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleEmitterCircleShape";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ParticleEmitterCircleShape;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ParticleEmitterCircleShape>();
		}
	};

	class B3D_CORE_EXPORT ParticleEmitterRectShapeRTTI : public RTTIType<ParticleEmitterRectShape, IReflectable, ParticleEmitterRectShapeRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN_NAMED(extents, mInfo.Extents, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleEmitterRectShape";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ParticleEmitterRectShape;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ParticleEmitterRectShape>();
		}
	};

	class B3D_CORE_EXPORT ParticleEmitterStaticMeshShapeRTTI : public RTTIType<ParticleEmitterStaticMeshShape, IReflectable, ParticleEmitterStaticMeshShapeRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN_NAMED(type, mInfo.Type, 0)
			B3D_RTTI_MEMBER_REFL_NAMED(mesh, mInfo.Mesh, 1)
			B3D_RTTI_MEMBER_PLAIN_NAMED(sequential, mInfo.Sequential, 2)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleEmitterStaticMeshShape";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ParticleEmitterStaticMeshShape;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ParticleEmitterStaticMeshShape>();
		}
	};

	class B3D_CORE_EXPORT ParticleEmitterSkinnedMeshShapeRTTI : public RTTIType<ParticleEmitterSkinnedMeshShape, IReflectable, ParticleEmitterSkinnedMeshShapeRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN_NAMED(type, mInfo.Type, 0)
			B3D_RTTI_MEMBER_PLAIN_NAMED(sequential, mInfo.Sequential, 1)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleEmitterSkinnedMeshShape";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ParticleEmitterSkinnedMeshShape;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ParticleEmitterSkinnedMeshShape>();
		}
	};

	template <>
	struct RTTIPlainType<ParticleBurst>
	{
		enum
		{
			id = TID_ParticleBurst
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const ParticleBurst& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr uint32_t kVersion = 0; // In case the data structure changes

			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += B3DRTTIWrite(kVersion, stream);
				size += B3DRTTIWrite(data.Time, stream);
				size += B3DRTTIWrite(data.Cycles, stream);
				size += B3DRTTIWrite(data.Count, stream);
				size += B3DRTTIWrite(data.Interval, stream);

				return size; });
		}

		static BitLength FromMemory(ParticleBurst& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			uint32_t version;
			B3DRTTIRead(version, stream);

			switch(version)
			{
			case 0:
				B3DRTTIRead(data.Time, stream);
				B3DRTTIRead(data.Cycles, stream);
				B3DRTTIRead(data.Count, stream);
				B3DRTTIRead(data.Interval, stream);
				break;
			default:
				B3D_LOG(Error, RTTI, "Unknown version of ParticleBurst data. Unable to deserialize.");
				break;
			}

			return size;
		}

		static BitLength GetSize(const ParticleBurst& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint32_t);
			dataSize += B3DRTTISize(data.Time);
			dataSize += B3DRTTISize(data.Cycles);
			dataSize += B3DRTTISize(data.Count);
			dataSize += B3DRTTISize(data.Interval);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	class B3D_CORE_EXPORT ParticleEmitterRTTI : public RTTIType<ParticleEmitter, IReflectable, ParticleEmitterRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(mEmissionRate, 0)
			B3D_RTTI_MEMBER_PLAIN(mInitialLifetime, 1)
			B3D_RTTI_MEMBER_PLAIN(mInitialSpeed, 2)
			B3D_RTTI_MEMBER_PLAIN(mInitialSize, 3)
			B3D_RTTI_MEMBER_PLAIN(mInitialSize3D, 4)
			B3D_RTTI_MEMBER_PLAIN(mUse3DSize, 5)
			B3D_RTTI_MEMBER_PLAIN(mInitialRotation, 6)
			B3D_RTTI_MEMBER_PLAIN(mInitialRotation3D, 7)
			B3D_RTTI_MEMBER_PLAIN(mUse3DRotation, 8)
			B3D_RTTI_MEMBER_PLAIN(mInitialColor, 9)
			B3D_RTTI_MEMBER_PLAIN(mFlipU, 10)
			B3D_RTTI_MEMBER_PLAIN(mFlipV, 11)
			B3D_RTTI_MEMBER_REFLPTR(mShape, 12)
			B3D_RTTI_MEMBER_PLAIN(mRandomOffset, 13)
			B3D_RTTI_MEMBER_PLAIN_ARRAY(mBursts, 14)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleEmitter";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ParticleEmitter;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ParticleEmitter>();
		}
	};

	class B3D_CORE_EXPORT ParticleTextureAnimationRTTI : public RTTIType<ParticleTextureAnimation, IReflectable, ParticleTextureAnimationRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN_NAMED(numCycles, mDesc.NumCycles, 0)
			B3D_RTTI_MEMBER_PLAIN_NAMED(randomizeRow, mDesc.RandomizeRow, 1)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleTextureAnimation";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ParticleTextureAnimation;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ParticleTextureAnimation>();
		}
	};

	class B3D_CORE_EXPORT ParticleOrbitRTTI : public RTTIType<ParticleOrbit, IReflectable, ParticleOrbitRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN_NAMED(center, mDesc.Center, 0)
			B3D_RTTI_MEMBER_PLAIN_NAMED(velocity, mDesc.Velocity, 1)
			B3D_RTTI_MEMBER_PLAIN_NAMED(radial, mDesc.Radial, 2)
			B3D_RTTI_MEMBER_PLAIN_NAMED(worldSpace, mDesc.WorldSpace, 3)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleOrbit";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ParticleOrbit;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ParticleOrbit>();
		}
	};

	class B3D_CORE_EXPORT ParticleVelocityRTTI : public RTTIType<ParticleVelocity, IReflectable, ParticleVelocityRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN_NAMED(velocity, mDesc.Velocity, 0)
			B3D_RTTI_MEMBER_PLAIN_NAMED(worldSpace, mDesc.WorldSpace, 1)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleVelocity";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ParticleVelocity;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ParticleVelocity>();
		}
	};

	class B3D_CORE_EXPORT ParticleForceRTTI : public RTTIType<ParticleForce, IReflectable, ParticleForceRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN_NAMED(force, mDesc.Force, 0)
			B3D_RTTI_MEMBER_PLAIN_NAMED(worldSpace, mDesc.WorldSpace, 1)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleForce";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ParticleForce;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ParticleForce>();
		}
	};

	class B3D_CORE_EXPORT ParticleGravityRTTI : public RTTIType<ParticleGravity, IReflectable, ParticleGravityRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN_NAMED(scale, mDesc.Scale, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleGravity";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ParticleGravity;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ParticleGravity>();
		}
	};

	class B3D_CORE_EXPORT ParticleColorRTTI : public RTTIType<ParticleColor, IReflectable, ParticleColorRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN_NAMED(color, mDesc.Color, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleColor";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ParticleColor;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ParticleColor>();
		}
	};

	class B3D_CORE_EXPORT ParticleSizeRTTI : public RTTIType<ParticleSize, IReflectable, ParticleSizeRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN_NAMED(size, mDesc.Size, 0)
			B3D_RTTI_MEMBER_PLAIN_NAMED(size3D, mDesc.Size3D, 1)
			B3D_RTTI_MEMBER_PLAIN_NAMED(use3DSize, mDesc.Use3DSize, 2)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleSize";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ParticleSize;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ParticleSize>();
		}
	};

	class B3D_CORE_EXPORT ParticleRotationRTTI : public RTTIType<ParticleRotation, IReflectable, ParticleRotationRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN_NAMED(rotation, mDesc.Rotation, 0)
			B3D_RTTI_MEMBER_PLAIN_NAMED(rotation3D, mDesc.Rotation3D, 1)
			B3D_RTTI_MEMBER_PLAIN_NAMED(use3DRotation, mDesc.Use3DRotation, 2)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleRotation";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ParticleRotation;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ParticleRotation>();
		}
	};

	class B3D_CORE_EXPORT ParticleCollisionsRTTI : public RTTIType<ParticleCollisions, IReflectable, ParticleCollisionsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN_NAMED(radius, mDesc.Radius, 0)
			B3D_RTTI_MEMBER_PLAIN_NAMED(dampening, mDesc.Dampening, 1)
			B3D_RTTI_MEMBER_PLAIN_NAMED(layer, mDesc.Layer, 2)
			B3D_RTTI_MEMBER_PLAIN_NAMED(lifetimeLoss, mDesc.LifetimeLoss, 3)
			B3D_RTTI_MEMBER_PLAIN_NAMED(mode, mDesc.Mode, 4)
			B3D_RTTI_MEMBER_PLAIN_NAMED(restitution, mDesc.Restitution, 5)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleCollisions";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ParticleCollisions;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ParticleCollisions>();
		}
	};

	class B3D_CORE_EXPORT ParticleVectorFieldSettingsRTTI : public RTTIType<ParticleVectorFieldSettings, IReflectable, ParticleVectorFieldSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_REFL(VectorField, 0)
			B3D_RTTI_MEMBER_PLAIN(Intensity, 1)
			B3D_RTTI_MEMBER_PLAIN(Tightness, 2)
			B3D_RTTI_MEMBER_PLAIN(Scale, 3)
			B3D_RTTI_MEMBER_PLAIN(Offset, 4)
			B3D_RTTI_MEMBER_PLAIN(Rotation, 5)
			B3D_RTTI_MEMBER_PLAIN(RotationRate, 6)
			B3D_RTTI_MEMBER_PLAIN(TilingX, 7)
			B3D_RTTI_MEMBER_PLAIN(TilingY, 8)
			B3D_RTTI_MEMBER_PLAIN(TilingZ, 9)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleVectorFieldSettings";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ParticleVectorFieldSettings;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ParticleVectorFieldSettings>();
		}
	};

	class B3D_CORE_EXPORT ParticleDepthCollisionSettingsRTTI : public RTTIType<ParticleDepthCollisionSettings, IReflectable, ParticleDepthCollisionSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(Enabled, 0)
			B3D_RTTI_MEMBER_PLAIN(Restitution, 1)
			B3D_RTTI_MEMBER_PLAIN(Dampening, 2)
			B3D_RTTI_MEMBER_PLAIN(RadiusScale, 3)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleDepthCollisionSettings";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ParticleDepthCollisionSettings;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ParticleDepthCollisionSettings>();
		}
	};

	class B3D_CORE_EXPORT ParticleGpuSimulationSettingsRTTI : public RTTIType<ParticleGpuSimulationSettings, IReflectable, ParticleGpuSimulationSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_REFL(VectorField, 0)
			B3D_RTTI_MEMBER_PLAIN(ColorOverLifetime, 1)
			B3D_RTTI_MEMBER_PLAIN(SizeScaleOverLifetime, 2)
			B3D_RTTI_MEMBER_REFL(DepthCollision, 3)
			B3D_RTTI_MEMBER_PLAIN(Acceleration, 4)
			B3D_RTTI_MEMBER_PLAIN(Drag, 5)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleGpuSimulationSettings";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ParticleGpuSimulationSettings;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ParticleGpuSimulationSettings>();
		}
	};

	class B3D_CORE_EXPORT ParticleSystemSettingsRTTI : public RTTIType<ParticleSystemSettings, IReflectable, ParticleSystemSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(SimulationSpace, 0)
			B3D_RTTI_MEMBER_PLAIN(Orientation, 1)
			B3D_RTTI_MEMBER_PLAIN(OrientationLockY, 2)
			B3D_RTTI_MEMBER_PLAIN(OrientationPlaneNormal, 3)
			B3D_RTTI_MEMBER_PLAIN(SortMode, 4)
			B3D_RTTI_MEMBER_PLAIN(Duration, 5)
			B3D_RTTI_MEMBER_PLAIN(IsLooping, 6)
			B3D_RTTI_MEMBER_PLAIN(MaxParticles, 7)
			B3D_RTTI_MEMBER_PLAIN(UseAutomaticSeed, 8)
			// B3D_RTTI_MEMBER_PLAIN(gravityScale, 9)
			B3D_RTTI_MEMBER_PLAIN(ManualSeed, 10)
			B3D_RTTI_MEMBER_REFL(Material, 11)
			B3D_RTTI_MEMBER_PLAIN(UseAutomaticBounds, 12)
			B3D_RTTI_MEMBER_PLAIN(CustomBounds, 13)
			B3D_RTTI_MEMBER_PLAIN(RenderMode, 14)
			B3D_RTTI_MEMBER_REFL(Mesh, 15)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleSystemSettings";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ParticleSystemSettings;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ParticleSystemSettings>();
		}
	};

	class B3D_CORE_EXPORT ParticleSystemRTTI : public RTTIType<ParticleSystem, IReflectable, ParticleSystemRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_REFL(mSettings, 0)
			B3D_RTTI_MEMBER_REFLPTR_ARRAY(mEmitters, 1)
			B3D_RTTI_MEMBER_REFLPTR_ARRAY(mEvolvers, 2)
			B3D_RTTI_MEMBER_REFL(mGpuSimulationSettings, 3)
			B3D_RTTI_MEMBER_PLAIN(mLayer, 4)
		B3D_RTTI_END_MEMBERS

	public:
		void OnDeserializationEnded(IReflectable* obj, RTTIOperationContext* context) override
		{
			// Note: Since this is a CoreObject I should call Initialize() right after deserialization,
			// but since this specific type is used in Components we delay initialization until Component
			// itself does it. Keep this is mind in case this ever needs to be deserialized for non-Component
			// purposes (you'll need to call initialize manually).
		}

		const String& GetRttiName() override
		{
			static String name = "ParticleSystem";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ParticleSystem;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return ParticleSystem::CreateEmpty();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
