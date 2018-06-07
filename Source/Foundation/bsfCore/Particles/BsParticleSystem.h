//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Image/BsColor.h"
#include "Math/BsVector3.h"
#include "Math/BsRandom.h"
#include "Scene/BsSceneActor.h"
#include "CoreThread/BsCoreObject.h"
#include "Image/BsPixelData.h"
#include "Math/BsAABox.h"
#include "Particles/BsParticleEmitter.h"
#include "Particles/BsParticleEvolver.h"

namespace bs 
{
	class SkeletonMask;
	class ParticleSystem;
	class ParticleEmitter;
	class ParticleEvolver;
	class ParticleSet;

	namespace ct { class ParticleSystem; }

	/** @addtogroup Particles
	 *  @{
	 */

	/** Possible orientations when rendering billboard particles. */
	enum class ParticleOrientation
	{
		/** Orient towards view (camera) plane. */
		ViewPlane,

		/** Orient towards view (camera) position. */
		ViewPosition,

		/** Orient with normal parallel to a specific axis. */
		Axis
	};

	/** 
	 * Controls spawning, evolution and rendering of particles. Particles can be 2D or 3D, with a variety of rendering
	 * options. Particle system should be used for rendering objects that cannot properly be represented using static or
	 * animated meshes, like liquids, smoke or flames.
	 * 
	 * The particle system requires you to specify at least one ParticleEmitter, which controls how are new particles
	 * generated. You will also want to specify one or more ParticleEvolver%s, which change particle properties over time.
	 */
	class BS_CORE_EXPORT ParticleSystem final : public IReflectable, public CoreObject, public SceneActor, public INonCopyable
	{
	public:
		~ParticleSystem() final;

		/** Registers a new particle emitter. */
		void addEmitter(UPtr<ParticleEmitter> emitter)
		{
			mEmitters.push_back(std::move(emitter));
		}

		/** Registers a new particle evolver. */
		void addEvolver(UPtr<ParticleEvolver> evolver)
		{
			mEvolvers.push_back(std::move(evolver));
		}

		/** Returns the number of particle emitters present in this system. */
		UINT32 getNumEmitters() const { return (UINT32)mEmitters.size(); }

		/** Returns the number of particle evolvers present in this system. */
		UINT32 getNumEvolvers() const { return (UINT32)mEvolvers.size(); }

		/** 
		 * Returns the particle emitter present at the specified sequential index. Returns null if provided index is 
		 * invalid. 
		 */
		ParticleEmitter* getEmitter(UINT32 idx)
		{
			if(idx >= (UINT32)mEmitters.size())
				return nullptr;

			return mEmitters[idx].get();
		}
		/** 
		 * Returns the particle evolver present at the specified sequential index. Returns null if provided index is 
		 * invalid. 
		 */
		ParticleEvolver* getEvolver(UINT32 idx)
		{
			if(idx >= (UINT32)mEvolvers.size())
				return nullptr;

			return mEvolvers[idx].get();
		}

		/** Removes a particle emitter. */
		void removeEmitter(ParticleEmitter* emitter)
		{
			const auto iterFind = std::find_if(mEmitters.begin(), mEmitters.end(), 
				[emitter](const UPtr<ParticleEmitter>& curEmitter)
			{
				return curEmitter.get() == emitter;
				
			});

			if(iterFind != mEmitters.end())
				mEmitters.erase(iterFind);
		}

		/** Removes a particle evolver. */
		void removeEvolver(ParticleEvolver* evolver)
		{
			const auto iterFind = std::find_if(mEvolvers.begin(), mEvolvers.end(), 
				[evolver](const UPtr<ParticleEvolver>& curEvolver)
			{
				return curEvolver.get() == evolver;
				
			});

			if(iterFind != mEvolvers.end())
				mEvolvers.erase(iterFind);
		}

		/** Material to render the particles with. */
		void setMaterial(const HMaterial& material)
		{
			mMaterial = material;
			_markCoreDirty();
		}

		/** @copydoc setMaterial */
		const HMaterial& getMaterial() const { return mMaterial; }

		/**	Retrieves an implementation of the particle system usable only from the core thread. */
		SPtr<ct::ParticleSystem> getCore() const;

		/** Creates a new empty ParticleSystem object. */
		static SPtr<ParticleSystem> create();

		/** 
		 * @name Internal
		 */

		/** 
		 * Updates the particle simulation by advancing it by @p timeDelta. New state will be updated in the internal
		 * ParticleSet.
		 */
		void _simulate(float timeDelta);

		/** 
		 * Calculates the bounds of all the particles in the system. Should be called after a call to _simulate() to get
		 * up-to-date bounds.
		 */
		AABox _calculateBounds() const;

		/** @} */
	private:
		friend class ParticleManager;

		friend class ParticleSystemRTTI;

		ParticleSystem();

		/** @copydoc CoreObject::createCore */
		SPtr<ct::CoreObject> createCore() const override;

		/** @copydoc SceneActor::_markCoreDirty */
		void _markCoreDirty(ActorDirtyFlag flag = ActorDirtyFlag::Everything) override;

		/** @copydoc CoreObject::syncToCore */
		CoreSyncData syncToCore(FrameAlloc* allocator) override;

		/**	Creates a new ParticleSystem instance without initializing it. */
		static SPtr<ParticleSystem> createEmpty();

		UINT32 mId = 0;

		Vector<UPtr<ParticleEmitter>> mEmitters;
		Vector<UPtr<ParticleEvolver>> mEvolvers;
		HMaterial mMaterial;

		Random mRandom;
		ParticleSet* mParticleSet;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ParticleSystemRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */

	/** @addtogroup Particles-Internal
	 *  @{
	 */

	namespace ct
	{
		/** 
		 * Contains a set of textures used for rendering a particle system. Each pixel in a texture represent properties
		 * of a single particle.
		 */
		struct ParticleTextures
		{
			SPtr<Texture> positionAndRotation;
			SPtr<Texture> color;
			SPtr<Texture> size;
		};

		/** Core thread counterpart of bs::ParticleSystem. */
		class BS_CORE_EXPORT ParticleSystem final : public CoreObject, public SceneActor
		{
		public:
			~ParticleSystem();

			/**	Sets an ID that can be used for uniquely identifying this object by the renderer. */
			void setRendererId(UINT32 id) { mRendererId = id; }

			/**	Retrieves an ID that can be used for uniquely identifying this object by the renderer. */
			UINT32 getRendererId() const { return mRendererId; }

			/** 
			 * Returns an ID that uniquely identifies the particle system. Can be used for locating evaluated particle 
			 * system render data in the structure output by the ParticlesManager. 
			 */
			UINT32 getId() const { return mId; }

			/** Material to render the particles with. */
			void setMaterial(const SPtr<Material>& material) { mMaterial = material; }

			/** @copydoc setMaterial() */
			const SPtr<Material>& getMaterial() const { return mMaterial; }

			/** @copydoc CoreObject::initialize */
			void initialize() override;
		private:
			friend class bs::ParticleSystem;

			ParticleSystem(UINT32 id)
				:mId(id)
			{ }

			/** @copydoc CoreObject::syncToCore */
			void syncToCore(const CoreSyncData& data) override;

			UINT32 mRendererId = 0;
			UINT32 mId;

			SPtr<Material> mMaterial;
		};
	}

	/** @} */
}
