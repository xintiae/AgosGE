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

	class AG_API AGSModelFile
	{
	private:

		std::ifstream m_Stream;

	public:

		static void generate_model_file(const std::string& path, const std::string& output);
		
		template <typename T>
		AGSFileSection<T> read_model_file(AGSFileSectionType type)
		{
			bool isInSection = false;
			std::string line;
			AGSFileSection<T> section{ type };

			// Iterate through every line of the file
			while (std::getline(m_Stream, line)) {

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

				if (isInSection) {
					
					T data;
					data.set_data_from_string(line);

					section.add_data(data);
				}

			}

			return section;
		};

		AGSModelFile(const std::string& path);
		~AGSModelFile();

	};

}