#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"

#include "Agos/src/logger/logger.h"
#include "Agos/src/file_format/ags_file_section.h"

#include <iostream>
#include <fstream>

namespace Agos {

	typedef class AG_API AGSModelFile
	{
	private:

	public:

		void generate_model_file(const std::string& path, const std::string& output);

	};

}