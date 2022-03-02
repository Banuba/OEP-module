# Offscreen Effect Player module

## Description

Common submodule for some projects using **offscreen_effect_player**
The module implements interaction with the effect player. The module consists of six main interfaces:

- **effect_player** - implementation contains interaction with the effect player. Should be implemented on the application side.
- **image_processing_result** - the implementation contains the output image processor in the required format. Some formats are processed on GPU or CPU side.
- **offscreen_effect_player** - the implementation connects all interfaces into a single whole. The main interface through which the entire pipeline for processing a stream of frames goes.
- **offscreen_render_target** - implementation creates and configures offscreen rendering. Uses the OpenGL renderer.
- **pixel_buffer** - input and output pixel buffer (image). Contains an image in any of the supported formats: RGB, RGBA, BGR, BGRA, ARGB, nv12, i420.
- **render_context** - platform-specific rendering context. Should be implemented on the application side.

## Structure

- **offscreen_effect_player** - is a wrapper for effect_player. It allows you to use your own implementation for offscreen_render_target
- **offscreen_render_target** - is an implementation option for the offscreen_render_target interface. Allows to prepare gl framebuffers and textures for receiving a frame from gpu, receive bytes of the processed frame from the gpu and pass them to the cpu, as well as, if necessary, set the orientation for the received frame. This implementation uses GLFW to work with gl context
- **interfaces** - offscreen effect player interfaces
- **opengl** - OpenGL utilities

## Note

To integrate the Offscreen effect player into your application, you need to copy the submodule folder and implement interfaces in which there must be logic different from the current implementation.

This module is configurable, see CMakeLists.txt for details.
