#include "binfile.h"

BinFile::BinFile(const std::string& fileName, const std::string& filePath)
    : fileName(fileName)
    , filePath(filePath)
{
}

std::string BinFile::getFileName() const {
    return fileName;
}

std::string BinFile::getFilePath() const {
    return filePath;
}

void BinFile::setFileName(const std::string& fileName) {
    this->fileName = fileName;
}

void BinFile::setFilePath(const std::string& filePath) {
    this->filePath = filePath;
} 