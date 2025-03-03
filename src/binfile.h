#ifndef BINFILE_H
#define BINFILE_H

#include <string>

class BinFile {
public:
    BinFile(const std::string& fileName, const std::string& filePath)
        : fileName(fileName), filePath(filePath) {}

    std::string getFileName() const { return fileName; }
    std::string getFilePath() const { return filePath; }

private:
    std::string fileName;
    std::string filePath;
};

#endif // BINFILE_H 