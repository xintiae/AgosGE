#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"

#include "Agos/src/logger/logger.h"

#include <string>
#include <vector>
#include <any>

namespace Agos {

	typedef enum AG_API AGSFileSectionType
	{
		TYPE,
		NAME,
		VERSION,
		VERTICES,
		VERTICES_NORMALS,
		VERTICES_TEXTURES,
		FACES
	};

	typedef struct AG_API AGSFileSection
	{
		Agos::AGSFileSectionType m_Type;
		std::vector<std::any> m_Data;

		std::string serialize_section();
		void write_to_file(std::ostream &stream);
	};

}