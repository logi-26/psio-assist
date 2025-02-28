#include "cuesheet.h"

CueSheet::CueSheet(const std::string& fileName, const std::string& filePath, const std::string& gameName)
    : fileName(fileName)
    , filePath(filePath)
    , gameName(gameName)
{
}

std::string CueSheet::getFileName() const {
    return fileName;
}

std::string CueSheet::getFilePath() const {
    return filePath;
}

std::string CueSheet::getGameName() const {
    return gameName;
}

const std::vector<BinFile>& CueSheet::getBinFiles() const {
    return binFiles;
}

void CueSheet::setFileName(const std::string& fileName) {
    this->fileName = fileName;
}

void CueSheet::setFilePath(const std::string& filePath) {
    this->filePath = filePath;
}

void CueSheet::setGameName(const std::string& gameName) {
    this->gameName = gameName;
}

void CueSheet::addBinFile(const BinFile& binFile) {
    binFiles.push_back(binFile);
}

size_t CueSheet::getBinFileCount() const {
    return binFiles.size();
}

void CueSheet::clearBinFiles() {
    binFiles.clear();
} 