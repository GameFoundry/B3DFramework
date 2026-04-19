//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIECSField.h"
#include "Components/B3DParticleSystem.h"
#include "Particles/B3DParticleEmitter.h"
#include "RTTI/B3DGameObjectRTTI.h"
#include "Particles/B3DParticleEvolver.h"
#include "RTTI/B3DColorGradientRTTI.h"
#include "RTTI/B3DMathRTTI.h"
#include "RTTI/B3DParticleDistributionRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT ParticleEmitterConeShapeRTTI : public TRTTIType<ParticleEmitterConeShape, IReflectable, ParticleEmitterConeShapeRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(type, mSettings.Type, 0)
			B3D_RTTI_MEMBER_NAMED(radius, mSettings.Radius, 1)
			B3D_RTTI_MEMBER_NAMED(angle, mSettings.Angle, 2)
			B3D_RTTI_MEMBER_NAMED(length, mSettings.Length, 3)
			B3D_RTTI_MEMBER_NAMED(thickness, mSettings.Thickness, 4)
			B3D_RTTI_MEMBER_NAMED(arc, mSettings.Arc, 5)
			B3D_RTTI_MEMBER_NAMED(modeType, mSettings.Mode.Type, 6)
			B3D_RTTI_MEMBER_NAMED(modeInterval, mSettings.Mode.Interval, 7)
			B3D_RTTI_MEMBER_NAMED(modeSpeed, mSettings.Mode.Speed, 8)
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

	class B3D_EXPORT ParticleEmitterSphereShapeRTTI : public TRTTIType<ParticleEmitterSphereShape, IReflectable, ParticleEmitterSphereShapeRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(radius, mSettings.Radius, 0)
			B3D_RTTI_MEMBER_NAMED(thickness, mSettings.Thickness, 1)
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

	class B3D_EXPORT ParticleEmitterHemisphereShapeRTTI : public TRTTIType<ParticleEmitterHemisphereShape, IReflectable, ParticleEmitterHemisphereShapeRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(radius, mSettings.Radius, 0)
			B3D_RTTI_MEMBER_NAMED(thickness, mSettings.Thickness, 1)
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

	class B3D_EXPORT ParticleEmitterBoxShapeRTTI : public TRTTIType<ParticleEmitterBoxShape, IReflectable, ParticleEmitterBoxShapeRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(type, mSettings.Type, 0)
			B3D_RTTI_MEMBER_NAMED(extents, mSettings.Extents, 1)
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

	class B3D_EXPORT ParticleEmitterLineShapeRTTI : public TRTTIType<ParticleEmitterLineShape, IReflectable, ParticleEmitterLineShapeRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(length, mSettings.Length, 0)
			B3D_RTTI_MEMBER_NAMED(modeType, mSettings.Mode.Type, 1)
			B3D_RTTI_MEMBER_NAMED(modeInterval, mSettings.Mode.Interval, 2)
			B3D_RTTI_MEMBER_NAMED(modeSpeed, mSettings.Mode.Speed, 3)
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

	class B3D_EXPORT ParticleEmitterCircleShapeRTTI : public TRTTIType<ParticleEmitterCircleShape, IReflectable, ParticleEmitterCircleShapeRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(radius, mSettings.Radius, 0)
			B3D_RTTI_MEMBER_NAMED(thickness, mSettings.Thickness, 1)
			B3D_RTTI_MEMBER_NAMED(arc, mSettings.Arc, 2)
			B3D_RTTI_MEMBER_NAMED(modeType, mSettings.Mode.Type, 3)
			B3D_RTTI_MEMBER_NAMED(modeInterval, mSettings.Mode.Interval, 4)
			B3D_RTTI_MEMBER_NAMED(modeSpeed, mSettings.Mode.Speed, 5)
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

	class B3D_EXPORT ParticleEmitterRectShapeRTTI : public TRTTIType<ParticleEmitterRectShape, IReflectable, ParticleEmitterRectShapeRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(extents, mSettings.Extents, 0)
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

	class B3D_EXPORT ParticleEmitterStaticMeshShapeRTTI : public TRTTIType<ParticleEmitterStaticMeshShape, IReflectable, ParticleEmitterStaticMeshShapeRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(type, mSettings.Type, 0)
			B3D_RTTI_MEMBER_NAMED(mesh, mSettings.Mesh, 1)
			B3D_RTTI_MEMBER_NAMED(sequential, mSettings.Sequential, 2)
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

	class B3D_EXPORT ParticleEmitterSkinnedMeshShapeRTTI : public TRTTIType<ParticleEmitterSkinnedMeshShape, IReflectable, ParticleEmitterSkinnedMeshShapeRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(type, mSettings.Type, 0)
			B3D_RTTI_MEMBER_NAMED(sequential, mSettings.Sequential, 1)
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
				B3D_LOG(Error, LogRTTI, "Unknown version of ParticleBurst data. Unable to deserialize.");
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

	class B3D_EXPORT ParticleEmitterRTTI : public TRTTIType<ParticleEmitter, IReflectable, ParticleEmitterRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mEmissionRate, 0)
			B3D_RTTI_MEMBER(mInitialLifetime, 1)
			B3D_RTTI_MEMBER(mInitialSpeed, 2)
			B3D_RTTI_MEMBER(mInitialSize, 3)
			B3D_RTTI_MEMBER(mInitialSize3D, 4)
			B3D_RTTI_MEMBER(mUse3DSize, 5)
			B3D_RTTI_MEMBER(mInitialRotation, 6)
			B3D_RTTI_MEMBER(mInitialRotation3D, 7)
			B3D_RTTI_MEMBER(mUse3DRotation, 8)
			B3D_RTTI_MEMBER(mInitialColor, 9)
			B3D_RTTI_MEMBER(mFlipU, 10)
			B3D_RTTI_MEMBER(mFlipV, 11)
			B3D_RTTI_MEMBER(mShape, 12)
			B3D_RTTI_MEMBER(mRandomOffset, 13)
			B3D_RTTI_MEMBER_CONTAINER(mBursts, 14)
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

	class B3D_EXPORT ParticleTextureAnimationRTTI : public TRTTIType<ParticleTextureAnimation, IReflectable, ParticleTextureAnimationRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(numCycles, mSettings.CycleCount, 0)
			B3D_RTTI_MEMBER_NAMED(randomizeRow, mSettings.RandomizeRow, 1)
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

	class B3D_EXPORT ParticleOrbitRTTI : public TRTTIType<ParticleOrbit, IReflectable, ParticleOrbitRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(center, mSettings.Center, 0)
			B3D_RTTI_MEMBER_NAMED(velocity, mSettings.Velocity, 1)
			B3D_RTTI_MEMBER_NAMED(radial, mSettings.Radial, 2)
			B3D_RTTI_MEMBER_NAMED(worldSpace, mSettings.WorldSpace, 3)
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

	class B3D_EXPORT ParticleVelocityRTTI : public TRTTIType<ParticleVelocity, IReflectable, ParticleVelocityRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(velocity, mSettings.Velocity, 0)
			B3D_RTTI_MEMBER_NAMED(worldSpace, mSettings.WorldSpace, 1)
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

	class B3D_EXPORT ParticleForceRTTI : public TRTTIType<ParticleForce, IReflectable, ParticleForceRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(force, mSettings.Force, 0)
			B3D_RTTI_MEMBER_NAMED(worldSpace, mSettings.WorldSpace, 1)
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

	class B3D_EXPORT ParticleGravityRTTI : public TRTTIType<ParticleGravity, IReflectable, ParticleGravityRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(scale, mSettings.Scale, 0)
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

	class B3D_EXPORT ParticleColorRTTI : public TRTTIType<ParticleColor, IReflectable, ParticleColorRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(color, mSettings.Color, 0)
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

	class B3D_EXPORT ParticleSizeRTTI : public TRTTIType<ParticleSize, IReflectable, ParticleSizeRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(size, mSettings.Size, 0)
			B3D_RTTI_MEMBER_NAMED(size3D, mSettings.Size3D, 1)
			B3D_RTTI_MEMBER_NAMED(use3DSize, mSettings.Use3DSize, 2)
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

	class B3D_EXPORT ParticleRotationRTTI : public TRTTIType<ParticleRotation, IReflectable, ParticleRotationRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(rotation, mSettings.Rotation, 0)
			B3D_RTTI_MEMBER_NAMED(rotation3D, mSettings.Rotation3D, 1)
			B3D_RTTI_MEMBER_NAMED(use3DRotation, mSettings.Use3DRotation, 2)
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

	class B3D_EXPORT ParticleCollisionsRTTI : public TRTTIType<ParticleCollisions, IReflectable, ParticleCollisionsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(radius, mSettings.Radius, 0)
			B3D_RTTI_MEMBER_NAMED(dampening, mSettings.Dampening, 1)
			B3D_RTTI_MEMBER_NAMED(layer, mSettings.Layer, 2)
			B3D_RTTI_MEMBER_NAMED(lifetimeLoss, mSettings.LifetimeLoss, 3)
			B3D_RTTI_MEMBER_NAMED(mode, mSettings.Mode, 4)
			B3D_RTTI_MEMBER_NAMED(restitution, mSettings.Restitution, 5)
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

	class B3D_EXPORT ParticleVectorFieldSettingsRTTI : public TRTTIType<ParticleVectorFieldSettings, IReflectable, ParticleVectorFieldSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(VectorField, 0)
			B3D_RTTI_MEMBER(Intensity, 1)
			B3D_RTTI_MEMBER(Tightness, 2)
			B3D_RTTI_MEMBER(Scale, 3)
			B3D_RTTI_MEMBER(Offset, 4)
			B3D_RTTI_MEMBER(Rotation, 5)
			B3D_RTTI_MEMBER(RotationRate, 6)
			B3D_RTTI_MEMBER(TilingX, 7)
			B3D_RTTI_MEMBER(TilingY, 8)
			B3D_RTTI_MEMBER(TilingZ, 9)
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

	class B3D_EXPORT ParticleDepthCollisionSettingsRTTI : public TRTTIType<ParticleDepthCollisionSettings, IReflectable, ParticleDepthCollisionSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Enabled, 0)
			B3D_RTTI_MEMBER(Restitution, 1)
			B3D_RTTI_MEMBER(Dampening, 2)
			B3D_RTTI_MEMBER(RadiusScale, 3)
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

	class B3D_EXPORT ParticleGpuSimulationSettingsRTTI : public TRTTIType<ParticleGpuSimulationSettings, IReflectable, ParticleGpuSimulationSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(VectorField, 0)
			B3D_RTTI_MEMBER(ColorOverLifetime, 1)
			B3D_RTTI_MEMBER(SizeScaleOverLifetime, 2)
			B3D_RTTI_MEMBER(DepthCollision, 3)
			B3D_RTTI_MEMBER(Acceleration, 4)
			B3D_RTTI_MEMBER(Drag, 5)
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

	class B3D_EXPORT ParticleSystemSettingsRTTI : public TRTTIType<ParticleSystemSettings, IReflectable, ParticleSystemSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(SimulationSpace, 0)
			B3D_RTTI_MEMBER(Orientation, 1)
			B3D_RTTI_MEMBER(OrientationLockY, 2)
			B3D_RTTI_MEMBER(OrientationPlaneNormal, 3)
			B3D_RTTI_MEMBER(SortMode, 4)
			B3D_RTTI_MEMBER(Duration, 5)
			B3D_RTTI_MEMBER(IsLooping, 6)
			B3D_RTTI_MEMBER(MaxParticles, 7)
			B3D_RTTI_MEMBER(UseAutomaticSeed, 8)
			// B3D_RTTI_MEMBER_PLAIN(gravityScale, 9)
			B3D_RTTI_MEMBER(ManualSeed, 10)
			B3D_RTTI_MEMBER(Material, 11)
			B3D_RTTI_MEMBER(UseAutomaticBounds, 12)
			B3D_RTTI_MEMBER(CustomBounds, 13)
			B3D_RTTI_MEMBER(RenderMode, 14)
			B3D_RTTI_MEMBER(Mesh, 15)
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

	class B3D_EXPORT ParticleSystemRTTI : public TRTTIType<ParticleSystem, Component, ParticleSystemRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_ECS(ParticleSystem, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleSystem";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ParticleSystem;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<ParticleSystem>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d

namespace b3d::ecs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT ECSParticleSystemRTTI : public TRTTIType<ParticleSystem, IReflectable, ECSParticleSystemRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Settings, 0)
			B3D_RTTI_MEMBER(GpuSimulationSettings, 1)
			B3D_RTTI_MEMBER(Layer, 2)
			B3D_RTTI_MEMBER_CONTAINER(Emitters, 3)
			B3D_RTTI_MEMBER_CONTAINER(Evolvers, 4)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ECSParticleSystem";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ECSParticleSystem;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ParticleSystem>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d::ecs
