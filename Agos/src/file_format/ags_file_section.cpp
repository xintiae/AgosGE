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

	for (auto i : m_Data) {
		out += i.convert_to_string() + "\n";
	}

	return out;
}

std::string Agos::AGSFileSectionDataType::convert_to_string()
{
	return std::string();
}

Agos::AGSFileSectionDataTypeString::AGSFileSectionDataTypeString(const std::string& data)
	: m_Data(data)
{

}

std::string Agos::AGSFileSectionDataTypeString::convert_to_string()
{
	return m_Data;
}

Agos::AGSFileSectionDataTypeInt::AGSFileSectionDataTypeInt(const int& data)
	: m_Data(data)
{
}

std::string Agos::AGSFileSectionDataTypeInt::convert_to_string()
{
	return std::to_string(m_Data);
}
