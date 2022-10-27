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

	class BS_CORE_EXPORT ParticleEmitterConeShapeRTTI : public RTTIType<ParticleEmitterConeShape, IReflectable, ParticleEmitterConeShapeRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(type, mInfo.Type, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(radius, mInfo.Radius, 1)
			BS_RTTI_MEMBER_PLAIN_NAMED(angle, mInfo.Angle, 2)
			BS_RTTI_MEMBER_PLAIN_NAMED(length, mInfo.Length, 3)
			BS_RTTI_MEMBER_PLAIN_NAMED(thickness, mInfo.Thickness, 4)
			BS_RTTI_MEMBER_PLAIN_NAMED(arc, mInfo.Arc, 5)
			BS_RTTI_MEMBER_PLAIN_NAMED(modeType, mInfo.Mode.Type, 6)
			BS_RTTI_MEMBER_PLAIN_NAMED(modeInterval, mInfo.Mode.Interval, 7)
			BS_RTTI_MEMBER_PLAIN_NAMED(modeSpeed, mInfo.Mode.Speed, 8)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleEmitterConeShape";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ParticleEmitterConeShape;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return bs_shared_ptr_new<ParticleEmitterConeShape>();
		}
	};

	class BS_CORE_EXPORT ParticleEmitterSphereShapeRTTI : public RTTIType<ParticleEmitterSphereShape, IReflectable, ParticleEmitterSphereShapeRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(radius, mInfo.Radius, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(thickness, mInfo.Thickness, 1)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleEmitterSphereShape";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ParticleEmitterSphereShape;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return bs_shared_ptr_new<ParticleEmitterSphereShape>();
		}
	};

	class BS_CORE_EXPORT ParticleEmitterHemisphereShapeRTTI : public RTTIType<ParticleEmitterHemisphereShape, IReflectable, ParticleEmitterHemisphereShapeRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(radius, mInfo.Radius, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(thickness, mInfo.Thickness, 1)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleEmitterHemisphereShape";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ParticleEmitterHemisphereShape;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return bs_shared_ptr_new<ParticleEmitterHemisphereShape>();
		}
	};

	class BS_CORE_EXPORT ParticleEmitterBoxShapeRTTI : public RTTIType<ParticleEmitterBoxShape, IReflectable, ParticleEmitterBoxShapeRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(type, mInfo.Type, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(extents, mInfo.Extents, 1)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleEmitterBoxShape";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ParticleEmitterBoxShape;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return bs_shared_ptr_new<ParticleEmitterBoxShape>();
		}
	};

	class BS_CORE_EXPORT ParticleEmitterLineShapeRTTI : public RTTIType<ParticleEmitterLineShape, IReflectable, ParticleEmitterLineShapeRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(length, mInfo.Length, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(modeType, mInfo.Mode.Type, 1)
			BS_RTTI_MEMBER_PLAIN_NAMED(modeInterval, mInfo.Mode.Interval, 2)
			BS_RTTI_MEMBER_PLAIN_NAMED(modeSpeed, mInfo.Mode.Speed, 3)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleEmitterLineShape";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ParticleEmitterLineShape;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return bs_shared_ptr_new<ParticleEmitterLineShape>();
		}
	};

	class BS_CORE_EXPORT ParticleEmitterCircleShapeRTTI : public RTTIType<ParticleEmitterCircleShape, IReflectable, ParticleEmitterCircleShapeRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(radius, mInfo.Radius, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(thickness, mInfo.Thickness, 1)
			BS_RTTI_MEMBER_PLAIN_NAMED(arc, mInfo.Arc, 2)
			BS_RTTI_MEMBER_PLAIN_NAMED(modeType, mInfo.Mode.Type, 3)
			BS_RTTI_MEMBER_PLAIN_NAMED(modeInterval, mInfo.Mode.Interval, 4)
			BS_RTTI_MEMBER_PLAIN_NAMED(modeSpeed, mInfo.Mode.Speed, 5)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleEmitterCircleShape";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ParticleEmitterCircleShape;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return bs_shared_ptr_new<ParticleEmitterCircleShape>();
		}
	};

	class BS_CORE_EXPORT ParticleEmitterRectShapeRTTI : public RTTIType<ParticleEmitterRectShape, IReflectable, ParticleEmitterRectShapeRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(extents, mInfo.Extents, 0)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleEmitterRectShape";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ParticleEmitterRectShape;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return bs_shared_ptr_new<ParticleEmitterRectShape>();
		}
	};

	class BS_CORE_EXPORT ParticleEmitterStaticMeshShapeRTTI : public RTTIType<ParticleEmitterStaticMeshShape, IReflectable, ParticleEmitterStaticMeshShapeRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(type, mInfo.Type, 0)
			BS_RTTI_MEMBER_REFL_NAMED(mesh, mInfo.Mesh, 1)
			BS_RTTI_MEMBER_PLAIN_NAMED(sequential, mInfo.Sequential, 2)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleEmitterStaticMeshShape";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ParticleEmitterStaticMeshShape;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return bs_shared_ptr_new<ParticleEmitterStaticMeshShape>();
		}
	};

	class BS_CORE_EXPORT ParticleEmitterSkinnedMeshShapeRTTI : public RTTIType<ParticleEmitterSkinnedMeshShape, IReflectable, ParticleEmitterSkinnedMeshShapeRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(type, mInfo.Type, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(sequential, mInfo.Sequential, 1)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleEmitterSkinnedMeshShape";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ParticleEmitterSkinnedMeshShape;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return bs_shared_ptr_new<ParticleEmitterSkinnedMeshShape>();
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

		/** @copydoc RTTIPlainType::toMemory */
		static BitLength ToMemory(const ParticleBurst& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr uint32_t kVersion = 0; // In case the data structure changes

			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += rtti_write(kVersion, stream);
				size += rtti_write(data.Time, stream);
				size += rtti_write(data.Cycles, stream);
				size += rtti_write(data.Count, stream);
				size += rtti_write(data.Interval, stream);

				return size; });
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static BitLength FromMemory(ParticleBurst& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			uint32_t version;
			rtti_read(version, stream);

			switch(version)
			{
			case 0:
				rtti_read(data.Time, stream);
				rtti_read(data.Cycles, stream);
				rtti_read(data.Count, stream);
				rtti_read(data.Interval, stream);
				break;
			default:
				BS_LOG(Error, RTTI, "Unknown version of ParticleBurst data. Unable to deserialize.");
				break;
			}

			return size;
		}

		/** @copydoc RTTIPlainType::getSize */
		static BitLength GetSize(const ParticleBurst& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint32_t);
			dataSize += rtti_size(data.Time);
			dataSize += rtti_size(data.Cycles);
			dataSize += rtti_size(data.Count);
			dataSize += rtti_size(data.Interval);

			rtti_add_header_size(dataSize, compress);
			return dataSize;
		}
	};

	class BS_CORE_EXPORT ParticleEmitterRTTI : public RTTIType<ParticleEmitter, IReflectable, ParticleEmitterRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mEmissionRate, 0)
			BS_RTTI_MEMBER_PLAIN(mInitialLifetime, 1)
			BS_RTTI_MEMBER_PLAIN(mInitialSpeed, 2)
			BS_RTTI_MEMBER_PLAIN(mInitialSize, 3)
			BS_RTTI_MEMBER_PLAIN(mInitialSize3D, 4)
			BS_RTTI_MEMBER_PLAIN(mUse3DSize, 5)
			BS_RTTI_MEMBER_PLAIN(mInitialRotation, 6)
			BS_RTTI_MEMBER_PLAIN(mInitialRotation3D, 7)
			BS_RTTI_MEMBER_PLAIN(mUse3DRotation, 8)
			BS_RTTI_MEMBER_PLAIN(mInitialColor, 9)
			BS_RTTI_MEMBER_PLAIN(mFlipU, 10)
			BS_RTTI_MEMBER_PLAIN(mFlipV, 11)
			BS_RTTI_MEMBER_REFLPTR(mShape, 12)
			BS_RTTI_MEMBER_PLAIN(mRandomOffset, 13)
			BS_RTTI_MEMBER_PLAIN_ARRAY(mBursts, 14)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleEmitter";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ParticleEmitter;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return bs_shared_ptr_new<ParticleEmitter>();
		}
	};

	class BS_CORE_EXPORT ParticleTextureAnimationRTTI : public RTTIType<ParticleTextureAnimation, IReflectable, ParticleTextureAnimationRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(numCycles, mDesc.NumCycles, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(randomizeRow, mDesc.RandomizeRow, 1)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleTextureAnimation";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ParticleTextureAnimation;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return bs_shared_ptr_new<ParticleTextureAnimation>();
		}
	};

	class BS_CORE_EXPORT ParticleOrbitRTTI : public RTTIType<ParticleOrbit, IReflectable, ParticleOrbitRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(center, mDesc.Center, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(velocity, mDesc.Velocity, 1)
			BS_RTTI_MEMBER_PLAIN_NAMED(radial, mDesc.Radial, 2)
			BS_RTTI_MEMBER_PLAIN_NAMED(worldSpace, mDesc.WorldSpace, 3)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleOrbit";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ParticleOrbit;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return bs_shared_ptr_new<ParticleOrbit>();
		}
	};

	class BS_CORE_EXPORT ParticleVelocityRTTI : public RTTIType<ParticleVelocity, IReflectable, ParticleVelocityRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(velocity, mDesc.Velocity, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(worldSpace, mDesc.WorldSpace, 1)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleVelocity";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ParticleVelocity;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return bs_shared_ptr_new<ParticleVelocity>();
		}
	};

	class BS_CORE_EXPORT ParticleForceRTTI : public RTTIType<ParticleForce, IReflectable, ParticleForceRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(force, mDesc.Force, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(worldSpace, mDesc.WorldSpace, 1)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleForce";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ParticleForce;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return bs_shared_ptr_new<ParticleForce>();
		}
	};

	class BS_CORE_EXPORT ParticleGravityRTTI : public RTTIType<ParticleGravity, IReflectable, ParticleGravityRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(scale, mDesc.Scale, 0)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleGravity";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ParticleGravity;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return bs_shared_ptr_new<ParticleGravity>();
		}
	};

	class BS_CORE_EXPORT ParticleColorRTTI : public RTTIType<ParticleColor, IReflectable, ParticleColorRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(color, mDesc.Color, 0)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleColor";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ParticleColor;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return bs_shared_ptr_new<ParticleColor>();
		}
	};

	class BS_CORE_EXPORT ParticleSizeRTTI : public RTTIType<ParticleSize, IReflectable, ParticleSizeRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(size, mDesc.Size, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(size3D, mDesc.Size3D, 1)
			BS_RTTI_MEMBER_PLAIN_NAMED(use3DSize, mDesc.Use3DSize, 2)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleSize";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ParticleSize;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return bs_shared_ptr_new<ParticleSize>();
		}
	};

	class BS_CORE_EXPORT ParticleRotationRTTI : public RTTIType<ParticleRotation, IReflectable, ParticleRotationRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(rotation, mDesc.Rotation, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(rotation3D, mDesc.Rotation3D, 1)
			BS_RTTI_MEMBER_PLAIN_NAMED(use3DRotation, mDesc.Use3DRotation, 2)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleRotation";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ParticleRotation;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return bs_shared_ptr_new<ParticleRotation>();
		}
	};

	class BS_CORE_EXPORT ParticleCollisionsRTTI : public RTTIType<ParticleCollisions, IReflectable, ParticleCollisionsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(radius, mDesc.Radius, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(dampening, mDesc.Dampening, 1)
			BS_RTTI_MEMBER_PLAIN_NAMED(layer, mDesc.Layer, 2)
			BS_RTTI_MEMBER_PLAIN_NAMED(lifetimeLoss, mDesc.LifetimeLoss, 3)
			BS_RTTI_MEMBER_PLAIN_NAMED(mode, mDesc.Mode, 4)
			BS_RTTI_MEMBER_PLAIN_NAMED(restitution, mDesc.Restitution, 5)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleCollisions";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ParticleCollisions;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return bs_shared_ptr_new<ParticleCollisions>();
		}
	};

	class BS_CORE_EXPORT ParticleVectorFieldSettingsRTTI : public RTTIType<ParticleVectorFieldSettings, IReflectable, ParticleVectorFieldSettingsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFL(VectorField, 0)
			BS_RTTI_MEMBER_PLAIN(Intensity, 1)
			BS_RTTI_MEMBER_PLAIN(Tightness, 2)
			BS_RTTI_MEMBER_PLAIN(Scale, 3)
			BS_RTTI_MEMBER_PLAIN(Offset, 4)
			BS_RTTI_MEMBER_PLAIN(Rotation, 5)
			BS_RTTI_MEMBER_PLAIN(RotationRate, 6)
			BS_RTTI_MEMBER_PLAIN(TilingX, 7)
			BS_RTTI_MEMBER_PLAIN(TilingY, 8)
			BS_RTTI_MEMBER_PLAIN(TilingZ, 9)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleVectorFieldSettings";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ParticleVectorFieldSettings;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return bs_shared_ptr_new<ParticleVectorFieldSettings>();
		}
	};

	class BS_CORE_EXPORT ParticleDepthCollisionSettingsRTTI : public RTTIType<ParticleDepthCollisionSettings, IReflectable, ParticleDepthCollisionSettingsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(Enabled, 0)
			BS_RTTI_MEMBER_PLAIN(Restitution, 1)
			BS_RTTI_MEMBER_PLAIN(Dampening, 2)
			BS_RTTI_MEMBER_PLAIN(RadiusScale, 3)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleDepthCollisionSettings";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ParticleDepthCollisionSettings;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return bs_shared_ptr_new<ParticleDepthCollisionSettings>();
		}
	};

	class BS_CORE_EXPORT ParticleGpuSimulationSettingsRTTI : public RTTIType<ParticleGpuSimulationSettings, IReflectable, ParticleGpuSimulationSettingsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFL(VectorField, 0)
			BS_RTTI_MEMBER_PLAIN(ColorOverLifetime, 1)
			BS_RTTI_MEMBER_PLAIN(SizeScaleOverLifetime, 2)
			BS_RTTI_MEMBER_REFL(DepthCollision, 3)
			BS_RTTI_MEMBER_PLAIN(Acceleration, 4)
			BS_RTTI_MEMBER_PLAIN(Drag, 5)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleGpuSimulationSettings";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ParticleGpuSimulationSettings;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return bs_shared_ptr_new<ParticleGpuSimulationSettings>();
		}
	};

	class BS_CORE_EXPORT ParticleSystemSettingsRTTI : public RTTIType<ParticleSystemSettings, IReflectable, ParticleSystemSettingsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(SimulationSpace, 0)
			BS_RTTI_MEMBER_PLAIN(Orientation, 1)
			BS_RTTI_MEMBER_PLAIN(OrientationLockY, 2)
			BS_RTTI_MEMBER_PLAIN(OrientationPlaneNormal, 3)
			BS_RTTI_MEMBER_PLAIN(SortMode, 4)
			BS_RTTI_MEMBER_PLAIN(Duration, 5)
			BS_RTTI_MEMBER_PLAIN(IsLooping, 6)
			BS_RTTI_MEMBER_PLAIN(MaxParticles, 7)
			BS_RTTI_MEMBER_PLAIN(UseAutomaticSeed, 8)
			// BS_RTTI_MEMBER_PLAIN(gravityScale, 9)
			BS_RTTI_MEMBER_PLAIN(ManualSeed, 10)
			BS_RTTI_MEMBER_REFL(Material, 11)
			BS_RTTI_MEMBER_PLAIN(UseAutomaticBounds, 12)
			BS_RTTI_MEMBER_PLAIN(CustomBounds, 13)
			BS_RTTI_MEMBER_PLAIN(RenderMode, 14)
			BS_RTTI_MEMBER_REFL(Mesh, 15)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ParticleSystemSettings";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ParticleSystemSettings;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return bs_shared_ptr_new<ParticleSystemSettings>();
		}
	};

	class BS_CORE_EXPORT ParticleSystemRTTI : public RTTIType<ParticleSystem, IReflectable, ParticleSystemRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFL(mSettings, 0)
			BS_RTTI_MEMBER_REFLPTR_ARRAY(mEmitters, 1)
			BS_RTTI_MEMBER_REFLPTR_ARRAY(mEvolvers, 2)
			BS_RTTI_MEMBER_REFL(mGpuSimulationSettings, 3)
			BS_RTTI_MEMBER_PLAIN(mLayer, 4)
		BS_END_RTTI_MEMBERS

	public:
		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
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

		u32 GetRttiId() override
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
