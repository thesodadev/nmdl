#pragma once

#include <cinttypes>
#include <vector>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <cstdio>
#include <map>
#include <string>
#include <sstream>

struct Vector3Df
{
    float x {0.0f};
    float y {0.0f};
    float z {0.0f};
};

struct Vector2Df
{
    float x {0.0f};
    float y {0.0f};
};

struct FaceInfo
{
    std::vector<uint32_t> vertex;
    std::vector<uint32_t> uv;
    std::vector<uint32_t> normal;
};

enum class EFaceType { eTriangle, eQuad };

struct PolygonsData
{
    std::vector<Vector3Df> vertices;
    std::vector<Vector2Df> uvs;
    std::vector<Vector3Df> normals;
};
