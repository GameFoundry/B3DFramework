Rendering objects						{#renderingObjects}
===============

We are now finally ready to start displaying things on the screen. To render something you must create the @ref bs::CRenderable "Renderable" component.

# Creating a renderable
**Renderable** is created as any component, and requires no additional parameters.

~~~~~~~~~~~~~{.cpp}
HSceneObject renderableSO = SceneObject::create("3D object");
HRenderable renderable = renderableSO->addComponent<CRenderable>();
~~~~~~~~~~~~~

# Rendering a mesh
Once created you must assign it a **Mesh** to render, and a **Material** to render it with, both of which we have discussed in the previous chapters. Use @ref bs::CRenderable::setMesh "CRenderable::setMesh" and @ref bs::CRenderable::setMaterial "CRenderable::setMaterial".

~~~~~~~~~~~~~{.cpp}
... set up a camera ...

// Create a standard material as in the previous chapter
HShader shader = gBuiltinResources().getBuiltinShader(BuiltinShader::Standard);
HMaterial material = Material::create(shader);

// Import and assign a texture to the material
HTexture texture = gImporter().import<Texture>("myTexture.png");
material->setTexture("gAlbedoTex", texture);

// Import a mesh
HMesh mesh = gImporter().import<Mesh>("myMesh.fbx");

// Create a renderable
HSceneObject renderableSO = SceneObject::create("3D object");
HRenderable renderable = renderableSO->addComponent<CRenderable>();

// Assign material and mesh to the renderable
renderable->setMesh(mesh);
renderable->setMaterial(material);

// Optionally position the renderable in the scene
renderableSO->setPosition(Vector3(0.0f, 15.0f, 30.0f));
~~~~~~~~~~~~~

> Note that even though we always import resources in these examples, in production code you should load previously saved resources instead of importing them every time, as earlier described in the resource related chapters.

After the renderable has been set up, it will now be displayed in your camera view, if the camera is facing the direction of the renderable object. You are of course allowed to change the renderable mesh and material, as well as reposition, scale or orient the renderable during runtime.

## Multiple materials
In the example above we use a single material for a single mesh, but it can sometimes be useful to use different materials for different parts of the mesh.

It is up to the artist (creator of the mesh) to specify the regions of the mesh that will use separate materials. Upon mesh import those regions will be recognized as sub-meshes.

You can assign a different material to a sub-mesh by calling a @ref bs::CRenderable::setMaterial(UINT32, HMaterial) "Renderable::setMaterial" overload which accepts an additional index parameter, specifying which sub-mesh to apply the material on.

~~~~~~~~~~~~~{.cpp}
HMesh mesh = gImporter().import<Mesh>("myMesh.fbx");
renderable->setMesh(mesh);

// Count the number of sub-meshes
auto& meshProps = mesh->getProperties();
UINT32 numSubMeshes = meshProps.getNumSubMeshes();

... create necessary materials ...

// Assign a different material on every submesh
for(UINT32 i = 0; i < numSubMeshes; i++)
	renderable->setMaterial(i, materials[i]);
~~~~~~~~~~~~~