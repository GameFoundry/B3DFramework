//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Collections.Generic;
using System.Text;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup AnimationEditor
     *  @{
     */


    /// <summary>
    /// A set of animation curves for a field of a certain type.
    /// </summary>
    internal struct FieldAnimCurves
    {
        public SerializableProperty.FieldType type;
        public EdAnimationCurve[] curves;
    }

    /// <summary>
    /// Stores tangent modes for an 3D vector animation curve (one mode for each keyframe).
    /// </summary>
    [SerializeObject]
    internal class EditorVector3CurveTangents
    {
        public string name;
        public TangentMode[] tangentsX;
        public TangentMode[] tangentsY;
        public TangentMode[] tangentsZ;
    }

    /// <summary>
    /// Stores tangent modes for an float animation curve (one mode for each keyframe).
    /// </summary>
    [SerializeObject]
    internal class EditorFloatCurveTangents
    {
        public string name;
        public TangentMode[] tangents;
    }

    /// <summary>
    /// Stores tangent information for all curves in an animation clip.
    /// </summary>
    [SerializeObject]
    internal class EditorAnimClipTangents
    {
        public EditorVector3CurveTangents[] positionCurves;
        public EditorVector3CurveTangents[] rotationCurves;
        public EditorVector3CurveTangents[] scaleCurves;
        public EditorFloatCurveTangents[] floatCurves;
    }

    /// <summary>
    /// Stores animation clip data for clips that are currently being edited.
    /// </summary>
    internal class EditorAnimClipInfo
    {
        public AnimationClip clip;
        public Dictionary<string, FieldAnimCurves> curves = new Dictionary<string, FieldAnimCurves>();
        public AnimationEvent[] events = new AnimationEvent[0];

        /// <summary>
        /// Loads curve and event information from the provided clip, and creates a new instance of this object containing
        /// the required data for editing the source clip in the animation editor.
        /// </summary>
        /// <param name="clip">Clip to load.</param>
        /// <returns>Editor specific editable information about an animation clip.</returns>
        public static EditorAnimClipInfo Create(AnimationClip clip)
        {
            EditorAnimClipInfo clipInfo = new EditorAnimClipInfo();
            clipInfo.clip = clip;

            AnimationCurves clipCurves = clip.Curves;
            EditorAnimClipTangents editorCurveData = null;

            string resourcePath = ProjectLibrary.GetPath(clip);
            if (!string.IsNullOrEmpty(resourcePath))
            {
                LibraryEntry entry = ProjectLibrary.GetEntry(resourcePath);
                string clipName = PathEx.GetTail(resourcePath);

                if (entry != null && entry.Type == LibraryEntryType.File)
                {
                    FileEntry fileEntry = (FileEntry)entry;
                    ResourceMeta[] metas = fileEntry.ResourceMetas;

                    for (int i = 0; i < metas.Length; i++)
                    {
                        if (clipName == metas[i].SubresourceName)
                        {
                            editorCurveData = metas[i].EditorData as EditorAnimClipTangents;
                            break;
                        }
                    }
                }
            }

            if (editorCurveData == null)
                editorCurveData = new EditorAnimClipTangents();

            Action<NamedVector3Curve[], EditorVector3CurveTangents[], string> loadVector3Curve =
                (curves, tangents, subPath) =>
                {
                    foreach (var curveEntry in curves)
                    {
                        TangentMode[] tangentsX = null;
                        TangentMode[] tangentsY = null;
                        TangentMode[] tangentsZ = null;
                        foreach (var tangentEntry in tangents)
                        {
                            if (tangentEntry.name == curveEntry.Name)
                            {
                                tangentsX = tangentEntry.tangentsX;
                                tangentsY = tangentEntry.tangentsY;
                                tangentsZ = tangentEntry.tangentsZ;
                                break;
                            }
                        }

                        FieldAnimCurves fieldCurves = new FieldAnimCurves();
                        fieldCurves.type = SerializableProperty.FieldType.Vector3;
                        fieldCurves.curves = new EdAnimationCurve[3];

                        fieldCurves.curves[0] = new EdAnimationCurve(curveEntry.X, tangentsX);
                        fieldCurves.curves[1] = new EdAnimationCurve(curveEntry.Y, tangentsY);
                        fieldCurves.curves[2] = new EdAnimationCurve(curveEntry.Z, tangentsZ);

                        string curvePath = curveEntry.Name.TrimEnd('/') + subPath;
                        clipInfo.curves[curvePath] = fieldCurves;
                    }
                };

            loadVector3Curve(clipCurves.PositionCurves, editorCurveData.positionCurves, "/Position");
            loadVector3Curve(clipCurves.RotationCurves, editorCurveData.rotationCurves, "/Rotation");
            loadVector3Curve(clipCurves.ScaleCurves, editorCurveData.scaleCurves, "/Scale");

            // Find which individual float curves belong to the same field
            Dictionary<string, Tuple<int, int, bool>[]> floatCurveMapping = new Dictionary<string, Tuple<int, int, bool>[]>();
            {
                int curveIdx = 0;
                foreach (var curveEntry in clipCurves.FloatCurves)
                {
                    string path = curveEntry.Name;
                    string pathNoSuffix = null;

                    string pathSuffix;
                    if (path.Length >= 2)
                    {
                        pathSuffix = path.Substring(path.Length - 2, 2);
                        pathNoSuffix = path.Substring(0, path.Length - 2);
                    }
                    else
                        pathSuffix = "";

                    int tangentIdx = -1;
                    int currentTangentIdx = 0;
                    foreach (var tangentEntry in editorCurveData.floatCurves)
                    {
                        if (tangentEntry.name == curveEntry.Name)
                        {
                            tangentIdx = currentTangentIdx;
                            break;
                        }

                        currentTangentIdx++;
                    }

                    Animation.PropertySuffixInfo suffixInfo;
                    if (Animation.PropertySuffixInfos.TryGetValue(pathSuffix, out suffixInfo))
                    {
                        Tuple<int, int, bool>[] curveInfo;
                        if (!floatCurveMapping.TryGetValue(pathNoSuffix, out curveInfo))
                            curveInfo = new Tuple<int, int, bool>[4];

                        curveInfo[suffixInfo.elementIdx] = Tuple.Create(curveIdx, tangentIdx, suffixInfo.isVector);
                        floatCurveMapping[pathNoSuffix] = curveInfo;
                    }
                    else
                    {
                        Tuple<int, int, bool>[] curveInfo = new Tuple<int, int, bool>[4];
                        curveInfo[0] = Tuple.Create(curveIdx, tangentIdx, suffixInfo.isVector);

                        floatCurveMapping[path] = curveInfo;
                    }

                    curveIdx++;
                }
            }

            foreach (var KVP in floatCurveMapping)
            {
                int numCurves = 0;
                for (int i = 0; i < 4; i++)
                {
                    if (KVP.Value[i] == null)
                        continue;

                    numCurves++;
                }

                if (numCurves == 0)
                    continue; // Invalid curve

                FieldAnimCurves fieldCurves = new FieldAnimCurves();

                // Deduce type (note that all single value types are assumed to be float even if their source type is int or bool)
                if (numCurves == 1)
                    fieldCurves.type = SerializableProperty.FieldType.Float;
                else if (numCurves == 2)
                    fieldCurves.type = SerializableProperty.FieldType.Vector2;
                else if (numCurves == 3)
                    fieldCurves.type = SerializableProperty.FieldType.Vector3;
                else // 4 curves
                {
                    bool isVector = KVP.Value[0].Item3;
                    if (isVector)
                        fieldCurves.type = SerializableProperty.FieldType.Vector4;
                    else
                        fieldCurves.type = SerializableProperty.FieldType.Color;
                }

                fieldCurves.curves = new EdAnimationCurve[numCurves];

                for (int i = 0; i < numCurves; i++)
                {
                    int curveIdx = KVP.Value[i].Item1;
                    int tangentIdx = KVP.Value[i].Item2;

                    TangentMode[] tangents = null;
                    if (tangentIdx != -1)
                        tangents = editorCurveData.floatCurves[tangentIdx].tangents;

                    fieldCurves.curves[i] = new EdAnimationCurve(clipCurves.FloatCurves[curveIdx].Curve, tangents);
                }

                string curvePath = KVP.Key;
                clipInfo.curves[curvePath] = fieldCurves;
            }

            // Add events
            clipInfo.events = clip.Events;
            return clipInfo;
        }

        /// <summary>
        /// Checks is the specified animation clip is imported from an external file, or created within the editor.
        /// </summary>
        /// <param name="clip">Clip to check.</param>
        /// <returns>True if the clip is imported from an external file (e.g. FBX file), or false if the clip is a native
        ///          resource created within the editor.</returns>
        public static bool IsClipImported(AnimationClip clip)
        {
            string resourcePath = ProjectLibrary.GetPath(clip);
            return ProjectLibrary.IsSubresource(resourcePath);
        }

        /// <summary>
        /// Saves the animation curves and events stored in this object, into the associated animation clip resource.
        /// Relevant animation clip resource must already be created and exist in the project library.
        /// </summary>
        public void SaveToClip()
        {
            bool clipIsImported = IsClipImported(clip);

            if (!clipIsImported)
            {
                List<NamedVector3Curve> positionCurves = new List<NamedVector3Curve>();
                List<NamedVector3Curve> rotationCurves = new List<NamedVector3Curve>();
                List<NamedVector3Curve> scaleCurves = new List<NamedVector3Curve>();
                List<NamedFloatCurve> floatCurves = new List<NamedFloatCurve>();

                List<EditorVector3CurveTangents> positionTangents = new List<EditorVector3CurveTangents>();
                List<EditorVector3CurveTangents> rotationTangents = new List<EditorVector3CurveTangents>();
                List<EditorVector3CurveTangents> scaleTangents = new List<EditorVector3CurveTangents>();
                List<EditorFloatCurveTangents> floatTangents = new List<EditorFloatCurveTangents>();

                foreach (var kvp in curves)
                {
                    string[] pathEntries = kvp.Key.Split('/');
                    if (pathEntries.Length == 0)
                        continue;

                    string lastEntry = pathEntries[pathEntries.Length - 1];

                    if (lastEntry == "Position" || lastEntry == "Rotation" || lastEntry == "Scale")
                    {
                        StringBuilder sb = new StringBuilder();
                        for (int i = 0; i < pathEntries.Length - 2; i++)
                            sb.Append(pathEntries[i] + "/");

                        if (pathEntries.Length > 1)
                            sb.Append(pathEntries[pathEntries.Length - 2]);

                        string curvePath = sb.ToString();

                        NamedVector3Curve curve = new NamedVector3Curve(curvePath,
                            new AnimationCurve(kvp.Value.curves[0].KeyFrames),
                            new AnimationCurve(kvp.Value.curves[1].KeyFrames),
                            new AnimationCurve(kvp.Value.curves[2].KeyFrames));

                        EditorVector3CurveTangents tangents = new EditorVector3CurveTangents();
                        tangents.name = curvePath;
                        tangents.tangentsX = kvp.Value.curves[0].TangentModes;
                        tangents.tangentsY = kvp.Value.curves[1].TangentModes;
                        tangents.tangentsZ = kvp.Value.curves[2].TangentModes;

                        if (lastEntry == "Position")
                        {
                            positionCurves.Add(curve);
                            positionTangents.Add(tangents);
                        }
                        else if (lastEntry == "Rotation")
                        {
                            rotationCurves.Add(curve);
                            rotationTangents.Add(tangents);
                        }
                        else if (lastEntry == "Scale")
                        {
                            scaleCurves.Add(curve);
                            scaleTangents.Add(tangents);
                        }
                    }
                    else
                    {
                        Action<int, string> addCurve = (idx, subPath) =>
                        {
                            string path = kvp.Key + subPath;

                            NamedFloatCurve curve = new NamedFloatCurve(path,
                            new AnimationCurve(kvp.Value.curves[idx].KeyFrames));

                            EditorFloatCurveTangents tangents = new EditorFloatCurveTangents();
                            tangents.name = path;
                            tangents.tangents = kvp.Value.curves[idx].TangentModes;

                            floatCurves.Add(curve);
                            floatTangents.Add(tangents);
                        };

                        switch (kvp.Value.type)
                        {
                            case SerializableProperty.FieldType.Vector2:
                                addCurve(0, ".x");
                                addCurve(1, ".y");
                                break;
                            case SerializableProperty.FieldType.Vector3:
                                addCurve(0, ".x");
                                addCurve(1, ".y");
                                addCurve(2, ".z");
                                break;
                            case SerializableProperty.FieldType.Vector4:
                                addCurve(0, ".x");
                                addCurve(1, ".y");
                                addCurve(2, ".z");
                                addCurve(3, ".w");
                                break;
                            case SerializableProperty.FieldType.Color:
                                addCurve(0, ".r");
                                addCurve(1, ".g");
                                addCurve(2, ".b");
                                addCurve(3, ".a");
                                break;
                            case SerializableProperty.FieldType.Bool:
                            case SerializableProperty.FieldType.Int:
                            case SerializableProperty.FieldType.Float:
                                addCurve(0, "");
                                break;
                        }
                    }
                }

                AnimationCurves newClipCurves = new AnimationCurves();
                newClipCurves.PositionCurves = positionCurves.ToArray();
                newClipCurves.RotationCurves = rotationCurves.ToArray();
                newClipCurves.ScaleCurves = scaleCurves.ToArray();
                newClipCurves.FloatCurves = floatCurves.ToArray();

                clip.Curves = newClipCurves;
                clip.Events = events;

                string resourcePath = ProjectLibrary.GetPath(clip);
                ProjectLibrary.Save(clip);

                // Save tangents for editor only use
                LibraryEntry entry = ProjectLibrary.GetEntry(resourcePath);
                string clipName = PathEx.GetTail(resourcePath);

                if (entry != null && entry.Type == LibraryEntryType.File)
                {
                    FileEntry fileEntry = (FileEntry)entry;
                    ResourceMeta[] metas = fileEntry.ResourceMetas;

                    for (int i = 0; i < metas.Length; i++)
                    {
                        if (clipName == metas[i].SubresourceName)
                        {
                            EditorAnimClipTangents newCurveData = new EditorAnimClipTangents();
                            newCurveData.positionCurves = positionTangents.ToArray();
                            newCurveData.rotationCurves = rotationTangents.ToArray();
                            newCurveData.scaleCurves = scaleTangents.ToArray();
                            newCurveData.floatCurves = floatTangents.ToArray();

                            ProjectLibrary.SetEditorData(resourcePath, newCurveData);
                            break;
                        }
                    }
                }
            }
            else
            {
                string resourcePath = ProjectLibrary.GetPath(clip);
                LibraryEntry entry = ProjectLibrary.GetEntry(resourcePath);

                if (entry != null && entry.Type == LibraryEntryType.File)
                {
                    FileEntry fileEntry = (FileEntry)entry;
                    MeshImportOptions meshImportOptions = (MeshImportOptions)fileEntry.Options;

                    string clipName = PathEx.GetTail(resourcePath);

                    List<ImportedAnimationEvents> newEvents = new List<ImportedAnimationEvents>();
                    newEvents.AddRange(meshImportOptions.AnimationEvents);

                    bool isExisting = false;
                    for (int i = 0; i < newEvents.Count; i++)
                    {
                        if (newEvents[i].name == clipName)
                        {
                            newEvents[i].events = events;
                            isExisting = true;
                            break;
                        }
                    }

                    if (!isExisting)
                    {
                        ImportedAnimationEvents newEntry = new ImportedAnimationEvents();
                        newEntry.name = clipName;
                        newEntry.events = events;

                        newEvents.Add(newEntry);
                    }

                    meshImportOptions.AnimationEvents = newEvents.ToArray();

                    ProjectLibrary.Reimport(resourcePath, meshImportOptions, true);
                }
            }
        }
    }

    /** @} */
}
          