---
title: Lights
---

Lights control the rendering of the nearby **Renderable** objects, by attempting to simulate how a real light would affect them. They are represented by the @b3d::Light component. They are essential for creating a realistic 3D scene.

# Creating a light
**Light** is created as any component, and requires no additional parameters.

~~~~~~~~~~~~~{.cpp}
HSceneObject lightSceneObject = SceneObject::Create("Light");
HLight light = lightSceneObject->AddComponent<Light>();
~~~~~~~~~~~~~

Once created light can be positioned and oriented normally using its **SceneObject**.

# Light types
Lights have three variants that determine how light affects surrounding objects: radial, spot and directional. You can change between light variants with @b3d::Light::SetType.

~~~~~~~~~~~~~{.cpp}
// Make a radial light
light->SetType(LightType::Radial);
~~~~~~~~~~~~~

## Radial light

Radial light affects everything within a certain radius from the light (i.e. its sphere of influence). It is the most basic type of light.

~~~~~~~~~~~~~{.cpp}
light->SetType(LightType::Radial);
light->SetAttenuationRadius(10.0f); // 10 meter radius
~~~~~~~~~~~~~

## Spot light

Spot lights only illuminate a certain direction (i.e. like a flash-light). The user can specify the angle of illumination.

~~~~~~~~~~~~~{.cpp}
light->SetType(LightType::Spot);
light->SetSpotAngle(Degree(45)); // 45 degree cone
light->SetSpotFalloffAngle(Degree(35)); // Smooth falloff starts at 35 degrees
~~~~~~~~~~~~~

## Directional light

Directional lights are a special type of light primarily used for simulating sun-light, or otherwise far-away objects. Unlike radial or spot lights they cannot be positioned. Only their orientation matters and they equally affect every object in the scene.

~~~~~~~~~~~~~{.cpp}
light->SetType(LightType::Directional);
lightSceneObject->SetRotation(Quaternion(Degree(-45), Vector3::kUnitX)); // Angle sunlight
~~~~~~~~~~~~~

# Light properties
Each light has a set of properties you can adjust (aside from position/orientation, which are handled by the **SceneObject**). Most of the properties are common for all light types but some are only relevant for specific types. We'll go over all of the properties below.

## Color
As the name implies, this controls what color light is being emitted from the source. It can be set by calling @b3d::Light::SetColor and it is valid for all light types.

~~~~~~~~~~~~~{.cpp}
// Set light to warm white color
light->SetColor(Color(1.0f, 0.9f, 0.8f));

// Set light to red
light->SetColor(Color::Red);
~~~~~~~~~~~~~

## Attenuation radius
Attenuation radius determines how far away does the light's influence reach. For radial lights this is the radius of the sphere of influence, and for spot lights this is the distance from the origin in the direction of the light. This property is not relevant for directional lights as their range is infinite. Use @b3d::Light::SetAttenuationRadius to set the range.

~~~~~~~~~~~~~{.cpp}
// Set light radius to 20 meters
light->SetAttenuationRadius(20.0f);
~~~~~~~~~~~~~

Note that the manually set radius will only be used if automatic attenuation is disabled. You can toggle this by calling @b3d::Light::SetUseAutoAttenuation. When automatic attenuation is enabled the maximum range is calculated automatically based on the light intensity (described below). This automatic attenuation will smoothly cut off the light influence when it reaches roughly 10% of its intensity, but can result in very large attenuation radius, which can affect performance.

~~~~~~~~~~~~~{.cpp}
// Enable automatic attenuation (default is false)
light->SetUseAutoAttenuation(true);

// Disable to use manual radius
light->SetUseAutoAttenuation(false);
light->SetAttenuationRadius(15.0f);
~~~~~~~~~~~~~

## Intensity
This controls how strong is the light. Although you could technically control light intensity using the color property (by using a lighter or darker color), using the intensity allows the engine to simulate high-dynamic range.

In nature the range of light intensities varies highly - standing outside at a sunlit day may be hundreds or thousands of times brighter than standing indoors illuminated by a lightbulb. We won't perceive such a large difference because our eyes are able to adjust to different intensities.

The framework uses a HDR algorithm to try to approximate this adjustment, which results in more realistic and higher quality lighting. Therefore it can be important for realism to set up the light intensities similar to what they would be in nature.

Use @b3d::Light::SetIntensity to change the light intensity.

The specific units used for intensity (in case you want to look them up for actual light sources) are *luminous flux* for radial/spot lights, and *luminance* for directional lights. Generally this means directional light intensity should be much lower than for radial/spot lights.

## Source radius
By default all lights are considered point (punctual) lights, meaning they have no surface area. In case you want to make an area light set the source radius of the light by calling @b3d::Light::SetSourceRadius. The value of this property is interpreted differently depending on light type:
 - Radial - Light represents a spherical area light and source radius is the sphere's radius
 - Spot - Light represents a disc area light (oriented toward spot direction) and the source radius is the disc radius
 - Directional - Light represents a disc area light projected on the sky, and the source radius represents the angular radius (in degrees) of the light. For example, Sun's angular radius is roughly 0.25°

~~~~~~~~~~~~~{.cpp}
// Create a spherical area light with 0.5 meter radius
light->SetType(LightType::Radial);
light->SetSourceRadius(0.5f);

// Create a sun-like directional light
light->SetType(LightType::Directional);
light->SetSourceRadius(0.25f); // Angular radius in degrees
~~~~~~~~~~~~~

Area light types are particularly important for physically based rendering, as they can produce realistic looking specular reflections, which is not the case for point lights.

## Shadows
Lights may or may not cast shadows. For realism all lights should cast shadows, but that is not feasible due to the high performance costs of using shadows. Therefore you should enable shadows only for one, or a few important lights. Use @b3d::Light::SetCastsShadow to enable or disable shadow casting.

~~~~~~~~~~~~~{.cpp}
// Enable shadow casting
light->SetCastsShadow(true);

// Disable shadow casting (default)
light->SetCastsShadow(false);
~~~~~~~~~~~~~

Casting shadows can cause artifacts called "shadow acne" in the scene. These artifacts occur due to an object casting a shadow on itself, caused by limited precision of the calculations used. To combat this effect you can tweak the shadow bias property. Shadow bias moves the distance from which the shadow is cast, ensuring incorrect self-shadowing is avoided. Shadow bias can be tweaked by calling @b3d::Light::SetShadowBias.

~~~~~~~~~~~~~{.cpp}
// Increase shadow bias to reduce shadow acne
light->SetShadowBias(1.0f);

// Default value
light->SetShadowBias(0.5f);

// Negative values can fix both acne and Peter Panning (if geometry has thickness)
light->SetShadowBias(-0.5f);
~~~~~~~~~~~~~

Valid shadow bias values are from -1 to 1. When value is 0 no shadow bias will be applied, while positive shadow bias values will offset the shadow distance as described above. However offsetting the shadow distance may cause the objects to appear like they are floating even if they are in contact with a surface.

By setting the shadow bias to a negative value you move the shadow backwards, resolving both the shadow acne and the floating object issue. However this only works if your geometry has thickness, otherwise elements behind the object will be incorrectly shadowed. This is generally the best option as long as you can set up your geometry correctly.

## Spot angles
Spot lights have a property that defines at how wide an angle do they cast light in. Narrower angle means a more focused light beam, while wider angle means a weaker light covering a larger area.

You can set the spot light angle (also known as total angle) with @b3d::Light::SetSpotAngle. Note that light intensity will be spread out over the range, so when increasing the angle, you might also want to increase the intensity to keep the perceived brightness the same.

~~~~~~~~~~~~~{.cpp}
light->SetType(LightType::Spot);

// Set a narrow focused beam (flashlight)
light->SetSpotAngle(Degree(30));

// Set a wider area light
light->SetSpotAngle(Degree(90));
~~~~~~~~~~~~~

You can also call @b3d::Light::SetSpotFalloffAngle to set an angle that allows a smooth falloff at the edge of the light's radius. This must be an angle smaller than the total spot angle. Within this angle the light intensity will be uniform, but as the angle increases past the falloff angle and towards the total angle, intensity will slowly reduce, resulting in a smooth falloff curve. If you don't want the smooth falloff, set this angle to the same value as the total angle.

~~~~~~~~~~~~~{.cpp}
// Set spot angle with smooth falloff
light->SetSpotAngle(Degree(45));        // Total angle
light->SetSpotFalloffAngle(Degree(35)); // Falloff starts at 35 degrees

// Hard edge (no falloff)
light->SetSpotAngle(Degree(45));
light->SetSpotFalloffAngle(Degree(45)); // Same as total angle
~~~~~~~~~~~~~

# Retrieving light properties
You can retrieve the current values of all light properties:

~~~~~~~~~~~~~{.cpp}
// Get light type
LightType type = light->GetType();

// Get color
Color color = light->GetColor();

// Get intensity
float intensity = light->GetIntensity();

// Get attenuation radius
float radius = light->GetAttenuationRadius();

// Get source radius
float sourceRadius = light->GetSourceRadius();

// Check if shadows are enabled
bool castsShadows = light->GetCastsShadow();

// Get shadow bias
float shadowBias = light->GetShadowBias();

// Check if auto attenuation is enabled
bool autoAtten = light->GetUseAutoAttenuation();

// Get spot angles
Degree spotAngle = light->GetSpotAngle();
Degree falloffAngle = light->GetSpotFalloffAngle();
~~~~~~~~~~~~~

# Light bounds
You can retrieve the world-space bounding sphere that encompasses the light's area of influence:

~~~~~~~~~~~~~{.cpp}
// Get light bounds
Sphere bounds = light->GetBounds();

Vector3 center = bounds.GetCenter();
float radius = bounds.GetRadius();

// Check if a point is within light influence
Vector3 point(10.0f, 5.0f, 0.0f);
if (bounds.Contains(point))
{
    // Point is lit by this light
}
~~~~~~~~~~~~~

# ECS fragments

The **Light** component stores its data internally as ECS fragments, enabling the renderer to batch-process all lights in the scene efficiently.

## Data fragment

The primary fragment is @b3d::ecs::Light, which stores all the light's visual properties:
 - **Type** - Light type (directional, radial, or spot)
 - **CastsShadows** - Whether the light casts shadows
 - **LightColor** - Color emitted by the light
 - **AttRadius** - Attenuation radius (range of influence)
 - **SourceRadius** - Radius of the light source for area lights
 - **Intensity** - Power of the light source
 - **SpotAngle** - Total angle covered by a spot light
 - **SpotFalloffAngle** - Angle at which spot light falloff begins
 - **AutoAttenuation** - Whether attenuation radius is computed automatically
 - **Bounds** - World-space bounding sphere of the light's influence
 - **ShadowBias** - Shadow rendering offset to reduce artifacts

When you call setter methods like @b3d::Light::SetColor or @b3d::Light::SetIntensity, the component modifies this fragment and marks the entity as dirty for synchronization with the render thread.

## ID fragment

Each light also has an @b3d::ecs::LightId fragment that stores a persistent renderer ID used by the @b3d::RendererObjectStorage system for mapping to the packed render-thread representation.

## Dirty tags

The renderer uses two ECS tag types to track which lights need synchronization:
 - `ecs::LightDirty` - Added when any light property changes. Triggers a full sync.
 - `ecs::LightTransformDirty` - Added when only the transform changes.

## Using raw ECS fragments

You can bypass the **Light** component and create `ecs::Light` fragments directly for maximum performance. You must manage the renderer ID, world transform, and dirty tags manually.

~~~~~~~~~~~~~{.cpp}
const SPtr<SceneInstance>& scene = SceneManager::Instance().GetMainScene();
ecs::Registry& registry = scene->GetECSRegistry();
const SPtr<RendererScene>& rendererScene = scene->GetRendererScene();

// Create an entity with the light data fragment
ecs::Entity entity = registry.CreateEntity();
ecs::Light& fragment = registry.AddComponent<ecs::Light>(entity);
fragment.Type = LightType::Radial;
fragment.LightColor = Color::kWhite;
fragment.Intensity = 100.0f;
fragment.AttRadius = 10.0f;

// Add a world transform — the renderer reads this to position the light
registry.AddComponent<ecs::WorldTransform>(entity, ecs::WorldTransform(myTransform));

// Allocate a persistent renderer ID
rendererScene->AllocateLightId(registry, entity);

// Mark dirty for initial sync
registry.AddTag<ecs::LightDirty>(entity);
~~~~~~~~~~~~~

When modifying the fragment, add the appropriate dirty tag:

~~~~~~~~~~~~~{.cpp}
ecs::Light& fragment = registry.GetComponents<ecs::Light>(entity);
fragment.Intensity = 200.0f;
registry.AddTag<ecs::LightDirty>(entity);

// For transform-only changes
registry.GetComponents<ecs::WorldTransform>(entity) = ecs::WorldTransform(newTransform);
registry.AddTag<ecs::LightTransformDirty>(entity);
~~~~~~~~~~~~~

When destroying the entity, deallocate the renderer ID first:

~~~~~~~~~~~~~{.cpp}
rendererScene->DeallocateLightId(registry, entity);
registry.RemoveComponents<ecs::LightDirty>(entity);
registry.RemoveComponents<ecs::LightTransformDirty>(entity);
registry.RemoveComponents<ecs::Light>(entity);
registry.EraseEntity(entity);
~~~~~~~~~~~~~