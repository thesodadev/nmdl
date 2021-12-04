#include "obj_reader.h"
#include "nmdl_writer.h"

std::string toString(EFaceType face)
{
    switch (face)
    {
    case EFaceType::eTriangle: return "Triangle";
    case EFaceType::eQuad: return "Quad";
    }

    return "error";
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

std::vector<char> readFile(const std::string& file_name) {
    std::ifstream input_file(file_name, std::ios::in);

    if (!input_file.is_open()) {
        throw std::logic_error("Cannot open input file");
    }

    input_file.seekg(0, std::ios::end);

    uint32_t size = input_file.tellg();
    
    std::vector<char> file_data;
    file_data.resize(size);

    input_file.seekg(0, std::ios::beg);
    input_file.read(file_data.data(), size);
    input_file.close();

    return file_data;
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
        file_data = readFile(input_file_path);
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
