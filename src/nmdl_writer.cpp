#include "nmdl_writer.h"

namespace {
    // Convert float to vertex 4.12 fixed format
    inline int16_t floatToV16(float value) { return (int16_t)((value) * (1 << 12)); }

    inline uint32_t pack2xV16Into32Bit(int16_t value_1, int16_t value_2) {
        return (uint32_t)(((value_1) & 0xFFFF) | ((value_2) << 16));
    }
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
