---
title: Render settings
---

@b3d::RenderSettings is an object present on every **Camera** object. It can be retrieved through @b3d::Camera::GetRenderSettings() and allows you to customize what rendering effects are executed when rendering the scene through that view.

For complete list of tweakable properties check the API reference, and here we'll just cover the main points.

Note that after you change any of the properties in **RenderSettings** you must call @b3d::Camera::SetRenderSettings() to apply the changes to the camera.

~~~~~~~~~~~~~{.cpp}
HCamera camera = ...;

// Tweak the render settings by disabling some effects
TShared<RenderSettings> renderSettings = camera->GetRenderSettings();
renderSettings->ScreenSpaceReflections.Enabled = false;
renderSettings->AmbientOcclusion.Enabled = false;
renderSettings->EnableIndirectLighting = false;
renderSettings->EnableFxaa = false;

camera->SetRenderSettings(renderSettings);
~~~~~~~~~~~~~

# HDR and tonemapping
HDR stands for high-dynamic range, and it allows the lights in the scene to use a large range of intensity values that can more closely approximate a real-world scene. Lighting information is first written to a floating point texture that can store a wider range of values than a normal RGB texture. These high range lighting values are then used throughout the calculations in the engine, ensuring a higher quality final result. You can toggle HDR rendering through @b3d::RenderSettings::EnableHdr.

Before the image is output to the screen it goes through the process called tonemapping, which converts the high range values into low range that a normal output device (like a monitor or a TV) can display. Tonemapping can be toggled through @b3d::RenderSettings::EnableTonemapping, but this is generally something you always want to have on when HDR rendering is enabled.

@b3d::RenderSettings::Tonemapping provides you with a variety of tweakable options on how is tonemapping performed. Tonemapping operators are a complex topic, but it's enough to know that these options affect the feel and tone of the final image, and it is generally something you will tweak to suit the best look for your project.

~~~~~~~~~~~~~{.cpp}
HCamera camera = ...;
TShared<RenderSettings> renderSettings = camera->GetRenderSettings();

// Enable HDR and tonemapping
renderSettings->EnableHdr = true;
renderSettings->EnableTonemapping = true;

// Customize tonemapping settings
renderSettings->Tonemapping.FilmicCurveShoulderStrength = 0.22f;
renderSettings->Tonemapping.FilmicCurveLinearStrength = 0.3f;
renderSettings->Tonemapping.FilmicCurveLinearAngle = 0.1f;
renderSettings->Tonemapping.FilmicCurveToeStrength = 0.2f;

camera->SetRenderSettings(renderSettings);
~~~~~~~~~~~~~

HDR and tonemapping is also closely related to exposure, which we'll cover next.

# Exposure
Exposure determines which part of the high range image should be converted to low range (e.g. the very bright parts, the very dark parts, or somewhere in the middle). Generally this is a property you will only use when HDR is enabled, as LDR doesn't offer a high enough range for this property to be relevant.

By default the system will calculate the exposure automatically, based on how the human eye determines exposure. Generally this means if you are in a very bright area, it will be hard to see into darker areas (imagine standing outside in sunlight and looking into house lit only by artifical light), or when in a very dark area the bright areas will be overexposed. As you move between areas of different light intensity the exposure will slowly adjust accordingly. You can tweak automatic exposure options through @b3d::RenderSettings::AutoExposure (similar to human eyes - imagine walking into a house on a brightly Sun-lit day, it takes a while for your eyes to adjust).

~~~~~~~~~~~~~{.cpp}
HCamera camera = ...;
TShared<RenderSettings> renderSettings = camera->GetRenderSettings();

// Enable automatic exposure
renderSettings->EnableAutoExposure = true;

// Customize auto exposure settings
renderSettings->AutoExposure.HistogramLog2Min = -8.0f;
renderSettings->AutoExposure.HistogramLog2Max = 4.0f;
renderSettings->AutoExposure.MinEyeAdaptation = 0.003f;
renderSettings->AutoExposure.MaxEyeAdaptation = 2.0f;
renderSettings->AutoExposure.EyeAdaptationSpeedUp = 3.0f;
renderSettings->AutoExposure.EyeAdaptationSpeedDown = 3.0f;

camera->SetRenderSettings(renderSettings);
~~~~~~~~~~~~~

Automatic exposure can be disabled through @b3d::RenderSettings::EnableAutoExposure. In this case you will want to set the exposure manually through @b3d::RenderSettings::ExposureScale. This allows for more control over the exposure, which is sometimes required.

~~~~~~~~~~~~~{.cpp}
HCamera camera = ...;
TShared<RenderSettings> renderSettings = camera->GetRenderSettings();

// Disable automatic exposure and set manual exposure
renderSettings->EnableAutoExposure = false;
renderSettings->ExposureScale = 1.25f;

camera->SetRenderSettings(renderSettings);
~~~~~~~~~~~~~

[TODO_IMAGE]()
Image with different exposure levels

# White balance
White balance is a process that occurs during tonemapping and therefore requires tonemapping to be enabled. It is intended to emulate the effect of human vision called 'chromatic adaptation', where our eyes are able to adjust to different lighting conditions while still being able to tell actual colors of a surface (e.g. a blue car illuminated by a strong red light still looks blue).

When it comes to virtual lighting our eyes cannot perform the same adapation, as they will adjust to the real-world environment instead of the in-game environment (e.g. to the room where your screen is) . The white balance process converts in-game lighting to some real-world lighting, ensuring this adjustment process is emulated as if you were in the in-game lighting environment.

By default the real-world lighting is assumed to be a room lit by daylight, but the exact environment can be controlled through @b3d::RenderSettings::WhiteBalance. Since you cannot assume the lighting environment your application will be viewed under, this might be best left for the user to tweak.

~~~~~~~~~~~~~{.cpp}
HCamera camera = ...;
TShared<RenderSettings> renderSettings = camera->GetRenderSettings();

// Customize white balance settings
renderSettings->WhiteBalance.Temperature = 6500.0f;
renderSettings->WhiteBalance.Tint = 0.0f;

camera->SetRenderSettings(renderSettings);
~~~~~~~~~~~~~

[TODO_IMAGE]()
Image with different white balance lighting environments

# Color grading
Color grading allows you to perform additional artistic control over the final image and tweak settings like contrast and saturation. These effects are not physically based in nature and can be tweaked for purely artistic control.

The relevant options are present in @b3d::RenderSettings::ColorGrading.

~~~~~~~~~~~~~{.cpp}
HCamera camera = ...;
TShared<RenderSettings> renderSettings = camera->GetRenderSettings();

// Customize color grading settings
renderSettings->ColorGrading.Saturation = Vector3(1.2f, 1.2f, 1.2f);
renderSettings->ColorGrading.Contrast = Vector3(1.1f, 1.1f, 1.1f);
renderSettings->ColorGrading.Gain = Vector3::kOne;
renderSettings->ColorGrading.Offset = Vector3::kZero;

camera->SetRenderSettings(renderSettings);
~~~~~~~~~~~~~

[TODO_IMAGE]()
Image with and without custom color grading

# Screen space reflections (SSR)
This effect provides high quality, real-time reflections at a fairly low performance impact. The main limitation effect is that it is performed in screen-space, and therefore cannot reflect an object that's not currently on the screen. When reflection cannot be found the system will fall back onto reflection probes. The effect is also generally not suitable for perfect mirror-like reflections due to limited precision.

You can control and toggle the effect through @b3d::RenderSettings::ScreenSpaceReflections.

~~~~~~~~~~~~~{.cpp}
HCamera camera = ...;
TShared<RenderSettings> renderSettings = camera->GetRenderSettings();

// Enable and customize screen space reflections
renderSettings->ScreenSpaceReflections.Enabled = true;
renderSettings->ScreenSpaceReflections.Quality = 2;
renderSettings->ScreenSpaceReflections.Intensity = 1.0f;
renderSettings->ScreenSpaceReflections.MaxRoughness = 0.8f;

camera->SetRenderSettings(renderSettings);
~~~~~~~~~~~~~

[TODO_IMAGE]()
Left - SSR disabled, Right - SSR enabled

# Screen space ambient occlusion (SSAO)
This effect estimates ambient occlusion using screen-space information. The ambient occlusion is approximated by sampling the nearby geometry and determining the occlusion amount. More nearby geometry results in a higher occlusion value, and therefore the surface receiving less light. This produces more realistic lighting. Note that SSAO is by default applied only to indirect lighting, and as such it is mostly visible in shadows.

You can control and toggle the effect through @b3d::RenderSettings::AmbientOcclusion.

~~~~~~~~~~~~~{.cpp}
HCamera camera = ...;
TShared<RenderSettings> renderSettings = camera->GetRenderSettings();

// Enable and customize ambient occlusion
renderSettings->AmbientOcclusion.Enabled = true;
renderSettings->AmbientOcclusion.Radius = 1.5f;
renderSettings->AmbientOcclusion.Bias = 1.0f;
renderSettings->AmbientOcclusion.Intensity = 1.0f;
renderSettings->AmbientOcclusion.Power = 4.0f;
renderSettings->AmbientOcclusion.Quality = 3;

camera->SetRenderSettings(renderSettings);
~~~~~~~~~~~~~

[TODO_IMAGE]()
Left - SSAO disabled, Right - SSAO enabled

# Depth of field
By default the virtual camera focuses perfectly on all parts of the scene it views, but this is not the case with real-world cameras. Real-world cameras can instead set a focus distance at which the captured image will be perfectly in focus. Anything closer or further away from that distance will get progressively more out of focus (blurry). Depth of field effect emulates this functionality of the camera, allowing you to set a focus distance while blurring the near and/or far parts of the scene.

Depth of field options can be tweaked through @b3d::RenderSettings::DepthOfField.

~~~~~~~~~~~~~{.cpp}
HCamera camera = ...;
TShared<RenderSettings> renderSettings = camera->GetRenderSettings();

// Enable and customize depth of field with Gaussian blur
renderSettings->DepthOfField.Enabled = true;
renderSettings->DepthOfField.Type = DepthOfFieldType::Gaussian;
renderSettings->DepthOfField.FocalDistance = 0.75f;
renderSettings->DepthOfField.FocalRange = 0.75f;
renderSettings->DepthOfField.NearTransitionRange = 0.25f;
renderSettings->DepthOfField.FarTransitionRange = 0.25f;
renderSettings->DepthOfField.NearBlurAmount = 0.02f;
renderSettings->DepthOfField.FarBlurAmount = 0.02f;

camera->SetRenderSettings(renderSettings);
~~~~~~~~~~~~~

For more advanced depth of field effects, you can use the Bokeh type which allows you to control the shape of the blur using a texture.

~~~~~~~~~~~~~{.cpp}
HCamera camera = ...;
TShared<RenderSettings> renderSettings = camera->GetRenderSettings();

// Enable Bokeh depth of field
renderSettings->DepthOfField.Enabled = true;
renderSettings->DepthOfField.Type = DepthOfFieldType::Bokeh;
renderSettings->DepthOfField.FocalDistance = 0.75f;
renderSettings->DepthOfField.FocalRange = 0.0f;
renderSettings->DepthOfField.MaxBokehSize = 0.025f;
renderSettings->DepthOfField.ApertureSize = 50.0f;
renderSettings->DepthOfField.FocalLength = 50.0f;
renderSettings->DepthOfField.SensorSize = Vector2(22.2f, 14.8f);
renderSettings->DepthOfField.BokehOcclusion = true;

camera->SetRenderSettings(renderSettings);
~~~~~~~~~~~~~

[TODO_IMAGE]()
Left - DOF disabled, Right - DOF enabled

# Motion blur
Motion blur simulates the effect of objects or the camera moving quickly, creating a streak effect that can enhance the sense of motion and speed. The effect can be applied to camera movement, object movement, or both.

Motion blur options can be tweaked through @b3d::RenderSettings::MotionBlur.

~~~~~~~~~~~~~{.cpp}
HCamera camera = ...;
TShared<RenderSettings> renderSettings = camera->GetRenderSettings();

// Enable and customize motion blur
renderSettings->MotionBlur.Enabled = true;
renderSettings->MotionBlur.Domain = MotionBlurDomain::CameraAndObject;
renderSettings->MotionBlur.Filter = MotionBlurFilter::Reconstruction;
renderSettings->MotionBlur.Quality = MotionBlurQuality::Medium;
renderSettings->MotionBlur.MaximumRadius = 0.01f;

camera->SetRenderSettings(renderSettings);
~~~~~~~~~~~~~

# Temporal anti-aliasing (TAA)
Temporal anti-aliasing is an advanced anti-aliasing technique that uses information from previous frames to reduce aliasing artifacts. It provides better quality than FXAA while being more performant than traditional MSAA.

Temporal anti-aliasing options can be tweaked through @b3d::RenderSettings::TemporalAa.

~~~~~~~~~~~~~{.cpp}
HCamera camera = ...;
TShared<RenderSettings> renderSettings = camera->GetRenderSettings();

// Enable and customize temporal anti-aliasing
renderSettings->TemporalAa.Enabled = true;
renderSettings->TemporalAa.JitteredPositionCount = 8;
renderSettings->TemporalAa.Sharpness = 1.0f;

camera->SetRenderSettings(renderSettings);
~~~~~~~~~~~~~

# Fast approximate anti-aliasing (FXAA)
This is a screen space effect that reduces the aliasing artifacts known as 'jaggies'. These artifacts occur when there are discontinuities while rendering a pixel, for example when a pixel contains an edge between two surfaces. This effect is an alternative to other anti-aliasing methods like MSAA or TAA. It is significantly faster than MSAA but can result in lower quality and greater overall bluriness of the resulting image.

It can be toggled through @b3d::RenderSettings::EnableFxaa.

~~~~~~~~~~~~~{.cpp}
HCamera camera = ...;
TShared<RenderSettings> renderSettings = camera->GetRenderSettings();

// Enable FXAA
renderSettings->EnableFxaa = true;

camera->SetRenderSettings(renderSettings);
~~~~~~~~~~~~~

[TODO_IMAGE]()
Image without FXAA

[TODO_IMAGE]()
Image with FXAA

# Gamma
Tweaks the gamma value that's applied to the image before being sent to the output device. Mainly affects the brightness of the image.

Controlled through @b3d::RenderSettings::Gamma.

~~~~~~~~~~~~~{.cpp}
HCamera camera = ...;
TShared<RenderSettings> renderSettings = camera->GetRenderSettings();

// Set gamma value
renderSettings->Gamma = 2.2f;

camera->SetRenderSettings(renderSettings);
~~~~~~~~~~~~~

# Shadows
Shadow rendering for a specific view can be completely disabled through @b3d::RenderSettings::EnableShadows. Shadow options that are view-specific can be controlled through @b3d::RenderSettings::ShadowSettings.

~~~~~~~~~~~~~{.cpp}
HCamera camera = ...;
TShared<RenderSettings> renderSettings = camera->GetRenderSettings();

// Enable shadows and customize settings
renderSettings->EnableShadows = true;
renderSettings->ShadowSettings.DirectionalShadowDistance = 250.0f;
renderSettings->ShadowSettings.NumCascades = 4;
renderSettings->ShadowSettings.CascadeDistributionExponent = 3.0f;
renderSettings->ShadowSettings.ShadowFilteringQuality = 4;

camera->SetRenderSettings(renderSettings);
~~~~~~~~~~~~~

# Indirect lighting
Indirect lighting provided by **LightProbeVolume** can be fully disabled for a view through @b3d::RenderSettings::EnableIndirectLighting.

~~~~~~~~~~~~~{.cpp}
HCamera camera = ...;
TShared<RenderSettings> renderSettings = camera->GetRenderSettings();

// Enable or disable indirect lighting
renderSettings->EnableIndirectLighting = true;

camera->SetRenderSettings(renderSettings);
~~~~~~~~~~~~~

# Bloom
Bloom provides an extra highlight to already bright areas of the scene, simulating the real-world camera effect where an extremely bright light overwhelms the camera. The effect produces a fringe extending from the borders of the bright areas of the image. This effect is only used if tonemapping and HDR is enabled.

![Image without bloom](../../Images/BloomOffSmall.png)
![Image with bloom](../../Images/BloomOnSmall.png)

You can enable/disable, as well as tweak the effect through @b3d::RenderSettings::Bloom. You can control the intensity treshold at which bloom will be shown, as well as tweak the global intensity and tint, along with a quality setting for performance/quality tradeoff.

~~~~~~~~~~~~~{.cpp}
HCamera camera = ...;
TShared<RenderSettings> renderSettings = camera->GetRenderSettings();

// Enable and customize bloom
renderSettings->Bloom.Enabled = true;
renderSettings->Bloom.Quality = 2;
renderSettings->Bloom.Threshold = 1.0f;
renderSettings->Bloom.Intensity = 0.5f;
renderSettings->Bloom.Tint = Color::kWhite;
renderSettings->Bloom.FilterSize = 0.15f;

camera->SetRenderSettings(renderSettings);
~~~~~~~~~~~~~

# Screen space lens flare
This effect simulates lens flare artifacts that occur in real-world cameras when bright light sources are visible. The effect can add ghost features and halos to enhance the cinematic look of your scene.

~~~~~~~~~~~~~{.cpp}
HCamera camera = ...;
TShared<RenderSettings> renderSettings = camera->GetRenderSettings();

// Enable and customize lens flare
renderSettings->ScreenSpaceLensFlare.Enabled = true;
renderSettings->ScreenSpaceLensFlare.DownsampleCount = 4;
renderSettings->ScreenSpaceLensFlare.Threshold = 32.0f;
renderSettings->ScreenSpaceLensFlare.GhostCount = 2;
renderSettings->ScreenSpaceLensFlare.GhostSpacing = 0.5f;
renderSettings->ScreenSpaceLensFlare.Brightness = 0.05f;
renderSettings->ScreenSpaceLensFlare.Halo = true;
renderSettings->ScreenSpaceLensFlare.HaloRadius = 0.35f;

camera->SetRenderSettings(renderSettings);
~~~~~~~~~~~~~

# Chromatic aberration
Chromatic aberration simulates the lens distortion effect where colors are separated at the edges of high-contrast areas. This can add a cinematic or stylized look to your scene.

~~~~~~~~~~~~~{.cpp}
HCamera camera = ...;
TShared<RenderSettings> renderSettings = camera->GetRenderSettings();

// Enable and customize chromatic aberration
renderSettings->ChromaticAberration.Enabled = true;
renderSettings->ChromaticAberration.Type = ChromaticAberrationType::Simple;
renderSettings->ChromaticAberration.ShiftAmount = 0.05f;

camera->SetRenderSettings(renderSettings);
~~~~~~~~~~~~~

# Film grain
Film grain adds a time-varying noise effect over the entire image, simulating the grain pattern found in analog film photography.

~~~~~~~~~~~~~{.cpp}
HCamera camera = ...;
TShared<RenderSettings> renderSettings = camera->GetRenderSettings();

// Enable and customize film grain
renderSettings->FilmGrain.Enabled = true;
renderSettings->FilmGrain.Intensity = 16.0f;
renderSettings->FilmGrain.Speed = 10.0f;

camera->SetRenderSettings(renderSettings);
~~~~~~~~~~~~~

# Additional settings

## Lighting
Scene lighting can be completely disabled through @b3d::RenderSettings::EnableLighting. When disabled, objects will be rendered using only their albedo texture with no lighting applied.

~~~~~~~~~~~~~{.cpp}
HCamera camera = ...;
TShared<RenderSettings> renderSettings = camera->GetRenderSettings();

renderSettings->EnableLighting = true;

camera->SetRenderSettings(renderSettings);
~~~~~~~~~~~~~

## Skybox
The camera can be configured to use the skybox for rendering the background through @b3d::RenderSettings::EnableSkybox. When disabled, the camera will use the clear color for rendering the background.

~~~~~~~~~~~~~{.cpp}
HCamera camera = ...;
TShared<RenderSettings> renderSettings = camera->GetRenderSettings();

renderSettings->EnableSkybox = true;

camera->SetRenderSettings(renderSettings);
~~~~~~~~~~~~~

## Cull distance
The cull distance determines at what distance objects will no longer be rendered. This can improve performance by not rendering distant objects.

~~~~~~~~~~~~~{.cpp}
HCamera camera = ...;
TShared<RenderSettings> renderSettings = camera->GetRenderSettings();

renderSettings->CullDistance = 500.0f;

camera->SetRenderSettings(renderSettings);
~~~~~~~~~~~~~
