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

    public partial class Animation
    {
        private FloatCurvePropertyInfo[] floatProperties;

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
        /// Allows the caller to play an animation clip during edit mode. This form of animation playback is limited as
        /// you have no control over clip properties, and features like blending, cross fade or animation events are not
        /// supported.
        /// 
        /// Caller will need to manually call <see cref="UpdateFloatProperties"/> in order to apply evaluated animation data
        /// to relevant float properties (if required).
        /// 
        /// Caller will also need to manually call <see cref="RefreshClipMappings"/> whenever the curves internal to the 
        /// animation clip change. This should be called before the call to <see cref="UpdateFloatProperties"/>.
        /// </summary>
        /// <param name="clip">Animation clip to play.</param>
        /// <param name="startTime">Time to start playing at, in seconds.</param>
        /// <param name="freeze">If true, only the frame at the specified time will be shown, without advancing the 
        ///                      animation.</param>
        internal void EditorPlay(AnimationClip clip, float startTime, bool freeze = false)
        {
            bool inPreviewMode = Internal__togglePreviewMode(mCachedPtr, true);

            if (!inPreviewMode)
                return;

            if (freeze)
                Sample(clip, startTime);
            else
            {
                AnimationClipState clipState = AnimationClipState.Default();
                clipState.time = startTime;

                SetState(clip, clipState);
            }

            Internal__refreshClipMappings(mCachedPtr);
        }

        /// <summary>
        /// Stops playback of animation whose playback what started using <see cref="EditorPlay"/>.
        /// </summary>
        internal void EditorStop()
        {
            Internal__togglePreviewMode(mCachedPtr, false);
        }

        /// <summary>
        /// Returns the current time of the currently playing editor animation clip.
        /// </summary>
        /// <returns>Time in seconds.</returns>
        internal float EditorGetTime()
        {
            AnimationClip clip = Internal_getClip(mCachedPtr, 0);

            AnimationClipState clipState;
            if (clip != null && GetState(clip, out clipState))
                return clipState.time;

            return 0.0f;
        }

        /// <summary>
        /// Updates generic float properties on relevant objects, based on the most recently evaluated animation curve 
        /// values.
        /// </summary>
        internal void UpdateFloatProperties()
        {
            _UpdateFloatProperties();
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
        ///                    "Position", "Rotation" and "Scale" fields are supported in such case). Only one component
        ///                    path element per path is allowed.
        /// 
        ///                    Path entries with no prefix are considered regular script object fields. Each path must have
        ///                    at least one such entry. 
        /// 
        ///                    A field path can be followed by an indexer [n] where n is a zero-based index. Such paths
        ///                    are assumed to be referencing an index within an array or a list.
        /// 
        ///                    A field path can also be followed by a suffix (after the indexer, if any) separated from the
        ///                    path name with ".". This suffix is not parsed internally, but will be returned as 
        ///                    <paramref name="suffix"/>.
        /// 
        ///                    Path examples:
        ///                     :MyComponent/myInt (path to myInt variable on a component attached to the root object)
        ///                     :MyComponent/myArray[0] (path to first element of myArray on the same component as above)
        ///                     !childSO/:MyComponent/myInt (path to myInt variable on a child scene object)
        ///                     !childSO/Position (path to the scene object position)
        ///                     :MyComponent/myVector.z (path to the z component of myVector on the root object)
        /// </param>
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
                        (x) => so.LocalPosition = (Vector3)x);

                    return property;
                }
                else if (entry == "Rotation")
                {
                    SerializableProperty property = new SerializableProperty(
                        SerializableProperty.FieldType.Vector3,
                        typeof(Vector3),
                        () => so.LocalRotation.ToEuler(),
                        (x) => so.LocalRotation = Quaternion.FromEuler((Vector3)x));

                    return property;
                }
                else if (entry == "Scale")
                {
                    SerializableProperty property = new SerializableProperty(
                        SerializableProperty.FieldType.Vector3,
                        typeof(Vector3),
                        () => so.LocalScale,
                        (x) => so.LocalScale = (Vector3)x);

                    return property;
                }

                return null;
            }
        }

        /// <summary>
        /// Builds a list of properties that will be animated using float animation curves.
        /// </summary>
        /// <param name="clip">Clip to retrieve the float animation curves from.</param>
        partial void RebuildFloatProperties(AnimationClip clip)
        {
            if (clip == null)
            {
                floatProperties = null;
                return;
            }

            AnimationCurves curves = clip.Curves;

            List<FloatCurvePropertyInfo> newFloatProperties = new List<FloatCurvePropertyInfo>();
            for (int i = 0; i < curves.Generic.Length; i++)
            {
                bool isMorphCurve = curves.Generic[i].flags.HasFlag(AnimationCurveFlags.MorphWeight) ||
                                    curves.Generic[i].flags.HasFlag(AnimationCurveFlags.MorphFrame);

                if (isMorphCurve)
                    continue;

                string suffix;
                SerializableProperty property = FindProperty(SceneObject, curves.Generic[i].name, out suffix);
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
        partial void EventTriggered(AnimationClip clip, string name)
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
        /// Partial method implementation for <see cref="UpdateFloatProperties"/>
        /// </summary>
        partial void _UpdateFloatProperties()
        {
            // Apply values from generic float curves
            if (floatProperties != null)
            {
                foreach (var entry in floatProperties)
                {
                    float curveValue;
                    if (Internal__getGenericCurveValue(mCachedPtr, (uint)entry.curveIdx, out curveValue))
                        entry.setter(curveValue);
                }
            }
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
    }

    /** @} */
}