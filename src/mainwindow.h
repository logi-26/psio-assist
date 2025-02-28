#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressBar>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QShortcut>
#include <QGroupBox>
#include <QCheckBox>
#include <QHeaderView>
#include <QApplication>
#include <QContextMenuEvent>
#include <QDesktopServices>
#include <QRegularExpression>
#include <QSqlDatabase>
#include <QDirIterator>
#include "game.h"
#include "database.h"
#include "utils.h"
#include "preferencesdialog.h"
#include "uiconfig.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;

private slots:
    void onSelectDirectory();
    void onProcessGames();
    void onGameSelected(int row, int column);
    void onSearchTextChanged(const QString &text);
    void onExportDatabase();
    void onImportDatabase();
    void onAbout();
    void onVerifyFiles();
    void onFixCueFiles();
    void onCreateBackup();
    void onPreferences();

private:
    void setupUI();
    void setupMenus();
    void updateGameList();
    void loadGames(const QString &directory);
    void saveToDatabase();
    void loadFromDatabase();
    bool verifyGameFiles(const Game &game);
    void showGameDetails(const Game &game);
    bool fixCueFile(Game& game);
    void processGame(const Game& game);
    void generateCu2File(const Game& game);
    std::string extractGameId(const QString& binPath);
    bool isValidGameName(const std::string& name);
    bool isMultiDisc(const Game& game);
    void processMultiDisc(Game& game);
    void mergeBinFiles(Game& game);
    void fixGameName(Game& game);
    void autoRenameGame(Game& game);
    void setUiEnabled(bool enabled);

    // UI Elements
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QHBoxLayout *topLayout;
    QPushButton *selectDirButton;
    QPushButton *processButton;
    QPushButton *verifyButton;
    QLineEdit *searchBox;
    QTableWidget *gameTable;
    QLabel *statusLabel;
    QProgressBar *progressBar;

    // Menus
    QMenu *fileMenu;
    QMenu *toolsMenu;
    QMenu *helpMenu;

    // Data
    QString currentDirectory;
    std::vector<Game> games;
    Database db;

    // Checkboxes
    QCheckBox *mergeBinFilesCheck;
    QCheckBox *fixInvalidNameCheck;
    QCheckBox *addCoverArtCheck;
    QCheckBox *cu2ForAllCheck;
    QCheckBox *autoRenameCheck;
    QCheckBox *createMultiDiscCheck;
};

#endif // MAINWINDOW_H 