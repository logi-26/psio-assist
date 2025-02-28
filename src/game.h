#ifndef GAME_H
#define GAME_H

#include <string>
#include <vector>
#include "cuesheet.h"

class Game {
public:
    Game(const std::string& dirName, 
         const std::string& dirPath,
         const std::string& id,
         int discNum,
         const std::vector<std::string>& discCollection,
         const CueSheet& cue,
         bool hasCover,
         bool hasCu2File);
    ~Game() = default;

    // Getters - declarados apenas
    std::string getDirectoryName() const;
    std::string getDirectoryPath() const;
    std::string getId() const;
    int getDiscNumber() const;
    const std::vector<std::string>& getDiscCollection() const;
    const CueSheet& getCueSheet() const;
    bool hasCoverArt() const;
    bool hasCu2() const;

    // Setters - declarados apenas
    void setDirectoryName(const std::string& directoryName);
    void setDirectoryPath(const std::string& directoryPath);
    void setId(const std::string& id);
    void setDiscNumber(int num);
    void setDiscCollection(const std::vector<std::string>& collection);
    void setCueSheet(const CueSheet& cueSheet);
    void setCoverArt(bool has);
    void setCu2Present(bool present);

    // Adicionar ao cabeçalho da classe Game
    bool isRelatedDisc(const Game& other) const;
    std::string getBaseGameName() const;
    int extractDiscNumber() const;

private:
    std::string directoryName;
    std::string directoryPath;
    std::string gameId;
    int discNumber;
    std::vector<std::string> discCollection;
    CueSheet cueSheet;
    bool hasCoverArtFlag;
    bool hasCu2Flag;
};

#endif // GAME_H 