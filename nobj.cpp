#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <cstdio>
#include <map>
#include <string>
#include <sstream>
#include "steroids.h"


// Convert float to vertex 4.12 fixed format
inline int16_t floatToV16(float value) { return (int16_t)((value) * (1 << 12)); }

inline uint32_t pack2xV16Into32Bit(int16_t value_1, int16_t value_2)
{
    return (uint32_t)(((value_1) & 0xFFFF) | ((value_2) << 16));
}

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

std::string toString(EFaceType face)
{
    switch (face)
    {
    case EFaceType::eTriangle: return "Triangle";
    case EFaceType::eQuad: return "Quad";
    }

    return "error";
}

inline const char* findNext(const char* start, const char* end, char target)
{
    const char* p = start;

    while (*p != target && p < end) p++;

    return p != end ? p : nullptr;
}

inline uint64_t distance(const char* start, const char* end) { return (uint64_t)(end - start); }

inline uint32_t count(const char* start, const char* end, char target) 
{
    const char* p = start;

    uint32_t c = 0;
    while (p <= end)
    {
        if (*p == target) c++;
        p++;
    }

    return c;
}

inline bool isInBounds(const char* p, const char* end, uint32_t offset = 0) { return p + offset <= end; }

inline bool readVector3Df(const char* str, const char* end, Vector3Df& out_vector)
{
    const char* p = str;

    if (!ST::strToFloat(p, distance(p, end) + 1, out_vector.x))
    {
        std::cerr << "Cannot convert to float" << std::endl;
        return false;
    }

    const char* next_space = findNext(p, end, ' ');
    if (!next_space)
    {
        std::cerr << "Cannot find space delimeter" << std::endl;
        return false;
    }

    p = next_space + 1;

    if (!ST::strToFloat(p, distance(p, end) + 1, out_vector.y))
    {
        std::cerr << "Cannot convert to float" << std::endl;
        return false;
    }

    next_space = findNext(p, end, ' ');
    if (!next_space)
    {
        std::cerr << "Cannot find space delimeter" << std::endl;
        return false;
    }

    p = next_space + 1;

    if (!ST::strToFloat(p, distance(p, end) + 1, out_vector.z))
    {
        std::cerr << "Cannot convert to float" << std::endl;
        return false;
    }

    return true;
}

inline bool readVector2Df(const char* str, const char* end, Vector2Df& out_vector)
{
    const char* p = str;

    if (!ST::strToFloat(p, distance(p, end) + 1, out_vector.x))
    {
        std::cerr << "Cannot convert to float" << std::endl;
        return false;
    }

    const char* next_space = findNext(p, end, ' ');
    if (!next_space)
    {
        std::cerr << "Cannot find space delimeter" << std::endl;
        return false;
    }

    p = next_space + 1;

    if (!ST::strToFloat(p, distance(p, end) + 1, out_vector.y))
    {
        std::cerr << "Cannot convert to float" << std::endl;
        return false;
    }

    return true;
}

inline bool readFaceInfo(const char* str, const char* end, FaceInfo& out_entry)
{
    // format: %u/%u/%u %u/%u/%u %u/%u/%u ...

    const char* p = str;

    while(true)
    {
        out_entry.vertex.push_back(atoi(p));

        const char* next_slash = findNext(p, end, '/');
        if (!next_slash)
        {
            std::cerr << "Cannot find slash delimeter" << std::endl;
            return false;
        }

        p = next_slash + 1;

        out_entry.uv.push_back(atoi(p));
        
        next_slash = findNext(p, end, '/');
        if (!next_slash)
        {
            std::cerr << "Cannot find slash delimeter" << std::endl;
            return false;
        }
        p = next_slash + 1;

        out_entry.normal.push_back(atoi(p));

        const char* next_space = findNext(p, end, ' ');
        if (!next_space) break;
        p = next_space + 1;
    }

    return true;
}

bool parseObjFile(const char* file_data, uint64_t size, 
                  std::vector<Vector3Df>& out_vertices, std::vector<Vector2Df>& out_uvs,
                  std::vector<Vector3Df>& out_normals, std::vector<FaceInfo>& out_faces)
{
    const char* p = file_data;
    const char* const data_end = file_data + size;

    uint32_t line_n = 1;

    while (p < data_end)
    {
        const char* newline_pos = findNext(p, data_end, '\n');

        if (!newline_pos)
        {
            std::cerr << "Line: '" << line_n << "': ";
            std::cerr << "Cannot find newline" << std::endl;
            return false;
        }

        uint64_t d = distance(p, newline_pos);

        if (d > 2 && p[0] == 'v' && p[1] == ' ') // vertex
        {
            if (count(p, newline_pos, ' ') != 3)
            {
                std::cerr << "Line: '" << line_n << "': ";
                std::cerr << "Vertex entry doesn't match syntax" << std::endl;
                return false;
            }

            Vector3Df buffer;
            if (!readVector3Df(p + 2, newline_pos, buffer))
            {
                std::cerr << "Line: '" << line_n << "': ";
                std::cerr << "Cannot read vertex" << std::endl;
                return false;
            }

            out_vertices.push_back(buffer);
        }
        else if (d > 3 && p[0] == 'v' && p[1] == 't' && p[2] == ' ') // UV
        {
            if (count(p, newline_pos, ' ') != 2)
            {
                std::cerr << "Line: '" << line_n << "': ";
                std::cerr << "UV entry doesn't match syntax" << std::endl;
                return false;
            }

            Vector2Df buffer;
            if (!readVector2Df(p + 3, newline_pos, buffer))
            {
                std::cerr << "Line: '" << line_n << "': ";
                std::cerr << "Cannot read UV" << std::endl;
                return false;
            }

            out_uvs.push_back(buffer);
        }
        else if (d > 3 && p[0] == 'v' && p[1] == 'n' && p[2] == ' ') // normal
        {
            if (count(p, newline_pos, ' ') != 3)
            {
                std::cerr << "Line: '" << line_n << "': ";
                std::cerr << "Normal entry doesn't match syntax" << std::endl;
                return false;
            }

            Vector3Df buffer;
            if (!readVector3Df(p + 3, newline_pos, buffer))
            {
                std::cerr << "Line: '" << line_n << "': ";
                std::cerr << "Cannot read normal" << std::endl;
                return false;
            }

            out_normals.push_back(buffer);
        }
        else if (d > 2 && p[0] == 'f' && p[1] == ' ') // face
        {
            FaceInfo buffer;
            if (!readFaceInfo(p + 2, newline_pos, buffer))
            {
                std::cerr << "Line: '" << line_n << "': ";
                std::cerr << "Cannot read face" << std::endl;
                return false;
            }

            out_faces.push_back(buffer);
        }

        p = newline_pos + 1;
        line_n++;
    }

    return true;
}

void writeNMDLFile(const std::string& file_name, const std::map<EFaceType, PolygonsData>& polygons)
{
    std::ofstream output_file(file_name, std::ios::binary);

    if (!output_file.is_open())
    {
        throw std::logic_error("Cannot open or create output file");
    }

    for (EFaceType face : {EFaceType::eTriangle, EFaceType::eQuad})
    {
        if (!polygons.contains(face))
        {
            uint32_t out_size = 0;
            output_file.write((const char*)&out_size, sizeof(uint32_t));
            continue;
        }

        PolygonsData data = polygons.at(face);

        uint32_t out_size = data.vertices.size();
        output_file.write((const char*)&out_size, sizeof(uint32_t));

        for (Vector3Df vertex : data.vertices)
        {
            static uint32_t buffer[3];
            memset(buffer, 0, sizeof(buffer));

            buffer[0] = floatToV16(vertex.x);
            buffer[1] = floatToV16(vertex.y);
            buffer[2] = floatToV16(vertex.z);

            output_file.write((const char*)buffer, sizeof(buffer));
        }
    }

    output_file.close();
}

std::vector<char> readObjFile(const std::string& file_name)
{
    std::vector<char> file_data;

    std::ifstream input_file(file_name, std::ios::in);

    if (!input_file.is_open())
    {
        throw std::logic_error("Cannot open input file");
    }

    input_file.seekg(0, std::ios::end);

    uint32_t size = input_file.tellg();
    file_data.resize(size);

    input_file.seekg(0, std::ios::beg);
    input_file.read(file_data.data(), size);
    input_file.close();

    return file_data;
}

std::string toString(const FaceInfo& face)
{
    std::stringstream s_stream;

    s_stream << "Vertex indecies: '";
    for (uint32_t index : face.vertex)
    {
        s_stream << index <<  " ";
    }
    s_stream << "';";

    s_stream << "Normal indecies: '";
    for (uint32_t index : face.normal)
    {
        s_stream << index << " ";
    }
    s_stream << "';";

    s_stream << "UV indecies: '";
    for (uint32_t index : face.uv)
    {
        s_stream << index << " ";
    }
    s_stream << "';";

    return s_stream.str();
}

std::map<EFaceType, PolygonsData> processFaces(const std::vector<Vector3Df>& vertices, const std::vector<Vector2Df>& uvs,
                                               const std::vector<Vector3Df>& normals, const std::vector<FaceInfo>& faces)
{
    std::map<EFaceType, PolygonsData> result;

    auto determineFaceType = [](const std::vector<uint32_t>& vertices)
    {
        switch (vertices.size())
        {
        case 3: return EFaceType::eTriangle;
        case 4: return EFaceType::eQuad;
        }

        throw std::logic_error("Face with '" + std::to_string(vertices.size()) + "' vertices is unsupported");
    };

    for (const FaceInfo& face : faces)
    {
        if (face.normal.size() != face.uv.size() || face.normal.size() != face.vertex.size())
        {
            throw std::logic_error("Sizes of face vertices, normals and UVs are different. Face: " + toString(face));
        }

        EFaceType face_type = determineFaceType(face.vertex);

        if (!result.contains(face_type))
        {
            result.emplace(face_type, PolygonsData{});
        }
        
        PolygonsData& data = result.at(face_type);

        for (uint32_t index : face.vertex)
        {
            if (index == 0 || index > vertices.size())
                throw std::logic_error("Invalid vertex index '" + std::to_string(index) + "'. Face: " + toString(face));
            
            // -1 'cause .obj enumerates from 1
            data.vertices.push_back(vertices[index - 1]);
        }

        for (uint32_t index: face.uv)
        {
            if (index == 0 || index > uvs.size())
                throw std::logic_error("Invalid UV index '" + std::to_string(index) + "'. Face: " + toString(face));

            data.uvs.push_back(uvs[index - 1]);
        }

        for (uint32_t index : face.normal)
        {
            if (index == 0 || index > normals.size())
                throw std::logic_error("Invalid normal index '" + std::to_string(index) + "'. Face: " + toString(face));

            data.normals.push_back(normals[index - 1]);
        }
    }

    return result;
}

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " input.obj output.nmdl\n";
        exit(1);
    }

    std::string input_file_path(argv[1]);
    std::cout << "Reading file: '" << input_file_path << "'" << std::endl;

    std::vector<char> file_data;
    try
    {
        file_data = readObjFile(input_file_path);
    }
    catch (const std::logic_error& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    std::cout << "File lenght: " << file_data.size() << std::endl;

    // Parse data
    std::vector<Vector3Df> parsed_vertices;
    std::vector<Vector2Df> parsed_uvs;
    std::vector<Vector3Df> parsed_normals;
    std::vector<FaceInfo> parsed_faces;

    std::cout << "Parsing..." << std::endl;

    if (!parseObjFile(file_data.data(), file_data.size(), parsed_vertices, parsed_uvs, parsed_normals, parsed_faces))
    {
        std::cerr << "Failed to parse" << std::endl;
        return 1;
    }

    std::cout << "Got:" << std::endl;
    std::cout << "Vertices: '" << parsed_vertices.size() << "'" << std::endl;
    std::cout << "UVs: '" << parsed_uvs.size() << "'" << std::endl;
    std::cout << "Normals: '" << parsed_normals.size() << "'" << std::endl;
    std::cout << "Faces: '" << parsed_faces.size() << "'" << std::endl;

    std::cout << "Processing faces..." << std::endl;
    std::map<EFaceType, PolygonsData> polygons;

    try
    {
        polygons = processFaces(parsed_vertices, parsed_uvs, parsed_normals, parsed_faces);
    }
    catch (const std::logic_error& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    for (auto [face, data] : polygons)
    {
        std::cout << "Polygon type:" << toString(face) << std::endl;
        std::cout << "Vertices: '" << data.vertices.size() << "'" << std::endl;
        std::cout << "UVs: '" << data.uvs.size() << "'" << std::endl;
        std::cout << "Normals: '" << data.normals.size() << "'" << std::endl;
    }

    std::string output_file_path(argv[2]);
    std::cout << "Writing to file '" << output_file_path << "'" << std::endl;

    try
    {
        writeNMDLFile(output_file_path, polygons);
    }
    catch (const std::logic_error& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    std::cout << "Done!" << std::endl;

    return 0;
}
