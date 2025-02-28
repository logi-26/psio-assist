#ifndef CU2GENERATOR_H
#define CU2GENERATOR_H

#include <QString>
#include <QFile>
#include <QDataStream>
#include "game.h"

class Cu2Generator {
public:
    static bool generateCu2File(const Game& game) {
        QString cu2Path = QString::fromStdString(game.getDirectoryPath()) + "/" +
                         QString::fromStdString(game.getDirectoryName()) + ".cu2";

        QFile file(cu2Path);
        if (!file.open(QIODevice::WriteOnly)) {
            return false;
        }

        QDataStream stream(&file);
        stream.setByteOrder(QDataStream::LittleEndian);

        // Cabeçalho CU2
        stream << (quint32)0x32554323;  // Magic number "CU2"
        stream << (quint32)0x00000001;  // Version

        // Informações do jogo
        QString gameId = QString::fromStdString(game.getId());
        stream.writeRawData(gameId.toLatin1().constData(), 10);

        // Número de tracks
        quint32 numTracks = game.getCueSheet().getBinFileCount();
        stream << numTracks;

        // Informações das tracks
        for (const auto& binFile : game.getCueSheet().getBinFiles()) {
            QString binName = QString::fromStdString(binFile.getFileName());
            stream.writeRawData(binName.toLatin1().constData(), 32);
            stream << (quint32)0; // Offset
            stream << (quint32)0; // Length
        }

        file.close();
        return true;
    }
};

#endif // CU2GENERATOR_H 