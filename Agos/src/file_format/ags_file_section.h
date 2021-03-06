#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"

#include "Agos/src/logger/logger.h"

#include <type_traits>
#include <string>
#include <sstream>
#include <vector>
#include <any>

#include AG_GLM_STRING_CAST

namespace Agos {

	template <typename T>
	struct AG_API AGSFileSectionDataType
	{
	public:

		T m_Data;

		/** Defines how the data will be converted from a string. As a consequence, this method is called in the string constructor
		* @param data The incoming string which will be converted to the actual data of this entry
		*/
		virtual void set_data_from_string(const std::string& data) = 0;

		// Note: Do NOT append \n at the end!
		virtual std::string convert_to_string() = 0;

		AGSFileSectionDataType();
		AGSFileSectionDataType(const T& data);
	};

	struct AG_API AGSFileSectionDataTypeString : AGSFileSectionDataType<std::string>
	{
		void set_data_from_string(const std::string& data);
		std::string convert_to_string();

		AGSFileSectionDataTypeString();
		AGSFileSectionDataTypeString(const std::string& data);
	};

	struct AG_API AGSFileSectionDataTypeInt : AGSFileSectionDataType<int>
	{
		void set_data_from_string(const std::string& data);
		std::string convert_to_string();

		AGSFileSectionDataTypeInt();
		AGSFileSectionDataTypeInt(const int& data);
	};

	struct AG_API AGSFileSectionDataTypeVector2 : AGSFileSectionDataType<glm::vec2>
	{
		void set_data_from_string(const std::string& data);
		std::string convert_to_string();

		AGSFileSectionDataTypeVector2();
		AGSFileSectionDataTypeVector2(const glm::vec2 data);
		AGSFileSectionDataTypeVector2(const float& x, const float& y);
	};

	enum AG_API AGSFileSectionType
	{
		NONE,
		TYPE,
		NAME,
		VERSION,
		VERTICES,
		VERTICES_NORMALS,
		VERTICES_TEXTURES,
		FACES
	};

	template <typename T> // This class requires a subclass of AGSFileSectionDataType
	class AG_API AGSFileSection
	{
	private:

		inline static std::unordered_map<std::string, AGSFileSectionType> m_typeTable = {
			{"#type#", TYPE},
			{"#name#", NAME},
			{"#version#", VERSION},
			{"#vertices#", VERTICES},
			{"#vertices_normals#", VERTICES_NORMALS},
			{"#vertices_textures#", VERTICES_TEXTURES},
			{"#faces#", FACES}
		};
		
	public:

		Agos::AGSFileSectionType m_Type;

		std::vector<std::shared_ptr<T>> m_Data;

		/** Adds a new data entry to the m_Data vector
		* @param data The new data 'entry' under this section
		*/
		void add_data(const T& data) { m_Data.push_back(std::make_shared<T>(data)); };

		/** Converts the data of this section into its string representation, so that it can be printed out to a file
		* @return A string representing how the section can be printed out to a file
		*/
		std::string serialize_section()
		{
			std::string out;

			// Start with the name of the section wrapped in hashtags
			out += section_type_to_string(m_Type) + "\n";

			for (auto const& i : m_Data) {
				out += i.get()->convert_to_string() + "\n";
			}

			return out;
		};

		/** Gets the type of a section by its string representation
		* @param input The type of the section wrapped in hashtags
		* @return The AGSFileSectionType of the input
		*/
		static AGSFileSectionType get_section_type(const std::string& input)
		{
			auto it = m_typeTable.find(input);
			if (it != m_typeTable.end()) {
				return it->second;
			}

			return AGSFileSectionType::NONE;
		};

		/** Converts an AGSFileSectionType to its respective string wrapped in hashtags
		* @param type The type of the AGSFileSectionType
		* @return The string representation of the AGSFileSectionType
		*/
		static std::string section_type_to_string(AGSFileSectionType type)
		{
			for (auto it = m_typeTable.begin(); it != m_typeTable.end(); ++it) {
				if (it->second == type) {
					return it->first;
				}
			}

			return "#none#";
		};

	};

}
