#include "mainwindow.h"
#include <QDir>
#include <QHeaderView>
#include <QDirIterator>
#include <QFileInfo>
#include <QRegularExpression>
#include <algorithm>
#include <filesystem>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "utils.h"
#include <QDebug>
#include "config.h"
#include "cu2generator.h"
#include "uiconfig.h"
#include "preferencesdialog.h"

namespace fs = std::filesystem;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Definir tamanho inicial da janela
    resize(800, 600);
    setMinimumSize(800, 600);
    
    Config::getInstance().load();
    setupUI();
    setupMenus();
    loadFromDatabase();

    // Restaurar última pasta usada
    if (!Config::getInstance().getLastDirectory().isEmpty()) {
        loadGames(Config::getInstance().getLastDirectory());
    }
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI() {
    setStyleSheet(UIConfig::MAIN_STYLE);
    
    // Widget central
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // SD Root section
    auto *sdRootGroup = new QGroupBox("SD Root", this);
    auto *sdRootLayout = new QHBoxLayout(sdRootGroup);  // Definido o parent diretamente aqui
    
    selectDirButton = new QPushButton("Browse", this);
    searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText("Pesquisar jogos...");
    
    sdRootLayout->addWidget(selectDirButton);
    sdRootLayout->addWidget(searchBox, 1);
    
    mainLayout->addWidget(sdRootGroup);

    // Files section
    gameTable = new QTableWidget(this);
    gameTable->setColumnCount(5);
    gameTable->setHorizontalHeaderLabels({"ID", "Nome", "Disco", "BIN Files", "Status"});
    
    // Configurar larguras das colunas
    gameTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);  // ID
    gameTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch); // Nome
    gameTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);  // Disco
    gameTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);  // BIN Files
    gameTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);  // Status
    
    gameTable->setColumnWidth(0, 75);   // ID
    gameTable->setColumnWidth(2, 60);   // Disco
    gameTable->setColumnWidth(3, 80);   // BIN Files
    gameTable->setColumnWidth(4, 150);  // Status
    
    mainLayout->addWidget(gameTable);

    // Tools section
    auto *toolsGroup = new QGroupBox("Tools", this);
    auto *toolsLayout = new QGridLayout(toolsGroup);  // Definido o parent diretamente aqui
    
    mergeBinFilesCheck = new QCheckBox("Merge Bin Files", this);
    cu2ForAllCheck = new QCheckBox("CU2 For All", this);
    fixInvalidNameCheck = new QCheckBox("Fix Invalid Name", this);
    autoRenameCheck = new QCheckBox("Auto Rename", this);
    addCoverArtCheck = new QCheckBox("Add Cover Art", this);
    createMultiDiscCheck = new QCheckBox("Create Multi-Disc", this);
    
    toolsLayout->addWidget(mergeBinFilesCheck, 0, 0);
    toolsLayout->addWidget(cu2ForAllCheck, 1, 0);
    toolsLayout->addWidget(fixInvalidNameCheck, 0, 1);
    toolsLayout->addWidget(autoRenameCheck, 1, 1);
    toolsLayout->addWidget(addCoverArtCheck, 0, 2);
    toolsLayout->addWidget(createMultiDiscCheck, 1, 2);
    
    mainLayout->addWidget(toolsGroup);

    // Progress section
    auto *progressGroup = new QGroupBox("Progress", this);
    auto *progressLayout = new QVBoxLayout(progressGroup);  // Definido o parent diretamente aqui
    
    progressBar = new QProgressBar(this);
    progressBar->setTextVisible(true);
    progressBar->setAlignment(Qt::AlignCenter);
    
    statusLabel = new QLabel("Status: Ready", this);
    processButton = new QPushButton("Process", this);
    processButton->setEnabled(false);
    
    progressLayout->addWidget(progressBar);
    progressLayout->addWidget(statusLabel);
    progressLayout->addWidget(processButton);
    
    mainLayout->addWidget(progressGroup);

    // Conectar sinais
    connect(selectDirButton, &QPushButton::clicked, this, &MainWindow::onSelectDirectory);
    connect(searchBox, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    connect(processButton, &QPushButton::clicked, this, &MainWindow::onProcessGames);
    connect(gameTable, &QTableWidget::cellClicked, this, &MainWindow::onGameSelected);
}

void MainWindow::setupMenus() {
    // File Menu
    fileMenu = menuBar()->addMenu("&Arquivo");
    fileMenu->addAction("Selecionar Diretório", this, &MainWindow::onSelectDirectory);
    fileMenu->addSeparator();
    fileMenu->addAction("Exportar Database", this, &MainWindow::onExportDatabase);
    fileMenu->addAction("Importar Database", this, &MainWindow::onImportDatabase);
    fileMenu->addSeparator();
    fileMenu->addAction("Sair", this, &QWidget::close);

    // Tools Menu
    toolsMenu = menuBar()->addMenu("&Ferramentas");
    toolsMenu->addAction("Processar Jogos", this, &MainWindow::onProcessGames);
    toolsMenu->addAction("Verificar Arquivos", this, &MainWindow::onVerifyFiles);
    toolsMenu->addAction("Corrigir Arquivos CUE", this, &MainWindow::onFixCueFiles);
    toolsMenu->addAction("Criar Backup", this, &MainWindow::onCreateBackup);
    toolsMenu->addSeparator();
    toolsMenu->addAction("Preferências", this, &MainWindow::onPreferences);

    // Help Menu
    helpMenu = menuBar()->addMenu("&Ajuda");
    helpMenu->addAction("Sobre", this, &MainWindow::onAbout);
}

std::string MainWindow::extractGameId(const QString& binPath) {
    static const QStringList REGION_CODES = {
        "DTLS", "SCES", "SLES", "SLED", "SCED", "SCUS", "SLUS",
        "SLPS", "SCAJ", "SLKA", "SLPM", "SCPS", "SCPM", "PCPX",
        "PAPX", "PTPX", "LSP0", "LSP1", "LSP2", "LSP9", "SIPS",
        "ESPM", "SCZS", "SPUS", "PBPX", "LSP"
    };

    QFile file(binPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Não foi possível abrir o arquivo BIN:" << binPath;
        return "";
    }

    // Ler os primeiros 64KB do arquivo em blocos
    char buffer[4096];
    QByteArray content;
    qint64 totalRead = 0;
    while (totalRead < 65536) {  // 64KB
        qint64 bytesRead = file.read(buffer, sizeof(buffer));
        if (bytesRead <= 0) break;
        content.append(buffer, bytesRead);
        totalRead += bytesRead;
    }
    file.close();

    // Procurar por códigos de região no conteúdo binário
    for (const QString& code : REGION_CODES) {
        QByteArray searchCode = code.toLatin1() + "_";  // Adiciona underscore para busca
        int index = content.indexOf(searchCode);
        if (index != -1) {
            // Extrair o código e o número
            QByteArray idBytes = content.mid(index, 11);
            QString fullId = QString::fromLatin1(idBytes.constData(), idBytes.length()).trimmed();
            
            // Substituir underscore por hífen
            fullId.replace("_", "-");
            
            qDebug() << "ID encontrado:" << fullId << "para" << binPath;
            return fullId.toStdString();
        }
    }

    qDebug() << "Nenhum ID encontrado para:" << binPath;
    return "";
}

void MainWindow::onSelectDirectory() {
    QString dir = QFileDialog::getExistingDirectory(this,
        "Selecionar Diretório", 
        QString(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty()) return;

    // Limpar lista atual
    games.clear();
    gameTable->setRowCount(0);
    
    // Configurar progresso para scan
    progressBar->setMaximum(0);
    progressBar->setValue(0);
    processButton->setEnabled(false);
    statusLabel->setText("Escaneando diretório...");
    
    QApplication::processEvents();

    // Encontrar todos os diretórios que contêm arquivos BIN ou CU2
    QDirIterator it(dir, QStringList() << "*.bin" << "*.cu2", QDir::Files, QDirIterator::Subdirectories);
    QSet<QString> gameDirs;
    
    while (it.hasNext()) {
        QString filePath = it.next();
        gameDirs.insert(QFileInfo(filePath).dir().absolutePath());
    }
    
    // Configurar progresso
    progressBar->setMaximum(gameDirs.size());
    progressBar->setValue(0);
    
    // Processar cada diretório encontrado
    int count = 0;
    for (const QString& gamePath : gameDirs) {
        QDir gameDir(gamePath);
        statusLabel->setText(QString("Processando: %1").arg(gameDir.dirName()));
        
        try {
            // Verificar se tem capa
            bool hasCoverArt = QFileInfo::exists(gameDir.filePath("cover.jpg")) ||
                              QFileInfo::exists(gameDir.filePath("cover.png")) ||
                              QFileInfo::exists(gameDir.filePath("cover.bmp"));
            
            // Verificar se tem CU2
            bool hasCu2 = QFileInfo::exists(gameDir.filePath(gameDir.dirName() + ".cu2"));
            
            // Encontrar arquivo CUE (se existir)
            QStringList cueFiles = gameDir.entryList({"*.cue"}, QDir::Files);
            CueSheet cueSheet;
            
            if (!cueFiles.isEmpty()) {
                QString cuePath = gameDir.filePath(cueFiles.first());
                cueSheet = CueSheet(
                    cueFiles.first().toStdString(),
                    cuePath.toStdString(),
                    gameDir.dirName().toStdString()
                );
            }

            // Encontrar todos os arquivos BIN
            QStringList binFiles = gameDir.entryList({"*.bin"}, QDir::Files);
            
            // Extrair o ID do jogo do arquivo BIN principal ou qualquer BIN disponível
            QString binPath = gameDir.filePath(gameDir.dirName() + ".bin");
            if (!QFile::exists(binPath) && !binFiles.isEmpty()) {
                binPath = gameDir.filePath(binFiles.first());
            }
            
            std::string gameId = extractGameId(binPath);
            if (gameId.empty()) {
                qDebug() << "Aviso: Não foi possível extrair ID do jogo:" << gameDir.dirName();
                gameId = "UNKNOWN";
            }
            
            // Criar objeto Game
            Game game(
                gameDir.dirName().toStdString(),
                gameDir.absolutePath().toStdString(),
                gameId,
                1,  // TODO: detectar número do disco
                std::vector<std::string>(),
                cueSheet,
                hasCoverArt,
                hasCu2
            );
            
            games.push_back(game);
        }
        catch (const std::exception& e) {
            qDebug() << "Erro ao processar" << gameDir.dirName() << ":" << e.what();
        }
        
        progressBar->setValue(++count);
        QApplication::processEvents();
    }

    // Atualizar interface
    updateGameList();
    processButton->setEnabled(!games.empty());
    progressBar->setValue(0);
    statusLabel->setText(QString("Jogos encontrados: %1").arg(games.size()));
}

void MainWindow::loadGames(const QString &directory) {
    games.clear();
    QDir dir(directory);
    QStringList gameDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &gameDir : gameDirs) {
        QDir fullGameDir(dir.filePath(gameDir));
        QStringList cueFiles = fullGameDir.entryList({"*.cue"}, QDir::Files);
        
        if (cueFiles.isEmpty()) continue;

        // Verificar arquivos necessários
        bool hasCoverArt = fullGameDir.exists("cover.jpg") || fullGameDir.exists("cover.png");
        bool hasCu2 = fullGameDir.exists(gameDir + ".cu2");
        
        // Extrair ID do jogo (assumindo formato SLUS-XXXXX)
        QRegularExpression idRegex("([A-Z]{4}-\\d{5})");
        auto match = idRegex.match(gameDir);
        QString gameId = match.hasMatch() ? match.captured(1) : "";

        // Encontrar arquivos BIN relacionados
        QStringList binFiles = fullGameDir.entryList({"*.bin"}, QDir::Files);
        
        // Criar CueSheet para o jogo
        CueSheet cueSheet(cueFiles[0].toStdString(),
                         fullGameDir.filePath(cueFiles[0]).toStdString(),
                         gameDir.toStdString());

        // Adicionar arquivos BIN ao CueSheet
        for (const QString &binFile : binFiles) {
            cueSheet.addBinFile(BinFile(
                binFile.toStdString(),
                fullGameDir.filePath(binFile).toStdString()
            ));
        }

        // Criar objeto Game
        games.emplace_back(
            gameDir.toStdString(),
            fullGameDir.path().toStdString(),
            gameId.toStdString(),
            1, // Disco número (será atualizado depois)
            std::vector<std::string>(), // Coleção de discos (será atualizada depois)
            cueSheet,
            hasCoverArt,
            hasCu2
        );
    }

    updateGameList();
    statusLabel->setText(QString("Jogos encontrados: %1").arg(games.size()));
}

void MainWindow::updateGameList() {
    gameTable->setRowCount(0);
    
    for (const Game &game : games) {
        int row = gameTable->rowCount();
        gameTable->insertRow(row);

        // ID do jogo
        auto* idItem = new QTableWidgetItem(QString::fromStdString(game.getId()));
        idItem->setTextAlignment(Qt::AlignCenter);
        gameTable->setItem(row, 0, idItem);
        
        // Nome do jogo
        auto* nameItem = new QTableWidgetItem(QString::fromStdString(game.getDirectoryName()));
        nameItem->setToolTip(QString::fromStdString(game.getDirectoryPath()));
        gameTable->setItem(row, 1, nameItem);
        
        // Número do disco
        auto* discItem = new QTableWidgetItem(QString::number(game.getDiscNumber()));
        discItem->setTextAlignment(Qt::AlignCenter);
        gameTable->setItem(row, 2, discItem);

        // Número de arquivos BIN
        QDir gameDir(QString::fromStdString(game.getDirectoryPath()));
        QStringList binFiles = gameDir.entryList({"*.bin"}, QDir::Files);
        auto* binCountItem = new QTableWidgetItem(QString::number(binFiles.size()));
        binCountItem->setTextAlignment(Qt::AlignCenter);
        gameTable->setItem(row, 3, binCountItem);
        
        // Status com cores
        QStringList statusList;
        QColor bgColor = UIConfig::ERROR_COLOR;
        
        if (!game.hasCoverArt()) {
            statusList << "Sem capa";
        }
        
        if (!game.hasCu2()) {
            statusList << "Sem CU2";
        }
        
        // Verificar nome inválido (você precisa implementar esta lógica)
        if (!isValidGameName(game.getDirectoryName())) {
            statusList << "Nome inválido";
        }
        
        QString status = statusList.isEmpty() ? "OK" : statusList.join(", ");
        
        if (statusList.isEmpty()) {
            bgColor = UIConfig::SUCCESS_COLOR;
        }
        
        auto* statusItem = new QTableWidgetItem(status);
        statusItem->setTextAlignment(Qt::AlignCenter);
        statusItem->setBackground(bgColor);
        statusItem->setForeground(Qt::white);
        gameTable->setItem(row, 4, statusItem);
    }

    statusLabel->setText(QString("Jogos encontrados: %1").arg(games.size()));
}

bool MainWindow::isValidGameName(const std::string& name) {
    QString gameName = QString::fromStdString(name);
    
    // Verificar caracteres especiais não permitidos
    static const QRegularExpression invalidChars("[\\\\/:*?\"<>|]");
    if (gameName.contains(invalidChars)) {
        return false;
    }
    
    // Verificar se começa ou termina com espaço
    if (gameName.startsWith(' ') || gameName.endsWith(' ')) {
        return false;
    }
    
    // Verificar espaços duplos
    if (gameName.contains("  ")) {
        return false;
    }
    
    return true;
}

void MainWindow::onGameSelected(int row, int column) {
    if (row >= 0 && row < static_cast<int>(games.size())) {
        const Game &selectedGame = games[row];
        QString details = QString("Jogo: %1\n"
                                "ID: %2\n"
                                "Caminho: %3\n"
                                "Arquivos BIN: %4\n"
                                "CUE: %5")
                             .arg(QString::fromStdString(selectedGame.getDirectoryName()))
                             .arg(QString::fromStdString(selectedGame.getId()))
                             .arg(QString::fromStdString(selectedGame.getDirectoryPath()))
                             .arg(selectedGame.getCueSheet().getBinFileCount())
                             .arg(QString::fromStdString(selectedGame.getCueSheet().getFileName()));

        QMessageBox::information(this, "Detalhes do Jogo", details);
    }
}

void MainWindow::onSearchTextChanged(const QString &text) {
    for (int row = 0; row < gameTable->rowCount(); ++row) {
        bool match = false;
        for (int col = 0; col < gameTable->columnCount(); ++col) {
            QTableWidgetItem *item = gameTable->item(row, col);
            if (item && item->text().contains(text, Qt::CaseInsensitive)) {
                match = true;
                break;
            }
        }
        gameTable->setRowHidden(row, !match);
    }
}

void MainWindow::onProcessGames() {
    if (games.empty()) {
        QMessageBox::warning(this, "Aviso", "Nenhum jogo para processar!");
        return;
    }

    // Desabilitar botão durante o processamento
    processButton->setEnabled(false);
    
    // Configurar barra de progresso
    progressBar->setMaximum(games.size());
    progressBar->setValue(0);
    
    int processed = 0;
    int errors = 0;
    QStringList errorList;

    Database db;  // Instância do banco de dados

    for (size_t i = 0; i < games.size(); ++i) {
        Game& game = games[i];
        
        // Atualizar barra de progresso
        progressBar->setValue(i + 1);
        statusLabel->setText(QString("Processando: %1").arg(
            QString::fromStdString(game.getDirectoryName())));

        try {
            // Add Cover Art
            if (addCoverArtCheck->isChecked() && !game.hasCoverArt()) {
                statusLabel->setText("Baixando capa...");
                
                // Buscar capa no banco de dados
                QString gameId = QString::fromStdString(game.getId()).replace("-", "_");
                QByteArray coverData = db.getCoverArt(gameId);
                
                if (!coverData.isEmpty()) {
                    // Criar arquivo BMP
                    QString coverPath = QString::fromStdString(game.getDirectoryPath()) + 
                                     "/" + QString::fromStdString(game.getDirectoryName()) + ".bmp";
                    
                    QFile coverFile(coverPath);
                    if (coverFile.open(QIODevice::WriteOnly)) {
                        coverFile.write(coverData);
                        coverFile.close();
                        game.setCoverArt(true);
                    } else {
                        throw std::runtime_error("Não foi possível salvar a capa");
                    }
                } else {
                    throw std::runtime_error("Capa não encontrada no banco de dados");
                }
            }

            // Merge Bin Files
            if (mergeBinFilesCheck->isChecked()) {
                // Implementar lógica de merge
                statusLabel->setText("Mesclando arquivos BIN...");
                // mergeBinFiles(game);
            }

            // Fix Invalid Name
            if (fixInvalidNameCheck->isChecked()) {
                statusLabel->setText("Corrigindo nome...");
                if (!fixCueFile(game)) {
                    throw std::runtime_error("Erro ao corrigir arquivo CUE");
                }
            }

            // CU2 For All
            if (cu2ForAllCheck->isChecked()) {
                statusLabel->setText("Gerando arquivo CU2...");
                // generateCu2File(game);
            }

            // Auto Rename
            if (autoRenameCheck->isChecked()) {
                statusLabel->setText("Renomeando arquivos...");
                // autoRenameFiles(game);
            }

            // Create Multi-Disc
            if (createMultiDiscCheck->isChecked()) {
                statusLabel->setText("Configurando multi-disco...");
                // setupMultiDisc(game);
            }

            processed++;
        }
        catch (const std::exception& e) {
            errors++;
            errorList.append(QString("%1: %2")
                .arg(QString::fromStdString(game.getDirectoryName()))
                .arg(e.what()));
        }

        QApplication::processEvents();
    }

    // Atualizar a interface
    updateGameList();

    // Restaurar estado original
    processButton->setEnabled(true);
    progressBar->setValue(0);
    statusLabel->setText(QString("Jogos encontrados: %1").arg(games.size()));
    
    // Mostrar resultados
    QString message = QString("Processamento concluído!\n\n"
                            "Jogos processados: %1\n"
                            "Erros encontrados: %2").arg(processed).arg(errors);
    
    if (!errorList.isEmpty()) {
        message += "\n\nErros:\n" + errorList.join("\n");
    }
    
    QMessageBox::information(this, "Resultado", message);
}

bool MainWindow::fixCueFile(Game& game) {
    QString cuePath = QString::fromStdString(game.getCueSheet().getFilePath());
    
    // Fazer backup se necessário
    if (Config::getInstance().createBackups) {
        QString backupPath = cuePath + ".backup";
        if (QFile::exists(backupPath)) {
            QFile::remove(backupPath);
        }
        if (!QFile::copy(cuePath, backupPath)) {
            return false;
        }
    }

    // Ler entradas do arquivo CUE
    auto entries = Utils::parseCueFile(cuePath);
    
    // Se não houver entradas válidas, criar novas
    if (entries.empty()) {
        QDir gameDir(QString::fromStdString(game.getDirectoryPath()));
        QStringList binFiles = gameDir.entryList({"*.bin"}, QDir::Files);
        
        for (const QString& binFile : binFiles) {
            Utils::CueEntry entry;
            entry.type = "FILE";
            entry.file = binFile.toStdString();
            entry.trackNumber = entries.empty() ? 1 : entries.back().trackNumber + 1;
            entry.trackType = "BINARY";
            entries.push_back(entry);
        }
    }

    // Gerar novo conteúdo do arquivo CUE
    QString newContent = Utils::generateCueFile(entries);
    
    // Salvar novo arquivo CUE
    QFile file(cuePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out << newContent;
    file.close();

    return true;
}

void MainWindow::onExportDatabase() {
    QString fileName = QFileDialog::getSaveFileName(this,
        "Exportar Database", "", "JSON Files (*.json)");
    
    if (fileName.isEmpty()) return;

    QJsonArray gamesArray;
    for (const auto &game : games) {
        QJsonObject gameObj;
        gameObj["id"] = QString::fromStdString(game.getId());
        gameObj["directory_name"] = QString::fromStdString(game.getDirectoryName());
        gameObj["directory_path"] = QString::fromStdString(game.getDirectoryPath());
        gameObj["disc_number"] = game.getDiscNumber();
        gameObj["has_cover_art"] = game.hasCoverArt();
        gameObj["has_cu2"] = game.hasCu2();

        gamesArray.append(gameObj);
    }

    QJsonDocument doc(gamesArray);
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        QMessageBox::information(this, "Sucesso", "Database exportada com sucesso!");
    } else {
        QMessageBox::critical(this, "Erro", "Erro ao exportar database!");
    }
}

void MainWindow::onImportDatabase() {
    QString fileName = QFileDialog::getOpenFileName(this,
        "Importar Database", "", "JSON Files (*.json)");
    
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Erro", "Erro ao abrir arquivo!");
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (doc.isArray()) {
        games.clear();
        QJsonArray gamesArray = doc.array();
        
        for (const auto &gameValue : gamesArray) {
            QJsonObject gameObj = gameValue.toObject();
            
            // Criar CueSheet vazio (será atualizado depois)
            CueSheet cueSheet("", "", "");
            
            Game game(
                gameObj["directory_name"].toString().toStdString(),
                gameObj["directory_path"].toString().toStdString(),
                gameObj["id"].toString().toStdString(),
                gameObj["disc_number"].toInt(),
                std::vector<std::string>(),
                cueSheet,
                gameObj["has_cover_art"].toBool(),
                gameObj["has_cu2"].toBool()
            );
            
            games.push_back(game);
        }
        
        updateGameList();
        saveToDatabase();
        QMessageBox::information(this, "Sucesso", "Database importada com sucesso!");
    }
}

void MainWindow::onVerifyFiles() {
    if (games.empty()) {
        QMessageBox::warning(this, "Aviso", "Nenhum jogo para verificar!");
        return;
    }

    progressBar->setMaximum(games.size());
    progressBar->setValue(0);
    processButton->setEnabled(false);
    
    int errors = 0;
    QStringList errorList;

    for (size_t i = 0; i < games.size(); ++i) {
        const Game& game = games[i];
        statusLabel->setText(QString("Verificando: %1").arg(
            QString::fromStdString(game.getDirectoryName())));

        if (!verifyGameFiles(game)) {
            errors++;
            errorList.append(QString::fromStdString(game.getDirectoryName()));
        }

        progressBar->setValue(i + 1);
        QApplication::processEvents();
    }

    processButton->setEnabled(true);
    progressBar->setValue(0);
    statusLabel->setText(QString("Jogos encontrados: %1").arg(games.size()));

    if (errors > 0) {
        QMessageBox::warning(this, "Resultado", 
            QString("Verificação concluída com %1 erro(s):\n\n%2")
                .arg(errors)
                .arg(errorList.join("\n")));
    } else {
        QMessageBox::information(this, "Resultado", "Todos os arquivos estão OK!");
    }
}

bool MainWindow::verifyGameFiles(const Game &game) {
    QDir gameDir(QString::fromStdString(game.getDirectoryPath()));
    
    // Verificar arquivo CUE
    QString cuePath = gameDir.filePath(QString::fromStdString(game.getCueSheet().getFileName()));
    if (!QFile::exists(cuePath))
        return false;
        
    // Verificar arquivos BIN
    for (const auto &binFile : game.getCueSheet().getBinFiles()) {
        QString binPath = gameDir.filePath(QString::fromStdString(binFile.getFileName()));
        if (!QFile::exists(binPath))
            return false;
    }
    
    // Verificar cover art
    if (game.hasCoverArt() && 
        !QFile::exists(gameDir.filePath("cover.jpg")) &&
        !QFile::exists(gameDir.filePath("cover.png")))
        return false;
        
    // Verificar CU2
    if (game.hasCu2() &&
        !QFile::exists(gameDir.filePath(QString::fromStdString(game.getDirectoryName() + ".cu2"))))
        return false;
        
    return true;
}

void MainWindow::onCreateBackup() {
    if (games.empty()) {
        QMessageBox::warning(this, "Aviso", "Nenhum jogo para backup!");
        return;
    }

    progressBar->setMaximum(games.size());
    progressBar->setValue(0);
    processButton->setEnabled(false);

    int processed = 0;
    for (size_t i = 0; i < games.size(); ++i) {
        const Game& game = games[i];
        statusLabel->setText(QString("Backup: %1").arg(
            QString::fromStdString(game.getDirectoryName())));

        // Seu código de backup aqui
        processed++;

        progressBar->setValue(i + 1);
        QApplication::processEvents();
    }

    processButton->setEnabled(true);
    progressBar->setValue(0);
    statusLabel->setText(QString("Jogos encontrados: %1").arg(games.size()));

    QMessageBox::information(this, "Resultado", 
        QString("Backup concluído! %1 jogos processados.").arg(processed));
}

void MainWindow::onAbout() {
    QMessageBox::about(this, "Sobre PSIO Assistant",
        "PSIO Assistant v1.0\n\n"
        "Uma ferramenta para gerenciar jogos de PlayStation para PSIO.\n\n"
        "Desenvolvido com Qt 6");
}

void MainWindow::onFixCueFiles() {
    if (games.empty()) {
        QMessageBox::warning(this, "Aviso", "Nenhum jogo para processar!");
        return;
    }

    progressBar->setMaximum(games.size());
    progressBar->setValue(0);
    processButton->setEnabled(false);

    int fixed = 0;
    for (size_t i = 0; i < games.size(); ++i) {
        Game& game = games[i];
        statusLabel->setText(QString("Verificando CUE: %1").arg(
            QString::fromStdString(game.getDirectoryName())));

        if (fixCueFile(game)) {
            fixed++;
        }

        progressBar->setValue(i + 1);
        QApplication::processEvents();
    }

    processButton->setEnabled(true);
    progressBar->setValue(0);
    statusLabel->setText(QString("Jogos encontrados: %1").arg(games.size()));

    QMessageBox::information(this, "Resultado", 
        QString("%1 arquivo(s) CUE corrigido(s)").arg(fixed));
}

void MainWindow::saveToDatabase() {
    for (const auto& game : games) {
        if (!db.updateGame(game)) {
            if (!db.addGame(game)) {
                qDebug() << "Erro ao salvar jogo:" << QString::fromStdString(game.getId());
            }
        }
    }
}

void MainWindow::loadFromDatabase() {
    games = db.getAllGames();
    updateGameList();
    statusLabel->setText(QString("Jogos carregados: %1").arg(games.size()));
}

void MainWindow::contextMenuEvent(QContextMenuEvent* event) {
    QModelIndex index = gameTable->indexAt(gameTable->viewport()->mapFrom(this, event->pos()));
    
    if (index.isValid()) {
        QMenu contextMenu(this);
        
        QAction* processAction = contextMenu.addAction("Processar Jogo");
        QAction* verifyAction = contextMenu.addAction("Verificar Arquivos");
        QAction* openFolderAction = contextMenu.addAction("Abrir Pasta");
        QAction* removeAction = contextMenu.addAction("Remover");
        
        QAction* selectedAction = contextMenu.exec(event->globalPos());
        
        if (selectedAction) {
            const Game& game = games[index.row()];
            
            if (selectedAction == processAction) {
                processGame(game);
            }
            else if (selectedAction == verifyAction) {
                verifyGameFiles(game);
            }
            else if (selectedAction == openFolderAction) {
                QDesktopServices::openUrl(QUrl::fromLocalFile(
                    QString::fromStdString(game.getDirectoryPath())));
            }
            else if (selectedAction == removeAction) {
                if (QMessageBox::question(this, "Confirmar Remoção",
                    "Deseja remover este jogo da lista?") == QMessageBox::Yes) {
                    db.removeGame(game.getId());
                    games.erase(games.begin() + index.row());
                    updateGameList();
                }
            }
        }
    }
}

void MainWindow::processGame(const Game& game) {
    QString gameName = QString::fromStdString(game.getDirectoryName());
    
    progressBar->setMaximum(3);
    progressBar->setValue(0);
    processButton->setEnabled(false);
    
    statusLabel->setText(QString("Processando: %1").arg(gameName));
    
    // Processar etapas do jogo
    // ... seu código de processamento aqui ...
    
    for (int i = 0; i < 3; i++) {
        progressBar->setValue(i + 1);
        QApplication::processEvents();
    }
    
    processButton->setEnabled(true);
    progressBar->setValue(0);
    statusLabel->setText(QString("Jogos encontrados: %1").arg(games.size()));
}

void MainWindow::onPreferences() {
    PreferencesDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        // Recarregar configurações se necessário
        updateGameList();
    }
} 