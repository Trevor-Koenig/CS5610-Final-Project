/**
* 
* Majority code implementation from:
* https://www.omardelarosa.com/posts/2020/01/06/infinity-terrain-in-c++-using-perlin-noise-and-opengl
* https://github.com/omardelarosa/infinity-terrain-3d
* 
**/

#include "Mesh.h"

/// <summary>
/// Generate the attributes of this Mesh instance
/// </summary>
/// <param name="w">width of the mesh(num of vertices)</param>
/// <param name="h">height of the mesh(num of vertices)</param>
void Mesh::generateVertices(unsigned int w, unsigned int h) {
    // Create vertices
    vertices.reserve(w * h);
    vertex_width = w;
    vertex_length = h;
    spacing = 10.0;
    // Rows
    for (int r = 0; r < h; r++) {
        // Cols
        for (int c = 0; c < w; c++) {
            // NOTE: origin is not at center of mesh
            float x = c; // col
            float z = r; // row
            float y =
                noise_callback( // perlin or other noise func
                    (float)c / w, // x
                    (float)r / h // y
                );
            cy::Vec3f v(x * spacing, y, z * spacing); //
            vertices.emplace_back(v);

            // Vertex colors not supported!
            vertex_colors.push_back(cy::Vec3f(0.0, 0.0, 0.0));

            // Create a vector in the mapping
            std::vector<int> triangles;
            vertex_to_triangles_map.emplace(vertices.size() - 1, triangles);
        }
    }

    // Generate faces

    // Rows (-1 for last)
    for (int r = 0; r < h - 1; r++) {
        // Cols (-1 for last)
        for (int c = 0; c < w - 1; c++) {
            // Upper triangle
            /*

                v0 -- v2
                |    /
                |  /
                v1
            */
            int f0_0 = (r * w) + c;
            int f0_1 = ((r + 1) * w) + c;
            int f0_2 = (r * w) + c + 1;
            cy::Vec3f f0(f0_0, f0_1, f0_2);
            faces.push_back(f0); // TODO: emplace

            // Lower triangle
            /*

                      v2
                     / |
                   /   |
                v0 --- v1
            */
            int f1_0 = ((r + 1) * w) + c;
            ;
            int f1_1 = ((r + 1) * w) + c + 1;
            int f1_2 = (r * w) + c + 1;

            cy::Vec3f f1(f1_0, f1_1, f1_2);
            faces.push_back(f1); // TODO: emplace
        }
    }

    // Generate Normals
    normals.reserve(w * h);
    for (int r = 0; r < h; r++) {
        // Cols
        for (int c = 0; c < w; c++) {

            // account for edge normals
            if (c == 0 || r == 0 || r == h - 1 || c == w-1)
            {
                normals.push_back(cy::Vec3f(0.0f, 1.0f, 0.0f));
                continue;
            }

            // read neightbor heights using an arbitrary small offset
            // first vector is position, second is offset
            float hL = height(cy::Vec2f(c, r) - cy::Vec2f(1.0, 0.0));
            float hR = height(cy::Vec2f(c, r) + cy::Vec2f(1.0, 0.0));
            float hD = height(cy::Vec2f(c, r) - cy::Vec2f(0.0, 1.0));
            float hU = height(cy::Vec2f(c, r) + cy::Vec2f(0.0, 1.0));

            // deduce terrain normal
            cy::Vec3f N;
            N.x = hL - hR;
            N.y = 2.0;
            N.z = hD - hU;
            normals.push_back(cy::Normalize(N));
        }
    }

    // Post-processing code
    std::vector<cy::Vec3f> newVerts;
    std::vector<cy::Vec3f> newNorms;
    int faceCount = faces.size();
    newVerts.reserve(faceCount);
    newNorms.reserve(faceCount);
    for (int i = 0; i < faceCount; i++)
    {
        // arrange vertices
        newVerts.emplace_back(vertices[faces[i][2]]);
        newVerts.emplace_back(vertices[faces[i][1]]);
        newVerts.emplace_back(vertices[faces[i][0]]);

        // arrange normals
        newNorms.emplace_back(normals[faces[i][2]]);
        newNorms.emplace_back(normals[faces[i][1]]);
        newNorms.emplace_back(normals[faces[i][0]]);
    }

    //  udate this field to make it easier to draw arrays and access the ordered list later.
    vertices = newVerts;
    normals = newNorms;
}

/// <summary>
/// Generate noise to determine the height map of the mesh.
/// Do not call manually.
/// </summary>
/// <param name="x">x coordinate in the mesh</param>
/// <param name="y">y coordinate within the mesh</param>
/// <returns>z value indicating the height of the mesh</returns>
float Mesh::noise_callback(float x, float y)
{
    return rand() % 50;
}

float Mesh::height(cy::Vec2f loc)
{
    return vertices[(loc.x * vertex_width) + loc.y].y;
}

/// <summary>
/// Get Vertices List - returns a deep copy of vertices
/// Designed to be rendered using glDrawArrays()
/// </summary>
/// <returns>vector of cy::Vec3f indicating vertex positions</returns>
std::vector<cy::Vec3f> Mesh::getVertices() { return vertices; }

/// <summary>
/// Get vector of normals for the mesh
/// </summary>
/// <returns>v</returns>
std::vector<cy::Vec3f> Mesh::getNorms() { return normals; }

/// <summary>
/// Currently unsupported.
/// </summary>
/// <returns>list of zeros</returns>
std::vector<cy::Vec3f> Mesh::getColors() { return vertex_colors; }

/// <summary>
/// Get Face Indices List - returns a deep copy of indices
/// Designed to be rendered using glDrawArrays()
/// </summary>
/// <returns>vector of cy::Vec3f indicating vertex indices in face groups of triangles</returns>
std::vector<cy::Vec3f> Mesh::getFaces() { return faces; }

/// <summary>
/// Get map of triangles - currently unsupported
/// </summary>
/// <returns> returns map of triangles(vector<int>)</returns>
std::map<unsigned long, std::vector<int>>  Mesh::getTrianglesMap() { return vertex_to_triangles_map;  }

/// <summary>
/// calculates mesh width in world coordinates
/// </summary>
/// <returns>a float representing the mesh width in world coordinates</returns>
float Mesh::getMeshWidth()
{
    return spacing * (vertex_width-1);
}

/// <summary>
/// calculates mesh height in world coordinates
/// </summary>
/// <returns>a float representing the mesh height in world coordinates</returns>
float Mesh::getMeshLength()
{
    return spacing * (vertex_length-1);
}