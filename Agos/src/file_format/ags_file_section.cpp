#include "ags_file_section.h"

std::string Agos::AGSFileSection::serialize_section()
{
	std::string out;

	// Start with the name of the section wrapped in hashtags
	switch (m_Type)
	{
	case TYPE:
		out += "#type#\n";
		break;
	case NAME:
		out += "#name#\n";
		break;
	case VERTICES:
		out += "#vertices#\n";
		break;
	case VERTICES_NORMALS:
		out += "#vertices_normal#\n";
		break;
	case VERTICES_TEXTURES:
		out += "#vertices_textures#\n";
		break;
	case FACES:
		out += "#faces#\n";
		break;
	}

	for (std::any i : m_Data) {
		
	}

	return out;
}

void Agos::AGSFileSection::write_to_file(std::ostream& stream)
{
	for (std::any i : m_Data) {
		stream << i;
	}
}
