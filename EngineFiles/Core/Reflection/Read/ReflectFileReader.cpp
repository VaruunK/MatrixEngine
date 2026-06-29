#include <filesystem>
#include <fstream>

static int FindMarkedFile(std::filesystem::path& filePath) {
	std::ofstream file;
	file.open(filePath.string());

	file.close();
	return 0;
}