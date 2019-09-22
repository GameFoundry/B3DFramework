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

	class BS_CORE_EXPORT ParticleEmitterConeShapeRTTI :
		public RTTIType<ParticleEmitterConeShape, IReflectable, ParticleEmitterConeShapeRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(type, mInfo.type, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(radius, mInfo.radius, 1)
			BS_RTTI_MEMBER_PLAIN_NAMED(angle, mInfo.angle, 2)
			BS_RTTI_MEMBER_PLAIN_NAMED(length, mInfo.length, 3)
			BS_RTTI_MEMBER_PLAIN_NAMED(thickness, mInfo.thickness, 4)
			BS_RTTI_MEMBER_PLAIN_NAMED(arc, mInfo.arc, 5)
			BS_RTTI_MEMBER_PLAIN_NAMED(modeType, mInfo.mode.type, 6)
			BS_RTTI_MEMBER_PLAIN_NAMED(modeInterval, mInfo.mode.interval, 7)
			BS_RTTI_MEMBER_PLAIN_NAMED(modeSpeed, mInfo.mode.speed, 8)
		BS_END_RTTI_MEMBERS

	public:
		const String& getRTTIName() override
		{
			static String name = "ParticleEmitterConeShape";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ParticleEmitterConeShape;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ParticleEmitterConeShape>();
		}
	};

	class BS_CORE_EXPORT ParticleEmitterSphereShapeRTTI :
		public RTTIType<ParticleEmitterSphereShape, IReflectable, ParticleEmitterSphereShapeRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(radius, mInfo.radius, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(thickness, mInfo.thickness, 1)
		BS_END_RTTI_MEMBERS

	public:
		const String& getRTTIName() override
		{
			static String name = "ParticleEmitterSphereShape";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ParticleEmitterSphereShape;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ParticleEmitterSphereShape>();
		}
	};

	class BS_CORE_EXPORT ParticleEmitterHemisphereShapeRTTI :
		public RTTIType<ParticleEmitterHemisphereShape, IReflectable, ParticleEmitterHemisphereShapeRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(radius, mInfo.radius, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(thickness, mInfo.thickness, 1)
		BS_END_RTTI_MEMBERS

	public:
		const String& getRTTIName() override
		{
			static String name = "ParticleEmitterHemisphereShape";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ParticleEmitterHemisphereShape;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ParticleEmitterHemisphereShape>();
		}
	};

	class BS_CORE_EXPORT ParticleEmitterBoxShapeRTTI :
		public RTTIType<ParticleEmitterBoxShape, IReflectable, ParticleEmitterBoxShapeRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(type, mInfo.type, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(extents, mInfo.extents, 1)
		BS_END_RTTI_MEMBERS

	public:
		const String& getRTTIName() override
		{
			static String name = "ParticleEmitterBoxShape";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ParticleEmitterBoxShape;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ParticleEmitterBoxShape>();
		}
	};

	class BS_CORE_EXPORT ParticleEmitterLineShapeRTTI :
		public RTTIType<ParticleEmitterLineShape, IReflectable, ParticleEmitterLineShapeRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(length, mInfo.length, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(modeType, mInfo.mode.type, 1)
			BS_RTTI_MEMBER_PLAIN_NAMED(modeInterval, mInfo.mode.interval, 2)
			BS_RTTI_MEMBER_PLAIN_NAMED(modeSpeed, mInfo.mode.speed, 3)
		BS_END_RTTI_MEMBERS

	public:
		const String& getRTTIName() override
		{
			static String name = "ParticleEmitterLineShape";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ParticleEmitterLineShape;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ParticleEmitterLineShape>();
		}
	};

	class BS_CORE_EXPORT ParticleEmitterCircleShapeRTTI :
		public RTTIType<ParticleEmitterCircleShape, IReflectable, ParticleEmitterCircleShapeRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(radius, mInfo.radius, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(thickness, mInfo.thickness, 1)
			BS_RTTI_MEMBER_PLAIN_NAMED(arc, mInfo.arc, 2)
			BS_RTTI_MEMBER_PLAIN_NAMED(modeType, mInfo.mode.type, 3)
			BS_RTTI_MEMBER_PLAIN_NAMED(modeInterval, mInfo.mode.interval, 4)
			BS_RTTI_MEMBER_PLAIN_NAMED(modeSpeed, mInfo.mode.speed, 5)
		BS_END_RTTI_MEMBERS

	public:
		const String& getRTTIName() override
		{
			static String name = "ParticleEmitterCircleShape";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ParticleEmitterCircleShape;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ParticleEmitterCircleShape>();
		}
	};

	class BS_CORE_EXPORT ParticleEmitterRectShapeRTTI :
		public RTTIType<ParticleEmitterRectShape, IReflectable, ParticleEmitterRectShapeRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(extents, mInfo.extents, 0)
		BS_END_RTTI_MEMBERS

	public:
		const String& getRTTIName() override
		{
			static String name = "ParticleEmitterRectShape";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ParticleEmitterRectShape;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ParticleEmitterRectShape>();
		}
	};

	class BS_CORE_EXPORT ParticleEmitterStaticMeshShapeRTTI :
		public RTTIType<ParticleEmitterStaticMeshShape, IReflectable, ParticleEmitterStaticMeshShapeRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(type, mInfo.type, 0)
			BS_RTTI_MEMBER_REFL_NAMED(mesh, mInfo.mesh, 1)
			BS_RTTI_MEMBER_PLAIN_NAMED(sequential, mInfo.sequential, 2)
		BS_END_RTTI_MEMBERS

	public:
		const String& getRTTIName() override
		{
			static String name = "ParticleEmitterStaticMeshShape";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ParticleEmitterStaticMeshShape;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ParticleEmitterStaticMeshShape>();
		}
	};

	class BS_CORE_EXPORT ParticleEmitterSkinnedMeshShapeRTTI :
		public RTTIType<ParticleEmitterSkinnedMeshShape, IReflectable, ParticleEmitterSkinnedMeshShapeRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(type, mInfo.type, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(sequential, mInfo.sequential, 1)
		BS_END_RTTI_MEMBERS

	public:
		const String& getRTTIName() override
		{
			static String name = "ParticleEmitterSkinnedMeshShape";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ParticleEmitterSkinnedMeshShape;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ParticleEmitterSkinnedMeshShape>();
		}
	};

	template<> struct RTTIPlainType<ParticleBurst>
	{
		enum { id = TID_ParticleBurst }; enum { hasDynamicSize = 1 };

		/** @copydoc RTTIPlainType::toMemory */
		static BitLength toMemory(const ParticleBurst& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr uint32_t VERSION = 0; // In case the data structure changes

			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
			{
				BitLength size = 0;
				size += rtti_write(VERSION, stream);
				size += rtti_write(data.time, stream);
				size += rtti_write(data.cycles, stream);
				size += rtti_write(data.count, stream);
				size += rtti_write(data.interval, stream);

				return size;
			});
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static BitLength fromMemory(ParticleBurst& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			uint32_t version;
			rtti_read(version, stream);

			switch(version)
			{
			case 0:
				rtti_read(data.time, stream);
				rtti_read(data.cycles, stream);
				rtti_read(data.count, stream);
				rtti_read(data.interval, stream);
				break;
			default:
				BS_LOG(Error, RTTI, "Unknown version of ParticleBurst data. Unable to deserialize.");
				break;
			}

			return size;
		}

		/** @copydoc RTTIPlainType::getSize */
		static BitLength getSize(const ParticleBurst& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint32_t);
			dataSize += rtti_size(data.time);
			dataSize += rtti_size(data.cycles);
			dataSize += rtti_size(data.count);
			dataSize += rtti_size(data.interval);

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
		const String& getRTTIName() override
		{
			static String name = "ParticleEmitter";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ParticleEmitter;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ParticleEmitter>();
		}
	};

	class BS_CORE_EXPORT ParticleTextureAnimationRTTI :
		public RTTIType<ParticleTextureAnimation, IReflectable, ParticleTextureAnimationRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(numCycles, mDesc.numCycles, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(randomizeRow, mDesc.randomizeRow, 1)
		BS_END_RTTI_MEMBERS

	public:
		const String& getRTTIName() override
		{
			static String name = "ParticleTextureAnimation";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ParticleTextureAnimation;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ParticleTextureAnimation>();
		}
	};

	class BS_CORE_EXPORT ParticleOrbitRTTI : public RTTIType<ParticleOrbit, IReflectable, ParticleOrbitRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(center, mDesc.center, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(velocity, mDesc.velocity, 1)
			BS_RTTI_MEMBER_PLAIN_NAMED(radial, mDesc.radial, 2)
			BS_RTTI_MEMBER_PLAIN_NAMED(worldSpace, mDesc.worldSpace, 3)
		BS_END_RTTI_MEMBERS

	public:
		const String& getRTTIName() override
		{
			static String name = "ParticleOrbit";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ParticleOrbit;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ParticleOrbit>();
		}
	};

	class BS_CORE_EXPORT ParticleVelocityRTTI : public RTTIType<ParticleVelocity, IReflectable, ParticleVelocityRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(velocity, mDesc.velocity, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(worldSpace, mDesc.worldSpace, 1)
		BS_END_RTTI_MEMBERS

	public:
		const String& getRTTIName() override
		{
			static String name = "ParticleVelocity";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ParticleVelocity;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ParticleVelocity>();
		}
	};

	class BS_CORE_EXPORT ParticleForceRTTI : public RTTIType<ParticleForce, IReflectable, ParticleForceRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(force, mDesc.force, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(worldSpace, mDesc.worldSpace, 1)
		BS_END_RTTI_MEMBERS

	public:
		const String& getRTTIName() override
		{
			static String name = "ParticleForce";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ParticleForce;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ParticleForce>();
		}
	};

	class BS_CORE_EXPORT ParticleGravityRTTI : public RTTIType<ParticleGravity, IReflectable, ParticleGravityRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(scale, mDesc.scale, 0)
		BS_END_RTTI_MEMBERS

	public:
		const String& getRTTIName() override
		{
			static String name = "ParticleGravity";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ParticleGravity;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ParticleGravity>();
		}
	};

	class BS_CORE_EXPORT ParticleColorRTTI : public RTTIType<ParticleColor, IReflectable, ParticleColorRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(color, mDesc.color, 0)
		BS_END_RTTI_MEMBERS

	public:
		const String& getRTTIName() override
		{
			static String name = "ParticleColor";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ParticleColor;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ParticleColor>();
		}
	};

	class BS_CORE_EXPORT ParticleSizeRTTI : public RTTIType<ParticleSize, IReflectable, ParticleSizeRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(size, mDesc.size, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(size3D, mDesc.size3D, 1)
			BS_RTTI_MEMBER_PLAIN_NAMED(use3DSize, mDesc.use3DSize, 2)
		BS_END_RTTI_MEMBERS

	public:
		const String& getRTTIName() override
		{
			static String name = "ParticleSize";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ParticleSize;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ParticleSize>();
		}
	};

	class BS_CORE_EXPORT ParticleRotationRTTI : public RTTIType<ParticleRotation, IReflectable, ParticleRotationRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(rotation, mDesc.rotation, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(rotation3D, mDesc.rotation3D, 1)
			BS_RTTI_MEMBER_PLAIN_NAMED(use3DRotation, mDesc.use3DRotation, 2)
		BS_END_RTTI_MEMBERS

	public:
		const String& getRTTIName() override
		{
			static String name = "ParticleRotation";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ParticleRotation;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ParticleRotation>();
		}
	};

	class BS_CORE_EXPORT ParticleCollisionsRTTI :
		public RTTIType<ParticleCollisions, IReflectable, ParticleCollisionsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(radius, mDesc.radius, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(dampening, mDesc.dampening, 1)
			BS_RTTI_MEMBER_PLAIN_NAMED(layer, mDesc.layer, 2)
			BS_RTTI_MEMBER_PLAIN_NAMED(lifetimeLoss, mDesc.lifetimeLoss, 3)
			BS_RTTI_MEMBER_PLAIN_NAMED(mode, mDesc.mode, 4)
			BS_RTTI_MEMBER_PLAIN_NAMED(restitution, mDesc.restitution, 5)
		BS_END_RTTI_MEMBERS

	public:
		const String& getRTTIName() override
		{
			static String name = "ParticleCollisions";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ParticleCollisions;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ParticleCollisions>();
		}
	};

	class BS_CORE_EXPORT ParticleVectorFieldSettingsRTTI : public RTTIType<ParticleVectorFieldSettings, IReflectable, ParticleVectorFieldSettingsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFL(vectorField, 0)
			BS_RTTI_MEMBER_PLAIN(intensity, 1)
			BS_RTTI_MEMBER_PLAIN(tightness, 2)
			BS_RTTI_MEMBER_PLAIN(scale, 3)
			BS_RTTI_MEMBER_PLAIN(offset, 4)
			BS_RTTI_MEMBER_PLAIN(rotation, 5)
			BS_RTTI_MEMBER_PLAIN(rotationRate, 6)
			BS_RTTI_MEMBER_PLAIN(tilingX, 7)
			BS_RTTI_MEMBER_PLAIN(tilingY, 8)
			BS_RTTI_MEMBER_PLAIN(tilingZ, 9)
		BS_END_RTTI_MEMBERS

	public:
		const String& getRTTIName() override
		{
			static String name = "ParticleVectorFieldSettings";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ParticleVectorFieldSettings;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ParticleVectorFieldSettings>();
		}
	};

	class BS_CORE_EXPORT ParticleDepthCollisionSettingsRTTI :
	public RTTIType<ParticleDepthCollisionSettings, IReflectable, ParticleDepthCollisionSettingsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(enabled, 0)
			BS_RTTI_MEMBER_PLAIN(restitution, 1)
			BS_RTTI_MEMBER_PLAIN(dampening, 2)
			BS_RTTI_MEMBER_PLAIN(radiusScale, 3)
		BS_END_RTTI_MEMBERS

	public:
		const String& getRTTIName() override
		{
			static String name = "ParticleDepthCollisionSettings";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ParticleDepthCollisionSettings;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ParticleDepthCollisionSettings>();
		}
	};

	class BS_CORE_EXPORT ParticleGpuSimulationSettingsRTTI :
	public RTTIType<ParticleGpuSimulationSettings, IReflectable, ParticleGpuSimulationSettingsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFL(vectorField, 0)
			BS_RTTI_MEMBER_PLAIN(colorOverLifetime, 1)
			BS_RTTI_MEMBER_PLAIN(sizeScaleOverLifetime, 2)
			BS_RTTI_MEMBER_REFL(depthCollision, 3)
			BS_RTTI_MEMBER_PLAIN(acceleration, 4)
			BS_RTTI_MEMBER_PLAIN(drag, 5)
		BS_END_RTTI_MEMBERS

	public:
		const String& getRTTIName() override
		{
			static String name = "ParticleGpuSimulationSettings";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ParticleGpuSimulationSettings;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ParticleGpuSimulationSettings>();
		}
	};

	class BS_CORE_EXPORT ParticleSystemSettingsRTTI :
		public RTTIType<ParticleSystemSettings, IReflectable, ParticleSystemSettingsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(simulationSpace, 0)
			BS_RTTI_MEMBER_PLAIN(orientation, 1)
			BS_RTTI_MEMBER_PLAIN(orientationLockY, 2)
			BS_RTTI_MEMBER_PLAIN(orientationPlaneNormal, 3)
			BS_RTTI_MEMBER_PLAIN(sortMode, 4)
			BS_RTTI_MEMBER_PLAIN(duration, 5)
			BS_RTTI_MEMBER_PLAIN(isLooping, 6)
			BS_RTTI_MEMBER_PLAIN(maxParticles, 7)
			BS_RTTI_MEMBER_PLAIN(useAutomaticSeed, 8)
			//BS_RTTI_MEMBER_PLAIN(gravityScale, 9)
			BS_RTTI_MEMBER_PLAIN(manualSeed, 10)
			BS_RTTI_MEMBER_REFL(material, 11)
			BS_RTTI_MEMBER_PLAIN(useAutomaticBounds, 12)
			BS_RTTI_MEMBER_PLAIN(customBounds, 13)
			BS_RTTI_MEMBER_PLAIN(renderMode, 14)
			BS_RTTI_MEMBER_REFL(mesh, 15)
		BS_END_RTTI_MEMBERS

	public:
		const String& getRTTIName() override
		{
			static String name = "ParticleSystemSettings";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ParticleSystemSettings;
		}

		SPtr<IReflectable> newRTTIObject() override
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
		void onDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			// Note: Since this is a CoreObject I should call initialize() right after deserialization,
			// but since this specific type is used in Components we delay initialization until Component
			// itself does it. Keep this is mind in case this ever needs to be deserialized for non-Component
			// purposes (you'll need to call initialize manually).
		}

		const String& getRTTIName() override
		{
			static String name = "ParticleSystem";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ParticleSystem;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return ParticleSystem::createEmpty();
		}
	};

	/** @} */
	/** @endcond */
}
