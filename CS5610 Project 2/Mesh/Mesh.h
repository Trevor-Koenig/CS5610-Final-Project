/**
*
* Majority code implementation from:
* https://www.omardelarosa.com/posts/2020/01/06/infinity-terrain-in-c++-using-perlin-noise-and-opengl
* https://github.com/omardelarosa/infinity-terrain-3d
*
**/

#pragma once

#include <vector>
#include <map>
#include <stdlib.h>
#include <math.h>
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"
#include "../CyCodeBase/cyVector.h"

class Mesh
{
public:
	void generateVertices(unsigned int w, unsigned int h);
	std::vector<cy::Vec3f> getVertices();
	std::vector<cy::Vec3f> getNorms();
	std::vector<cy::Vec4f> getColors();
	std::vector<cy::Vec3f> getFaces();
	std::map<unsigned long, std::vector<int>> getTrianglesMap();
	float getMeshWidth();
	float getMeshLength();

private:
	float noise_callback(float x, float y, float z, int octaves, double persistence);
	float height(cy::Vec2f loc);
	double perlin(double x, double y, double z);

	std::vector<cy::Vec3f> vertices;
	std::vector<cy::Vec3f> normals;
	std::vector<cy::Vec4f> vertex_colors;
	std::vector<cy::Vec3f> faces;
	std::map<unsigned long, std::vector<int>> vertex_to_triangles_map;
	float spacing;
	float vertex_width;
	float vertex_length;

	// Perlin Noise Generator from: https://adrianb.io/2014/08/09/perlinnoise.html
	// most of this code is static, const, or both because in this program it does not matter if every mesh looks the same
	static const int permutation[];
	static int* p;
	static double fade(double t);
	static double grad(int hash, double x, double y, double z);
	static double lerp(double a, double b, double x);
};

