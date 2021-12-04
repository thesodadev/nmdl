#pragma once
#include "common.h"

bool parseObjFile(const char* file_data, uint64_t size, 
                  std::vector<Vector3Df>& out_vertices, std::vector<Vector2Df>& out_uvs,
                  std::vector<Vector3Df>& out_normals, std::vector<FaceInfo>& out_faces);