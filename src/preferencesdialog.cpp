#include "preferencesdialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QFileDialog>

PreferencesDialog::PreferencesDialog(QWidget *parent) 
    : QDialog(parent)
    , autoFixCueBox(nullptr)
    , createBackupsBox(nullptr)
    , backupDirEdit(nullptr)
    , backupDirButton(nullptr)
{
    setWindowTitle("Preferências");
    setupUI();
    loadSettings();
}

PreferencesDialog::~PreferencesDialog() = default;

void PreferencesDialog::saveSettings() {
    Config::getInstance().autoFixCue = autoFixCueBox->isChecked();
    Config::getInstance().createBackups = createBackupsBox->isChecked();
    Config::getInstance().backupDirectory = backupDirEdit->text();
    Config::getInstance().save();
    accept();
}

void PreferencesDialog::selectBackupDir() {
    QString dir = QFileDialog::getExistingDirectory(this, 
        "Selecionar Diretório de Backup",
        backupDirEdit->text());
    if (!dir.isEmpty()) {
        backupDirEdit->setText(dir);
    }
}

void PreferencesDialog::setupUI() {
    auto *layout = new QVBoxLayout(this);
    auto *formLayout = new QFormLayout();

    // Opções de correção automática
    autoFixCueBox = new QCheckBox("Corrigir arquivos CUE automaticamente", this);
    formLayout->addRow(autoFixCueBox);

    // Opções de backup
    createBackupsBox = new QCheckBox("Criar backups antes de modificar arquivos", this);
    formLayout->addRow(createBackupsBox);

    // Diretório de backup
    auto *backupDirLayout = new QHBoxLayout();
    backupDirEdit = new QLineEdit(this);
    backupDirButton = new QPushButton("Selecionar", this);
    backupDirLayout->addWidget(backupDirEdit);
    backupDirLayout->addWidget(backupDirButton);
    formLayout->addRow("Diretório de Backup:", backupDirLayout);

    layout->addLayout(formLayout);

    // Botões
    auto *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        Qt::Horizontal, this);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &PreferencesDialog::saveSettings);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(backupDirButton, &QPushButton::clicked, this, &PreferencesDialog::selectBackupDir);
}

void PreferencesDialog::loadSettings() {
    const auto &config = Config::getInstance();
    autoFixCueBox->setChecked(config.autoFixCue);
    createBackupsBox->setChecked(config.createBackups);
    backupDirEdit->setText(config.backupDirectory);
} 