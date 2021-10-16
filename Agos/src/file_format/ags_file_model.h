#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"

#include "Agos/src/logger/logger.h"
#include "Agos/src/file_format/ags_file_section.h"
#include "Agos/src/renderer/model.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>

namespace Agos {

	// A struct containing the different sections required for a model (vertices, vertex_normals, ...)
	typedef struct AG_API AGSModelSections : AGSCombinedSections
	{
		std::string object_name;
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

		// A vector containing all the collections (different models) of the file
		std::vector<std::shared_ptr<AGSModelSections>> m_FileData;

		/** @brief Reads the data from a specified section of the AGS File
		* @param type The section type it should read
		* @param collectionIndex From which collection it should read the section (0 for the first one, if a wrong index is specified, an empty section is returned)
		* @return The section inside the collection which has the same type (or an empty one if there is no section with that type)
		*/
		template <typename T>
		AGSFileSection<T> read_section(AGSFileSectionType type, int collectionIndex = 0)
		{
			bool isInSection = false;
			int currentCollection = -1;
			std::string line;
			AGSFileSection<T> section{ type };

			// Iterate through every line of the file
			while (std::getline(m_Stream, line)) {

				if (line == "{\n") {
					currentCollection++;
				}

				// Returns the current section if there are no collections in this file or if the correct one was found
				if (currentCollection == -1 || currentCollection == collectionIndex) {
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
		*/
		void read_file();

		/** @brief Gets the amount of models (collections) inside the file
		* @return The number of models found inside the file
		*/
		int get_models_num();

	public:

		/** @brief Generates an AGS Model file from a given OBJ-File
		* @param path The path to the OBJ file
		* @param output The output to where the new AGS file will be saved to
		*/
		static void generate_model_file(const std::string& path, const std::string& output);
		
		/** @brief Reads the contents of the provided file and creates a model out of it
		* @param id A custom id for this model. If none is provided, the name section will be used. If none can be found, the AG_DEFAULT_MODEL_NAME macro will be used
		* @param modelIndex Which model should be used in case there are more than one models in the file
		* @return An AgModel which uses the contents of the provided file. If the file is corrupted, it will return a standard instance
		*/
		AgModel get_model(const std::string& id=std::string(), int modelIndex = 0);

		AGSModelFile(const std::string& path);
		~AGSModelFile();
	};


}