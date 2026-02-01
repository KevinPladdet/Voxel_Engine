#pragma once

#include "IncludeGraphics.h"
#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include<cerrno>

std::string get_file_contents(const char* filename);

class Shader
{
public:
	// Reference ID of the Shader Program
	GLuint ID;
	// Constructor that builds the Shader Program from 2 different shaders
	Shader(const char* vertexFile, const char* fragmentFile);

	void Activate();
	void Delete();
private:
	// Checks if the different Shaders have compiled properly
	void CompileErrors(unsigned int shader, const char* type);
};