//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Scene-Editor
     *  @{
     */

    /// <summary>
    /// Handles camera movement in the scene view.
    /// </summary>
    [RunInEditor]
    internal sealed class SceneCamera : ManagedComponent
    {
        #region Constants
        public const string MoveForwardBinding = "SceneForward";
        public const string MoveLeftBinding = "SceneLeft";
        public const string MoveRightBinding = "SceneRight";
        public const string MoveBackBinding = "SceneBackward";
        public const string MoveUpBinding = "SceneUp";
        public const string MoveDownBinding = "SceneDown";
        public const string FastMoveBinding = "SceneFastMove";
        public const string PanBinding = "ScenePan";
        public const string RotateBinding = "SceneRotate";
        public const string HorizontalAxisBinding = "SceneHorizontal";
        public const string VerticalAxisBinding = "SceneVertical";
        public const string ScrollAxisBinding = "SceneScroll";

        private const float StartSpeed = 4.0f;
        private const float TopSpeed = 12.0f;
        private const float Acceleration = 1.0f;
        private const float FastModeMultiplier = 2.0f;
        private const float PanSpeed = 3.0f;
        private const float ScrollSpeed = 3.0f;
        private const float RotationalSpeed = 3.0f;
        private readonly Degree FieldOfView = (Degree)90.0f;
        #endregion

        #region Fields
        private VirtualButton moveForwardBtn;
        private VirtualButton moveLeftBtn;
        private VirtualButton moveRightBtn;
        private VirtualButton moveBackwardBtn;
        private VirtualButton moveUpBtn;
        private VirtualButton moveDownBtn;
        private VirtualButton fastMoveBtn;
        private VirtualButton activeBtn;
        private VirtualButton panBtn;
        private VirtualAxis horizontalAxis;
        private VirtualAxis verticalAxis;
        private VirtualAxis scrollAxis;

        private float currentSpeed;
        private Degree yaw;
        private Degree pitch;
        private bool lastHideCursorState;
        private Camera camera;
        private bool inputEnabled = true;

        // Animating camera transitions
        private CameraAnimation animation = new CameraAnimation();
        private float frustumWidth = 50.0f;
        private float lerp;
        private bool isAnimating;
        #endregion

        #region Public properties
        /// <summary>
        /// Type of projection used by camera for rendering the scene.
        /// </summary>
        public ProjectionType ProjectionType
        {
            get { return camera.ProjectionType; }
            set
            {
                if (camera.ProjectionType != value)
                {
                    CameraState state = new CameraState();
                    state.Position = camera.SceneObject.Position;
                    state.Rotation = camera.SceneObject.Rotation;
                    state.Ortographic = value == ProjectionType.Orthographic;
                    state.FrustumWidth = frustumWidth;

                    SetState(state);
                }
            }
        }
        #endregion

        #region Public methods
        /// <summary>
        /// Enables or disables camera controls.
        /// </summary>
        /// <param name="enable">True to enable controls, false to disable.</param>
        public void EnableInput(bool enable)
        {
            inputEnabled = enable;
        }

        /// <summary>
        /// Focuses the camera on the currently selected object(s).
        /// </summary>
        public void FrameSelected()
        {
            SceneObject[] selectedObjects = Selection.SceneObjects;
            if (selectedObjects.Length > 0)
            {
                AABox box = EditorUtility.CalculateBounds(Selection.SceneObjects);
                FrameBounds(box);
            }
        }

        /// <summary>
        /// Orients the camera so it looks along the provided axis.
        /// </summary>
        public void LookAlong(Vector3 axis)
        {
            Vector3 up = Vector3.YAxis;
            if (MathEx.Abs(Vector3.Dot(axis, up)) > 0.9f)
                up = Vector3.ZAxis;

            CameraState state = new CameraState();
            state.Position = camera.SceneObject.Position;
            state.Rotation = Quaternion.LookRotation(axis, up);
            state.Ortographic = camera.ProjectionType == ProjectionType.Orthographic;
            state.FrustumWidth = frustumWidth;

            SetState(state);
        }

        #endregion

        #region Private methods
        private void OnReset()
        {
            camera = SceneObject.GetComponent<Camera>();

            moveForwardBtn = new VirtualButton(MoveForwardBinding);
            moveLeftBtn = new VirtualButton(MoveLeftBinding);
            moveRightBtn = new VirtualButton(MoveRightBinding);
            moveBackwardBtn = new VirtualButton(MoveBackBinding);
            moveUpBtn = new VirtualButton(MoveUpBinding);
            moveDownBtn = new VirtualButton(MoveDownBinding);
            fastMoveBtn = new VirtualButton(FastMoveBinding);
            activeBtn = new VirtualButton(RotateBinding);
            panBtn = new VirtualButton(PanBinding);
            horizontalAxis = new VirtualAxis(HorizontalAxisBinding);
            verticalAxis = new VirtualAxis(VerticalAxisBinding);
            scrollAxis = new VirtualAxis(ScrollAxisBinding);
        }

        private void OnUpdate()
        {
            bool isOrtographic = camera.ProjectionType == ProjectionType.Orthographic;

            if (inputEnabled)
            {
                bool goingForward = VirtualInput.IsButtonHeld(moveForwardBtn);
                bool goingBack = VirtualInput.IsButtonHeld(moveBackwardBtn);
                bool goingLeft = VirtualInput.IsButtonHeld(moveLeftBtn);
                bool goingRight = VirtualInput.IsButtonHeld(moveRightBtn);
                bool goingUp = VirtualInput.IsButtonHeld(moveUpBtn);
                bool goingDown = VirtualInput.IsButtonHeld(moveDownBtn);
                bool fastMove = VirtualInput.IsButtonHeld(fastMoveBtn);
                bool camActive = VirtualInput.IsButtonHeld(activeBtn);
                bool isPanning = VirtualInput.IsButtonHeld(panBtn);

                bool hideCursor = camActive || isPanning;
                if (hideCursor != lastHideCursorState)
                {
                    if (hideCursor)
                    {
                        Cursor.Hide();

                        Rect2I clipRect;
                        clipRect.x = Input.PointerPosition.x - 2;
                        clipRect.y = Input.PointerPosition.y - 2;
                        clipRect.width = 4;
                        clipRect.height = 4;

                        Cursor.ClipToRect(clipRect);
                    }
                    else
                    {
                        Cursor.Show();
                        Cursor.ClipDisable();
                    }

                    lastHideCursorState = hideCursor;
                }

                float frameDelta = Time.FrameDelta;
                if (camActive)
                {
                    float horzValue = VirtualInput.GetAxisValue(horizontalAxis);
                    float vertValue = VirtualInput.GetAxisValue(verticalAxis);

                    float rotationAmount = RotationalSpeed * EditorSettings.MouseSensitivity;

                    yaw += new Degree(horzValue * rotationAmount);
                    pitch += new Degree(vertValue * rotationAmount);

                    yaw = MathEx.WrapAngle(yaw);
                    pitch = MathEx.WrapAngle(pitch);

                    Quaternion yRot = Quaternion.FromAxisAngle(Vector3.YAxis, yaw);
                    Quaternion xRot = Quaternion.FromAxisAngle(Vector3.XAxis, pitch);

                    Quaternion camRot = yRot*xRot;
                    camRot.Normalize();

                    SceneObject.Rotation = camRot;

                    // Handle movement using movement keys
                    Vector3 direction = Vector3.Zero;

                    if (goingForward) direction += SceneObject.Forward;
                    if (goingBack) direction -= SceneObject.Forward;
                    if (goingRight) direction += SceneObject.Right;
                    if (goingLeft) direction -= SceneObject.Right;
                    if (goingUp) direction += SceneObject.Up;
                    if (goingDown) direction -= SceneObject.Up;

                    if (direction.SqrdLength != 0)
                    {
                        direction.Normalize();

                        float multiplier = 1.0f;
                        if (fastMove)
                            multiplier = FastModeMultiplier;

                        currentSpeed = MathEx.Clamp(currentSpeed + Acceleration*frameDelta, StartSpeed, TopSpeed);
                        currentSpeed *= multiplier;
                    }
                    else
                    {
                        currentSpeed = 0.0f;
                    }

                    const float tooSmall = 0.0001f;
                    if (currentSpeed > tooSmall)
                    {
                        Vector3 velocity = direction*currentSpeed;
                        SceneObject.Move(velocity*frameDelta);
                    }
                }

                // Pan
                if (isPanning)
                {
                    float horzValue = VirtualInput.GetAxisValue(horizontalAxis);
                    float vertValue = VirtualInput.GetAxisValue(verticalAxis);

                    Vector3 direction = new Vector3(horzValue, -vertValue, 0.0f);
                    direction = camera.SceneObject.Rotation.Rotate(direction);

                    SceneObject.Move(direction*PanSpeed*frameDelta);
                }
            }
            else
            {
                if (lastHideCursorState)
                {
                    Cursor.Show();
                    Cursor.ClipDisable();

                    lastHideCursorState = false;
                }
            }

            SceneWindow sceneWindow = EditorWindow.GetWindow<SceneWindow>();
            if ((sceneWindow.Active && sceneWindow.HasFocus) || sceneWindow.IsPointerHovering)
            {
                Rect2I bounds = sceneWindow.Bounds;

                // Move using scroll wheel
                if (bounds.Contains(Input.PointerPosition))
                {
                    float scrollAmount = VirtualInput.GetAxisValue(scrollAxis);
                    if (!isOrtographic)
                    {
                        SceneObject.Move(SceneObject.Forward*scrollAmount*ScrollSpeed);
                    }
                    else
                    {
                        float orthoHeight = MathEx.Max(1.0f, camera.OrthoHeight - scrollAmount);
                        camera.OrthoHeight = orthoHeight;
                    }
                }
            }

            UpdateAnim();
        }

        /// <summary>
        /// Moves and orients a camera so that the provided bounds end covering the camera's viewport.
        /// </summary>
        /// <param name="bounds">Bounds to frame in camera's view.</param>
        /// <param name="padding">Amount of padding to leave on the borders of the viewport, in percent [0, 1].</param>
        private void FrameBounds(AABox bounds, float padding = 0.0f)
        {
            // TODO - Use AABox bounds directly instead of a sphere to be more accurate
            float worldWidth = bounds.Size.Length;
            float worldHeight = worldWidth;

            if (worldWidth == 0.0f)
                worldWidth = 1.0f;

            if (worldHeight == 0.0f)
                worldHeight = 1.0f;

            float boundsAspect = worldWidth / worldHeight;
            float paddingScale = MathEx.Clamp01(padding) + 1.0f;
            float frustumWidth;

            // If camera has wider aspect than bounds then height will be the limiting dimension
            if (camera.AspectRatio > boundsAspect)
                frustumWidth = worldHeight * camera.AspectRatio * paddingScale;
            else // Otherwise width
                frustumWidth = worldWidth * paddingScale;

            float distance = CalcDistanceForFrustumWidth(frustumWidth);

            Vector3 forward = bounds.Center - SceneObject.Position;
            forward.Normalize();

            CameraState state = new CameraState();
            state.Position = bounds.Center - forward * distance;
            state.Rotation = Quaternion.LookRotation(forward, Vector3.YAxis);
            state.Ortographic = camera.ProjectionType == ProjectionType.Orthographic;
            state.FrustumWidth = frustumWidth;

            SetState(state);
        }

        /// <summary>
        /// Changes the state of the camera, either instantly or animated over several frames. The state includes
        /// camera position, rotation, type and possibly other parameters.
        /// </summary>
        /// <param name="state">New state of the camera.</param>
        /// <param name="animated">Should the state be linearly interpolated over a course of several frames.</param>
        private void SetState(CameraState state, bool animated = true)
        {
            CameraState startState = new CameraState();

            startState.Position = SceneObject.Position;
            startState.Rotation = SceneObject.Rotation;
            startState.Ortographic = camera.ProjectionType == ProjectionType.Orthographic;
            startState.FrustumWidth = frustumWidth;

            animation.Start(startState, state);
            if (!animated)
            {
                ApplyState(1.0f);

                isAnimating = false;
            }
            else
            {
                isAnimating = true;
                lerp = 0.0f;
            }
        }

        /// <summary>
        /// Applies the animation target state depending on the interpolation parameter. <see cref="SetState"/>.
        /// </summary>
        /// <param name="t">Interpolation parameter ranging [0, 1] that interpolated between the start state and the
        ///                 target state.</param>
        private void ApplyState(float t)
        {
            animation.Update(t);

            SceneObject.Position = animation.State.Position;
            SceneObject.Rotation = animation.State.Rotation;
            frustumWidth = animation.State.FrustumWidth;

            Vector3 eulerAngles = SceneObject.Rotation.ToEuler();
            pitch = (Degree)eulerAngles.x;
            yaw = (Degree)eulerAngles.y;

            Degree FOV = (Degree)(1.0f - animation.State.OrtographicPct)*FieldOfView;
            if (FOV < (Degree)5.0f)
            {
                camera.ProjectionType = ProjectionType.Orthographic;
                camera.OrthoHeight = frustumWidth * 0.5f / camera.AspectRatio;
            }
            else
            {
                camera.ProjectionType = ProjectionType.Perspective;
                camera.FieldOfView = FOV;
            }

            // Note: Consider having a global setting for near/far planes as changing it here might confuse the user
            float distance = CalcDistanceForFrustumWidth(frustumWidth);
            if (distance < 1)
            {
                camera.NearClipPlane = 0.005f;
                camera.FarClipPlane = 1000f;
            }
            if (distance < 100)
            {
                camera.NearClipPlane = 0.05f;
                camera.FarClipPlane = 2500f;
            }
            else if (distance < 1000)
            {
                camera.NearClipPlane = 0.5f;
                camera.FarClipPlane = 10000f;
            }
            else
            {
                camera.NearClipPlane = 5.0f;
                camera.FarClipPlane = 1000000f;
            }
        }

        /// <summary>
        /// Calculates distance at which the camera's frustum width is equal to the provided width.
        /// </summary>
        /// <param name="frustumWidth">Frustum width to find the distance for, in world units.</param>
        /// <returns>Distance at which the camera's frustum is the specified width, in world units.</returns>
        private float CalcDistanceForFrustumWidth(float frustumWidth)
        {
            if (camera.ProjectionType == ProjectionType.Perspective)
                return (frustumWidth*0.5f)/MathEx.Tan(camera.FieldOfView*0.5f);
            else
                return frustumWidth * 2.0f;
        }

        /// <summary>
        /// Updates camera state transition animation. Should be called every frame.
        /// </summary>
        private void UpdateAnim()
        {
            if (!isAnimating)
                return;

            const float ANIM_TIME = 0.5f; // 0.5f seconds
            lerp += Time.FrameDelta * (1.0f / ANIM_TIME);

            if (lerp >= 1.0f)
            {
                lerp = 1.0f;
                isAnimating = false;
            }

            ApplyState(lerp);
        }

        /// <summary>
        /// Contains data for a possible camera state. Camera states can be interpolated between each other as needed.
        /// </summary>
        private struct CameraState
        {
            private float _ortographic;

            public Vector3 Position { get; set; }
            public Quaternion Rotation { get; set; }
            public float FrustumWidth { get; set; }

            public bool Ortographic
            {
                get { return _ortographic > 0.5; }
                set { _ortographic = value ? 1.0f : 0.0f; }
            }

            public float OrtographicPct
            {
                get { return _ortographic; }
                set { _ortographic = value; }
            }
        }

        /// <summary>
        /// Helper class that performs linear interpolation between two camera states.
        /// </summary>
        private struct CameraAnimation
        {
            private CameraState start;
            private CameraState target;
            private CameraState interpolated;

            /// <summary>
            /// Returns currently interpolated animation state.
            /// </summary>
            public CameraState State
            {
                get { return interpolated; }
            }

            /// <summary>
            /// Initializes the animation with initial and target states.
            /// </summary>
            /// <param name="start">Initial state to animate from.</param>
            /// <param name="target">Target state to animate towards.</param>
            public void Start(CameraState start, CameraState target)
            {
                this.start = start;
                this.target = target;
            }

            /// <summary>
            /// Updates the animation by interpolating between the start and target states.
            /// </summary>
            /// <param name="t">Interpolation parameter in range [0, 1] that determines how much to interpolate between
            ///                 start and target states.</param>
            public void Update(float t)
            {
                interpolated.Position = start.Position * (1.0f - t) + target.Position * t;
                interpolated.Rotation = Quaternion.Slerp(start.Rotation, target.Rotation, t);
                interpolated.OrtographicPct = start.OrtographicPct * (1.0f - t) + target.OrtographicPct * t;

                interpolated.FrustumWidth = start.FrustumWidth * (1.0f - t) + target.FrustumWidth * t;
            }
        };
        #endregion 
    }

    /** @} */
}
