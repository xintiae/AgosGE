#include "ags_file_model.h"

void Agos::AGSModelFile::generate_model_file(const std::string& path, const std::string& output)
{
	AG_CORE_INFO("Loading AGS object: " + path);

	std::ifstream in_stream;
	std::ofstream out_stream;

	in_stream.open(path, std::ios::in);
	out_stream.open(output, std::ios::out);

	if (!in_stream) {
		AG_CORE_ERROR("Failed to open file " + path);
		return;
	}

	std::string line;

	// Add the current version of the AGS Model Format
	AGSFileSection version = AGSFileSection{ VERSION };
	version.m_Data.push_back(AGSFileSectionDataTypeInt(AG_AGS_MODEL_VERSION));

	AGSFileSection vertices = AGSFileSection{ VERTICES };
	AGSFileSection vertex_textures = AGSFileSection{ VERTICES_TEXTURES };
	AGSFileSection vertices_normals = AGSFileSection{ VERTICES_NORMALS };
	AGSFileSection faces = AGSFileSection{ FACES };

	// Loop through every line of the file
	while (std::getline(in_stream, line)) {

		// Populate data for the vertices
		if (line.find("v ") != std::string::npos) {
			vertices.m_Data.push_back(AGSFileSectionDataTypeString(line.substr(2)));
		}

		if (line.find("vt") != std::string::npos) {
			vertex_textures.m_Data.push_back(AGSFileSectionDataTypeString(line.substr(3)));
		}

		if (line.find("vn") != std::string::npos) {
			vertices_normals.m_Data.push_back(AGSFileSectionDataTypeString(line.substr(3)));
		}

		if (line.find("f") != std::string::npos) {
			faces.m_Data.push_back(AGSFileSectionDataTypeString(line.substr(2)));
		}

	}

	if (!out_stream) {
		AG_CORE_ERROR("Could not write to file " + output);
		return;
	}

	out_stream << version.serialize_section();
	out_stream << vertices.serialize_section();
	out_stream << vertex_textures.serialize_section();
	out_stream << vertices_normals.serialize_section();
	out_stream << faces.serialize_section();

	in_stream.close();
}
