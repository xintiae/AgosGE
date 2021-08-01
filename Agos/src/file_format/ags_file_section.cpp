#include "ags_file_section.h"

template<typename T>
Agos::AGSFileSectionDataType<T>::AGSFileSectionDataType()
{
}

template<typename T>
Agos::AGSFileSectionDataType<T>::AGSFileSectionDataType(const T& data)
	: m_Data(data)
{
}

void Agos::AGSFileSectionDataTypeString::set_data_from_string(const std::string& data)
{
	m_Data = data;
}

std::string Agos::AGSFileSectionDataTypeString::convert_to_string()
{
	return m_Data;
}

Agos::AGSFileSectionDataTypeString::AGSFileSectionDataTypeString()
{
}

Agos::AGSFileSectionDataTypeString::AGSFileSectionDataTypeString(const std::string& data)
	: AGSFileSectionDataType(data)
{
}

void Agos::AGSFileSectionDataTypeInt::set_data_from_string(const std::string& data)
{
	m_Data = std::stoi(data);
}

std::string Agos::AGSFileSectionDataTypeInt::convert_to_string()
{
	return std::to_string(m_Data);
}

Agos::AGSFileSectionDataTypeInt::AGSFileSectionDataTypeInt()
{
}

Agos::AGSFileSectionDataTypeInt::AGSFileSectionDataTypeInt(const std::string& data)
{
	set_data_from_string(data);
}

Agos::AGSFileSectionDataTypeInt::AGSFileSectionDataTypeInt(const int& data)
	: AGSFileSectionDataType(data)
{
}

void Agos::AGSFileSectionDataTypeVector2::set_data_from_string(const std::string& data)
{
	std::stringstream stream;
	stream << data;

	std::string segment;
	std::vector<std::string> list;

	while (std::getline(stream, segment, ' ')) {
		list.push_back(segment);
	}

	m_Data = glm::vec2(std::stof(list[0]), std::stof(list[1]));
}

std::string Agos::AGSFileSectionDataTypeVector2::convert_to_string()
{
	return std::to_string(m_Data.x) + " " + std::to_string(m_Data.y);
}

Agos::AGSFileSectionDataTypeVector2::AGSFileSectionDataTypeVector2()
{
}

Agos::AGSFileSectionDataTypeVector2::AGSFileSectionDataTypeVector2(const std::string& data)
{
	set_data_from_string(data);
}

Agos::AGSFileSectionDataTypeVector2::AGSFileSectionDataTypeVector2(const glm::vec2& data)
	: AGSFileSectionDataType(data)
{
}

Agos::AGSFileSectionDataTypeVector2::AGSFileSectionDataTypeVector2(const float& x, const float& y)
	: AGSFileSectionDataType(glm::vec2(x, y))
{
}

void Agos::AGSFileSectionDataTypeVector3::set_data_from_string(const std::string& data)
{
	std::stringstream stream;
	stream << data;

	std::string segment;
	std::vector<std::string> list;

	while (std::getline(stream, segment, ' ')) {
		list.push_back(segment);
	}

	m_Data = glm::vec3(std::stof(list[0]), std::stof(list[1]), std::stof(list[2]));
}

std::string Agos::AGSFileSectionDataTypeVector3::convert_to_string()
{
	return std::to_string(m_Data.x) + " " + std::to_string(m_Data.y) + " " + std::to_string(m_Data.z);
}

Agos::AGSFileSectionDataTypeVector3::AGSFileSectionDataTypeVector3()
{
}

Agos::AGSFileSectionDataTypeVector3::AGSFileSectionDataTypeVector3(const std::string& data)
{
	set_data_from_string(data);
}

Agos::AGSFileSectionDataTypeVector3::AGSFileSectionDataTypeVector3(const glm::vec3& data)
	: AGSFileSectionDataType(data)
{
}

Agos::AGSFileSectionDataTypeVector3::AGSFileSectionDataTypeVector3(const float& x, const float& y, const float& z)
	: AGSFileSectionDataType(glm::vec3(x, y, z))
{
}
