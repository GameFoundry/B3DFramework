Importing shaders 						{#importingShaders}
===============

All shaders are written using BSL syntax described in the previous manual. The code is stored as a raw text file using the .bsl extension (or .bslinc for include files). Shader can then be imported from a .bsl file as any other resources, using the **Importer**.

~~~~~~~~~~~~~{.cpp}
// Import a shader named "myShader.bsl" from disk
HShader shader = gImporter().import<Shader>("myShader.bsl");
~~~~~~~~~~~~~

After import, shader can be assigned to a **Material** and used for rendering, as shown in an earlier chapter.