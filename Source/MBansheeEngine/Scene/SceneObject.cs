//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /** @addtogroup Scene
     *  @{
     */

    /// <summary>
    /// An object in the scene graph. It has a position, place in the hierarchy and optionally a number of attached 
    /// components.
    /// </summary>
    public sealed class SceneObject : GameObject
    {
        /// <summary>
        /// Name of the scene object.
        /// </summary>
        public string Name
        {
            get { return Internal_GetName(mCachedPtr); }
            set { Internal_SetName(mCachedPtr, value); }
        }

        /// <summary>
        /// Parent in the scene object hierarchy. Null for hierarchy root.
        /// </summary>
        public SceneObject Parent
        {
            get { return Internal_GetParent(mCachedPtr); }
            set { Internal_SetParent(mCachedPtr, value); }
        }

        /// <summary>
        /// Determines if the object's components are being updated or not.
        /// </summary>
        public bool Active
        {
            get { return Internal_GetActive(mCachedPtr); }
            set { Internal_SetActive(mCachedPtr, value); }
        }

        /// <summary>
        /// Sets the mobility of a scene object. This is used primarily as a performance hint to engine systems. Objects
        /// with more restricted mobility will result in higher performance. Some mobility constraints will be enforced by
        /// the engine itself, while for others the caller must be sure not to break the promise he made when mobility was
        /// set. By default scene object's mobility is unrestricted.
        /// </summary>
        public ObjectMobility Mobility
        {
            get { return (ObjectMobility)Internal_GetMobility(mCachedPtr); }
            set { Internal_SetMobility(mCachedPtr, (int)value); }
        }

        /// <summary>
        /// World position. This includes local position of this object, plus position offset of any parents.
        /// </summary>
        public Vector3 Position
        {
            get
            {
                Vector3 value;
                Internal_GetPosition(mCachedPtr, out value);
                return value;
            }

            set
            {
                Internal_SetPosition(mCachedPtr, ref value);
            }
        }

        /// <summary>
        /// Local space position (relative to the parent).
        /// </summary>
        public Vector3 LocalPosition
        {
            get
            {
                Vector3 value;
                Internal_GetLocalPosition(mCachedPtr, out value);
                return value;
            }

            set
            {
                Internal_SetLocalPosition(mCachedPtr, ref value);
            }
        }

        /// <summary>
        /// World rotation. This includes local rotation of this object, plus rotation of any parents.
        /// </summary>
        public Quaternion Rotation
        {
            get
            {
                Quaternion value;
                Internal_GetRotation(mCachedPtr, out value);
                return value;
            }

            set
            {
                Internal_SetRotation(mCachedPtr, ref value);
            }
        }

        /// <summary>
        /// Local rotation (relative to the parent).
        /// </summary>
        public Quaternion LocalRotation
        {
            get
            {
                Quaternion value;
                Internal_GetLocalRotation(mCachedPtr, out value);
                return value;
            }

            set
            {
                Internal_SetLocalRotation(mCachedPtr, ref value);
            }
        }

        /// <summary>
        /// World space scale. This includes local scale of this object, plus scale of any parent.
        /// </summary>
        public Vector3 Scale
        {
            get
            {
                Vector3 value;
                Internal_GetScale(mCachedPtr, out value);
                return value;
            }
        }

        /// <summary>
        /// Local scale (relative to the parent).
        /// </summary>
        public Vector3 LocalScale
        {
            get
            {
                Vector3 value;
                Internal_GetLocalScale(mCachedPtr, out value);
                return value;
            }

            set
            {
                Internal_SetLocalScale(mCachedPtr, ref value);
            }
        }

        /// <summary>
        /// Returns the world transform matrix. This matrix accounts for position, rotation and scale transformations
        /// relative to the world basis.
        /// </summary>
        public Matrix4 WorldTransform
        {
            get
            {
                Matrix4 value;
                Internal_GetWorldTransform(mCachedPtr, out value);
                return value;
            }
        }

        /// <summary>
        /// Returns the local transform matrix. This matrix accounts for position, rotation and scale transformations
        /// relative to the parent's basis.
        /// </summary>
        public Matrix4 LocalTransform
        {
            get
            {
                Matrix4 value;
                Internal_GetLocalTransform(mCachedPtr, out value);
                return value;
            }
        }

        /// <summary>
        /// Direction in world space that points along the local negative Z axis.
        /// </summary>
        public Vector3 Forward
        {
            get
            {
                Vector3 value;
                Internal_GetForward(mCachedPtr, out value);
                return value;
            }
            set
            {
                Internal_SetForward(mCachedPtr, ref value);
            }
        }

        /// <summary>
        /// Direction in world space that points along the local positive X axis.
        /// </summary>
        public Vector3 Right
        {
            get
            {
                Vector3 value;
                Internal_GetRight(mCachedPtr, out value);
                return value;
            }
        }

        /// <summary>
        /// Direction in world space that points along the local positive Y axis.
        /// </summary>
        public Vector3 Up
        {
            get
            {
                Vector3 value;
                Internal_GetUp(mCachedPtr, out value);
                return value;
            }
        }

        /// <summary>
        /// Constructor for internal use by the runtime.
        /// </summary>
        private SceneObject()
        {
            
        }

        /// <summary>
        /// Creates a new scene object. Object will initially be parented to scene root and placed at the world origin.
        /// </summary>
        /// <param name="name">Name of the scene object.</param>
        public SceneObject(string name)
        {
            Internal_CreateInstance(this, name, 0);
        }

        /// <summary>
        /// Creates a new scene object. Object will initially be parented to scene root and placed at the world origin.
        /// </summary>
        /// <param name="name">Name of the scene object.</param>
        /// <param name="isInternal">Specifies this object is for internal use by the runtime. Internal object will not
        ///                          get saved, nor will they be displayed in the editor during non-debug mode.</param>
        internal SceneObject(string name, bool isInternal)
        {
            if(isInternal)
                Internal_CreateInstance(this, name, (int)(SceneObjectEditorFlags.DontSave | SceneObjectEditorFlags.Internal | SceneObjectEditorFlags.Persistent));
            else
                Internal_CreateInstance(this, name, 0);
        }

        /// <summary>
        /// Constructs a new component of the specified type and adds it to the internal component list.
        /// </summary>
        /// <typeparam name="T">Type of component to create.</typeparam>
        /// <returns>Instance of the new component.</returns>
        public T AddComponent<T>() where T : Component
        {
            return (T)Component.Internal_AddComponent(this, typeof (T));
        }

        /// <summary>
        /// Constructs a new component of the specified type and adds it to the internal component list.
        /// </summary>
        /// <param name="type">Type of component to create.</param>
        /// <returns>Instance of the new component.</returns>
        public Component AddComponent(Type type)
        {
            return Component.Internal_AddComponent(this, type);
        }

        /// <summary>
        /// Searches for a component of a specific type. If there are multiple components matching the type, only the first
        /// one found is returned.
        /// </summary>
        /// <typeparam name="T">Type of the component to search for. Includes any components derived from the type.
        /// </typeparam>
        /// <returns>Component instance if found, null otherwise.</returns>
        public T GetComponent<T>() where T : Component
        {
            return (T)Component.Internal_GetComponent(this, typeof(T));
        }

        /// <summary>
        /// Searches for all components of a specific type. 
        /// </summary>
        /// <typeparam name="T">Type of the component to search for. Includes any components derived from the type.
        /// </typeparam>
        /// <returns>All components matching the specified type.</returns>
        public T[] GetComponents<T>() where T : Component
        {
            Component[] components = Component.Internal_GetComponentsPerType(this, typeof (T));
            return Array.ConvertAll(components, x => (T) x);
        }

        /// <summary>
        /// Returns a list of all components attached to this object.
        /// </summary>
        /// <returns>All components attached to this object.</returns>
        public Component[] GetComponents()
        {
            return Component.Internal_GetComponents(this);
        }

        /// <summary>
        /// Removes a component from the scene object. If there are multiple components matching the type, only the first
        /// one found is removed.
        /// </summary>
        /// <typeparam name="T">Type of the component to remove. Includes any components derived from the type.</typeparam>
        public void RemoveComponent<T>() where T : Component
        {
            Component.Internal_RemoveComponent(this, typeof(T));
        }

        /// <summary>
        /// Removes a component from the scene object. If there are multiple components matching the type, only the first
        /// one found is removed.
        /// </summary>
        /// <param name="type">Type of the component to remove. Includes any components derived from the type.</param>
        public void RemoveComponent(Type type)
        {
            Component.Internal_RemoveComponent(this, type);
        }

        /// <summary>
        /// Returns the number of child scene objects this object is parent to.
        /// </summary>
        /// <returns>Number of child scene objects.</returns>
        public int GetNumChildren()
        {
            int value;
            Internal_GetNumChildren(mCachedPtr, out value);
            return value;
        }

        /// <summary>
        /// Returns a child scene object.
        /// </summary>
        /// <param name="idx">Index of the child scene object to retrieve.</param>
        /// <returns>Instance of the child scene object, or null if index is out of range.</returns>
        public SceneObject GetChild(int idx)
        {
            return Internal_GetChild(mCachedPtr, idx);
        }

        /// <summary>
        /// Searches the child objects for an object matching the specified name.
        /// </summary>
        /// <param name="name">Name of the object to locate.</param>
        /// <param name="recursive">If true all descendants of the scene object will be searched, otherwise only immediate 
        ///                         children.</param>
        /// <returns>First found scene object, or empty handle if none found.</returns>
        public SceneObject FindChild(string name, bool recursive = true)
        {
            return Internal_FindChild(mCachedPtr, name, recursive);
        }

        /// <summary>
        /// Searches the child objects for objects matching the specified name.
        /// </summary>
        /// <param name="name">Name of the objects to locate.</param>
        /// <param name="recursive">If true all descendants of the scene object will be searched, otherwise only immediate 
        ///                         children.</param>
        /// <returns>All scene objects matching the specified name.</returns>
        public SceneObject[] FindChildren(string name, bool recursive = true)
        {
            return Internal_FindChildren(mCachedPtr, name, recursive);
        }

        /// <summary>
        /// Orients the object so it is looking at the provided location.
        /// </summary>
        /// <param name="position">Position in local space where to look at.</param>
        public void LookAt(Vector3 position)
        {
            Vector3 up = Vector3.YAxis;
            Internal_LookAt(mCachedPtr, ref position, ref up);
        }

        /// <summary>
        /// Orients the object so it is looking at the provided location.
        /// </summary>
        /// <param name="position">Position in world space where to look at.</param>
        /// <param name="up">Determines the object's Y axis orientation.</param>
        public void LookAt(Vector3 position, Vector3 up)
        {
            Internal_LookAt(mCachedPtr, ref position, ref up);
        }

        /// <summary>
        /// Moves the object's position by the vector offset provided along world axes.
        /// </summary>
        /// <param name="amount">Amount and direction to move the object along.</param>
        public void Move(Vector3 amount)
        {
            Internal_Move(mCachedPtr, ref amount);
        }

        /// <summary>
        /// Moves the object's position by the vector offset provided along local axes.
        /// </summary>
        /// <param name="amount">Amount and direction to move the object along.</param>
        public void MoveLocal(Vector3 amount)
        {
            Internal_MoveLocal(mCachedPtr, ref amount);
        }

        /// <summary>
        /// Rotates the object by the quaternion, in world space.
        /// </summary>
        /// <param name="amount">Quaternion that specifies the rotation.</param>
        public void Rotate(Quaternion amount)
        {
            Internal_Rotate(mCachedPtr, ref amount);
        }

        /// <summary>
        /// Rotates around local Z axis.
        /// </summary>
        /// <param name="angle">Angle to rotate by.</param>
        public void Roll(Degree angle)
        {
            Radian radianAngle = angle;
            Internal_Roll(mCachedPtr, ref radianAngle);
        }

        /// <summary>
        /// Rotates around local Y axis.
        /// </summary>
        /// <param name="angle">Angle to rotate by.</param>
        public void Yaw(Degree angle)
        {
            Radian radianAngle = angle;
            Internal_Yaw(mCachedPtr, ref radianAngle);
        }

        /// <summary>
        /// Rotates around local X axis.
        /// </summary>
        /// <param name="angle">Angle to rotate by.</param>
        public void Pitch(Degree angle)
        {
            Radian radianAngle = angle;
            Internal_Pitch(mCachedPtr, ref radianAngle);
        }

        /// <summary>
        /// Destroys the scene object, removing it from scene and stopping component updates.
        /// </summary>
        /// <param name="immediate">If true the scene object will be fully destroyed immediately. This means that objects
        ///                         that are still referencing this scene object might fail. Normally destruction is delayed
        ///                         until the end of the frame to give other objects a chance to stop using it.</param>
        public void Destroy(bool immediate = false)
        {
            Internal_Destroy(mCachedPtr, immediate);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(SceneObject instance, string name, int flags);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetName(IntPtr nativeInstance, string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern string Internal_GetName(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetActive(IntPtr nativeInstance, bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetActive(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetMobility(IntPtr nativeInstance, int value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetMobility(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetParent(IntPtr nativeInstance, SceneObject parent);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SceneObject Internal_GetParent(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetNumChildren(IntPtr nativeInstance, out int value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SceneObject Internal_GetChild(IntPtr nativeInstance, int idx);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SceneObject Internal_FindChild(IntPtr nativeInstance, string name, bool recursive);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SceneObject[] Internal_FindChildren(IntPtr nativeInstance, string name, bool recursive);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetPosition(IntPtr nativeInstance, out Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetLocalPosition(IntPtr nativeInstance, out Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetRotation(IntPtr nativeInstance, out Quaternion value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetLocalRotation(IntPtr nativeInstance, out Quaternion value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetScale(IntPtr nativeInstance, out Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetLocalScale(IntPtr nativeInstance, out Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetPosition(IntPtr nativeInstance, ref Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetLocalPosition(IntPtr nativeInstance, ref Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetRotation(IntPtr nativeInstance, ref Quaternion value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetLocalRotation(IntPtr nativeInstance, ref Quaternion value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetLocalScale(IntPtr nativeInstance, ref Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetLocalTransform(IntPtr nativeInstance, out Matrix4 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetWorldTransform(IntPtr nativeInstance, out Matrix4 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_LookAt(IntPtr nativeInstance, ref Vector3 direction, ref Vector3 up);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Move(IntPtr nativeInstance, ref Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_MoveLocal(IntPtr nativeInstance, ref  Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Rotate(IntPtr nativeInstance, ref Quaternion value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Roll(IntPtr nativeInstance, ref Radian value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Yaw(IntPtr nativeInstance, ref Radian value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Pitch(IntPtr nativeInstance, ref Radian value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetForward(IntPtr nativeInstance, ref Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetForward(IntPtr nativeInstance, out Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetUp(IntPtr nativeInstance, out Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetRight(IntPtr nativeInstance, out Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Destroy(IntPtr nativeInstance, bool immediate);
    }

    /// <summary>
    /// Flags that can be used for controlling scene object behaviour.
    /// </summary>
    internal enum SceneObjectEditorFlags // Note: Must match C++ enum SceneObjectFlags
    {
        /// <summary>Object wont be in the main scene and its components won't receive updates.</summary>
        DontInstantiate = 0x01,

        /// <summary> Object will be skipped when saving the scene hierarchy or a prefab.</summary>
        DontSave = 0x02,

        /// <summary>
        /// Object will remain in the scene even after scene clear, unless destroyed directly. This only works with 
        /// top-level objects.
        /// </summary>
        Persistent = 0x04,

        /// <summary>
        /// Provides a hint to external systems that his object is used by engine internals. For example, those systems 
        /// might not want to display those objects together with the user created ones.
        /// </summary>
        Internal = 0x08
    }

    /** @} */
}
