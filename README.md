# ORVIS
**O**penGL **R**endering and **Vis**ualization

Orvis is a low-level OpenGL 4.6 rendering framework and meinly intended for my personal projects.
It was partially inspired by [Graphics2](https://github.com/Max1412/Graphics2) and [DINO](https://gitlab.uni-koblenz.de/CVFP_DF18/Framework) in both of which I was involved as well.

## Requirements
- OpenGL 4.6 and ARB_BINDLESS_TEXTURE support
- C++ 17
- Only tested on Windows, but Linux should work as well

## External dependencies
#### Build system
- [CMake](https://cmake.org/)
- [vcpkg](https://github.com/Microsoft/vcpkg)

#### Libraries
- [ImGui](https://github.com/ocornut/imgui)
- [stb_image & stb_image_write](https://github.com/nothings/stb)
- [GLshader](https://gitlab.uni-koblenz.de/johannesbraun/glshader) by Johannes Braun
- [Assimp](http://assimp.org/) (tested with 3.3.1)
- [glfw3](http://www.glfw.org/) (tested with 3.2.1)
- [glbinding](https://github.com/cginternals/glbinding) (tested with 2.1.1)
- [glm](https://glm.g-truc.net/0.9.8/index.html) (tested with 0.9.8.4)
