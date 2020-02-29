![Kaleido](/assets/engine-data/logo.png?style=centerme "Kaleido")

# Kaleido

Kaleido is a graphics engine focused on renderer extensibility, written in modern C++17.
It's primary focus is to support the creation of multiple renderers that can be hot-swapped in real-time, even while the scene is updating.

## Features

### Rendering

* Compile time registration of multiple renderers of different rendering contexts (renderer abstraction)
* Hot-swapping between renderers of different contexts at runtime
* Generic GPU asset caching 
* Sample OpenGL PBR (direct lighting) renderers
* Runtime shader editing and reloading (OpenGL)
* Support for rendering optimization techniques (eg. frustum culling)
* Sample Vulkan renderer (albedo only)

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

The development is done on MSVC 19.22. Clang-cl v9 is also regularly tested. 
CMake 3.11 is required.

## Getting started

```
 git clone --recursive https://github.com/RaygenGroup/kaleido
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
* [imfilebrowser](https://github.com/AirGuanZ/imgui-filebrowser)
* [minimal-ti](https://github.com/katagis/minimal-ti)

## Authors

| Info | Role |
| ------|-----|
|**John Moschos**, [Renoras](https://github.com/Renoras)| Co-Founder, Programmer |
|**Harry Katagis**, [katagis](https://github.com/katagis)| Co-Founder, Programmer |
