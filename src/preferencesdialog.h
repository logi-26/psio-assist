#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>

class PreferencesDialog : public QDialog {
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget* parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle("Preferências");
        setupUI();
    }

    ~PreferencesDialog() = default;

private:
    void setupUI() {
        auto* layout = new QVBoxLayout(this);
        auto* formLayout = new QFormLayout;
        
        // Adicionar campos de preferências aqui quando necessário
        
        auto* buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        
        connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        
        layout->addLayout(formLayout);
        layout->addWidget(buttonBox);
    }
};

#endif // PREFERENCESDIALOG_H 