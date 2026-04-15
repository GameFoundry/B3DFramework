//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Scene/B3DComponent.h"
#include "CoreObject/B3DCoreObject.h"
#include "Math/B3DVector3I.h"
#include "Math/B3DTransform.h"

namespace b3d
{
	namespace render
	{
		class RendererTask;
	}

	/** @addtogroup Rendering-Internal 
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

	/** @} */

	/** @addtogroup Rendering
	 *  @{
	 */

	/**
	 * Allows you to define a volume of light probes that will be used for indirect lighting. Lighting information in the
	 * scene will be interpolated from nearby probes to calculate the amount of indirect lighting at that position. It is
	 * up to the caller to place the light probes in areas where the lighting changes in order to yield the best results.
	 *
	 * The volume can never have less than 4 probes.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) LightProbeVolume : public Component, public CoreObject
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

		/** Information about a dirty probe to be synced to the render thread counterpart. */
		struct DirtyProbeInfo
		{
			DirtyProbeInfo(u32 probeIndex = ~0u, const Vector3& position = Vector3::kZero, LightProbeFlags flags = LightProbeFlags::Clean)
				: ProbeIndex(probeIndex), Position(position), Flags(flags)
			{ }

			u32 ProbeIndex;
			Vector3 Position;
			LightProbeFlags Flags;
		};

	public:
		LightProbeVolume(const HSceneObject& parent, const AABox& volume = AABox::kUnit, const Vector3I& cellCount = Vector3I(1, 1, 1));

		/**
		 * Adds a new probe at the specified position and returns a handle to the probe. The position is relative to
		 * the volume origin.
		 */
		B3D_SCRIPT_EXPORT()
		u32 AddProbe(const Vector3& position);

		/**
		 * Removes the probe with the specified handle. Note that if this is one of the last four remaining probes in the
		 * volume it cannot be removed.
		 */
		B3D_SCRIPT_EXPORT()
		void RemoveProbe(u32 handle);

		/** Updates the position of the probe with the specified handle. */
		B3D_SCRIPT_EXPORT()
		void SetProbePosition(u32 handle, const Vector3& position);

		/** Retrieves the position of the probe with the specified handle. */
		B3D_SCRIPT_EXPORT()
		Vector3 GetProbePosition(u32 handle) const;

		/** Returns a list of all light probes in the volume. */
		B3D_SCRIPT_EXPORT()
		Vector<LightProbeInfo> GetProbes() const;

		/**
		 * Causes the information for this specific light probe to be updated. You generally want to call this when the
		 * probe is moved or the scene around the probe changes.
		 */
		B3D_SCRIPT_EXPORT()
		void RenderProbe(u32 handle);

		/**
		 * Causes the information for all lights probes to be updated. You generally want to call this if you move the
		 * entire light volume or the scene around the volume changes.
		 */
		B3D_SCRIPT_EXPORT()
		void RenderProbes();

		/**
		 * Resizes the light probe grid and inserts new light probes, if the new size is larger than previous size.
		 * New probes are inserted in a grid pattern matching the new size and density parameters.
		 *
		 * Note that shrinking the volume will not remove light probes. In order to remove probes outside of the new volume
		 * call Clip().
		 *
		 * Resize will not change the positions of current light probes. If you wish to reset all probes to the currently
		 * set grid position, call Reset().

		 * @param	volume		Axis aligned volume to be covered by the light probes.
		 * @param	cellCount	Number of grid cells to split the volume into. Minimum number of 1, in which case each
		 *						corner of the volume is represented by a single probe. Higher values subdivide the
		 *						volume in an uniform way.
		 */
		B3D_SCRIPT_EXPORT()
		void Resize(const AABox& volume, const Vector3I& cellCount = Vector3I(1, 1, 1));

		/** Removes any probes outside of the current grid volume. */
		B3D_SCRIPT_EXPORT()
		void Clip();

		/**
		 * Resets all probes to match the original grid pattern. This will reset probe positions, as well as add/remove
		 * probes as necessary, essentially losing any custom changes to the probes.
		 */
		B3D_SCRIPT_EXPORT()
		void Reset();

		/** Returns the volume that's used for adding probes in a uniform grid pattern. */
		B3D_SCRIPT_EXPORT(ExportName(GridVolume), Property(Getter))
		const AABox& GetGridVolume() const { return mVolume; }

		/** Returns the cell count that's used for determining the density of probes within a grid volume. */
		B3D_SCRIPT_EXPORT(ExportName(CellCount), Property(Getter))
		const Vector3I& GetCellCount() const { return mCellCount; }

	protected:
		friend class render::LightProbeVolume;
		struct SyncPacket;

		/** Renders the light probe data on the render thread. */
		void RunRenderProbeTask();

		/**
		 * Fetches latest SH coefficient data from the render thread. Note this method will block the caller thread until
		 * the data is fetched from the render thread. It will also force any in-progress light probe updates to finish.
		 */
		void UpdateCoefficients();

		SPtr<render::RenderProxy> CreateRenderProxy() const override;
		RenderProxySyncPacket* CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags) override;

		UnorderedMap<u32, ProbeInfo> mProbes;
		AABox mVolume = AABox::kUnit;
		Vector3I mCellCount = { 1, 1, 1 };

		u32 mNextProbeId = 0;
		SPtr<render::RendererTask> mRendererTask;

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		void Initialize() override;
		void OnCreated() override;
		void OnDestroyed() override;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class LightProbeVolumeRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;

	protected:
		LightProbeVolume(); // Serialization only
	};

	/** @} */

	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	namespace render
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

		/** Render thread counterpart of b3d::LightProbeVolume. */
		class B3D_EXPORT LightProbeVolume : public RenderProxy
		{
		public:
			~LightProbeVolume();

			/**	Sets an ID that can be used for uniquely identifying this object by the renderer. */
			void SetRendererId(u32 id) { mRendererId = id; }

			/**	Retrieves an ID that can be used for uniquely identifying this object by the renderer. */
			u32 GetRendererId() const { return mRendererId; }

			/** Returns the world space transform for the volume. */
			const Transform& GetWorldTransform() const { return mTransform; }

			/** Returns the number of light probes that are active. */
			u32 GetActiveProbeCount() const { return (u32)mProbeMap.size(); }

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
			friend class b3d::LightProbeVolume;

			LightProbeVolume(const SPtr<SceneInstance>& scene, const UnorderedMap<u32, b3d::LightProbeVolume::ProbeInfo>& probes);

			void Initialize() override;
			void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) override;

			/**
			 * Renders dirty probes and updates their SH coefficients in the local GPU buffer.
			 *
			 * @param	commandBuffer	Command buffer to encode the operations on.
			 * @param	maxProbeCount	Maximum number of probes to render. Set to zero to render all dirty probes. Limiting the
			 *							number of probes allows the rendering to be distributed over multiple frames.
			 * @return					True if there are no more dirty probes to process.
			 */
			bool RenderProbes(GpuCommandBuffer& commandBuffer, u32 maxProbeCount);

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

			Transform mTransform;
			bool mActive = true;
			SPtr<SceneInstance> mSceneInstance;
		};
	} // namespace render

	/** @} */
} // namespace b3d
