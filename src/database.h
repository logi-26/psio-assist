#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <vector>
#include "game.h"
#include <QByteArray>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QObject>
#include <QImage>
#include <QBuffer>

class Database : public QObject {
    Q_OBJECT
public:
    Database();
    ~Database();
    
    bool init();
    bool addGame(const Game& game);
    bool updateGame(const Game& game);
    bool removeGame(const std::string& gameId);
    std::vector<Game> getAllGames();
    Game getGame(const std::string& gameId);
    
    QByteArray getCoverArt(const QString& gameId);
    bool saveGame(const Game& game);
    
    QByteArray downloadCoverArt(const QString& gameId);
    
    // Evitar cópia da instância singleton
    Database(const Database&) = delete;
    void operator=(const Database&) = delete;

private:
    QSqlDatabase db;
    static const QString DATABASE_NAME;
    
    bool createTables();
    bool openDatabase();
    bool initDatabase();
    
    QNetworkAccessManager networkManager;
};

#endif // DATABASE_H 