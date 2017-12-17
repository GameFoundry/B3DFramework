//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "CoreThread/BsCoreObject.h"
#include "Math/BsAABox.h"
#include "Math/BsVector3.h"
#include "Math/BsQuaternion.h"
#include "Math/BsVector3I.h"
#include "Scene/BsSceneActor.h"

namespace bs
{
	namespace ct
	{
		class RendererTask;
	}

	/** @addtogroup Implementation
	 *  @{
	 */

	/** Potential states the light probe can be in. */
	enum class LightProbeFlags
	{
		 Empty, Clean, Dirty, Removed 
	};

	/** @} */

	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	namespace ct { class LightProbeVolume; }

	/** Vector representing spherical harmonic coefficients for a light probe. */
	struct LightProbeSHCoefficients
	{
		LightProbeSHCoefficients()
			:coeffsR(), coeffsG(), coeffsB()
		{ }

		float coeffsR[9];
		float coeffsG[9];
		float coeffsB[9];
	};

	/** SH coefficients for a specific light probe, and its handle. */
	struct LightProbeCoefficientInfo
	{
		UINT32 handle;
		LightProbeSHCoefficients coefficients;
	};

	/** Information about a single probe in the light probe volume. */
	struct BS_SCRIPT_EXPORT(m:Rendering,pl:true) LightProbeInfo
	{
		UINT32 handle;
		Vector3 position;

		BS_SCRIPT_EXPORT(ex:true)
		LightProbeSHCoefficients shCoefficients;
	};

	/** 
	 * Allows you to define a volume of light probes that will be used for indirect lighting. Lighting information in the
	 * scene will be interpolated from nearby probes to calculate the amount of indirect lighting at that position. It is
	 * up to the caller to place the light probes in areas where the lighting changes in order to yield the best results.
	 *
	 * The volume can never have less than 4 probes.
	 */
	class BS_CORE_EXPORT LightProbeVolume : public IReflectable, public CoreObject, public SceneActor
	{
		/** Internal information about a single light probe. */
		struct ProbeInfo
		{
			ProbeInfo() {}
			ProbeInfo(LightProbeFlags flags, const Vector3& position)
				:flags(flags), position(position)
			{ }

			LightProbeFlags flags;
			Vector3 position;

			/** Coefficients are only valid directly after deserialization, or after updateCoefficients() is called. */
			LightProbeSHCoefficients coefficients;
		};
	public:
		~LightProbeVolume();

		/** Adds a new probe at the specified position and returns a handle to the probe. */
		UINT32 addProbe(const Vector3& position);

		/** Updates the position of the probe with the specified handle. */
		void setProbePosition(UINT32 handle, const Vector3& position);

		/** Retrieves the position of the probe with the specified handle. */
		Vector3 getProbePosition(UINT32 handle) const;

		/** 
		 * Removes the probe with the specified handle. Note that if this is one of the last four remaining probes in the
		 * volume it cannot be removed.
		 */
		void removeProbe(UINT32 handle);

		/** Returns a list of positions of all light probes in the volume. */
		Vector<LightProbeInfo> getProbes() const;

		/**
		 * Causes the information for this specific light probe to be updated. You generally want to call this when the
		 * probe is moved or the scene around the probe changes.
		 */
		void renderProbe(UINT32 handle);

		/**
		 * Causes the information for all lights probes to be updated. You generally want to call this if you move the
		 * entire light volume or the scene around the volume changes.
		 */
		void renderProbes();

		/** 
		 * Resizes the light probe grid and inserts new light probes, if the new size is larger than previous size.
		 * New probes are inserted in a grid pattern matching the new size and density parameters. 
		 * 
		 * Note that shrinking the volume will not remove light probes. In order to remove probes outside of the new volume
		 * call clip().
		 * 
		 * Resize will not change the positions of current light probes. If you wish to reset all probes to the currently
		 * set grid position, call reset().

		 * @param[in]	volume		Axis aligned volume to be covered by the light probes.
		 * @param[in]	cellCount	Number of grid cells to split the volume into. Minimum number of 1, in which case each
		 *							corner of the volume is represented by a single probe. Higher values subdivide the
		 *							volume in an uniform way.
		 */
		void resize(const AABox& volume, const Vector3I& cellCount = Vector3I(1, 1, 1));

		/** Removes any probes outside of the current grid volume. */
		void clip();

		/** 
		 * Resets all probes to match the original grid pattern. This will reset probe positions, as well as add/remove
		 * probes as necessary, essentially losing any custom changes to the probes.
		 */
		void reset();

		/** Returns the volume that's used for adding probes in a uniform grid pattern. */
		const AABox& getGridVolume() const { return mVolume; }

		/** Returns the cell count that's used for determining the density of probes within a grid volume. */
		const Vector3I& getCellCount() const { return mCellCount; }

		/**	Retrieves an implementation of the object usable only from the core thread. */
		SPtr<ct::LightProbeVolume> getCore() const;

		/**
		 * Creates a new light volume with probes aligned in a grid pattern.
		 * 
		 * @param[in]	volume		Axis aligned volume to be covered by the light probes.
		 * @param[in]	cellCount	Number of grid cells to split the volume into. Minimum number of 1, in which case each
		 *							corner of the volume is represented by a single probe. Higher values subdivide the
		 *							volume in an uniform way.
		 */
		static SPtr<LightProbeVolume> create(const AABox& volume = AABox::UNIT_BOX, 
			const Vector3I& cellCount = Vector3I(1, 1, 1));
	protected:
		friend class ct::LightProbeVolume;

		LightProbeVolume(const AABox& volume, const Vector3I& cellCount);

		/** Renders the light probe data on the core thread. */
		void runRenderProbeTask();

		/** 
		 * Fetches latest SH coefficient data from the core thread. Note this method will block the caller thread until
		 * the data is fetched from the core thread. It will also force any in-progress light probe updates to finish.
		 */
		void updateCoefficients();

		/** @copydoc CoreObject::createCore */
		SPtr<ct::CoreObject> createCore() const override;

		/** @copydoc SceneActor::_markCoreDirty */
		void _markCoreDirty(ActorDirtyFlag dirtFlags = ActorDirtyFlag::Everything) override;

		/** @copydoc CoreObject::syncToCore */
		CoreSyncData syncToCore(FrameAlloc* allocator) override;

		/**	Creates a light volume with without initializing it. Used for serialization. */
		static SPtr<LightProbeVolume> createEmpty();

	private:
		UnorderedMap<UINT32, ProbeInfo> mProbes;
		AABox mVolume = AABox::UNIT_BOX;
		Vector3I mCellCount;

		UINT32 mNextProbeId = 0;
		SPtr<ct::RendererTask> mRendererTask;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class LightProbeVolumeRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		LightProbeVolume(); // Serialization only
	};

	namespace ct
	{
	/** Information about a single light probe in a light probe volume. */
	struct LightProbeInfo
	{
		/** Unique handle representing the probe. Always remains the same. */
		UINT32 handle;

		/** Flags representing the current state of the probe. */
		LightProbeFlags flags;

		/** Index into the GPU buffer where probe coefficients are stored. -1 if not assigned. Transient. */
		UINT32 bufferIdx; 
	};

	/** Core thread usable version of bs::LightProbeVolume. */
	class BS_CORE_EXPORT LightProbeVolume : public CoreObject, public SceneActor
	{
	public:
		~LightProbeVolume();

		/**	Sets an ID that can be used for uniquely identifying this object by the renderer. */
		void setRendererId(UINT32 id) { mRendererId = id; }

		/**	Retrieves an ID that can be used for uniquely identifying this object by the renderer. */
		UINT32 getRendererId() const { return mRendererId; }

		/** Returns the number of light probes that are active. */
		UINT32 getNumActiveProbes() const { return (UINT32)mProbeMap.size(); }

		/** Returns a list of positions for all light probes. Only the first getNumActiveProbes() entries are active. */
		const Vector<Vector3>& getLightProbePositions() const { return mProbePositions; }

		/** 
		 * Returns non-positional information about all light probes. Only the first getNumActiveProbes() entries are 
		 * active. 
		 */
		const Vector<LightProbeInfo>& getLightProbeInfos() const { return mProbeInfos; }

		/** Populates the vector with SH coefficients for each light probe. Involves reading the GPU buffer. */
		void getProbeCoefficients(Vector<LightProbeCoefficientInfo>& output) const;

		/** Returns the texture containing SH coefficients for all probes in the volume. */
		SPtr<Texture> getCoefficientsTexture() const { return mCoefficients; }
	protected:
		friend class bs::LightProbeVolume;

		LightProbeVolume(const UnorderedMap<UINT32, bs::LightProbeVolume::ProbeInfo>& probes);

		/** @copydoc CoreObject::initialize */
		void initialize() override;

		/** @copydoc CoreObject::syncToCore */
		void syncToCore(const CoreSyncData& data) override;

		/** 
		 * Renders dirty probes and updates their SH coefficients in the local GPU buffer. 
		 *
		 * @param[in]	maxProbes	Maximum number of probes to render. Set to zero to render all dirty probes. Limiting the
		 *							number of probes allows the rendering to be distributed over multiple frames.
		 * @return					True if there are no more dirty probes to process.
		 */
		bool renderProbes(UINT32 maxProbes);

		/** 
		 * Resizes the internal texture that stores light probe SH coefficients, to the specified size (in the number
		 * of probes). 
		 */
		void resizeCoefficientTexture(UINT32 count);

		UINT32 mRendererId = 0;
		UnorderedMap<UINT32, UINT32> mProbeMap; // Map from static indices to compact list of probes
		UINT32 mFirstDirtyProbe = 0;

		Vector<Vector3> mProbePositions;
		Vector<LightProbeInfo> mProbeInfos;

		// Contains SH coefficients for the probes
		SPtr<Texture> mCoefficients;
		UINT32 mCoeffBufferSize = 0;

		// Temporary until initialization
		Vector<LightProbeSHCoefficients> mInitCoefficients;
	};
	}

	/** @} */
}
