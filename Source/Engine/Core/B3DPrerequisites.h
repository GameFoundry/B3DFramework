//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

/** @addtogroup Layers
 *  @{
 */

/** @defgroup Engine Engine
 *	Core engine functionality. Shared between editor and standalone applications.
 *  @{
 */

/** @defgroup Animation Animation
 *	%Animation clips, skeletal and blend shape animation, animation playback, blending and other features.
 */

/** @defgroup Audio Audio
 *	%Audio clips, 3D sound and music reproduction.
 */

/** @defgroup 2D 2D
  *	Two dimensional geometry (sprites).
  */

/** @defgroup Application Application
 *  Entry point into the application.
 */

/** @defgroup RenderThread Render thread
 *	Core objects and interaction with the render thread.
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

/** @defgroup Particles Particles
 *	Emission, updates and rendering of particles in the particle system.
 */

/** @defgroup Network Network
 * Sending and receiving data over the network.
 */

/** @defgroup GUI GUI
  *	Graphical user interface, including elements, styles, events and GUI manager.
  */

/** @defgroup Input Input
 *	User input (mouse, keyboard, gamepad, etc.).
 */

/** @defgroup Scene Scene
 *  Managing scene objects and their hierarchy.
 */

/** @defgroup Text Text
 *  Generating text geometry.
 */

/** @defgroup Platform Platform
 *  %Platform specific functionality.
 */

/** @defgroup Components Components
 *	Built-in components (elements that may be attached to scene objects).
 */

/** @defgroup Resources Resources
  *	Builtin engine resource types and a manager for such resources.
  */

/** @cond RTTI */
/** @defgroup RTTI-Impl-Engine RTTI types
 *  Types containing RTTI for specific classes.
 */
/** @endcond */

/** @defgroup Utility-Engine Utility
 *  Various utility methods and types used by the engine layer.
 */

/** @} */
/** @} */

/** @addtogroup Internals
 *  @{
 */

/** @defgroup Internal-Engine Engine
 *	Layer that builds upon Core, providing specific implementations of its interfaces as well as other high level systems.
 *  @{
 */

/** @defgroup 2D-Internal 2D
  *	Two dimensional geometry (sprites).
  */

/** @defgroup GUI-Internal GUI
  *	Graphical user interface, including elements, styles, events and GUI manager.
  */

/** @defgroup Renderer-Internal Renderer
  *	Abstract interface and helper functionality for rendering scene objects and other geometry.
  */

/** @defgroup Script-Internal Script
 *  Interaction with scripting languages and libraries.
 */

/** @defgroup Utility-Engine-Internal Utility
 *  Various utility methods and types used by the engine layer.
 */

/** @defgroup Animation-Internal Animation
 *	Animation clips, skeletal and blend shape animation, animation playback, blending and other features.
 */

/** @defgroup Audio-Internal Audio
 *	Audio clips, 3D sound and music reproduction.
 */

/** @defgroup RenderThread-Internal Render thread
 *	Core objects and interaction with the render thread.
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

/** @defgroup Particles-Internal Particles
 *	Emission, updates and rendering of particles in the particle system.
 */

/** @defgroup Network-Internal Network
 * Sending and receiving data over the network.
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

/** @defgroup Resources-Internal Resources
 *	Core resource types and resource management functionality (loading, saving, etc.).
 */

/** @defgroup Scene-Internal Scene
 *  Managing scene objects and their hierarchy.
 */

/** @defgroup Text-Internal Text
 *  Generating text geometry.
 */

/** @} */
/** @} */

/** Maximum number of color surfaces that can be attached to a multi render target. */
#define B3D_MAXIMUM_RENDER_TARGET_COUNT 8

/**
 * Runs the render thead on the application's main (initial) thread, rather than on a separate worker. Important for macOS
 * which has limitations regarding what can run on non-main threads.
 */
#define B3D_SWAP_RENDER_AND_MAIN_THREAD 0

/** Maximum number of individual GPU queues, per type. */
#define B3D_MAX_QUEUES_PER_TYPE 8

/** Maximum number of hardware devices usable at once. */
#define B3D_MAX_DEVICES 5U

#include "Localization/B3DHString.h"
#include "String/B3DStringID.h"

namespace b3d
{
	static const StringID kRendererDefault = "RenderBeast";

	// Core objects
	template <class T>
	struct RenderThreadType
	{
		typedef T Type;
	};

#define B3D_CORE_OBJECT_FORWARD_DECLARE(TYPE) \
	class TYPE;                               \
	namespace render                          \
	{                                         \
		class TYPE;                           \
	}                                         \
	template <>                               \
	struct RenderThreadType<TYPE>             \
	{                                         \
		typedef render::TYPE Type;            \
	};

#define B3D_CORE_OBJECT_FORWARD_DECLARE_STRUCT(TYPE) \
	struct TYPE;                                     \
	namespace render                                 \
	{                                                \
		struct TYPE;                                 \
	}                                                \
	template <>                                      \
	struct RenderThreadType<TYPE>                    \
	{                                                \
		typedef render::TYPE Type;                   \
	};

	class CoreObject;
	namespace render
	{
		class RenderProxy;
	}

	template <>
	struct RenderThreadType<CoreObject>
	{
		typedef render::RenderProxy Type;
	};

	B3D_CORE_OBJECT_FORWARD_DECLARE(Pass)
	B3D_CORE_OBJECT_FORWARD_DECLARE(Variation)
	B3D_CORE_OBJECT_FORWARD_DECLARE(Shader)
	B3D_CORE_OBJECT_FORWARD_DECLARE(Material)
	B3D_CORE_OBJECT_FORWARD_DECLARE(RenderTarget)
	B3D_CORE_OBJECT_FORWARD_DECLARE(RenderTexture)
	B3D_CORE_OBJECT_FORWARD_DECLARE(RenderWindow)
	B3D_CORE_OBJECT_FORWARD_DECLARE(Viewport)
	B3D_CORE_OBJECT_FORWARD_DECLARE(GpuParameterSet)
	B3D_CORE_OBJECT_FORWARD_DECLARE(MaterialParameterAdapter)
	B3D_CORE_OBJECT_FORWARD_DECLARE(GpuBuffer)
	B3D_CORE_OBJECT_FORWARD_DECLARE(MaterialParameters)
	B3D_CORE_OBJECT_FORWARD_DECLARE(Light)
	B3D_CORE_OBJECT_FORWARD_DECLARE(Camera)
	B3D_CORE_OBJECT_FORWARD_DECLARE(Renderable)
	B3D_CORE_OBJECT_FORWARD_DECLARE(Decal)
	B3D_CORE_OBJECT_FORWARD_DECLARE(ReflectionProbe)
	B3D_CORE_OBJECT_FORWARD_DECLARE(ParticleSystem)
	B3D_CORE_OBJECT_FORWARD_DECLARE(Texture)
	B3D_CORE_OBJECT_FORWARD_DECLARE(SpriteTexture)
	B3D_CORE_OBJECT_FORWARD_DECLARE(SpriteImage)
	B3D_CORE_OBJECT_FORWARD_DECLARE(SpriteGlyph)
	B3D_CORE_OBJECT_FORWARD_DECLARE(SpriteVectorPath)
	B3D_CORE_OBJECT_FORWARD_DECLARE(Mesh)
	B3D_CORE_OBJECT_FORWARD_DECLARE(VectorField)
	B3D_CORE_OBJECT_FORWARD_DECLARE(Skybox)
	B3D_CORE_OBJECT_FORWARD_DECLARE(AnimationClip)
	B3D_CORE_OBJECT_FORWARD_DECLARE(StringTable)
	B3D_CORE_OBJECT_FORWARD_DECLARE(ShaderInclude)
	B3D_CORE_OBJECT_FORWARD_DECLARE(LightProbeVolume)
	B3D_CORE_OBJECT_FORWARD_DECLARE(PersistentCacheObject)
	B3D_CORE_OBJECT_FORWARD_DECLARE(Font)
	B3D_CORE_OBJECT_FORWARD_DECLARE(Prefab)
	B3D_CORE_OBJECT_FORWARD_DECLARE(VectorPath)
	B3D_CORE_OBJECT_FORWARD_DECLARE(PlainText)
	B3D_CORE_OBJECT_FORWARD_DECLARE(ScriptCode)
	B3D_CORE_OBJECT_FORWARD_DECLARE(GUIStyleSheet)
	B3D_CORE_OBJECT_FORWARD_DECLARE(SpriteImageAllocation)
	B3D_CORE_OBJECT_FORWARD_DECLARE(SpriteGlyphAllocation)
	B3D_CORE_OBJECT_FORWARD_DECLARE(SpriteTextureAllocation)
	B3D_CORE_OBJECT_FORWARD_DECLARE(SpriteVectorPathAllocation)
	B3D_CORE_OBJECT_FORWARD_DECLARE(SceneInstance)
	B3D_CORE_OBJECT_FORWARD_DECLARE(RendererScene)
	B3D_CORE_OBJECT_FORWARD_DECLARE_STRUCT(DepthOfFieldSettings)
	B3D_CORE_OBJECT_FORWARD_DECLARE_STRUCT(ChromaticAberrationSettings)
	B3D_CORE_OBJECT_FORWARD_DECLARE_STRUCT(RenderSettings)
	B3D_CORE_OBJECT_FORWARD_DECLARE_STRUCT(ShaderInformation)
	B3D_CORE_OBJECT_FORWARD_DECLARE_STRUCT(ShaderCreateInformation)

	class Collider;
	class SamplerState;
	class Rigidbody;
	class BoxCollider;
	class SphereCollider;
	class PlaneCollider;
	class CapsuleCollider;
	class MeshCollider;
	class Joint;
	class FixedJoint;
	class DistanceJoint;
	class HingeJoint;
	class SphericalJoint;
	class SliderJoint;
	class D6Joint;
	class CharacterController;
	class Animation;
	class Bone;
	class AudioSource;
	class AudioListener;
	class Color;
	class GpuProgramManager;
	class GpuProgramManager;
	class GpuProgramFactory;
	class IndexData;
	class GpuDeviceCapabilities;
	class RenderTargetProperties;
	class TextureManager;
	class Input;
	struct PointerEvent;
	class RendererFactory;
	class FontManager;
	class GpuProgram;
	struct GpuProgramParameterDescription;
	struct GpuUniformBufferMemberInformation;
	struct GpuObjectParameterInformation;
	struct GpuUniformBufferInformation;
	class ShaderInclude;
	class ImportOptions;
	class TextureImportOptions;
	class FontImportOptions;
	class GpuProgramImportOptions;
	class MeshImportOptions;
	class GpuGraphicsPipelineState;
	class GpuComputePipelineState;
	class GpuPipelineParameterLayout;
	struct FontBitmapInformation;
	class GameObject;
	class GpuResourceData;
	struct RenderOperation;
	class RenderQueue;
	struct ProfilerReport;
	class VertexDescription;
	class FrameAllocator;
	class FolderMonitor;
	class VideoMode;
	class VideoOutputInfo;
	class VideoModeInfo;
	struct SubMesh;
	class IResourceListener;
	struct TextureProperties;
	class IShaderIncludeHandler;
	class Prefab;
	class SceneObjectHierarchyDelta;
	class RendererMeshData;
	class Win32Window;
	class GpuBackendFactory;
	class PhysicsManager;
	class Physics;
	class FCollider;
	class PhysicsMaterial;
	class ShaderDefines;
	class ShaderImportOptions;
	class AudioClipImportOptions;
	class AnimationClip;
	class GpuDevice;
	template <class T>
	class TAnimationCurve;
	struct AnimationCurves;
	class Skeleton;
	class MorphShapes;
	class MorphShape;
	class MorphChannel;
	class Transform;
	class CoreObjectManager;
	struct CollisionData;
	class VirtualButton;
	class VirtualInput;
	class InputConfiguration;
	struct DragCallbackInfo;
	struct ShortcutKey;

	// GUI
	class GUIWidget;
	class GUIManager;
	class GUIElement;
	class GUIInteractable;
	class GUILabel;
	class GUIClickable;
	class GUIButton;
	class GUITexture;
	class GUIToggle;
	class GUIInputBox;
	class GUISliderHandle;
	class GUIVerticalScrollBar;
	class GUIHorizontalScrollBar;
	class GUIScrollArea;
	class GUISkin;
	class GUIRenderTexture;
	struct GUIElementStyle;
	class GUIMouseEvent;
	class GUITextInputEvent;
	class GUICommandEvent;
	class GUIVirtualButtonEvent;
	class GUILayout;
	class GUILayoutX;
	class GUILayoutY;
	class GUIPanel;
	class GUIFixedSpace;
	class GUIFlexibleSpace;
	class GUIInputCaret;
	class GUIInputSelection;
	struct GUISizeConstraints;
	class GUIOptions;
	class GUIToggleGroup;
	class GUIListBox;
	class GUIDropDownDataEntry;
	class GUIDropDownMenu;
	class DragAndDrop;
	class GUIMenu;
	class GUIMenuItem;
	class GUIContent;
	class GUIContextMenu;
	class GUIDropDownHitBox;
	class GUIDropDownContent;
	class RenderElement;
	class GUISlider;
	class GUIVerticalSlider;
	class GUIHorizontalSlider;
	class GUIProgressBar;
	class GUICanvas;
	class GUIStyleSheet;

	class RenderableHandler;
	class CProfilerOverlay;
	class ProfilerOverlay;
	class DrawHelper;
	class PlainText;
	class ScriptCode;
	class ScriptCodeImportOptions;
	class RendererMeshData;

	// 2D
	class TextSprite;
	class ImageSprite;
	class SpriteMaterial;
	struct SpriteMaterialInfo;

	// Asset import
	class SpecificImporter;
	class Importer;
	// Resources
	class Resource;
	class Resources;
	class ResourceManifest;
	class MeshBase;
	class TransientMesh;
	class MeshHeap;
	class Font;
	class ResourceMetaData;
	class DropTarget;
	class StringTable;
	class PhysicsMaterial;
	class PhysicsMesh;
	class AudioClip;
	class VectorPath;
	// Scene
	class SceneObject;
	class Component;
	class SceneManager;
	class SceneInstance;
	class Scene;
	// RTTI
	class MeshRTTI;
	// Desc structs
	struct SamplerStateInformation;
	struct DepthStencilStateInformation;
	struct RasterizerStateInformation;
	struct BlendStateInformation;
	struct RenderTargetBlendStateInformation;
	struct RenderTextureCreateInformation;
	struct RenderWindowCreateInformation;
	struct CharacterControllerCreateInformation;
	struct JointCreateInformation;
	struct FixedJointCreateInformation;
	struct DistanceJointCreateInformation;
	struct HingeJointCreateInformation;
	struct SliderJointCreateInformation;
	struct SphericalJointCreateInformation;
	struct D6JointCreateInformation;
	struct AudioClipCreateInformation;

	namespace render
	{
		class Renderer;
		class VertexData;
		class GpuCommandBuffer;
		class EventQuery;
		class TimerQuery;
		class OcclusionQuery;
		class TextureView;
		class RenderElement;
		class RenderWindowManager;
		class GpuBuffer;
	} // namespace render
} // namespace b3d

/************************************************************************/
/* 									RTTI                      			*/
/************************************************************************/
namespace b3d
{
	enum TypeID_Core
	{
		TID_Texture = 1001,
		TID_Mesh = 1002,
		TID_MeshData = 1003,
		TID_VertexDeclaration = 1004,
		TID_VertexElementData = 1005,
		TID_Component = 1006,
		TID_StrongResourceHandle = 1009,
		TID_GpuProgram = 1010,
		TID_ResourceHandleData = 1011,
		TID_CgProgram = 1012,
		TID_Pass = 1014,
		TID_Variation = 1015,
		TID_Shader = 1016,
		TID_Material = 1017,
		TID_SamplerState = 1021,
		TID_BlendState = 1023,
		TID_RasterizerStateInformation = 1024,
		TID_DepthStencilStateInformation = 1025,
		TID_BlendStateInformation = 1034,
		TID_ShaderDataParameterInformation = 1035,
		TID_ShaderObjectParameterInformation = 1036,
		TID_ShaderParameterBlockInformation = 1047,
		TID_ImportOptions = 1048,
		TID_Font = 1051,
		//TID_FONT_DESC = 1052,
		TID_CharacterInformation = 1053,
		TID_FontImportOptions = 1056,
		TID_FontBitmapInformation = 1057,
		TID_SceneObject = 1059,
		TID_GameObject = 1060,
		TID_PixelData = 1062,
		TID_GpuResourceData = 1063,
		TID_VertexDescription = 1064,
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
		TID_SceneObjectHierarchyDelta = 1078,
		TID_SceneObjectHierarchyDeltaObject = 1079,
		//TID_ComponentDelta = 1080,
		TID_GUIWidget = 1081,
		/// TID_ProfilerOverlay = 1082,
		TID_StringTable = 1083,
		TID_LanguageData = 1084,
		TID_LocalizedStringData = 1085,
		TID_MaterialParamColor = 1086,
		TID_WeakResourceHandle = 1087,
		TID_TextureParamData = 1088,
		TID_StructParameterMetaData = 1089,
		TID_MaterialParameters = 1090,
		//TID_MaterialRTTIParam = 1091,
		TID_PhysicsMaterial = 1092,
		TID_Collider = 1093,
		TID_BoxCollider = 1094,
		TID_SphereCollider = 1095,
		TID_CapsuleCollider = 1096,
		TID_PlaneCollider = 1097,
		TID_Rigidbody = 1098,
		TID_PhysicsMesh = 1099,
		TID_MeshCollider = 1100,
		TID_Joint = 1101,
		TID_FixedJoint = 1102,
		TID_DistanceJoint = 1103,
		TID_HingeJoint = 1104,
		TID_SphericalJoint = 1105,
		TID_SliderJoint = 1106,
		TID_D6Joint = 1107,
		TID_CharacterController = 1108,
		TID_PhysicsMeshImplementation = 1109,
		TID_ShaderImportOptions = 1110,
		TID_AudioClip = 1111,
		TID_AudioClipImportOptions = 1112,
		TID_AudioListener = 1113,
		TID_AudioSource = 1114,
		TID_AnimationClip = 1115,
		TID_AnimationCurve = 1116,
		TID_KeyFrame = 1117,
		TID_NamedAnimationCurve = 1118,
		TID_Skeleton = 1119,
		TID_SkeletonBoneInfo = 1120,
		TID_AnimationSplitInfo = 1121,
		TID_Animation = 1122,
		TID_AnimationEvent = 1123,
		TID_ImportedAnimationEvents = 1124,
		TID_Bone = 1125,
		TID_MaterialParamData = 1126,
		TID_RenderSettings = 1127,
		TID_MorphShape = 1128,
		TID_MorphShapes = 1129,
		TID_MorphChannel = 1130,
		//TID_ReflectionProbe = 1131,
		TID_ReflectionProbe = 1132,
		TID_CachedTextureData = 1133,
		//TID_Skybox = 1134,
		TID_Skybox = 1135,
		//TID_LightProbeVolume = 1136,
		TID_SavedLightProbeInfo = 1137,
		TID_LightProbeVolume = 1138,
		TID_Transform = 1139,
		TID_SceneActor = 1140,
		//TID_AudioListener = 1141,
		//TID_AudioSource = 1142,
		TID_ShaderVariationParameter = 1143,
		TID_ShaderVariation = 1144,
		TID_GpuProgramBytecode = 1145,
		TID_GpuUniformBufferInformation = 1146,
		TID_GpuParamDataDesc = 1147,
		TID_GpuObjectParameterInformation = 1148,
		TID_GpuParameterDescription = 1149,
		TID_BlendStateDesc = 1150,
		TID_RasterizerStateDesc = 1151,
		TID_DepthStencilStateDesc = 1152,
		// TID_SerializedGpuProgramData = 1153,
		TID_SubShader = 1154,
		//TID_ParticleSystem = 1155,
		TID_ColorDistribution = 1156,
		TID_TDistribution = 1157,
		TID_ShaderParameterAttribute = 1158,
		TID_DataParamInfo = 1159,
		TID_SpriteSheetGridAnimation = 1160,
		TID_ParticleEmitter = 1161,
		TID_ParticleEmitterConeShape = 1162,
		TID_ParticleEmitterSphereShape = 1163,
		TID_ParticleEmitterHemisphereShape = 1164,
		TID_ParticleEmitterBoxShape = 1165,
		TID_ParticleEmitterCircleShape = 1166,
		TID_ParticleEmitterRectShape = 1167,
		TID_ParticleEmitterLineShape = 1168,
		TID_ParticleEmitterStaticMeshShape = 1169,
		TID_ParticleEmitterSkinnedMeshShape = 1170,
		TID_ParticleTextureAnimation = 1171,
		TID_ParticleCollisions = 1172,
		TID_ParticleOrbit = 1173,
		TID_ParticleVelocity = 1174,
		TID_ParticleSystemSettings = 1175,
		TID_ParticleSystemEmitters = 1176,
		TID_ParticleSystemEvolvers = 1177,
		TID_ParticleSystem = 1178,
		TID_ParticleGravity = 1179,
		TID_VectorField = 1180,
		TID_ParticleVectorFieldSettings = 1181,
		TID_ParticleGpuSimulationSettings = 1182,
		TID_ParticleDepthCollisionSettings = 1183,
		TID_BloomSettings = 1184,
		TID_ParticleBurst = 1185,
		TID_RTTIOperationEngineContext = 1186,
		TID_ParticleForce = 1187,
		TID_ParticleSize = 1188,
		TID_ParticleColor = 1189,
		TID_ParticleRotation = 1190,
		//TID_Decal = 1191,
		TID_Decal = 1192,
		TID_RenderTarget = 1193,
		TID_RenderTexture = 1194,
		TID_RenderWindow = 1195,
		TID_ShaderVariationParameterInfo = 1196,
		TID_ShaderVariationParameterValue = 1197,
		TID_ScreenSpaceLensFlareSettings = 1198,
		TID_ChromaticAberrationSettings = 1199,
		TID_FilmGrainSettings = 1200,
		TID_AutoExposureSettings = 1201,
		TID_TonemappingSettings = 1202,
		TID_WhiteBalanceSettings = 1203,
		TID_ColorGradingSettings = 1204,
		TID_DepthOfFieldSettings = 1205,
		TID_AmbientOcclusionSettings = 1206,
		TID_ScreenSpaceReflectionsSettings = 1207,
		TID_ShadowSettings = 1208,
		TID_MotionBlurSettings = 1209,
		TID_TemporalAASettings = 1210,
		TID_TextureSurface = 1211,
		TID_PackageMetaData = 1212,
		TID_PackageResourceMetaData = 1213,
		TID_PackageResourceUserMetaData = 1214,
		TID_Package = 1215,
		TID_PersistentCacheObject = 1216,
		TID_PersistentCacheMetaData = 1217,
		TID_GpuProgramCreateInformation = 1218,
		TID_PassRenderProxy = 1219,
		TID_VariationRenderProxy = 1220,
		TID_ShaderCompilerMetaData = 1221,
		TID_ShaderInformationBase = 1222,
		TID_ShaderInformation = 1223,
		TID_ShaderInformationRenderProxy = 1224,
		TID_ShaderRenderProxy = 1225,
		TID_VectorPathCommand = 1226,
		TID_VectorGraphicsPaint = 1227,
		TID_VectorPathState = 1228,
		TID_NVGRenderUniforms = 1229,
		TID_NVGRenderCommand = 1230,
		TID_VectorGraphicsSettings = 1231,
		TID_VectorPath = 1232,
		TID_VectorPathRenderable = 1233,
		TID_NVGVectorPathRenderable = 1234,
		TID_NVGPathRenderData = 1235,
		TID_FontBitmapPage = 1236,
		TID_SpriteImage = 1237,
		TID_SpriteGlyph = 1238,
		TID_SpriteVectorPath = 1239,
		TID_GUIElement = 1240,
		TID_GUIRenderable = 1241,
		TID_GUIInteractable = 1242,
		TID_GUILayout = 1243,
		TID_GUIPanel = 1244,
		TID_GUIFixedSpace = 1245,
		TID_GUIFlexibleSpace = 1246,
		TID_UnitTestSerializationObjectA = 1247,
		TID_UnitTestSerializationObjectB = 1248,
		TID_UnitTestComponentA = 1249,
		TID_UnitTestComponentB = 1250,
		TID_D6JointDrive = 1251,
		TID_BufferParamData = 1252,
		TID_SamplerStateParamData = 1253,
		TID_IEditorWindow = 1254,
		TID_EditorRenderWindow = 1255,
		TID_SecondaryEditorWindow = 1256,
		TID_MainEditorWindow = 1257,
		TID_ModalWindow = 1258,
		TID_DockableEditorWindow = 1259,
		TID_DropDownWindow = 1260,
		TID_GUIElementContainer = 1261,
		TID_GUIScrollArea = 1262,
		TID_ColliderShape = 1263,
		TID_ColliderShapeInformation = 1264,
		TID_PlaneColliderShape = 1265,
		TID_BoxColliderShape = 1266,
		TID_SphereColliderShape = 1267,
		TID_CapsuleColliderShape = 1268,
		TID_MeshColliderShape = 1269,
		TID_Scene = 1270,
#if B3D_WITH_EDITOR
		TID_IEditorSceneInstance = 1271,
#endif

		// Moved from Engine layer
		TID_Camera = 30000,
		//TID_Camera = 30003,
		TID_Renderable = 30001,
		TID_SpriteTexture = 30002,
		//TID_Light = 30011,
		TID_Light = 30012,
		/* TID_CCamera = 30000, */
		/* TID_CRenderable = 30001, */
		/* TID_SpriteTexture = 30002, */
		/* TID_Camera = 30003, */
		/* TID_Renderable = 30004, */
		TID_PlainText = 30005,
		TID_ScriptCode = 30006,
		TID_ScriptCodeImportOptions = 30007,
		TID_GUIElementStyle = 30008,
		//TID_GUISkin = 30009, - Class removed
		//TID_GUISkinEntry = 30010, - Class removed
		/* TID_Light = 30011, */
		/* TID_CLight = 30012, */
		TID_GameSettings = 30013,
		TID_ResourceMapping = 30014,
		//TID_AutoExposureSettings = 30016,
		//TID_TonemappingSettings = 30017,
		//TID_WhiteBalanceSettings = 30018,
		//TID_ColorGradingSettings = 30019,
		//TID_DepthOfFieldSettings = 30020,
		//TID_AmbientOcclusionSettings = 30021,
		//TID_ScreenSpaceReflectionsSettings = 30022
		TID_GUIStyleSheet = 30023,
		TID_GUIStyleSheetRuleset = 30024,
		TID_GUIStyleSheetRule = 30025,
		TID_GUIStyleSheetBorderElement = 30026,
		TID_GUIStyleSheetSelector = 30027,
		TID_GUIStyleSheetSelectorList = 30028,
		TID_DragAndDropData = 30029,
		TID_SceneObjectDragAndDropData = 30030,
		TID_ResourceDragAndDropData = 30031,
	};
} // namespace b3d

/************************************************************************/
/* 							Resource references                   		*/
/************************************************************************/

#include "Resources/B3DResourceHandle.h"

namespace b3d
{
	/** @addtogroup Resources
	 *  @{
	 */

	typedef TResourceHandle<Resource> HResource;
	typedef TResourceHandle<Texture> HTexture;
	typedef TResourceHandle<Mesh> HMesh;
	typedef TResourceHandle<Material> HMaterial;
	typedef TResourceHandle<ShaderInclude> HShaderInclude;
	typedef TResourceHandle<Font> HFont;
	typedef TResourceHandle<Shader> HShader;
	typedef TResourceHandle<Prefab> HPrefab;
	typedef TResourceHandle<Scene> HScene;
	typedef TResourceHandle<StringTable> HStringTable;
	typedef TResourceHandle<PhysicsMaterial> HPhysicsMaterial;
	typedef TResourceHandle<PhysicsMesh> HPhysicsMesh;
	typedef TResourceHandle<AudioClip> HAudioClip;
	typedef TResourceHandle<AnimationClip> HAnimationClip;
	typedef TResourceHandle<SpriteTexture> HSpriteTexture;
	typedef TResourceHandle<SpriteGlyph> HSpriteGlyph;
	typedef TResourceHandle<SpriteVectorPath> HSpriteVectorPath;
	typedef TResourceHandle<SpriteImage> HSpriteImage;
	typedef TResourceHandle<VectorField> HVectorField;
	typedef TResourceHandle<VectorPath> HVectorPath;
	typedef TResourceHandle<PlainText> HPlainText;
	typedef TResourceHandle<ScriptCode> HScriptCode;
	typedef TResourceHandle<GUIStyleSheet> HGUIStyleSheet;

	/** @} */
} // namespace b3d

#include "Scene/B3DGameObjectHandle.h"

namespace b3d
{
	/** @addtogroup Scene
	 *  @{
	 */

	// Game object handles
	typedef GameObjectHandle<GameObject> HGameObject;
	typedef GameObjectHandle<SceneObject> HSceneObject;
	typedef GameObjectHandle<Component> HComponent;
	typedef GameObjectHandle<Camera> HCamera;
	typedef GameObjectHandle<Renderable> HRenderable;
	typedef GameObjectHandle<Light> HLight;
	typedef GameObjectHandle<Animation> HAnimation;
	typedef GameObjectHandle<Bone> HBone;
	typedef GameObjectHandle<Rigidbody> HRigidbody;
	typedef GameObjectHandle<Collider> HCollider;
	typedef GameObjectHandle<BoxCollider> HBoxCollider;
	typedef GameObjectHandle<SphereCollider> HSphereCollider;
	typedef GameObjectHandle<CapsuleCollider> HCapsuleCollider;
	typedef GameObjectHandle<PlaneCollider> HPlaneCollider;
	typedef GameObjectHandle<MeshCollider> HMeshCollider;
	typedef GameObjectHandle<Joint> HJoint;
	typedef GameObjectHandle<HingeJoint> HHingeJoint;
	typedef GameObjectHandle<SliderJoint> HSliderJoint;
	typedef GameObjectHandle<DistanceJoint> HDistanceJoint;
	typedef GameObjectHandle<SphericalJoint> HSphericalJoint;
	typedef GameObjectHandle<FixedJoint> HFixedJoint;
	typedef GameObjectHandle<D6Joint> HD6Joint;
	typedef GameObjectHandle<CharacterController> HCharacterController;
	typedef GameObjectHandle<ReflectionProbe> HReflectionProbe;
	typedef GameObjectHandle<Skybox> HSkybox;
	typedef GameObjectHandle<LightProbeVolume> HLightProbeVolume;
	typedef GameObjectHandle<AudioSource> HAudioSource;
	typedef GameObjectHandle<AudioListener> HAudioListener;
	typedef GameObjectHandle<ParticleSystem> HParticleSystem;
	typedef GameObjectHandle<Decal> HDecal;
	typedef GameObjectHandle<GUIWidget> HGUIWidget;
	typedef GameObjectHandle<CProfilerOverlay> HProfilerOverlay;

	/** @} */
} // namespace b3d

namespace b3d
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
	void B3D_EXPORT DeferredCall(std::function<void()> callback);

	// Special types for use by profilers
	typedef std::basic_string<char, std::char_traits<char>, StdAlloc<char, ProfilerAllocatorTag>> ProfilerString;

	template <typename T, typename A = StdAlloc<T, ProfilerAllocatorTag>>
	using ProfilerVector = std::vector<T, A>;

	template <typename T, typename A = StdAlloc<T, ProfilerAllocatorTag>>
	using ProfilerStack = std::stack<T, std::deque<T, A>>;

	/** Default thread policy for the framework. Performs special startup/shutdown on threads managed by thread pool. */
	class B3D_EXPORT ThreadDefaultPolicy
	{
	public:
		static void OnThreadStarted(const String& name)
		{
			MemStack::BeginThread();
		}

		static void OnThreadEnded(const String& name)
		{
			MemStack::EndThread();
		}
	};

#define BS_ALL_LAYERS 0xFFFFFFFFFFFFFFFF

	/** Used for marking a CoreObject dependency as dirty. */
	static constexpr i32 kDirtyDependencyMask = 1 << 31;

	template <class T, bool IsRenderProxy>
	struct CoreVariant
	{};

	template <class T>
	struct CoreVariant<T, false>
	{
		typedef T Type;
	};

	template <class T>
	struct CoreVariant<T, true>
	{
		typedef typename RenderThreadType<T>::Type Type;
	};

	/** Allows a simple way to define a member that can be both CoreObject and its RenderProxy variant depending on the IsRenderProxy template parameter. */
	template <class T, bool IsRenderProxy>
	using CoreVariantType = typename CoreVariant<T, IsRenderProxy>::Type;

	template <class T, bool IsRenderProxy>
	struct CoreVariantHandle
	{};

	template <class T>
	struct CoreVariantHandle<T, false>
	{
		typedef TResourceHandle<T> Type;
	};

	template <class T>
	struct CoreVariantHandle<T, true>
	{
		typedef SPtr<typename RenderThreadType<T>::Type> Type;
	};

	/**
	 * Allows a simple way to define a member that can be both CoreObject and its RenderProxy variant depending on the IsRenderProxy template
	 * parameter. Main thread type is wrapped in as a resource handle while the render thread thread variant is wrapped in a shared pointer.
	 */
	template <class T, bool IsRenderProxy>
	using CoreVariantHandleType = typename CoreVariantHandle<T, IsRenderProxy>::Type;

	/** Checks if the object is not null. */
	template<class T>
	bool IsValid(const SPtr<T>& object) { return object != nullptr; }

	/** Checks if the object is not null. */
	template<class T>
	bool IsValid(const TResourceHandle<T>& object) { return object.IsValid(); }

	/** Checks if the object is not null. */
	template<class T>
	bool IsValid(const GameObjectHandle<T>& object) { return object.IsValid(); }

	/** Flags that are provided to the serialization system to control serialization/deserialization. */
	enum SerializationFlags
	{
		/**
		 * Used when deserializing resources. Lets the system know not to discard any intermediate resource data that might
		 * be required if the resource needs to be serialized.
		 */
		SF_KeepResourceSourceData = 1 << 0,

		/** Only serializes elements with network replication flag enabled. */
		SF_ReplicableOnly = 1 << 1
	};

	/**
	 * Maximum supported dimension of a 3D scene. Primarily provided to be used instead of infinity, as the engine compiles with
	 * fast math by default, which may not handle infinities correctly.
	 */
	static constexpr double kMaximumSceneSize = 1e30;

	/** Maximum supported extent of a 3D scene (Half of kMaximumSceneSize). */
	static constexpr double kMaximumSceneExtent = kMaximumSceneSize * 0.5;


	B3D_EXPORT B3D_LOG_CATEGORY_EXTERN(LogRenderThread, Log)
	B3D_EXPORT B3D_LOG_CATEGORY_EXTERN(LogRenderer, Log)
	B3D_EXPORT B3D_LOG_CATEGORY_EXTERN(LogScene, Log)
	B3D_EXPORT B3D_LOG_CATEGORY_EXTERN(LogPhysics, Log)
	B3D_EXPORT B3D_LOG_CATEGORY_EXTERN(LogAudio, Log)
	B3D_EXPORT B3D_LOG_CATEGORY_EXTERN(LogRenderBackend, Log)
	B3D_EXPORT B3D_LOG_CATEGORY_EXTERN(LogBSLCompiler, Log)
	B3D_EXPORT B3D_LOG_CATEGORY_EXTERN(LogParticles, Log)
	B3D_EXPORT B3D_LOG_CATEGORY_EXTERN(LogResources, Log)
	B3D_EXPORT B3D_LOG_CATEGORY_EXTERN(LogFBXImporter, Log)
	B3D_EXPORT B3D_LOG_CATEGORY_EXTERN(LogPixelUtility, Log)
	B3D_EXPORT B3D_LOG_CATEGORY_EXTERN(LogTexture, Log)
	B3D_EXPORT B3D_LOG_CATEGORY_EXTERN(LogMesh, Log)
	B3D_EXPORT B3D_LOG_CATEGORY_EXTERN(LogGUI, Log)
	B3D_EXPORT B3D_LOG_CATEGORY_EXTERN(LogProfiler, Log)
	B3D_EXPORT B3D_LOG_CATEGORY_EXTERN(LogMaterial, Log)
	B3D_EXPORT B3D_LOG_CATEGORY_EXTERN(LogFreeImageImporter, Log)
	B3D_EXPORT B3D_LOG_CATEGORY_EXTERN(LogScript, Log)
	B3D_EXPORT B3D_LOG_CATEGORY_EXTERN(LogImporter, Log)
	B3D_EXPORT B3D_LOG_CATEGORY_EXTERN(LogNetwork, Log)
	B3D_EXPORT B3D_LOG_CATEGORY_EXTERN(LogInput, Log)
} // namespace b3d

#include "Utility/B3DCommonTypes.h"
#include "Utility/B3DEnums.h"
#include "Utility/B3DPaths.h"
#include "Localization/B3DHEString.h"
#include "Scene/B3DGameObject.h"

