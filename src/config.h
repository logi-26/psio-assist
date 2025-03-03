#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QSettings>

class Config {
public:
    static Config& getInstance() {
        static Config instance;
        return instance;
    }

    void load();
    void save();

    QString getLastDirectory() const { return lastDirectory; }
    void setLastDirectory(const QString& dir) { lastDirectory = dir; }

    QString backupDirectory;
    bool autoFixCue;
    bool createBackups;

private:
    Config() { load(); }  // Construtor privado (Singleton)
    ~Config() { save(); }
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    QString lastDirectory;
    QSettings settings{"psio-assist", "config"};
};

#endif // CONFIG_H 