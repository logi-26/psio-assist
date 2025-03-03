#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

namespace Utils {

struct CueEntry {
    std::string type;
    std::string file;
    int trackNumber;
    std::string trackType;
};

inline std::vector<CueEntry> parseCueFile(const QString& filePath) {
    std::vector<CueEntry> entries;
    QFile file(filePath);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return entries;

    QTextStream in(&file);
    CueEntry currentEntry;
    currentEntry.trackNumber = -1;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        
        if (line.startsWith("FILE", Qt::CaseInsensitive)) {
            if (currentEntry.trackNumber != -1) {
                entries.push_back(currentEntry);
            }
            currentEntry = CueEntry();
            currentEntry.type = "FILE";
            
            // Extrair nome do arquivo entre aspas
            int start = line.indexOf('"') + 1;
            int end = line.lastIndexOf('"');
            if (start > 0 && end > start) {
                currentEntry.file = line.mid(start, end - start).toStdString();
            }
        }
        else if (line.startsWith("TRACK", Qt::CaseInsensitive)) {
            QStringList parts = line.split(' ', Qt::SkipEmptyParts);
            if (parts.size() >= 3) {
                currentEntry.trackNumber = parts[1].toInt();
                currentEntry.trackType = parts[2].toStdString();
            }
        }
    }

    if (currentEntry.trackNumber != -1) {
        entries.push_back(currentEntry);
    }

    file.close();
    return entries;
}

inline QString generateCueFile(const std::vector<CueEntry>& entries) {
    QString cueContent;
    QTextStream stream(&cueContent);

    for (const auto& entry : entries) {
        if (!entry.file.empty()) {
            stream << "FILE \"" << QString::fromStdString(entry.file) << "\" BINARY\n";
        }
        if (entry.trackNumber > 0) {
            stream << "  TRACK " << QString::number(entry.trackNumber).rightJustified(2, '0') 
                   << " " << QString::fromStdString(entry.trackType) << "\n";
            stream << "    INDEX 01 00:00:00\n";
        }
    }

    return cueContent;
}

inline bool isValidCueFile(const QString& filePath) {
    auto entries = parseCueFile(filePath);
    return !entries.empty() && 
           std::all_of(entries.begin(), entries.end(), 
                      [](const CueEntry& entry) { return !entry.file.empty(); });
}

inline QString sanitizeFileName(const QString& fileName) {
    QString result = fileName;
    result.replace(QRegularExpression("[\\\\/:*?\"<>|]"), "_");
    return result;
}

} // namespace Utils

#endif // UTILS_H 