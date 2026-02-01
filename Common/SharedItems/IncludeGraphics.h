#pragma once

#ifdef WINDOWS_BUILD
//include glad and glfw for Windows build
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#else
#include <GLES2/gl2.h>
#endif

#ifdef Raspberry_BUILD
#include <GLES3/gl3.h>
#endif