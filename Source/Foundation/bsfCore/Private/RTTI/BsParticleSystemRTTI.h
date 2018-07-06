//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Particles/BsParticleSystem.h"
#include "Particles/BsParticleEvolver.h"
#include "Private/RTTI/BsColorGradientRTTI.h"
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
		BS_END_RTTI_MEMBERS

		// TODO - Serialize shape
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
	class BS_CORE_EXPORT ParticleSystemRTTI : public RTTIType<ParticleSystem, IReflectable, ParticleSystemRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mSimulationSpace, 0)
			BS_RTTI_MEMBER_PLAIN(mOrientation, 1)
			BS_RTTI_MEMBER_PLAIN(mOrientationLockY, 2)
			BS_RTTI_MEMBER_PLAIN(mOrientationPlane, 3)
			BS_RTTI_MEMBER_PLAIN(mSortMode, 4)
			BS_RTTI_MEMBER_PLAIN(mDuration, 5)
			BS_RTTI_MEMBER_PLAIN(mIsLooping, 6)
			BS_RTTI_MEMBER_PLAIN(mMaxParticles, 7)
			BS_RTTI_MEMBER_PLAIN(mUseAutomaticSeed, 8)
			BS_RTTI_MEMBER_PLAIN(mGravityScale, 9)
			BS_RTTI_MEMBER_PLAIN(mManualSeed, 10)
			BS_RTTI_MEMBER_REFL(mMaterial, 11)
		BS_END_RTTI_MEMBERS

		// TODO - Serialize emitters & evolvers
	public:
		void onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
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
