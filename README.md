# Offscreen Effect Player module (OEP-module)

[Description of the **Offscreen Effect Player** on the Banuba SDK website](https://docs.banuba.com/face-ar-sdk-v1/core/cpp_api_overview#offscreen-effect-player)

## Description

A common submodule for projects based on C++ that integrate Banuba SDK. It is independent of the Banuba SDK and consists of implementations of interfaces for  [**Offscreen effect player**](./interfaces/offscreen_effect_player.hpp). The module is a wrapper over Banuba SDK interfaces and provides necessary interfaces for image processing including postprocessing (orientation change, format conversion), and effects management. This module demonstrates one of the approaches and simplifies integration with the application.

## Folders structure

- [**docs**](./docs/) - illustrations with screenshots and images
- [**interfaces**](./interfaces/) - contains the declaration of the offscreen effect player
- [**offscreen_effect_player**](./offscreen_effect_player/) - contains the implementation of the **offscreen_effect_player**, **image_processing_result** and **pixel_buffer** interfaces. The implementation of **offscreen_effect_player** manages the rendering via the **ofscreen_render_target** interface and manages **effect_player** providing the main API for image processing by the Banuba SDK.
- [**offscreen_render_target**](./offscreen_render_target/) - contains the implementation for the **offscreen_render_target** interface. The purpose of this submodule is to provide and manage the graphical context for offscreen rendering. By default, it implements OpenGL but can be overridden at the application level to use other rendering engines. The current implementation prepares OpenGL framebuffers and textures for rendering and frame postprocessing (the resulted image conversions and transformations).
- [**opengl**](./opengl/) - OpenGL utilities used by **offscreen_render_target** interface implementation
- [**third**](./third/) - third party libraries

## Description of interfaces

This module is a part of the image processing flow: it accepts images for processing, configures the offscreen rendering context, passes the images further to the main rendering engine of the Banuba SDK, and provides the capabilities for image postprocessing afterwards. Based on this, the module only implements some of the interfaces which don't include the specifics of a particular API. For example, Banuba SDK supports [**C++ API**](https://docs.banuba.com/face-ar-sdk-v1/core/cpp_api_overview) and **C API**, but the module isn't dependent on any of them. That's why effect_player interface is left unimplemented, and its implementation is provided by the application.
- [**effect_player**](./interfaces/effect_player.hpp) - abstraction over Banuba SDK. All interactions with Banuba SDK happen through this interface. Should be implemented on the application side
- [**image_processing_result**](./interfaces/image_processing_result.hpp) - controls the processing for output image in the required format
- [**offscreen_effect_player**](./interfaces/offscreen_effect_player.hpp) - connects all other interfaces into a single whole. The main interface through which the entire pipeline for processing a stream of frames goes.
- [**offscreen_render_target**](./interfaces/offscreen_render_target.hpp) - creates and configures offscreen rendering with the required rendering API
- [**pixel_buffer**](./interfaces/pixel_buffer.hpp) - input and output pixel buffer. Contains an image in any of the supported formats: [RGB, RGBA, BGR, BGRA, ARGB, nv12, i420](./interfaces/image_format.hpp)
- [**render_context**](./interfaces/render_context.hpp) - platform-specific rendering context. Should be implemented on the application side

## The scheme of interfaces interaction

![scheme of interfaces interaction](./docs/OEP-module-interfaces.png)


## The scheme of image processing

![sequence diagram](./docs/OEP-module-sequence-diagram.png)


## How to integrate OEP-module to the external application

1. Copy [**OEP-module**](https://github.com/Banuba/OEP-module) to your project, or make it as a [**submodule**](https://git-scm.com/book/en/v2/Git-Tools-Submodules);
2. Add the subfolder that contains the **OEP-module** to your **CMakeLists.txt**
3. Write an implementation of the following interfaces: **effect_player** and **render_context**
4. Now you can use the OEP-module in your code

This module is configurable, see [**CMakeLists.txt**](./CMakeLists.txt) for details.
If the logic of the OEP module does not allow you to do what you want, then you can implement interfaces that should have logic that is different from the current implementation. For example, by default, the OEP module works with OpenGL. If you want to use another rendering API, then you have to write your own implementation  [**offscreen render target**](./interfaces/offscreen_render_target.hpp) like this [**example**](https://github.com/Banuba/OEP-macos).

## List of examples using the OEP-module

All of the projects listed below have an implementation on the [**CPP-API**](https://docs.banuba.com/face-ar-sdk-v1/core/cpp_api_overview) or on the **C-API**. [**You can read more details here**](https://docs.banuba.com/face-ar-sdk-v1/)
- [OEP-desktop](https://github.com/Banuba/OEP-desktop) Windows and MacOS platforms using OpenGL and Banuba SDK C++ API
- [OEP-desktop-c-api](https://github.com/Banuba/OEP-desktop-c-api) Windows and MacOS platforms using OpenGL and Banuba SDK C API
- [quickstart-android-c-api](https://github.com/Banuba/quickstart-android-c-api) Android platform with using OpenGL and Banuba SDK C API
- [agora-plugin-filters-android](https://github.com/Banuba/agora-plugin-filters-android) Android platform using OpenGL and Banuba SDK C++ API
- [OEP-macos](https://github.com/Banuba/OEP-macos) MacOS platform using Metal and Banuba SDK C++ API
