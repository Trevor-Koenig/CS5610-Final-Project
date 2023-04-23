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
    spacing = 5.0;
    int maxHeight = 0;
    // Rows
    for (int r = 0; r < h; r++) {
        // Cols
        for (int c = 0; c < w; c++) {
            // NOTE: origin is not at center of mesh
            float x = c; // col
            float z = r; // row
            float y =
                
                noise_callback( // perlin or other noise func
                    (float)c / w,   // x - normalized between 0-1
                    1,              // y
                    (float)r / h,   // z - normalized between 0-1
                    6,              // octaves
                    0.5               // persistance
                );
            // modify y to make the terrain more extreme
            // this was determined by guess and test and is subjective
            y = pow(y, 2) * 200;

            if (y * spacing > maxHeight) { maxHeight = y * spacing; }
            cy::Vec3f v(x * spacing, y * spacing, z * spacing); //

            // Vertex colors supported!
            if (v.y < (.3 * maxHeight))
            {
                // blue lakes
                vertex_colors.push_back(cy::Vec4f(0.0, 0.0, 1.0, 1.0));
                // flatten lakes
                v.y = .3 * maxHeight;
            }
            else if (v.y < (.4 * maxHeight))
            {
                // color: https://htmlcolorcodes.com/colors/sand/
                vertex_colors.push_back(cy::Vec4f(0.7578, 0.6953, 0.5, 1.0));
            }
            else if (v.y < (.6 * maxHeight))
            {
                // green grass
                vertex_colors.push_back(cy::Vec4f(0.0, 1.0, 0.0, 1.0));
            }
            else
            {
                // stone grey
                vertex_colors.push_back(cy::Vec4f(0.5, 0.5, 0.5, 1.0));
            }

            vertices.emplace_back(v);

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
    // rows (x)
    for (int r = 0; r < h; r++) {
        // Cols (z)
        for (int c = 0; c < w; c++) {

            float x = c; // col
            float z = r; // row

            // account for edge normals
            if (x == 0 || z == 0 || z == h - 1 || x == w-1)
            {
                normals.push_back(cy::Vec3f(0.0f, 1.0f, 0.0f));
                continue;
            }

            // read neightbor heights using an arbitrary small offset
            // first vector is position, second is offset
            float hL = height(cy::Vec2f(x, z) - cy::Vec2f(1.0, 0.0));
            float hR = height(cy::Vec2f(x, z) + cy::Vec2f(1.0, 0.0));
            float hD = height(cy::Vec2f(x, z) - cy::Vec2f(0.0, 1.0));
            float hU = height(cy::Vec2f(x, z) + cy::Vec2f(0.0, 1.0));

            // deduce terrain normal
            cy::Vec3f N;
            N.x = hL - hR;
            N.y = hD - hU;
            N.z = 1.0 * spacing;
            normals.push_back(cy::Normalize(N));
        }
    }

    // Post-processing code
    std::vector<cy::Vec3f> newVerts;
    std::vector<cy::Vec3f> newNorms;
    std::vector<cy::Vec4f> newColors;
    int faceCount = faces.size();
    newVerts.reserve(faceCount);
    newNorms.reserve(faceCount);
    newColors.reserve(faceCount);
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

        // arrange colors
        newColors.emplace_back(vertex_colors[faces[i][2]]);
        newColors.emplace_back(vertex_colors[faces[i][1]]);
        newColors.emplace_back(vertex_colors[faces[i][0]]);
    }

    //  udate this field to make it easier to draw arrays and access the ordered list later.
    vertices = newVerts;
    normals = newNorms;
    vertex_colors = newColors;
}


float Mesh::noise_callback(float x, float y, float z, int octaves, double persistence)
{
    double total = 0;
    double frequency = 4;
    double amplitude = 128;
    double maxValue = 0;  // Used for normalizing result to 0.0 - 1.0
    for (int i = 0; i < octaves; i++) {
        total += perlin(x * frequency, y * frequency, z * frequency) * amplitude;

        maxValue += amplitude;

        amplitude *= persistence;
        frequency *= 2;
    }

    // std::cout << "New Height: " << (total / maxValue) << std::endl;
    return (total / maxValue);
}

float Mesh::height(cy::Vec2f loc)
{
    return vertices[(loc.y * vertex_width) + loc.x].y;
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
std::vector<cy::Vec4f> Mesh::getColors() { return vertex_colors; }

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

/// <summary>
/// Generate Perlin noise
/// Source: https://adrianb.io/2014/08/09/perlinnoise.html
/// </summary>
/// <param name="x">x coordinate in the mesh</param>
/// <param name="y">y coordinate in the mesh</param>
/// <param name="z">z coordinate in the mesh</param>
/// <returns>a new a value representing the new height of the mesh at the  x,z location</returns>
double Mesh::perlin(double x, double y, double z)
{
    for (int x = 0; x < 512; x++) {
        p[x] = permutation[x % 256];
    }

    int xi = (int)x & 255;                              // Calculate the "unit cube" that the point asked will be located in
    int yi = (int)y & 255;                              // The left bound is ( |_x_|,|_y_|,|_z_| ) and the right bound is that
    int zi = (int)z & 255;                              // plus 1.  Next we calculate the location (from 0.0 to 1.0) in that cube.
    double xf = x - (int)x;
    double yf = y - (int)y;
    double zf = z - (int)z;

    double u = fade(xf);
    double v = fade(yf);
    double w = fade(zf);

    int aaa, aba, aab, abb, baa, bba, bab, bbb;
    aaa = p[p[p[xi] + yi] + zi];
    aba = p[p[p[xi] + yi + 1] + zi];
    aab = p[p[p[xi] + yi] + zi + 1];
    abb = p[p[p[xi] + yi + 1] + zi + 1];
    baa = p[p[p[xi + 1] + yi] + zi];
    bba = p[p[p[xi + 1] + yi + 1] + zi];
    bab = p[p[p[xi + 1] + yi] + zi + 1];
    bbb = p[p[p[xi + 1] + yi + 1] + zi + 1];

    double x1, x2, y1, y2;
    x1 = lerp(grad(aaa, xf, yf, zf),           // The gradient function calculates the dot product between a pseudorandom
        grad(baa, xf - 1, yf, zf),             // gradient vector and the vector from the input coordinate to the 8
        u);                                    // surrounding points in its unit cube.
    x2 = lerp(grad(aba, xf, yf - 1, zf),       // This is all then lerped together as a sort of weighted average based on the faded (u,v,w)
        grad(bba, xf - 1, yf - 1, zf),         // values we made earlier.
        u);
    y1 = lerp(x1, x2, v);

    x1 = lerp(grad(aab, xf, yf, zf - 1),
        grad(bab, xf - 1, yf, zf - 1),
        u);
    x2 = lerp(grad(abb, xf, yf - 1, zf - 1),
        grad(bbb, xf - 1, yf - 1, zf - 1),
        u);
    y2 = lerp(x1, x2, v);

    return (lerp(y1, y2, w) + 1) / 2;                      // For convenience we bind the result to 0 - 1 (theoretical min/max before is [-1, 1])
}

/// <summary>
/// This creates a more soft appearence on the noise
/// Source: https://adrianb.io/2014/08/09/perlinnoise.html
/// </summary>
/// <param name="t">the value to be softened</param>
/// <returns>a softened value</returns>
double Mesh::fade(double t) {
    // Fade function as defined by Ken Perlin.  This eases coordinate values
    // so that they will ease towards integral values.  This ends up smoothing
    // the final output.
    return t * t * t * (t * (t * 6 - 15) + 10);         // 6t^5 - 15t^4 + 10t^3
}

/// <summary>
/// calculate the dot product of a randomly selected gradient vector and the 8 location vectors
/// Source: https://adrianb.io/2014/08/09/perlinnoise.html
/// Source: http://riven8192.blogspot.com/2010/08/calculate-perlinnoise-twice-as-fast.html
/// 
/// This code picks a random vector from the following 12 vectors:
/// (1, 1, 0), (-1, 1, 0), (1, -1, 0), (-1, -1, 0),
/// (1, 0, 1), (-1, 0, 1), (1, 0, -1), (-1, 0, -1),
/// (0, 1, 1), (0, -1, 1), (0, 1, -1), (0, -1, -1)
/// </summary>
/// <param name="hash">a hashed value</param>
/// <param name="x">coordinate in the mesh</param>
/// <param name="y">coordinate in the mesh</param>
/// <param name="z">coordinate in the mesh</param>
/// <returns>a gradient value for the noise generator</returns>
double Mesh::grad(int hash, double x, double y, double z) {
    switch (hash & 0xF)
    {
    case 0x0: return  x + y;
    case 0x1: return -x + y;
    case 0x2: return  x - y;
    case 0x3: return -x - y;
    case 0x4: return  x + z;
    case 0x5: return -x + z;
    case 0x6: return  x - z;
    case 0x7: return -x - z;
    case 0x8: return  y + z;
    case 0x9: return -y + z;
    case 0xA: return  y - z;
    case 0xB: return -y - z;
    case 0xC: return  y + x;
    case 0xD: return -y + z;
    case 0xE: return  y - x;
    case 0xF: return -y - z;
    default: return 0; // never happens
    }
}

// Linear Interpolate
// Source: https://adrianb.io/2014/08/09/perlinnoise.html
double Mesh::lerp(double a, double b, double x) {
    return a + x * (b - a);
}

/// <summary>
/// Initalize permutation array for Perlin Noise
/// Source: https://adrianb.io/2014/08/09/perlinnoise.html
/// </summary>
const int Mesh::permutation[] = { 151, 160, 137, 91, 90, 15,												// Hash lookup table as defined by Ken Perlin.  This is a randomly
        131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,		// arranged array of all numbers from 0-255 inclusive.
        190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
        88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
        77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
        102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
        135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
        5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
        223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
        129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
        251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
        49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
        138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
};

int* Mesh::p = new int[512];                           // Doubled permutation to avoid overflow