#include "ags_file_section.h"

std::string Agos::AGSFileSection::serialize_section()
{
	std::string out;

	// Start with the name of the section wrapped in hashtags
	switch (m_Type)
	{
	case VERSION:
		out += "#version#\n";
		break;
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

	for (auto const& i : m_Data) {
		out += i.get()->convert_to_string() + "\n";
	}

	return out;
}

Agos::AGSFileSectionDataTypeString::AGSFileSectionDataTypeString(const std::string& data)
	: m_Data(data)
{
}

Agos::AGSFileSectionDataTypeInt::AGSFileSectionDataTypeInt(const int& data)
	: m_Data(data)
{
}
