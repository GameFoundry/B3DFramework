---
title: Importing shaders
---
All shaders are written using BSL syntax described in the previous manual. The code is stored as a raw text file using the .bsl extension (or .bslinc for include files). Shaders can then be imported from a .bsl file as any other resources, using the **Importer**.

~~~~~~~~~~~~~{.cpp}
// Import a shader named "MyShader.bsl" from disk
HShader shader = GetImporter().Import<Shader>("MyShader.bsl");
~~~~~~~~~~~~~

After import, the shader can be assigned to a **Material** and used for rendering, as shown in an earlier chapter.

~~~~~~~~~~~~~{.cpp}
// Import shader
HShader shader = GetImporter().Import<Shader>("MyShader.bsl");

// Create material from shader
HMaterial material = Material::Create(shader);

// Set material parameters (if the shader defines them)
material->SetFloat("roughness", 0.5f);
material->SetColor("tint", Color::Red);
material->SetTexture("albedoMap", albedoTexture);
~~~~~~~~~~~~~
