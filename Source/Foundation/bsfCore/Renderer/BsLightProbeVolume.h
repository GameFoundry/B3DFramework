//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
		Empty,
		Clean,
		Dirty,
		Removed
	};

	/** @} */

	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	namespace ct
	{
		class LightProbeVolume;
	}

	/** Vector representing spherical harmonic coefficients for a light probe. */
	struct LightProbeSHCoefficients
	{
		LightProbeSHCoefficients()
			: CoeffsR(), CoeffsG(), CoeffsB()
		{}

		float CoeffsR[9];
		float CoeffsG[9];
		float CoeffsB[9];
	};

	/** SH coefficients for a specific light probe, and its handle. */
	struct LightProbeCoefficientInfo
	{
		u32 Handle;
		LightProbeSHCoefficients Coefficients;
	};

	/** Information about a single probe in the light probe volume. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering), ExportAsStruct(true)) LightProbeInfo
	{
		u32 Handle;
		Vector3 Position;

		B3D_SCRIPT_EXPORT(Exclude(true))
		LightProbeSHCoefficients ShCoefficients;
	};

	/**
	 * Allows you to define a volume of light probes that will be used for indirect lighting. Lighting information in the
	 * scene will be interpolated from nearby probes to calculate the amount of indirect lighting at that position. It is
	 * up to the caller to place the light probes in areas where the lighting changes in order to yield the best results.
	 *
	 * The volume can never have less than 4 probes.
	 */
	class B3D_CORE_EXPORT LightProbeVolume : public IReflectable, public CoreObject, public SceneActor
	{
		/** Internal information about a single light probe. */
		struct ProbeInfo
		{
			ProbeInfo() = default;

			ProbeInfo(LightProbeFlags flags, const Vector3& position)
				: Flags(flags), Position(position)
			{}

			LightProbeFlags Flags;
			Vector3 Position;

			/** Coefficients are only valid directly after deserialization, or after updateCoefficients() is called. */
			LightProbeSHCoefficients Coefficients;
		};

	public:
		~LightProbeVolume();

		/**
		 * Adds a new probe at the specified position and returns a handle to the probe. The position is relative to
		 * the volume origin.
		 */
		u32 AddProbe(const Vector3& position);

		/** Updates the position of the probe with the specified handle. */
		void SetProbePosition(u32 handle, const Vector3& position);

		/** Retrieves the position of the probe with the specified handle. */
		Vector3 GetProbePosition(u32 handle) const;

		/**
		 * Removes the probe with the specified handle. Note that if this is one of the last four remaining probes in the
		 * volume it cannot be removed.
		 */
		void RemoveProbe(u32 handle);

		/** Returns a list of positions of all light probes in the volume. */
		Vector<LightProbeInfo> GetProbes() const;

		/**
		 * Causes the information for this specific light probe to be updated. You generally want to call this when the
		 * probe is moved or the scene around the probe changes.
		 */
		void RenderProbe(u32 handle);

		/**
		 * Causes the information for all lights probes to be updated. You generally want to call this if you move the
		 * entire light volume or the scene around the volume changes.
		 */
		void RenderProbes();

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
		void Resize(const AABox& volume, const Vector3I& cellCount = Vector3I(1, 1, 1));

		/** Removes any probes outside of the current grid volume. */
		void Clip();

		/**
		 * Resets all probes to match the original grid pattern. This will reset probe positions, as well as add/remove
		 * probes as necessary, essentially losing any custom changes to the probes.
		 */
		void Reset();

		/** Returns the volume that's used for adding probes in a uniform grid pattern. */
		const AABox& GetGridVolume() const { return mVolume; }

		/** Returns the cell count that's used for determining the density of probes within a grid volume. */
		const Vector3I& GetCellCount() const { return mCellCount; }

		/**	Retrieves an implementation of the object usable only from the core thread. */
		SPtr<ct::LightProbeVolume> GetCore() const;

		/**
		 * Creates a new light volume with probes aligned in a grid pattern.
		 *
		 * @param[in]	volume		Axis aligned volume to be covered by the light probes.
		 * @param[in]	cellCount	Number of grid cells to split the volume into. Minimum number of 1, in which case each
		 *							corner of the volume is represented by a single probe. Higher values subdivide the
		 *							volume in an uniform way.
		 */
		static SPtr<LightProbeVolume> Create(const AABox& volume = AABox::kUnitBox, const Vector3I& cellCount = Vector3I(1, 1, 1));

	protected:
		friend class ct::LightProbeVolume;

		LightProbeVolume(const AABox& volume, const Vector3I& cellCount);

		/** Renders the light probe data on the core thread. */
		void RunRenderProbeTask();

		/**
		 * Fetches latest SH coefficient data from the core thread. Note this method will block the caller thread until
		 * the data is fetched from the core thread. It will also force any in-progress light probe updates to finish.
		 */
		void UpdateCoefficients();

		SPtr<ct::CoreObject> CreateCore() const override;
		void MarkCoreDirtyInternal(ActorDirtyFlag dirtFlags = ActorDirtyFlag::Everything) override;
		CoreSyncData SyncToCore(FrameAlloc* allocator) override;

		/**	Creates a light volume with without initializing it. Used for serialization. */
		static SPtr<LightProbeVolume> CreateEmpty();

	private:
		UnorderedMap<u32, ProbeInfo> mProbes;
		AABox mVolume = AABox::kUnitBox;
		Vector3I mCellCount = { 1, 1, 1 };

		u32 mNextProbeId = 0;
		SPtr<ct::RendererTask> mRendererTask;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class LightProbeVolumeRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const;

	protected:
		LightProbeVolume() = default; // Serialization only
	};

	namespace ct
	{
		/** Information about a single light probe in a light probe volume. */
		struct LightProbeInfo
		{
			/** Unique handle representing the probe. Always remains the same. */
			u32 Handle;

			/** Flags representing the current state of the probe. */
			LightProbeFlags Flags;

			/** Index into the GPU buffer where probe coefficients are stored. -1 if not assigned. Transient. */
			u32 BufferIdx;
		};

		/** Core thread usable version of bs::LightProbeVolume. */
		class B3D_CORE_EXPORT LightProbeVolume : public CoreObject, public SceneActor
		{
		public:
			~LightProbeVolume();

			/**	Sets an ID that can be used for uniquely identifying this object by the renderer. */
			void SetRendererId(u32 id) { mRendererId = id; }

			/**	Retrieves an ID that can be used for uniquely identifying this object by the renderer. */
			u32 GetRendererId() const { return mRendererId; }

			/** Returns the number of light probes that are active. */
			u32 GetNumActiveProbes() const { return (u32)mProbeMap.size(); }

			/** Returns a list of positions for all light probes. Only the first getNumActiveProbes() entries are active. */
			const Vector<Vector3>& GetLightProbePositions() const { return mProbePositions; }

			/**
			 * Returns non-positional information about all light probes. Only the first getNumActiveProbes() entries are
			 * active.
			 */
			const Vector<LightProbeInfo>& GetLightProbeInfos() const { return mProbeInfos; }

			/** Populates the vector with SH coefficients for each light probe. Involves reading the GPU buffer. */
			void GetProbeCoefficients(Vector<LightProbeCoefficientInfo>& output) const;

			/** Returns the texture containing SH coefficients for all probes in the volume. */
			SPtr<Texture> GetCoefficientsTexture() const { return mCoefficients; }

		protected:
			friend class bs::LightProbeVolume;

			LightProbeVolume(const UnorderedMap<u32, bs::LightProbeVolume::ProbeInfo>& probes);

			void Initialize() override;
			void SyncToCore(const CoreSyncData& data, FrameAlloc& allocator) override;

			/**
			 * Renders dirty probes and updates their SH coefficients in the local GPU buffer.
			 *
			 * @param	commandBuffer	Command buffer to encode the operations on.
			 * @param	maxProbes		Maximum number of probes to render. Set to zero to render all dirty probes. Limiting the
			 *							number of probes allows the rendering to be distributed over multiple frames.
			 * @return					True if there are no more dirty probes to process.
			 */
			bool RenderProbes(GpuCommandBuffer& commandBuffer, u32 maxProbes);

			/**
			 * Resizes the internal texture that stores light probe SH coefficients, to the specified size (in the number
			 * of probes).
			 */
			void ResizeCoefficientTexture(u32 count);

			u32 mRendererId = 0;
			UnorderedMap<u32, u32> mProbeMap; // Map from static indices to compact list of probes
			u32 mFirstDirtyProbe = 0;

			Vector<Vector3> mProbePositions;
			Vector<LightProbeInfo> mProbeInfos;

			// Contains SH coefficients for the probes
			SPtr<Texture> mCoefficients;
			u32 mCoeffBufferSize = 0;

			// Temporary until initialization
			Vector<LightProbeSHCoefficients> mInitCoefficients;
		};
	} // namespace ct

	/** @} */
} // namespace bs
