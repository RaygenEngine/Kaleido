![Kaleido](/assets/engine-data/logo.png?style=centerme "Kaleido")

# Kaleido

Kaleido is a graphics engine focused on renderer extensibility, written in modern C++17.
It's primary focus is to support the creation of multiple renderers that can be hot-swapped in real-time, even while the scene is updating.

## Features

### Rendering
* Hot-swappable renderers of different contexts
* Sample OpenGL Forward PBR & Deferred PBR renderers
* Stereo VR Renderer
* Directional, omni & spot lights with shadow mapping
* Basic post-process
* Frustum culling

### World & Editor
* Extensible type-system with reflection support for user types
* GUI Real-time drag & drop scene editor
* Auto-generated GUI property editor for user types
* JSON Serialization of user types through reflection
* Scene saving and loading 

### Assets
* Runtime multi-threaded asset loading
* Dynamic loading, unloading & automated caching
* Extensible asset loaders
* Support for GLTF, Generic JSON, PNG, JPG, BMP

## Screenshots

![Kaleido](/assets/engine-data/screenshots/01.png "Screenshot 1")

![Kaleido](/assets/engine-data/screenshots/02.png "Screenshot 2")

## Requirements

The development is done mainly done on MSVC 19.22. Clang-cl v9 is also regularly tested. 
CMake 3.11 is required.

## Getting started

```
 git clone --recursive --shallow-submodules https://github.com/renoras/kaleido
 cd kaleido
 mkdir build
 cd build
 cmake ..
 ```
If you are using the .sln to build remember to change the startup project to Kaleido.
Then just build & run. Default assets are included for the preview scene.

## Dependencies (included as submodules)

* [glm](https://github.com/g-truc/glm)
* [ImGui](https://github.com/ocornut/imgui)
* [spdlog](https://github.com/gabime/spdlog)
* [stb](https://github.com/nothings/stb)
* [tinygltf](https://github.com/syoyo/tinygltf)
* [nlohmann/json](https://github.com/nlohmann/json)
* [magic_enum](https://github.com/Neargye/magic_enum)
* [glad](https://github.com/Dav1dde/glad)

## Authors

| Info | Role |
| ------|-----|
|**John Moschos**, [Renoras](https://github.com/Renoras)| Co-Founder, Programmer |
|**Harry Katagis**, [katagis](https://github.com/katagis)| Co-Founder, Programmer |
