#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"

#include <string>
#include <vector>
#include <any>

namespace Agos {

	typedef enum AG_API AGSFileSectionType
	{
		TYPE,
		NAME,
		VERTICES,
		VERTICES_NORMALS,
		VERTEX_TEXTURES
	};

	typedef struct AG_API AGSFileSection
	{
		Agos::AGSFileSectionType m_Type;
		std::vector<std::any> m_Data;
	};

}