//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsImportOptions.h"
#include "BsAnimationClip.h"

namespace BansheeEngine
{
	/** @addtogroup Importer
	 *  @{
	 */

	/** Controls what type of collision mesh should be imported during mesh import. */
	enum class CollisionMeshType
	{
		None, /**< No collision mesh will be imported. */
		Normal, /**< Normal triangle mesh will be imported. */
		Convex /**< A convex hull will be generated from the source mesh. */
	};

	/** Information about how to split an AnimationClip into multiple separate clips. */
	struct BS_CORE_EXPORT AnimationSplitInfo : IReflectable
	{
		AnimationSplitInfo() { }

		String name;
		UINT32 startFrame = 0;
		UINT32 endFrame = 0;
		bool isAdditive = false;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class AnimationSplitInfoRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** A set of animation events that will be added to an animation clip during animation import. */
	struct BS_CORE_EXPORT ImportedAnimationEvents : IReflectable
	{
		ImportedAnimationEvents() { }

		String name;
		Vector<AnimationEvent> events;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class ImportedAnimationEventsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**
	 * Contains import options you may use to control how is a mesh imported from some external format into engine format.
	 */
	class BS_CORE_EXPORT MeshImportOptions : public ImportOptions
	{
	public:
		MeshImportOptions();

		/**	Sets whether the texture data is also stored in CPU memory. */
		void setCPUReadable(bool readable) { mCPUReadable = readable; }
		
		/**	Retrieves whether the texture data is also stored in CPU memory. */
		bool getCPUReadable() const { return mCPUReadable; }

		/**	Sets a value that controls should mesh normals be imported if available. */
		void setImportNormals(bool import) { mImportNormals = import; }

		/**	Retrieves a value that controls should mesh normals be imported if available. */
		bool getImportNormals() const { return mImportNormals; }

		/**	Sets a value that controls should mesh tangents/bitangents be imported if available. */
		void setImportTangents(bool import) { mImportTangents = import; }

		/**	Retrieves a value that controls should mesh tangent/bitangent be imported if available. */
		bool getImportTangents() const { return mImportTangents; }

		/**	Sets a value that controls should mesh blend shapes be imported	if available. */
		void setImportBlendShapes(bool import) { mImportBlendShapes = import; }

		/**	Retrieves a value that controls should mesh blend shapes be imported if available. */
		bool getImportBlendShapes() const { return mImportBlendShapes; }

		/**	
		 * Sets a value that controls should mesh skin data like bone weights, indices and bind poses be imported if 
		 * available.
		 */
		void setImportSkin(bool import) { mImportSkin = import; }

		/**
		 * Retrieves a value that controls should mesh skin data like bone weights, indices and bind poses be imported if 
		 * available.
		 */
		bool getImportSkin() const { return mImportSkin; }

		/** Sets a value that controls should animation clips be imported if available. */
		void setImportAnimation(bool import) { mImportAnimation = import; }

		/**	Retrieves a value that controls should animation clips be imported if available. */
		bool getImportAnimation() const { return mImportAnimation; }

		/**	Sets a value that will uniformly scale the imported mesh by the specified value. */
		void setImportScale(float import) { mImportScale = import; }

		/**	Retrieves a value that will uniformly scale the imported mesh by the specified value. */
		float getImportScale() const { return mImportScale; }

		/**	Sets a value that controls what type (if any) of collision mesh should be imported. */
		void setCollisionMeshType(CollisionMeshType type) { mCollisionMeshType = type; }

		/**	Retrieves a value that controls what type (if any) of collision mesh should be imported. */
		CollisionMeshType getCollisionMeshType() const { return mCollisionMeshType; }

		/** 
		 * Registers animation split infos that determine how will the source animation clip be split. If no splits
		 * are present the data will be imported as one clip, but if splits are present the data will be split according
		 * to the split infos. Split infos only affect the primary animation clip, other clips will not be split.
		 */
		void setAnimationClipSplits(const Vector<AnimationSplitInfo>& splitInfos) { mAnimationSplits = splitInfos; }

		/** Returns a copy of the animation splits array. @see setAnimationClipSplits. */
		Vector<AnimationSplitInfo> getAnimationClipSplits() const { return mAnimationSplits; }

		/** Assigns a set of events that will be added to the animation clip, if animation import is enabled. */
		void setAnimationEvents(const Vector<ImportedAnimationEvents>& events) { mAnimationEvents = events; }

		/** Returns a copy of the animation events array. @see setAnimationEvents. */
		Vector<ImportedAnimationEvents> getAnimationEvents() const { return mAnimationEvents; }

		/**	
		 * Enables or disables keyframe reduction. Keyframe reduction will reduce the number of key-frames in an animation
		 * clip by removing identical keyframes, and therefore reducing the size of the clip.
		 */
		void setKeyFrameReduction(bool enabled) { mReduceKeyFrames = enabled; }

		/**	
		 * Checks is keyframe reduction enabled.
		 *
		 * @see	setKeyFrameReduction
		 */
		bool getKeyFrameReduction() const { return mReduceKeyFrames; }

		/**	
		 * Enables or disables import of root motion curves. When enabled, any animation curves in imported animations 
		 * affecting the root bone will be available through a set of separate curves in AnimationClip, and they won't be
		 * evaluated through normal animation process. Instead it is expected that the user evaluates the curves manually
		 * and applies them as required.
		 */
		void setImportRootMotion(bool enabled) { mImportRootMotion = enabled; }

		/**	
		 * Checks is root motion import enabled.
		 *
		 * @see	setImportRootMotion
		 */
		bool getImportRootMotion() const { return mImportRootMotion; }

	private:
		bool mCPUReadable;
		bool mImportNormals;
		bool mImportTangents;
		bool mImportBlendShapes;
		bool mImportSkin;
		bool mImportAnimation;
		bool mReduceKeyFrames;
		bool mImportRootMotion;
		float mImportScale;
		CollisionMeshType mCollisionMeshType;
		Vector<AnimationSplitInfo> mAnimationSplits;
		Vector<ImportedAnimationEvents> mAnimationEvents;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class MeshImportOptionsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}