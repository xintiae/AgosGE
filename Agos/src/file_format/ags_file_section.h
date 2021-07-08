#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"

#include "Agos/src/logger/logger.h"

#include <string>
#include <vector>
#include <any>

namespace Agos {

	typedef struct AG_API AGSFileSectionDataType
	{
		// Note: Do NOT append \n at the end!
		virtual std::string convert_to_string() = 0;
	};

	typedef struct AG_API AGSFileSectionDataTypeString : AGSFileSectionDataType
	{
		std::string m_Data;
		std::string convert_to_string() override;

		AGSFileSectionDataTypeString(const std::string& data);
	};

	typedef struct AG_API AGSFileSectionDataTypeInt : AGSFileSectionDataType
	{
		int m_Data;
		std::string convert_to_string() override;

		AGSFileSectionDataTypeInt(const int& data);
	};

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
		std::vector<std::shared_ptr<AGSFileSectionDataType>> m_Data;

		std::string serialize_section();
	};

}