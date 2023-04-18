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

    // Post-processing code
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
    // TODO: replace with heightmap code
    return 0.0;
}

/// <summary>
/// Order the given list of vertices according to the faces list in the current mesh.
/// Throws an exception if there is an error.
/// Updates the arrayVertices field as well with the 'ordered' Vertices.
/// </summary>
/// <param name="vertices">a list of vertices to be reordered</param>
/// <returns>returns a copy of the changed vertices</returns>
std::vector<cy::Vec3f> Mesh::orderByFaces(std::vector<cy::Vec3f> verts)
{
    std::vector<cy::Vec3f> newVerts;
    int iters = faces.size();
    for (int i = 0; i < iters; i++)
    {
        newVerts.emplace_back(verts[faces[i][2]]);
        newVerts.emplace_back(verts[faces[i][1]]);
        newVerts.emplace_back(verts[faces[i][0]]);
    }

    //  udate this field to make it easier to draw arrays and access the ordered list later.
    arrayVertices = newVerts;
    return newVerts;
}

/// <summary>
/// Get Vertices List - returns a deep copy of vertices
/// Designed to be rendered using glDrawArrays()
/// </summary>
/// <returns>vector of cy::Vec3f indicating vertex positions</returns>
std::vector<cy::Vec3f> Mesh::getVertices() { return vertices; }

/// <summary>
/// Gets the ordered list of vertices.
/// returns an empty list if orderByFaces is not called first.
/// </summary>
/// <returns></returns>
std::vector<cy::Vec3f> Mesh::getOrderedVertices() { return arrayVertices; }

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