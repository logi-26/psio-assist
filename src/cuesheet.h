#ifndef CUESHEET_H
#define CUESHEET_H

#include <string>
#include <vector>
#include "binfile.h"

class CueSheet {
public:
    CueSheet(const std::string& fileName, const std::string& filePath, const std::string& gameName);
    ~CueSheet() = default;

    // Getters
    std::string getFileName() const;
    std::string getFilePath() const;
    std::string getGameName() const;
    const std::vector<BinFile>& getBinFiles() const;

    // Setters
    void setFileName(const std::string& fileName);
    void setFilePath(const std::string& filePath);
    void setGameName(const std::string& gameName);

    // Bin file management
    void addBinFile(const BinFile& binFile);
    size_t getBinFileCount() const;
    void clearBinFiles();

private:
    std::string fileName;
    std::string filePath;
    std::string gameName;
    std::vector<BinFile> binFiles;
};

#endif // CUESHEET_H 