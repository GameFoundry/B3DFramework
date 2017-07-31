//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsIReflectable.h"
#include "BsCoreObject.h"
#include "BsVector3.h"
#include "BsQuaternion.h"
#include "BsSphere.h"

namespace bs
{
	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	/** Light type that determines how is light information parsed by the renderer and other systems. */
	enum BS_SCRIPT_EXPORT() class ReflectionProbeType
	{
		/**
		 * Reflection probe cubemap is generated, and box extents are used for calculating influence ranges and box 
		 * geometry.
		 */
		Box,
		/** 
		 * Reflection probe cubemap is generated, but sphere is used for calculating the influence radius and
		 * proxy geometry. 
		 */
		Sphere
	};

	/**	Signals which portion of a reflection probe is dirty. */
	enum class ReflectionProbeDirtyFlag
	{
		Transform = 0x01,
		Everything = 0x02
	};

	/** @} */

	/** @addtogroup Implementation
	 *  @{
	 */

	/** Base class for both core and sim thread implementations of a reflection probe. */
	class BS_CORE_EXPORT ReflectionProbeBase
	{
	public:
		ReflectionProbeBase();
		ReflectionProbeBase(ReflectionProbeType type, float radius, const Vector3& extents);
		virtual ~ReflectionProbeBase() { }

		/**	Returns the position of the probe, in world space. */
		Vector3 getPosition() const { return mPosition; }

		/**	Sets the position of the probe, in world space. */
		void setPosition(const Vector3& position) 
			{ mPosition = position; _markCoreDirty(ReflectionProbeDirtyFlag::Transform); updateBounds(); }

		/**	Returns the rotation of the probe, in world space. */
		Quaternion getRotation() const { return mRotation; }

		/**	Sets the rotation of the probe, in world space. */
		void setRotation(const Quaternion& rotation) 
			{ mRotation = rotation; _markCoreDirty(ReflectionProbeDirtyFlag::Transform); updateBounds(); }

		/**	Returns the scale of the probe. */
		Vector3 getScale() const { return mScale; }

		/**	Sets the scale of the probe. */
		void setScale(const Vector3& scale)
			{ mScale = scale; _markCoreDirty(ReflectionProbeDirtyFlag::Transform); updateBounds(); }

		/**	Returns the type of the probe. */
		ReflectionProbeType getType() const { return mType; }

		/**	Changes the type of the probe. */
		void setType(ReflectionProbeType type) { mType = type; _markCoreDirty(); updateBounds(); }

		/** Returns the radius of a sphere reflection probe. Determines range of influence. */
		float getRadius() const { return mRadius * std::max(std::max(mScale.x, mScale.y), mScale.z); }

		/** Sets the radius of a sphere reflection probe. */
		void setRadius(float radius) { mRadius = radius; _markCoreDirty(); updateBounds(); }

		/** Returns the extents of a box reflection probe. */
		Vector3 getExtents() const { return mExtents * mScale; }

		/** Sets the extents of a box reflection probe. Determines range of influence. */
		void setExtents(const Vector3& extents) { mExtents = extents; _markCoreDirty(); updateBounds(); }

		/**	Returns world space bounds that completely encompass the probe's area of influence. */
		Sphere getBounds() const { return mBounds; }
		
		/** 
		 * Sets a distance that will be used for fading out the box reflection probe with distance. By default it
		 * is equal to one, and can never be less than one. Only relevant for box probes.
		 */
		void setTransitionDistance(float distance) { mTransitionDistance = std::max(1.0f, distance); }

		/** Retrieves transition distance set by setTransitionDistance(). */
		float getTransitionDistance() const { return mTransitionDistance; }

		/**	Checks whether the probe should be used or not. */
		bool getIsActive() const { return mIsActive; }

		/**	Sets whether the probe should be used or not. */
		void setIsActive(bool active) { mIsActive = active; _markCoreDirty(); }

		/** Returns an identifier that uniquely identifies the probe. */
		const String& getUUID() const { return mUUID; }

		/** 
		 * Marks the simulation thread object as dirty and notifies the system its data should be synced with its core 
		 * thread counterpart. 
		 */
		virtual void _markCoreDirty(ReflectionProbeDirtyFlag flags = ReflectionProbeDirtyFlag::Everything) { }

	protected:
		/** Updates the internal bounds for the probe. Call this whenever a property affecting the bounds changes. */
		void updateBounds();

		Vector3 mPosition; /**< World space position. */
		Quaternion mRotation; /**< World space rotation. */
		Vector3 mScale; /** Scale applied to radius/extents. */

		ReflectionProbeType mType; /**< Type of probe that determines how are the rest of the parameters interpreted. */
		float mRadius; /**< Radius used for sphere reflection probes. */
		Vector3 mExtents; /**< Extents used by box reflection probe. */
		float mTransitionDistance; /**< Extra distance to used for fading out box probes. */
		String mUUID; /**< Identifier that uniquely identifies the probe. */

		bool mIsActive; /**< Whether the light should be rendered or not. */
		Sphere mBounds; /**< Sphere that bounds the light area of influence. */
	};

	/** @} */
	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	namespace ct 
	{
		class RendererTask;
		class ReflectionProbe; 
	}

	/**
	 * Specifies a location at which a pre-computed texture containing scene radiance will be generated. This texture will
	 * then be used by the renderer to provide specular reflections.
	 */
	class BS_CORE_EXPORT ReflectionProbe : public IReflectable, public CoreObject, public ReflectionProbeBase
	{
	public:
		/** 
		 * Allows you assign a custom texture to use as a reflection map. This will disable automatic generation of
		 * reflections. To re-enable auto-generation call this with a null parameter.
		 */
		void setCustomTexture(const HTexture& texture) { mCustomTexture = texture; filter(); }

		/** Gets the custom texture assigned through setCustomTexture(). */
		HTexture getCustomTexture() const { return mCustomTexture; }

		/** 
		 * Returns a pre-filtered texture that is generated either from the provided custom texture, or from scene capture.
		 */
		SPtr<Texture> getFilteredTexture() const { return mFilteredTexture; }

		/** 
		 * Captures the scene at the current location and generates a filtered reflection cubemap. No action is taken
		 * if a custom texture is set.
		 */
		void capture();

		/** 
		 * Filters the custom texture, making it usable for rendering. Called automatically when custom texture changes. If 
		 * no custom texture is set, no action is taken.
		 */
		void filter();

		/**	Retrieves an implementation of the reflection probe usable only from the core thread. */
		SPtr<ct::ReflectionProbe> getCore() const;

		/** Returns the hash value that can be used to identify if the internal data needs an update. */
		UINT32 _getLastModifiedHash() const { return mLastUpdateHash; }

		/**	Sets the hash value that can be used to identify if the internal data needs an update. */
		void _setLastModifiedHash(UINT32 hash) { mLastUpdateHash = hash; }

		/**
		 * Updates internal transform values from the specified scene object, in case that scene object's transform changed
		 * since the last call.
		 *
		 * @note	Assumes the same scene object will be provided every time.
		 */
		void _updateTransform(const HSceneObject& parent);

		/**
		 * Creates a new sphere reflection probe.
		 *
		 * @param[in]	radius	Radius in which the reflection probe will be rendered within. 
		 * @returns				New reflection probe.
		 */
		static SPtr<ReflectionProbe> createSphere(float radius);

		/**
		 * Creates a new box reflection probe.
		 *
		 * @param[in]	extents	Extents of the box in which the reflection probe will be rendered within. 
		 * @returns				New reflection probe.
		 */
		static SPtr<ReflectionProbe> createBox(const Vector3& extents);

	protected:
		ReflectionProbe(ReflectionProbeType type, float radius, const Vector3& extents);

		/** @copydoc CoreObject::createCore */
		SPtr<ct::CoreObject> createCore() const override;

		/** @copydoc ReflectionProbeBase::_markCoreDirty */
		void _markCoreDirty(ReflectionProbeDirtyFlag flags = ReflectionProbeDirtyFlag::Everything) override;

		/** @copydoc CoreObject::syncToCore */
		CoreSyncData syncToCore(FrameAlloc* allocator) override;

		/** 
		 * Captures the scene color at current probe location and generates a filtered map. If a custom texture is set then
		 * it will be filtered, instead of capturing scene color.
		 */
		void captureAndFilter();

		/**	Creates a light with without initializing it. Used for serialization. */
		static SPtr<ReflectionProbe> createEmpty();

		UINT32 mLastUpdateHash;
		HTexture mCustomTexture;

		SPtr<ct::RendererTask> mRendererTask;
		SPtr<Texture> mFilteredTexture;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ReflectionProbeRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		ReflectionProbe(); // Serialization only
	};

	namespace ct
	{
	/** Core thread usable version of a bs::ReflectionProbe */
	class BS_CORE_EXPORT ReflectionProbe : public CoreObject, public ReflectionProbeBase
	{
	public:
		~ReflectionProbe();

		/**	Sets an ID that can be used for uniquely identifying this object by the renderer. */
		void setRendererId(UINT32 id) { mRendererId = id; }

		/**	Retrieves an ID that can be used for uniquely identifying this object by the renderer. */
		UINT32 getRendererId() const { return mRendererId; }

		/** 
		 * Returns a pre-filtered texture that is generated either from the provided custom texture, or from scene capture.
		 */
		SPtr<Texture> getFilteredTexture() const { return mFilteredTexture; }
	protected:
		friend class bs::ReflectionProbe;

		ReflectionProbe(ReflectionProbeType type, float radius, const Vector3& extents, 
			const SPtr<Texture>& filteredTexture);

		/** @copydoc CoreObject::initialize */
		void initialize() override;

		/** @copydoc CoreObject::syncToCore */
		void syncToCore(const CoreSyncData& data) override;

		UINT32 mRendererId;
		SPtr<Texture> mFilteredTexture;
	};
	}

	/** @} */
}