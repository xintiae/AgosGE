#include "Agos/agos.h"
#include "Agos/src/file_format/ags_file_model.h"

#include <iostream>
#include <string>

int main() {
	Agos::ag_init_loggers();

	std::string in;
	std::string out;

	std::cout << "Type the path of the file which should be converted to an AGS file" << std::endl;
	std::cin >> in;

	std::cout << "Type the path of the output for the AGS file" << std::endl;
	std::cin >> out;

	Agos::AGSModelFile::generate_model_file(in, out);

	std::cout << "File saved in " << out << "!" << std::endl;
	system("pause");
}