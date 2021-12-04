#include "obj_reader.h"

#include <st/string.h>
#include <st/common.h>
#include <st/math.h>

namespace {

    inline bool readVector3Df(const char* str, const char* end, Vector3Df& out_vector)
    {
        const char* p = str;

        if (!ST::String::toFloat(p, end, out_vector.x))
        {
            std::cerr << "Cannot convert to float" << std::endl;
            return false;
        }

        const char* next_space = ST::Common::findNext(p, end, ' ');
        if (!next_space)
        {
            std::cerr << "Cannot find space delimeter" << std::endl;
            return false;
        }

        p = next_space + 1;

        if (!ST::String::toFloat(p, end, out_vector.y))
        {
            std::cerr << "Cannot convert to float" << std::endl;
            return false;
        }

        next_space = ST::Common::findNext(p, end, ' ');
        if (!next_space)
        {
            std::cerr << "Cannot find space delimeter" << std::endl;
            return false;
        }

        p = next_space + 1;

        if (!ST::String::toFloat(p, end, out_vector.z))
        {
            std::cerr << "Cannot convert to float" << std::endl;
            return false;
        }

        return true;
    }

    inline bool readVector2Df(const char* str, const char* end, Vector2Df& out_vector)
    {
        const char* p = str;

        if (!ST::String::toFloat(p, end, out_vector.x))
        {
            std::cerr << "Cannot convert to float" << std::endl;
            return false;
        }

        const char* next_space = ST::Common::findNext(p, end, ' ');
        if (!next_space)
        {
            std::cerr << "Cannot find space delimeter" << std::endl;
            return false;
        }

        p = next_space + 1;

        if (!ST::String::toFloat(p, end, out_vector.y))
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

            const char* next_slash = ST::Common::findNext(p, end, '/');
            if (!next_slash)
            {
                std::cerr << "Cannot find slash delimeter" << std::endl;
                return false;
            }

            p = next_slash + 1;

            out_entry.uv.push_back(atoi(p));
            
            next_slash = ST::Common::findNext(p, end, '/');
            if (!next_slash)
            {
                std::cerr << "Cannot find slash delimeter" << std::endl;
                return false;
            }
            p = next_slash + 1;

            out_entry.normal.push_back(atoi(p));

            const char* next_space = ST::Common::findNext(p, end, ' ');
            if (!next_space) break;
            p = next_space + 1;
        }

        return true;
    }
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
        const char* newline_pos = ST::Common::findNext(p, data_end, '\n');

        if (!newline_pos)
        {
            std::cerr << "Line: '" << line_n << "': ";
            std::cerr << "Cannot find newline" << std::endl;
            return false;
        }

        uint64_t d = ST::Common::distance(p, newline_pos);

        if (d > 2 && p[0] == 'v' && p[1] == ' ') // vertex
        {
            if (ST::Common::count(p, newline_pos, ' ') != 3)
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
            if (ST::Common::count(p, newline_pos, ' ') != 2)
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
            if (ST::Common::count(p, newline_pos, ' ') != 3)
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
