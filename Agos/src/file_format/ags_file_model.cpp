#include "ags_file_model.h"

Agos::AgModel Agos::AGSModelFile::get_model(const std::string& id, int modelIndex)
{
	AgModel model;

	

	return AgModel();
}

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
	AGSFileSection version = AGSFileSection<AGSFileSectionDataTypeInt>{ VERSION, std::vector<std::shared_ptr<AGSFileSectionDataTypeInt>>() };
	version.add_data(AG_AGS_MODEL_VERSION);

	AGSModelSections current_obj;

	// Loop through every line of the file
	while (std::getline(in_stream, line)) {

		if (line.find("o ") != std::string::npos) {
			// Prints the last object to the file
			out_stream << current_obj.serialize_sections();

			current_obj = AGSModelSections();
			current_obj.object_name = line.substr(2);
		}

		// Populate data for the vertices
		if (line.find("v ") != std::string::npos) {
			current_obj.vertices.add_data(line.substr(2));
		}

		if (line.find("vt") != std::string::npos) {
			current_obj.vertices_textures.add_data(line.substr(3));
		}

		if (line.find("vn") != std::string::npos) {
			current_obj.vertices_normals.add_data(line.substr(3));
		}

		if (line.find("f ") != std::string::npos) {
			current_obj.faces.add_data(line.substr(2));
		}

	}
	
	out_stream << current_obj.serialize_sections();

	if (!out_stream) {
		AG_CORE_ERROR("Could not write to file " + output);
		return;
	}

	in_stream.close();
}

std::string Agos::AGSModelSections::serialize_sections()
{
	std::string out;

	out += object_name;
	out += vertices.serialize_section();
	out += vertices_textures.serialize_section();
	out += vertices_normals.serialize_section();
	out += faces.serialize_section();

	// Wrap around curled brackets if string isn't empty
	if (out.size() != 0) {
		out.insert(0, "{\n");
		out += "}\n";
	}

	return out;
}

void Agos::AGSModelFile::read_file()
{
	// Clears file data vector to avoid duplicates if this function would be called twice
	if (!m_FileData.empty()) {
		m_FileData.clear();
	}

	int modelNum = get_models_num();

	for (int i = 0; i < modelNum; i++) {
		AGSModelSections sections;

		AGSFileSection name_section = read_section<AGSFileSectionDataTypeString>(AGSFileSectionType::OBJECT_NAME, i);
		if (!name_section.m_Data.empty()) {
			sections.object_name = name_section.m_Data.at(0)->m_Data;
		}
		else {
			sections.object_name = AG_DEFAULT_MODEL_NAME;
		}

		sections.vertices = read_section<AGSFileSectionDataTypeVector3>(AGSFileSectionType::VERTICES, i);
		sections.vertices_textures = read_section<AGSFileSectionDataTypeVector2>(AGSFileSectionType::VERTICES_TEXTURES, i);
		sections.vertices_normals = read_section<AGSFileSectionDataTypeVector3>(AGSFileSectionType::VERTICES_NORMALS, i);

		m_FileData.push_back(std::make_shared<AGSModelSections>(sections));
	}
}

int Agos::AGSModelFile::get_models_num()
{
	int modelNum = 0;

	std::string line;

	// Loop through every line of the file
	while (std::getline(m_Stream, line)) {
		if (line == "{\n") {
			modelNum++;
		}
	}

	return modelNum;
}
