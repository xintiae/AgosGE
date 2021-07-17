#include "ags_file_model.h"

Agos::AGSModelFile::AGSModelFile(const std::string& path)
{
	m_Stream.open(path, std::ios::in);

	if (std::filesystem::path(path).extension() != ".ags") {
		AG_CORE_WARN(path + " is not an .ags file. Are you sure the path is correct?");
	}

	if (!m_Stream) {
		AG_CORE_ERROR("Could not open file " + path);
	}
}

Agos::AGSModelFile::~AGSModelFile()
{
	if (m_Stream) {
		m_Stream.close();
	}
}

void Agos::AGSModelFile::generate_model_file(const std::string& path, const std::string& output)
{
	AG_CORE_INFO("Converting object " + path + " to AGS File " + output);

	std::ifstream in_stream;
	std::ofstream out_stream;

	in_stream.open(path, std::ios::in);
	out_stream.open(output, std::ios::out);

	if (!in_stream) {
		AG_CORE_ERROR("Failed to open file " + path);
		return;
	}

	std::string line;

	AGSFileSection file_type = AGSFileSection<AGSFileSectionDataTypeString>{ TYPE };
	file_type.add_data(std::string("model"));

	// Add the current version of the AGS Model Format
	AGSFileSection version = AGSFileSection<AGSFileSectionDataTypeInt>{ VERSION };
	version.add_data(AG_AGS_MODEL_VERSION);

	AGSFileSection vertices = AGSFileSection<AGSFileSectionDataTypeString>{ VERTICES };
	AGSFileSection vertex_textures = AGSFileSection<AGSFileSectionDataTypeString>{ VERTICES_TEXTURES };
	AGSFileSection vertices_normals = AGSFileSection<AGSFileSectionDataTypeString>{ VERTICES_NORMALS };
	AGSFileSection faces = AGSFileSection<AGSFileSectionDataTypeString>{ FACES };

	// Loop through every line of the file
	while (std::getline(in_stream, line)) {

		// Populate data for the vertices
		if (line.find("v ") != std::string::npos) {
			vertices.add_data(line.substr(2));
		}

		if (line.find("vt") != std::string::npos) {
			vertex_textures.add_data(line.substr(3));
		}

		if (line.find("vn") != std::string::npos) {
			vertices_normals.add_data(line.substr(3));
		}

		if (line.find("f") != std::string::npos) {
			faces.add_data(line.substr(2));
		}

	}

	if (!out_stream) {
		AG_CORE_ERROR("Could not write to file " + output);
		return;
	}

	out_stream << file_type.serialize_section();
	out_stream << version.serialize_section();
	out_stream << vertices.serialize_section();
	out_stream << vertex_textures.serialize_section();
	out_stream << vertices_normals.serialize_section();
	out_stream << faces.serialize_section();

	in_stream.close();
}
