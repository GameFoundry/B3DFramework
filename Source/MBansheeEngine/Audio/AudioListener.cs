//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
namespace BansheeEngine
{
    /** @addtogroup Audio
     *  @{
     */

    /// <summary>
    /// Represents a listener that hears audio sources. For spatial audio the volume and pitch of played audio is determined
    /// by the distance, orientation and velocity differences between the source and the listener.
    /// </summary>
    public abstract class AudioListener : Component
    {
        internal NativeAudioListener native;

        private Vector3 lastPosition;
        private Vector3 velocity;

        /// <summary>
        /// Updates the transform of the internal listener representation from the transform of the component's scene
        /// object.
        /// </summary>
        protected void UpdateTransform()
        {
            native.Position = SceneObject.Position;
            native.Direction = SceneObject.Forward;
            native.Up = SceneObject.Up;
            native.Velocity = velocity;
        }
        
        private void OnInitialize()
        {
            NotifyFlags = TransformChangedFlags.Transform;
        }

        private void OnUpdate()
        {
            Vector3 worldPos = SceneObject.Position;
            velocity = (worldPos - lastPosition)/Time.FrameDelta;
            lastPosition = worldPos;
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

        private void OnTransformChanged(TransformChangedFlags flags)
        {
            if (!SceneObject.Active)
                return;

            if ((flags & (TransformChangedFlags.Parent | TransformChangedFlags.Transform)) != 0)
                UpdateTransform();
        }

        /// <summary>
        /// Destroys the internal listener representation.
        /// </summary>
        private void DestroyNative()
        {
            if (native != null)
            {
                native.Destroy();
                native = null;
            }
        }

        /// <summary>
        /// Creates the internal representation of the listener and restores the values saved by the component.
        /// </summary>
        private void RestoreNative()
        {
            native = new NativeAudioListener();

            UpdateTransform();
        }
    }

    /** @} */
}
