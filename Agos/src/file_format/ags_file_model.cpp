#include "ags_file_model.h"

void Agos::AGSModelFile::generate_model_file(const std::string& path, const std::string& output)
{
	std::ifstream in_stream;
	std::ofstream out_stream;

	in_stream.open(path, std::ios::in);
	out_stream.open(output, std::ios::out);

	if (!in_stream) {
		AG_CORE_ERROR("Failed to open file " + path);
		return;
	}

	std::string line;

	AGSFileSection vertices = AGSFileSection{};
	vertices.m_Type = Agos::AGSFileSectionType::VERTICES;
	
	// Loop through every line of the file
	while (std::getline(in_stream, line)) {

		// Populate data for the vertices
		if (line.find("v ") != std::string::npos) {
			vertices.m_Data.push_back(line.substr(2));
		}

	}

	in_stream.close();
}
