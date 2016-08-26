//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace BansheeEngine
{
    /** @addtogroup Animation
     *  @{
     */

    /// <summary>
    /// Determines how an animation clip behaves when it reaches the end.
    /// </summary>
    public enum AnimWrapMode // Note: Must match C++ enum AnimWrapMode
    {
        /// <summary>
        /// Loop around to the beginning/end when the last/first frame is reached.
        /// </summary>
        Loop,
        /// <summary>
        /// Clamp to end/beginning, keeping the last/first frame active.
        /// </summary>
	    Clamp
    }

    /// <summary>
    /// Represents an animation clip used in 1D blending. Each clip has a position on the number line.
    /// </summary>
    public class BlendClipInfo
    {
        public AnimationClip clip;
        public float position;
    }

    /// <summary>
    /// Defines a 1D blend where two animation clips are blended between each other using linear interpolation.
    /// </summary>
    public class Blend1DInfo
    {
        public BlendClipInfo[] clips;
    }

    /// <summary>
    /// Defines a 2D blend where two animation clips are blended between each other using bilinear interpolation.
    /// </summary>
    public class Blend2DInfo
    {
        public AnimationClip topLeftClip;
        public AnimationClip topRightClip;
        public AnimationClip botLeftClip;
        public AnimationClip botRightClip;
    }

    /// <summary>
    /// Contains information about a currently playing animation clip.
    /// </summary>
    [StructLayout(LayoutKind.Sequential), SerializeObject]
    public struct AnimationClipState // Note: Must match C++ struct AnimationClipState
    {
        /// <summary>
        /// Layer the clip is playing on. Multiple clips can be played simulatenously on different layers.
        /// </summary>
        public int layer;

        /// <summary>
        /// Current time the animation is playing from.
        /// </summary>
        public float time;

        /// <summary>
        /// Speed at which the animation is playing.
        /// </summary>
        public float speed;

        /// <summary>
        /// Determines how much of an influence does the clip have on the final pose.
        /// </summary>
        public float weight;

        /// <summary>
        /// Determines what happens to other animation clips when a new clip starts playing.
        /// </summary>
        public AnimWrapMode wrapMode;

        /// <summary>
        /// Initializes the state with default values.
        /// </summary>
        public void InitDefault()
        {
            speed = 1.0f;
            weight = 1.0f;
            wrapMode = AnimWrapMode.Loop;
        }
    }

    /// <summary>
    /// Handles animation playback. Takes one or multiple animation clips as input and evaluates them every animation update
    /// tick depending on set properties.The evaluated data is used by the core thread for skeletal animation, by the sim
    /// thread for updating attached scene objects and bones (if skeleton is attached), or the data is made available for
    /// manual queries in the case of generic animation.
    /// </summary>
    public class Animation : Component
    {
        private NativeAnimation _native;

        [SerializeField] private SerializableData serializableData = new SerializableData();

        private FloatCurvePropertyInfo[] floatProperties;
        private List<SceneObjectMappingInfo> mappingInfo = new List<SceneObjectMappingInfo>();
        private AnimationClip primaryClip;

        /// <summary>
        /// Contains mapping for a suffix used by property paths used for curve identifiers, to their index and type.
        /// </summary>
        internal static readonly Dictionary<string, PropertySuffixInfo> PropertySuffixInfos = new Dictionary
            <string, PropertySuffixInfo>
        {
            {".x", new PropertySuffixInfo(0, true)},
            {".y", new PropertySuffixInfo(1, true)},
            {".z", new PropertySuffixInfo(2, true)},
            {".w", new PropertySuffixInfo(3, true)},
            {".r", new PropertySuffixInfo(0, false)},
            {".g", new PropertySuffixInfo(1, false)},
            {".b", new PropertySuffixInfo(2, false)},
            {".a", new PropertySuffixInfo(3, false)}
        };

        /// <summary>
        /// Returns the non-component version of Animation that is wrapped by this component. 
        /// </summary>
        internal NativeAnimation Native
        {
            get { return _native; }
        }

        /// <summary>
        /// Determines the default clip to play as soon as the component is enabled. If more control over playing clips is
        /// needed use the <see cref="Play"/>, <see cref="Blend1D"/>, <see cref="Blend2D"/> and <see cref="CrossFade"/> 
        /// methods to queue clips for playback manually, and <see cref="SetState"/> method for modify their states 
        /// individually.
        /// </summary>
        public AnimationClip DefaultClip
        {
            get { return serializableData.defaultClip; }
            set
            {
                serializableData.defaultClip = value;

                if (value != null && _native != null)
                    _native.Play(value);
            }
        }

        /// <summary>
        /// Determines the wrap mode for all active animations. Wrap mode determines what happens when animation reaches the 
        /// first or last frame.
        /// <see cref="AnimWrapMode"/>
        /// </summary>
        public AnimWrapMode WrapMode
        {
            get { return serializableData.wrapMode; }
            set
            {
                serializableData.wrapMode = value;

                if (_native != null)
                    _native.WrapMode = value;
            }
        }

        /// <summary>
        /// Determines the speed for all animations. The default value is 1.0f. Use negative values to play-back in reverse.
        /// </summary>
        public float Speed
        {
            get { return serializableData.speed; }
            set
            {
                serializableData.speed = value;

                if (_native != null)
                    _native.Speed = value;
            }
        }

        /// <summary>
        /// Checks if any animation clips are currently playing.
        /// </summary>
        public bool IsPlaying
        {
            get
            {
                if (_native != null)
                    return _native.IsPlaying();

                return false;
            }
        }

        /// <summary>
        /// Plays the specified animation clip. 
        /// </summary>
        /// <param name="clip">Clip to play.</param>
        public void Play(AnimationClip clip)
        {
            if (_native != null)
                _native.Play(clip);
        }

        /// <summary>
        /// Plays the specified animation clip on top of the animation currently playing in the main layer. Multiple such
        /// clips can be playing at once, as long as you ensure each is given its own layer. Each animation can also have a
        /// weight that determines how much it influences the main animation.        
        /// </summary>
        /// <param name="clip">Clip to additively blend. Must contain additive animation curves.</param>
        /// <param name="weight">Determines how much of an effect will the blended animation have on the final output.
        ///                      In range [0, 1].</param>
        /// <param name="fadeLength">Applies the blend over a specified time period, increasing the weight as the time
        ///                          passes. Set to zero to blend immediately. In seconds.</param>
        /// <param name="layer">Layer to play the clip in. Multiple additive clips can be playing at once in separate layers
        ///                     and each layer has its own weight.</param>
        public void BlendAdditive(AnimationClip clip, float weight, float fadeLength, int layer)
        {
            if (_native != null)
                _native.BlendAdditive(clip, weight, fadeLength, layer);
        }

        /// <summary>
        /// Blends multiple animation clips between each other using linear interpolation. Unlike normal animations these
        /// animations are not advanced with the progress of time, and is instead expected the user manually changes the
        /// <see cref="t"/> parameter.
        /// </summary>
        /// <param name="info">Information about the clips to blend. Clip positions must be sorted from lowest to highest.
        ///                    </param>
        /// <param name="t">Parameter that controls the blending, in range [0, 1]. t = 0 means left animation has full
        ///                 influence, t = 1 means right animation has full influence.</param>
        public void Blend1D(Blend1DInfo info, float t)
        {
            if (_native != null)
                _native.Blend1D(info, t);
        }

        /// <summary>
        /// Blend four animation clips between each other using bilinear interpolation. Unlike normal animations these
        /// animations are not advanced with the progress of time, and is instead expected the user manually changes the
        /// <see cref="t"/> parameter.
        /// </summary>
        /// <param name="info">Information about the clips to blend.</param>
        /// <param name="t">Parameter that controls the blending, in range [(0, 0), (1, 1)]. t = (0, 0) means top left
        ///                 animation has full influence, t = (0, 1) means top right animation has full influence, 
        ///                 t = (1, 0) means bottom left animation has full influence, t = (1, 1) means bottom right
        ///                 animation has full influence.
        ///                 </param>
        public void Blend2D(Blend2DInfo info, Vector2 t)
        {
            if (_native != null)
                _native.Blend2D(info, t);
        }

        /// <summary>
        /// Fades the specified animation clip in, while fading other playing animation out, over the specified time period.
        /// </summary>
        /// <param name="clip">Clip to fade in.</param>
        /// <param name="fadeLength">Determines the time period over which the fade occurs. In seconds.</param>
        public void CrossFade(AnimationClip clip, float fadeLength)
        {
            if (_native != null)
                _native.CrossFade(clip, fadeLength);
        }

        /// <summary>
        /// Stops playing all animations on the provided layer.
        /// </summary>
        /// <param name="layer">Layer on which to stop animations on.</param>
        public void Stop(int layer)
        {
            if (_native != null)
                _native.Stop(layer);
        }

        /// <summary>
        /// Stops playing all animations.
        /// </summary>
        public void StopAll()
        {
            if (_native != null)
                _native.StopAll();
        }

        /// <summary>
        /// Retrieves detailed information about a currently playing animation clip.
        /// </summary>
        /// <param name="clip">Clip to retrieve the information for.</param>
        /// <param name="state">Animation clip state containing the requested information. Only valid if the method returns
        ///                     true.</param>
        /// <returns>True if the state was found (animation clip is playing), false otherwise.</returns>
        public bool GetState(AnimationClip clip, out AnimationClipState state)
        {
            if (_native != null)
                return _native.GetState(clip, out state);

            state = new AnimationClipState();
            return false;
        }

        /// <summary>
        /// Searches the scene object hierarchy to find a property at the given path.
        /// </summary>
        /// <param name="root">Root scene object to which the path is relative to.</param>
        /// <param name="path">Path to the property, where each element of the path is separated with "/". 
        /// 
        ///                    Path elements prefixed with "!" signify names of child scene objects (first one relative to 
        ///                    <paramref name="root"/>. Name of the root element should not be included in the path. 
        /// 
        ///                    Path element prefixed with ":" signify names of components. If a path doesn't have a
        ///                    component element, it is assumed the field is relative to the scene object itself (only 
        ///                    "Translation", "Rotation" and "Scale fields are supported in such case). Only one component
        ///                    path element per path is allowed.
        /// 
        ///                    Path entries with no prefix are considered regular script object fields. Each path must have
        ///                    at least one such entry. Last field entry can optionally have a suffix separated from the
        ///                    path name with ".". This suffix is not parsed internally, but will be returned as 
        ///                    <paramref name="suffix"/>.
        /// 
        ///                    Path examples:
        ///                     :MyComponent/myInt (path to myInt variable on a component attached to this object)
        ///                     !childSO/:MyComponent/myInt (path to myInt variable on a child scene object)
        ///                     !childSO/Translation (path to the scene object translation)
        ///                     :MyComponent/myVector.z (path to the z component of myVector on this object)
        ///                    </param>
        /// <param name="suffix">Suffix of the last field entry, if it has any. Contains the suffix separator ".".</param>
        /// <returns>If found, property object you can use for setting and getting the value from the property, otherwise 
        ///          null.</returns>
        internal static SerializableProperty FindProperty(SceneObject root, string path, out string suffix)
        {
            suffix = null;

            if (string.IsNullOrEmpty(path) || root == null)
                return null;

            string trimmedPath = path.Trim('/');
            string[] entries = trimmedPath.Split('/');

            // Find scene object referenced by the path
            SceneObject so = root;
            int pathIdx = 0;
            for (; pathIdx < entries.Length; pathIdx++)
            {
                string entry = entries[pathIdx];

                if (string.IsNullOrEmpty(entry))
                    continue;

                // Not a scene object, break
                if (entry[0] != '!')
                    break;

                string childName = entry.Substring(1, entry.Length - 1);
                so = so.FindChild(childName);

                if (so == null)
                    break;
            }

            // Child scene object couldn't be found
            if (so == null)
                return null;

            // Path too short, no field entry
            if (pathIdx >= entries.Length)
                return null;

            // Check if path is referencing a component, and if so find it
            Component component = null;
            {
                string entry = entries[pathIdx];
                if (entry[0] == ':')
                {
                    string componentName = entry.Substring(1, entry.Length - 1);

                    Component[] components = so.GetComponents();
                    component = Array.Find(components, x => x.GetType().Name == componentName);

                    // Cannot find component with specified type
                    if (component == null)
                        return null;
                }
            }

            // Look for a field within a component
            if (component != null)
            {
                pathIdx++;
                if (pathIdx >= entries.Length)
                    return null;

                SerializableObject componentObj = new SerializableObject(component);

                StringBuilder pathBuilder = new StringBuilder();
                for (; pathIdx < entries.Length - 1; pathIdx++)
                    pathBuilder.Append(entries[pathIdx] + "/");

                // Check last path entry for suffix and remove it
                int suffixIdx = entries[pathIdx].LastIndexOf(".");
                if (suffixIdx != -1)
                {
                    string entryNoSuffix = entries[pathIdx].Substring(0, suffixIdx);
                    suffix = entries[pathIdx].Substring(suffixIdx, entries[pathIdx].Length - suffixIdx);

                    pathBuilder.Append(entryNoSuffix);
                }
                else
                    pathBuilder.Append(entries[pathIdx]);

                return componentObj.FindProperty(pathBuilder.ToString());
            }
            else // Field is one of the builtin ones on the SceneObject itself
            {
                if ((pathIdx + 1) < entries.Length)
                    return null;

                string entry = entries[pathIdx];
                if (entry == "Position")
                {
                    SerializableProperty property = new SerializableProperty(
                        SerializableProperty.FieldType.Vector3,
                        typeof(Vector3),
                        () => so.LocalPosition,
                        (x) => so.LocalPosition = (Vector3) x);

                    return property;
                }
                else if (entry == "Rotation")
                {
                    SerializableProperty property = new SerializableProperty(
                        SerializableProperty.FieldType.Vector3,
                        typeof(Vector3),
                        () => so.LocalRotation.ToEuler(),
                        (x) => so.LocalRotation = Quaternion.FromEuler((Vector3) x));

                    return property;
                }
                else if (entry == "Scale")
                {
                    SerializableProperty property = new SerializableProperty(
                        SerializableProperty.FieldType.Vector3,
                        typeof(Vector3),
                        () => so.LocalScale,
                        (x) => so.LocalScale = (Vector3) x);

                    return property;
                }

                return null;
            }
        }

        /// <summary>
        /// Searches the scene object hierarchy to find a child scene object using the provided path.
        /// </summary>
        /// <param name="root">Root scene object to which the path is relative to.</param>
        /// <param name="path">Path to the property, where each element of the path is separated with "/". 
        /// 
        ///                    Path elements signify names of child scene objects (first one relative to 
        ///                    <paramref name="root"/>. Name of the root element should not be included in the path.
        ///                    Elements must be prefixed with "!" in order to match the path format of 
        ///                    <see cref="FindProperty"/>.</param>
        /// <returns>Child scene object if found, or null otherwise.</returns>
        internal static SceneObject FindSceneObject(SceneObject root, string path)
        {
            if (string.IsNullOrEmpty(path) || root == null)
                return null;

            string trimmedPath = path.Trim('/');
            string[] entries = trimmedPath.Split('/');

            // Find scene object referenced by the path
            SceneObject so = root;
            int pathIdx = 0;
            for (; pathIdx < entries.Length; pathIdx++)
            {
                string entry = entries[pathIdx];

                if (string.IsNullOrEmpty(entry))
                    continue;

                // Not a scene object, break
                if (entry[0] != '!')
                    break;

                string childName = entry.Substring(1, entry.Length - 1);
                so = so.FindChild(childName);

                if (so == null)
                    break;
            }

            return so;
        }

        /// <summary>
        /// Changes the state of a playing animation clip. If animation clip is not currently playing the state change is
        /// ignored.
        /// </summary>
        /// <param name="clip">Clip to change the state for.</param>
        /// <param name="state">New state of the animation (e.g. changing the time for seeking).</param>
        public void SetState(AnimationClip clip, AnimationClipState state)
        {
            if (_native != null)
                _native.SetState(clip, state);
        }

        private void OnUpdate()
        {
            if (_native == null)
                return;

            AnimationClip newPrimaryClip = _native.GetClip(0);
            if (newPrimaryClip != primaryClip)
            {
                RebuildFloatProperties(newPrimaryClip);
                primaryClip = newPrimaryClip;

                UpdateSceneObjectMapping();
            }

            // Apply values from generic float curves
            if (floatProperties != null)
            {
                foreach (var entry in floatProperties)
                {
                    float curveValue;
                    if (_native.GetGenericCurveValue(entry.curveIdx, out curveValue))
                        entry.setter(curveValue);
                }
            }
        }

        private void OnEnable()
        {
            RestoreNative();
        }

        private void OnDisable()
        {
            DestroyNative();
        }

        private void OnDestroy()
        {
            DestroyNative();
        }

        /// <summary>
        /// Creates the internal representation of the animation and restores the values saved by the component.
        /// </summary>
        private void RestoreNative()
        {
            if (_native != null)
                _native.Destroy();

            _native = new NativeAnimation();
            _native.OnEventTriggered += EventTriggered;

            // Restore saved values after reset
            _native.WrapMode = serializableData.wrapMode;
            _native.Speed = serializableData.speed;

            if (serializableData.defaultClip != null)
                _native.Play(serializableData.defaultClip);

            primaryClip = _native.GetClip(0);
            if (primaryClip != null)
                RebuildFloatProperties(primaryClip);

            SetBoneMappings();
            UpdateSceneObjectMapping();

            Renderable renderable = SceneObject.GetComponent<Renderable>();
            if (renderable == null)
                return;

            renderable.NativeAnimation = _native;
        }

        /// <summary>
        /// Destroys the internal animation representation.
        /// </summary>
        private void DestroyNative()
        {
            Renderable renderableComponent = SceneObject.GetComponent<Renderable>();
            if (renderableComponent != null)
            {
                NativeRenderable renderable = renderableComponent.Native;

                if (renderable != null)
                    renderable.Animation = null;
            }

            if (_native != null)
            {
                _native.Destroy();
                _native = null;
            }

            primaryClip = null;
            mappingInfo.Clear();
            floatProperties = null;
        }

        /// <summary>
        /// Finds any curves that affect a transform of a specific scene object, and ensures that animation properly updates
        /// those transforms. This does not include curves referencing bones.
        /// </summary>
        private void UpdateSceneObjectMapping()
        {
            List<SceneObjectMappingInfo> newMappingInfos = new List<SceneObjectMappingInfo>();
            foreach(var entry in mappingInfo)
            {
                if (entry.isMappedToBone)
                    newMappingInfos.Add(entry);
                else
                    _native.UnmapSceneObject(entry.sceneObject);
            }

            if (primaryClip != null)
            {
                SceneObject root = SceneObject;

                Action<NamedVector3Curve[]> findMappings = x =>
                {
                    foreach (var curve in x)
                    {
                        if (curve.Flags.HasFlag(AnimationCurveFlags.ImportedCurve))
                            continue;

                        SceneObject currentSO = FindSceneObject(root, curve.Name);

                        bool found = false;
                        for (int i = 0; i < newMappingInfos.Count; i++)
                        {
                            if (newMappingInfos[i].sceneObject == currentSO)
                            {
                                found = true;
                                break;
                            }
                        }

                        if (!found)
                        {
                            SceneObjectMappingInfo newMappingInfo = new SceneObjectMappingInfo();
                            newMappingInfo.isMappedToBone = false;
                            newMappingInfo.sceneObject = currentSO;

                            newMappingInfos.Add(newMappingInfo);

                            _native.MapCurveToSceneObject(curve.Name, currentSO);
                        }
                    }
                };

                AnimationCurves curves = primaryClip.Curves;
                findMappings(curves.PositionCurves);
                findMappings(curves.RotationCurves);
                findMappings(curves.ScaleCurves);
            }

            mappingInfo = newMappingInfos;
        }

        /// <summary>
        /// Registers a new bone component, creating a new transform mapping from the bone name to the scene object
        /// the component is attached to.
        /// </summary>
        /// <param name="bone">Bone component to register.</param>
        internal void AddBone(Bone bone)
        {
            if (_native == null)
                return;

            SceneObject currentSO = bone.SceneObject;

            SceneObjectMappingInfo newMapping = new SceneObjectMappingInfo();
            newMapping.sceneObject = currentSO;
            newMapping.isMappedToBone = true;
            newMapping.bone = bone;

            mappingInfo.Add(newMapping);
            _native.MapCurveToSceneObject(bone.Name, newMapping.sceneObject);
        }

        /// <summary>
        /// Unregisters a bone component, removing the bone -> scene object mapping.
        /// </summary>
        /// <param name="bone">Bone to unregister.</param>
        internal void RemoveBone(Bone bone)
        {
            if (_native == null)
                return;

            SceneObject newSO = null;
            for (int i = 0; i < mappingInfo.Count; i++)
            {
                if (mappingInfo[i].bone == bone)
                {
                    mappingInfo.RemoveAt(i);
                    _native.UnmapSceneObject(mappingInfo[i].sceneObject);
                    i--;
                }
            }
        }

        /// <summary>
        /// Called whenever the bone the <see cref="Bone"/> component points to changed.
        /// </summary>
        /// <param name="bone">Bone component to modify.</param>
        internal void NotifyBoneChanged(Bone bone)
        {
            if (_native == null)
                return;

            for (int i = 0; i < mappingInfo.Count; i++)
            {
                if (mappingInfo[i].bone == bone)
                {
                    _native.UnmapSceneObject(mappingInfo[i].sceneObject);
                    _native.MapCurveToSceneObject(bone.Name, mappingInfo[i].sceneObject);
                    break;
                }
            }
        }

        /// <summary>
        /// Finds any scene objects that are mapped to bone transforms. Such object's transforms will be affected by
        /// skeleton bone animation.
        /// </summary>
        private void SetBoneMappings()
        {
            mappingInfo.Clear();

            SceneObjectMappingInfo rootMapping = new SceneObjectMappingInfo();
            rootMapping.sceneObject = SceneObject;
            rootMapping.isMappedToBone = true;

            mappingInfo.Add(rootMapping);
            _native.MapCurveToSceneObject("", rootMapping.sceneObject);

            Bone[] childBones = FindChildBones();
            foreach (var entry in childBones)
                AddBone(entry);
        }

        /// <summary>
        /// Searches child scene objects for <see cref="Bone"/> components and returns any found ones.
        /// </summary>
        private Bone[] FindChildBones()
        {
            Stack<SceneObject> todo = new Stack<SceneObject>();
            todo.Push(SceneObject);

            List<Bone> bones = new List<Bone>();
            while (todo.Count > 0)
            {
                SceneObject currentSO = todo.Pop();

                Bone bone = currentSO.GetComponent<Bone>();
                if (bone != null)
                {
                    bone.SetParent(this, true);
                    bones.Add(bone);
                }

                int childCount = currentSO.GetNumChildren();
                for (int i = 0; i < childCount; i++)
                {
                    SceneObject child = currentSO.GetChild(i);
                    if (child.GetComponent<Animation>() != null)
                        continue;

                    todo.Push(child);
                }
            }

            return bones.ToArray();
        }

        /// <summary>
        /// Builds a list of properties that will be animated using float animation curves.
        /// </summary>
        /// <param name="clip">Clip to retrieve the float animation curves from.</param>
        private void RebuildFloatProperties(AnimationClip clip)
        {
            if (clip == null)
            {
                floatProperties = null;
                return;
            }

            AnimationCurves curves = clip.Curves;

            List<FloatCurvePropertyInfo> newFloatProperties = new List<FloatCurvePropertyInfo>();
            for (int i = 0; i < curves.FloatCurves.Length; i++)
            {
                string suffix;
                SerializableProperty property = FindProperty(SceneObject, curves.FloatCurves[i].Name, out suffix);
                if (property == null)
                    continue;

                int elementIdx = 0;
                if (!string.IsNullOrEmpty(suffix))
                {
                    PropertySuffixInfo suffixInfo;
                    if (PropertySuffixInfos.TryGetValue(suffix, out suffixInfo))
                        elementIdx = suffixInfo.elementIdx;
                }

                Action<float> setter = null;

                Type internalType = property.InternalType;
                switch (property.Type)
                {
                    case SerializableProperty.FieldType.Vector2:
                        if (internalType == typeof(Vector2))
                        {
                            setter = f =>
                            {
                                Vector2 value = property.GetValue<Vector2>();
                                value[elementIdx] = f;
                                property.SetValue(value);
                            };
                        }

                        break;
                    case SerializableProperty.FieldType.Vector3:
                        if (internalType == typeof(Vector3))
                        {
                            setter = f =>
                            {
                                Vector3 value = property.GetValue<Vector3>();
                                value[elementIdx] = f;
                                property.SetValue(value);
                            };
                        }
                        break;
                    case SerializableProperty.FieldType.Vector4:
                        if (internalType == typeof(Vector4))
                        {
                            setter = f =>
                            {
                                Vector4 value = property.GetValue<Vector4>();
                                value[elementIdx] = f;
                                property.SetValue(value);
                            };
                        }
                        else if (internalType == typeof(Quaternion))
                        {
                            setter = f =>
                            {
                                Quaternion value = property.GetValue<Quaternion>();
                                value[elementIdx] = f;
                                property.SetValue(value);
                            };
                        }
                        break;
                    case SerializableProperty.FieldType.Color:
                        if (internalType == typeof(Color))
                        {
                            setter = f =>
                            {
                                Color value = property.GetValue<Color>();
                                value[elementIdx] = f;
                                property.SetValue(value);
                            };
                        }
                        break;
                    case SerializableProperty.FieldType.Bool:
                            setter = f =>
                            {
                                bool value = f > 0.0f;
                                property.SetValue(value);
                            };
                        break;
                    case SerializableProperty.FieldType.Int:
                        setter = f =>
                        {
                            int value = (int)f;
                            property.SetValue(value);
                        };
                        break;
                    case SerializableProperty.FieldType.Float:
                        setter = f =>
                        {
                            property.SetValue(f);
                        };
                        break;
                }

                if (setter == null)
                    continue;

                FloatCurvePropertyInfo propertyInfo = new FloatCurvePropertyInfo();
                propertyInfo.curveIdx = i;
                propertyInfo.setter = setter;

                newFloatProperties.Add(propertyInfo);
            }

            floatProperties = newFloatProperties.ToArray();
        }

        /// <summary>
        /// Called whenever an animation event triggers.
        /// </summary>
        /// <param name="clip">Clip that the event originated from.</param>
        /// <param name="name">Name of the event.</param>
        private void EventTriggered(AnimationClip clip, string name)
        {
            // Event should be in format "ComponentType/MethodName"
            if (string.IsNullOrEmpty(name))
                return;

            string[] nameEntries = name.Split('/');
            if (nameEntries.Length != 2)
                return;

            string typeName = nameEntries[0];
            string methodName = nameEntries[1];

            Component[] components = SceneObject.GetComponents();
            for (int i = 0; i < components.Length; i++)
            {
                if (components[i].GetType().Name == typeName)
                {
                    components[i].Invoke(methodName);
                    break;
                }
            }
        }

        /// <summary>
        /// Holds all data the animation component needs to persist through serialization.
        /// </summary>
        [SerializeObject]
        private class SerializableData
        {
            public AnimationClip defaultClip;
            public AnimWrapMode wrapMode = AnimWrapMode.Loop;
            public float speed = 1.0f;
        }

        /// <summary>
        /// Contains information about a property animated by a generic animation curve.
        /// </summary>
        private class FloatCurvePropertyInfo
        {
            public int curveIdx;
            public Action<float> setter;
        }

        /// <summary>
        /// Information about a suffix used in a property path.
        /// </summary>
        internal struct PropertySuffixInfo
        {
            public PropertySuffixInfo(int elementIdx, bool isVector)
            {
                this.elementIdx = elementIdx;
                this.isVector = isVector;
            }

            public int elementIdx;
            public bool isVector;
        }

        /// <summary>
        /// Information about scene objects bound to a specific animation curve.
        /// </summary>
        internal struct SceneObjectMappingInfo
        {
            public SceneObject sceneObject;
            public bool isMappedToBone;
            public Bone bone;
        }
    }

    /** @} */
}