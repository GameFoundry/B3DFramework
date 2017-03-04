//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisitesUtil.h"

/** @addtogroup Layers
 *  @{
 */

/** @defgroup Core Core
 *	Second lowest layer that provides core engine functionality and abstract interfaces for various systems.
 *  @{
 */

/** @defgroup Animation Animation
 *	%Animation clips, skeletal and blend shape animation, animation playback, blending and other features.
 */

/** @defgroup Application-Core Application
 *  Entry point into the application and other general functionality.
 */

/** @defgroup Audio Audio
 *	%Audio clips, 3D sound and music reproduction.
 */

/** @defgroup Components-Core Components
  *	Built-in components (elements that may be attached to scene objects).
  */

/** @defgroup CoreThread Core thread
 *	Core objects and interaction with the core (rendering) thread.
 */

/** @defgroup Importer Importer
 *	Import of resources into engine friendly format.
 */

/** @defgroup Input Input
 *	%Input (mouse, keyboard, gamepad, etc.).
 */

/** @defgroup Localization Localization
 *	GUI localization.
 */

/** @defgroup Material Material
 *	Materials, shaders and related functionality.
 */

/** @defgroup Physics Physics
 *	%Physics system: colliders, triggers, rigidbodies, joints, scene queries, etc.
 */

 /** @defgroup Profiling Profiling
  *	Measuring CPU and GPU execution times and memory usage.
  */

/** @defgroup RenderAPI RenderAPI
  *	Interface for interacting with the render API (DirectX, OpenGL, etc.).
  */

/** @defgroup Renderer Renderer
  *	Abstract interface and helper functionality for rendering scene objects.
  */

/** @defgroup Resources Resources
  *	Core resource types and resource management functionality (loading, saving, etc.).
  */

/** @cond RTTI */
/** @defgroup RTTI-Impl-Core RTTI types
 *  RTTI implementations for classes within the core layer.
 */
/** @endcond */

/** @defgroup Scene Scene
 *  Managing scene objects and their hierarchy.
 */

/** @defgroup Text Text
 *  Generating text geometry.
 */

/** @defgroup Utility-Core Utility
 *  Various utility methods and types used by the core layer.
 */

/** @} */
/** @} */

/** @addtogroup Internals
 *  @{
 */

/** @defgroup Internal-Core Core
 *	Second lowest layer that provides core engine functionality and abstract interfaces for various systems.
 *  @{
 */

/** @defgroup Animation-Internal Animation
 *	Animation clips, skeletal and blend shape animation, animation playback, blending and other features.
 */

/** @defgroup Audio-Internal Audio
 *	Audio clips, 3D sound and music reproduction.
 */

/** @defgroup CoreThread-Internal Core thread
 *	Core objects and interaction with the core (rendering) thread.
 */

/** @defgroup Importer-Internal Importer
 *	Import of resources into engine friendly format.
 */

/** @defgroup Input-Internal Input
 *	Input (mouse, keyboard, gamepad, etc.).
 */

/** @defgroup Localization-Internal Localization
 *	GUI localization.
 */

/** @defgroup Material-Internal Material
 *	Materials, shaders and related functionality.
 */

/** @defgroup Physics-Internal Physics
 *	Physics system: colliders, triggers, rigidbodies, joints, scene queries, etc.
 */

/** @defgroup Platform-Internal Platform
 *	Interface for interacting with the platform (OS).
 */

 /** @defgroup Profiling-Internal Profiling
  *	Measuring CPU and GPU execution times and memory usage.
  */

/** @defgroup RenderAPI-Internal RenderAPI
  *	Interface for interacting with the render API (DirectX, OpenGL, etc.).
  */

/** @defgroup Renderer-Internal Renderer
  *	Abstract interface and helper functionality for rendering scene objects.
  */

/** @defgroup Resources-Internal Resources
  *	Core resource types and resource management functionality (loading, saving, etc.).
  */

/** @defgroup Scene-Internal Scene
 *  Managing scene objects and their hierarchy.
 */

/** @defgroup Text-Internal Text
 *  Generating text geometry.
 */

/** @defgroup Utility-Core-Internal Utility
 *  Various utility methods and types used by the core layer.
 */

/** @} */
/** @} */

/** Maximum number of color surfaces that can be attached to a multi render target. */
#define BS_MAX_MULTIPLE_RENDER_TARGETS 8
#define BS_FORCE_SINGLETHREADED_RENDERING 0

/** Maximum number of individual GPU queues, per type. */
#define BS_MAX_QUEUES_PER_TYPE 8

/** Maximum number of hardware devices usable at once. */
#define BS_MAX_DEVICES 5U

/** Maximum number of devices one resource can exist at the same time. */
#define BS_MAX_LINKED_DEVICES 4U

// DLL export
#if BS_PLATFORM == BS_PLATFORM_WIN32 // Windows
#  if BS_COMPILER == BS_COMPILER_MSVC
#    if defined(BS_STATIC_LIB)
#      define BS_CORE_EXPORT
#    else
#      if defined(BS_CORE_EXPORTS)
#        define BS_CORE_EXPORT __declspec(dllexport)
#      else
#        define BS_CORE_EXPORT __declspec(dllimport)
#      endif
#	 endif
#  else
#    if defined(BS_STATIC_LIB)
#      define BS_CORE_EXPORT
#    else
#      if defined(BS_CORE_EXPORTS)
#        define BS_CORE_EXPORT __attribute__ ((dllexport))
#      else
#        define BS_CORE_EXPORT __attribute__ ((dllimport))
#      endif
#	 endif
#  endif
#  define BS_CORE_HIDDEN
#else // Linux/Mac settings
#  define BS_CORE_EXPORT __attribute__ ((visibility ("default")))
#  define BS_CORE_HIDDEN __attribute__ ((visibility ("hidden")))
#endif

#include "BsHString.h"

namespace bs 
{
	static const StringID RenderAPIAny = "AnyRenderAPI";
	static const StringID RendererAny = "AnyRenderer";

    class Color;
    class GpuProgram;
    class GpuProgramManager;
    class IndexBuffer;
    class VertexBuffer;
	class GpuBuffer;
	class GpuProgramManager;
	class GpuProgramFactory;
    class IndexData;
    class Pass;
	class Technique;
	class Shader;
	class Material;
    class RenderAPICapabilities;
    class RenderTarget;
    class RenderTexture;
    class RenderWindow;
	class RenderTargetProperties;
    class SamplerState;
    class TextureManager;
    class Viewport;
    class VertexDeclaration;
	class Input;
	struct PointerEvent;
	class RawInputHandler;
	class RendererFactory;
	class AsyncOp;
	class HardwareBufferManager;
	class FontManager;
	class DepthStencilState;
	class RenderStateManager;
	class RasterizerState;
	class BlendState;
	class GpuParamBlock;
	class GpuParamBlockBuffer;
	class GpuParams;
	struct GpuParamDesc;
	struct GpuParamDataDesc;
	struct GpuParamObjectDesc;
	struct GpuParamBlockDesc;
	class ShaderInclude;
	class CoreObject;
	class ImportOptions;
	class TextureImportOptions;
	class FontImportOptions;
	class GpuProgramImportOptions;
	class MeshImportOptions;
	struct FontBitmap;
	class GameObject;
	class GpuResourceData;
	struct RenderOperation;
	class RenderQueue;
	struct ProfilerReport;
	class VertexDataDesc;
	class FrameAlloc;
	class FolderMonitor;
	class VideoMode;
	class VideoOutputInfo;
	class VideoModeInfo;
	struct SubMesh;
	class IResourceListener;
	class TextureProperties;
	class IShaderIncludeHandler;
	class Prefab;
	class PrefabDiff;
	class RendererMeshData;
	class Light;
	class Win32Window;
	class RenderAPIFactory;
	class PhysicsManager;
	class Physics;
	class FCollider;
	class Collider;
	class Rigidbody;
	class PhysicsMaterial;
	class BoxCollider;
	class SphereCollider;
	class PlaneCollider;
	class CapsuleCollider;
	class MeshCollider;
	class CCollider;
	class CRigidbody;
	class CBoxCollider;
	class CSphereCollider;
	class CPlaneCollider;
	class CCapsuleCollider;
	class CMeshCollider;
	class Joint;
	class FixedJoint;
	class DistanceJoint;
	class HingeJoint;
	class SphericalJoint;
	class SliderJoint;
	class D6Joint;
	class CharacterController;
	class CJoint;
	class CHingeJoint;
	class CDistanceJoint;
	class CFixedJoint;
	class CSphericalJoint;
	class CSliderJoint;
	class CD6Joint;
	class CCharacterController;
	class ShaderDefines;
	class ShaderImportOptions;
	class AudioListener;
	class AudioSource;
	class AudioClipImportOptions;
	class AnimationClip;
	class CCamera;
	class CRenderable;
	class CLight;
	class CAnimation;
	class CBone;
	class GpuPipelineParamInfo;
	class MaterialParams;
	template <class T> class TAnimationCurve;
	struct AnimationCurves;
	class Skeleton;
	class Animation;
	class GpuParamsSet;
	class Camera;
	class Renderable;
	class MorphShapes;
	class MorphShape;
	class MorphChannel;
	class GraphicsPipelineState;
	class ComputePipelineState;
	class ReflectionProbe;
	// Asset import
	class SpecificImporter;
	class Importer;
	// Resources
	class Resource;
	class Resources;
	class ResourceManifest;
	class Texture;
	class Mesh;
	class MeshBase;
	class TransientMesh;
	class MeshHeap;
	class Font;
	class ResourceMetaData;
	class OSDropTarget;
	class StringTable;
	class PhysicsMaterial;
	class PhysicsMesh;
	class AudioClip;
	class CoreObjectManager;
	struct CollisionData;
	// Scene
	class SceneObject;
	class Component;
	class SceneManager;
	// RTTI
	class MeshRTTI;
	// Desc structs
	struct SAMPLER_STATE_DESC;
	struct DEPTH_STENCIL_STATE_DESC;
	struct RASTERIZER_STATE_DESC;
	struct BLEND_STATE_DESC;
	struct RENDER_TARGET_BLEND_STATE_DESC;
	struct RENDER_TEXTURE_DESC;
	struct RENDER_WINDOW_DESC;
	struct FONT_DESC;
	struct CHAR_CONTROLLER_DESC;
	struct JOINT_DESC;
	struct FIXED_JOINT_DESC;
	struct DISTANCE_JOINT_DESC;
	struct HINGE_JOINT_DESC;
	struct SLIDER_JOINT_DESC;
	struct SPHERICAL_JOINT_DESC;
	struct D6_JOINT_DESC;
	struct AUDIO_CLIP_DESC;

	template<class T>
	class TCoreThreadQueue;
	class CommandQueueNoSync;
	class CommandQueueSync;

	namespace ct
	{
		class Renderer;
		class VertexData;
		class SamplerState;
		class IndexBuffer;
		class VertexBuffer;
		class RenderAPI;
		class RenderTarget;
		class RenderTexture;
		class RenderWindow;
		class DepthStencilState;
		class RasterizerState;
		class BlendState;
		class CoreObject;
		class Camera;
		class Renderable;
		class MeshBase;
		class Mesh;
		class TransientMesh;
		class Texture;
		class MeshHeap;
		class VertexDeclaration;
		class GpuBuffer;
		class GpuParamBlockBuffer;
		class GpuParams;
		class Shader;
		class Viewport;
		class Pass;
		class GpuParamsSet;
		class Technique;
		class Material;
		class GpuProgram;
		class Light;
		class ComputePipelineState;
		class GraphicsPipelineState;
		class Camera;
		class GpuParamsSet;
		class MaterialParams;
		class GpuPipelineParamInfo;
		class CommandBuffer;
		class EventQuery;
		class TimerQuery;
		class OcclusionQuery;
		class TextureView;
		class RenderableElement;
		class RenderWindowManager;
		class RenderStateManager;
		class HardwareBufferManager;
		class ReflectionProbe;
	}
}

/************************************************************************/
/* 						         Typedefs								*/
/************************************************************************/

namespace bs
{
	typedef TCoreThreadQueue<CommandQueueNoSync> CoreThreadQueue;
}

/************************************************************************/
/* 									RTTI                      			*/
/************************************************************************/
namespace bs
{
	enum TypeID_Core
	{
		TID_Texture = 1001,
		TID_Mesh = 1002,
		TID_MeshData = 1003,
		TID_VertexDeclaration = 1004,
		TID_VertexElementData = 1005,
		TID_Component = 1006,
		TID_ResourceHandle = 1009,
		TID_GpuProgram = 1010,
		TID_ResourceHandleData = 1011,
		TID_CgProgram = 1012,
		TID_Pass = 1014,
		TID_Technique = 1015,
		TID_Shader = 1016,
		TID_Material = 1017,
		TID_SamplerState = 1021,
		TID_BlendState = 1023,
		TID_RasterizerState = 1024,
		TID_DepthStencilState = 1025,
		TID_BLEND_STATE_DESC = 1034,
		TID_SHADER_DATA_PARAM_DESC = 1035,
		TID_SHADER_OBJECT_PARAM_DESC = 1036,
		TID_SHADER_PARAM_BLOCK_DESC = 1047,
		TID_ImportOptions = 1048,
		TID_Font = 1051,
		TID_FONT_DESC = 1052,
		TID_CHAR_DESC = 1053,
		TID_FontImportOptions = 1056,
		TID_FontBitmap = 1057,
		TID_SceneObject = 1059,
		TID_GameObject = 1060,
		TID_PixelData = 1062,
		TID_GpuResourceData = 1063,
		TID_VertexDataDesc = 1064,
		TID_MeshBase = 1065,
		TID_GameObjectHandleBase = 1066,
		TID_ResourceManifest = 1067,
		TID_ResourceManifestEntry = 1068,
		TID_EmulatedParamBlock = 1069,
		TID_TextureImportOptions = 1070,
		TID_ResourceMetaData = 1071,
		TID_ShaderInclude = 1072,
		TID_Viewport = 1073,
		TID_ResourceDependencies = 1074,
		TID_ShaderMetaData = 1075,
		TID_MeshImportOptions = 1076,
		TID_Prefab = 1077,
		TID_PrefabDiff = 1078,
		TID_PrefabObjectDiff = 1079,
		TID_PrefabComponentDiff = 1080,
		TID_CGUIWidget = 1081,
		TID_ProfilerOverlay = 1082,
		TID_StringTable = 1083,
		TID_LanguageData = 1084,
		TID_LocalizedStringData = 1085,
		TID_MaterialParamColor = 1086,
		TID_WeakResourceHandle = 1087,
		TID_TextureParamData = 1088,
		TID_StructParamData = 1089,
		TID_MaterialParams = 1090,
		TID_MaterialRTTIParam = 1091,
		TID_PhysicsMaterial = 1092,
		TID_CCollider = 1093,
		TID_CBoxCollider = 1094,
		TID_CSphereCollider = 1095,
		TID_CCapsuleCollider = 1096,
		TID_CPlaneCollider = 1097,
		TID_CRigidbody = 1098,
		TID_PhysicsMesh = 1099,
		TID_CMeshCollider = 1100,
		TID_CJoint = 1101,
		TID_CFixedJoint = 1102,
		TID_CDistanceJoint = 1103,
		TID_CHingeJoint = 1104,
		TID_CSphericalJoint = 1105,
		TID_CSliderJoint = 1106,
		TID_CD6Joint = 1107,
		TID_CCharacterController = 1108,
		TID_FPhysicsMesh = 1109,
		TID_ShaderImportOptions = 1110,
		TID_AudioClip = 1111,
		TID_AudioClipImportOptions = 1112,
		TID_CAudioListener = 1113,
		TID_CAudioSource = 1114,
		TID_AnimationClip = 1115,
		TID_AnimationCurve = 1116,
		TID_KeyFrame = 1117,
		TID_NamedAnimationCurve = 1118,
		TID_Skeleton = 1119,
		TID_SkeletonBoneInfo = 1120,
		TID_AnimationSplitInfo = 1121,
		TID_CAnimation = 1122,
		TID_AnimationEvent = 1123,
		TID_ImportedAnimationEvents = 1124,
		TID_CBone = 1125,
		TID_MaterialParamData = 1126,
		TID_PostProcessSettings = 1127,
		TID_MorphShape = 1128,
		TID_MorphShapes = 1129,
		TID_MorphChannel = 1130,
		TID_ReflectionProbe = 1131,
		TID_CReflectionProbe = 1132,
		TID_CachedTextureData = 1133,

		// Moved from Engine layer
		TID_CCamera = 30000,
		TID_Camera = 30003,
		TID_CRenderable = 30001,
		TID_Renderable = 30004,
		TID_Light = 30011,
		TID_CLight = 30012,
	};
}

/************************************************************************/
/* 							Resource references                   		*/
/************************************************************************/

#include "BsResourceHandle.h"

namespace bs
{
	/** @addtogroup Resources
	 *  @{
	 */

	typedef ResourceHandle<Resource> HResource;
	typedef ResourceHandle<Texture> HTexture;
	typedef ResourceHandle<Mesh> HMesh;
	typedef ResourceHandle<Material> HMaterial;
	typedef ResourceHandle<ShaderInclude> HShaderInclude;
	typedef ResourceHandle<Font> HFont;
	typedef ResourceHandle<Shader> HShader;
	typedef ResourceHandle<Prefab> HPrefab;
	typedef ResourceHandle<StringTable> HStringTable;
	typedef ResourceHandle<PhysicsMaterial> HPhysicsMaterial;
	typedef ResourceHandle<PhysicsMesh> HPhysicsMesh;
	typedef ResourceHandle<AudioClip> HAudioClip;
	typedef ResourceHandle<AnimationClip> HAnimationClip;

	/** @} */
}

#include "BsGameObjectHandle.h"

namespace bs
{
	/** @addtogroup Scene
	 *  @{
	 */

	// Game object handles
	typedef GameObjectHandle<GameObject> HGameObject;
	typedef GameObjectHandle<SceneObject> HSceneObject;
	typedef GameObjectHandle<Component> HComponent;
	typedef GameObjectHandle<CCamera> HCamera;
	typedef GameObjectHandle<CRenderable> HRenderable;
	typedef GameObjectHandle<CLight> HLight;
	typedef GameObjectHandle<CAnimation> HAnimation;
	typedef GameObjectHandle<CBone> HBone;
	typedef GameObjectHandle<CRigidbody> HRigidbody;
	typedef GameObjectHandle<CCollider> HCollider;
	typedef GameObjectHandle<CBoxCollider> HBoxCollider;
	typedef GameObjectHandle<CSphereCollider> HSphereCollider;
	typedef GameObjectHandle<CCapsuleCollider> HCapsuleCollider;
	typedef GameObjectHandle<CPlaneCollider> HPlaneCollider;
	typedef GameObjectHandle<CJoint> HJoint;
	typedef GameObjectHandle<CHingeJoint> HHingeJoint;
	typedef GameObjectHandle<CSliderJoint> HSliderJoint;
	typedef GameObjectHandle<CDistanceJoint> HDistanceJoint;
	typedef GameObjectHandle<CSphericalJoint> HSphericalJoint;
	typedef GameObjectHandle<CFixedJoint> HFixedJoint;
	typedef GameObjectHandle<CD6Joint> HD6Joint;
	typedef GameObjectHandle<CCharacterController> HCharacterController;

	/** @} */
}

namespace bs
{
	/**
	 * Defers function execution until the next frame. If this function is called within another deferred call, then it will
	 * be executed the same frame, but only after all existing deferred calls are done.
	 * 			
	 * @note	
	 * This method can be used for breaking dependencies among other things. If a class A depends on class B having
	 * something done, but class B also depends in some way on class A, you can break up the initialization into two
	 * separate steps, queuing the second step using this method.
	 * @note
	 * Similar situation can happen if you have multiple classes being initialized in an undefined order but some of them
	 * depend on others. Using this method you can defer the dependent step until next frame, which will ensure everything
	 * was initialized.
	 *
	 * @param[in]	callback	The callback.
	 */
	void BS_CORE_EXPORT deferredCall(std::function<void()> callback);

	// Special types for use by profilers
	typedef std::basic_string<char, std::char_traits<char>, StdAlloc<char, ProfilerAlloc>> ProfilerString;

	template <typename T, typename A = StdAlloc<T, ProfilerAlloc>>
	using ProfilerVector = std::vector<T, A>;

	template <typename T, typename A = StdAlloc<T, ProfilerAlloc>>
	using ProfilerStack = std::stack<T, std::deque<T, A>>;

	/** Banshee thread policy that performs special startup/shutdown on threads managed by thread pool. */
	class BS_CORE_EXPORT ThreadBansheePolicy
	{
	public:
		static void onThreadStarted(const String& name)
		{
			MemStack::beginThread();
		}

		static void onThreadEnded(const String& name)
		{
			MemStack::endThread();
		}
	};

	#define BS_ALL_LAYERS 0xFFFFFFFFFFFFFFFF
}

#include "BsCommonTypes.h"