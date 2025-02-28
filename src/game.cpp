#include "game.h"

Game::Game(const std::string& dirName, 
           const std::string& dirPath,
           const std::string& id,
           int discNum,
           const std::vector<std::string>& discCollection,
           const CueSheet& cue,
           bool hasCover,
           bool hasCu2File)
    : directoryName(dirName)
    , directoryPath(dirPath)
    , gameId(id)
    , discNumber(discNum)
    , discCollection(discCollection)
    , cueSheet(cue)
    , hasCoverArtFlag(hasCover)
    , hasCu2Flag(hasCu2File)
{
}

std::string Game::getDirectoryName() const {
    return directoryName;
}

std::string Game::getDirectoryPath() const {
    return directoryPath;
}

std::string Game::getId() const {
    return gameId;
}

int Game::getDiscNumber() const {
    return discNumber;
}

const std::vector<std::string>& Game::getDiscCollection() const {
    return discCollection;
}

const CueSheet& Game::getCueSheet() const {
    return cueSheet;
}

bool Game::hasCoverArt() const {
    return hasCoverArtFlag;
}

bool Game::hasCu2() const {
    return hasCu2Flag;
}

void Game::setDirectoryName(const std::string& directoryName) {
    this->directoryName = directoryName;
}

void Game::setDirectoryPath(const std::string& directoryPath) {
    this->directoryPath = directoryPath;
}

void Game::setId(const std::string& id) {
    gameId = id;
}

void Game::setDiscNumber(int num) {
    discNumber = num;
}

void Game::setDiscCollection(const std::vector<std::string>& collection) {
    discCollection = collection;
}

void Game::setCueSheet(const CueSheet& cueSheet) {
    this->cueSheet = cueSheet;
}

void Game::setCoverArt(bool has) {
    hasCoverArtFlag = has;
}

void Game::setCu2Present(bool present) {
    hasCu2Flag = present;
} 