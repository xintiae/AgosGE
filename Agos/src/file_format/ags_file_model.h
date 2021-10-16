#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"

#include "Agos/src/logger/logger.h"
#include "Agos/src/file_format/ags_file_section.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>

namespace Agos {

	// A struct containing the different sections required for a model (vertices, vertex_normals, ...)
	typedef struct AG_API AGSModelSections : AGSCombinedSections
	{
		AGSFileSection<AGSFileSectionDataTypeString> object_name = AGSFileSection<AGSFileSectionDataTypeString>{ OBJECT_NAME };
		AGSFileSection<AGSFileSectionDataTypeVector3> vertices = AGSFileSection<AGSFileSectionDataTypeVector3>{ VERTICES };
		AGSFileSection<AGSFileSectionDataTypeVector2> vertices_textures = AGSFileSection<AGSFileSectionDataTypeVector2>{ VERTICES_TEXTURES };
		AGSFileSection<AGSFileSectionDataTypeVector3> vertices_normals = AGSFileSection<AGSFileSectionDataTypeVector3>{ VERTICES_NORMALS };
		AGSFileSection<AGSFileSectionDataTypeString> faces = AGSFileSection<AGSFileSectionDataTypeString>{ FACES };

		std::string serialize_sections();
	};

	typedef class AG_API AGSModelFile
	{
	private:

		std::ifstream m_Stream;

	public:

		/** @brief Generates an AGS Model file from a given OBJ-File
		* @param path The path to the OBJ file
		* @param output The output to where the new AGS file will be saved to
		*/
		static void generate_model_file(const std::string& path, const std::string& output);
		
		/** @brief Reads the data from a specified section of the AGS File
		* @param type The section type it should read
		* @param collectionIndex From which collection it should read the section (1 for the first one, if a wrong index is specified, the last collection's section is used)
		*/
		template <typename T>
		AGSFileSection<T> read_section(AGSFileSectionType type, int collectionIndex = 1)
		{
			bool isInSection = false;
			int currentCollection = 0;
			std::string line;
			AGSFileSection<T> section{ type };

			// Iterate through every line of the file
			while (std::getline(m_Stream, line)) {

				if (line == "{\n") {
					currentCollection++;
				}

				if (currentCollection >= collectionIndex) {
					if (AGSFileSection<T>::get_section_type(line) != AGSFileSectionType::NONE) {

						if (AGSFileSection<T>::get_section_type(line) == type) {
							isInSection = true;
							continue;
						}
						else {
							// Stops the loop as soon as the next section is reached
							break;
						}

					}
				}

				if (isInSection) {
					
					T data;
					data.set_data_from_string(line);

					section.add_data(data);
				}

			}

			return section;
		};

		/** @brief Reads an entire AGS model file
		* @return An instance of the wrapper class AGSModelSections, which will contain all the required sections for a model
		*/
		AGSModelSections read_file();

		AGSModelFile(const std::string& path);
		~AGSModelFile();

	};


}