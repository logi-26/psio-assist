#ifndef UICONFIG_H
#define UICONFIG_H

#include <QColor>
#include <QString>

namespace UIConfig {
    // Cores
    const QColor BACKGROUND_COLOR(45, 45, 45);
    const QColor TEXT_COLOR(200, 200, 200);
    const QColor HEADER_COLOR(35, 35, 35);
    const QColor BUTTON_COLOR(70, 130, 180);
    const QColor BUTTON_HOVER_COLOR(100, 149, 237);
    const QColor TABLE_BASE_COLOR(45, 45, 45);
    const QColor TABLE_ALTERNATE_COLOR(40, 40, 40);
    const QColor SUCCESS_COLOR(40, 167, 69);
    const QColor ERROR_COLOR(220, 53, 69);
    
    // Estilos
    const QString MAIN_STYLE = R"(
        QMainWindow {
            background-color: #2d2d2d;
            color: #c8c8c8;
        }
        
        QMenuBar {
            background-color: #2d2d2d;
            color: #c8c8c8;
        }
        
        QMenuBar::item:selected {
            background-color: #3d3d3d;
        }
        
        QMenu {
            background-color: #2d2d2d;
            color: #c8c8c8;
            border: 1px solid #3d3d3d;
        }
        
        QMenu::item:selected {
            background-color: #3d3d3d;
        }
        
        QPushButton {
            background-color: #4682b4;
            color: white;
            border: none;
            padding: 5px 15px;
            border-radius: 2px;
            min-height: 25px;
        }
        
        QPushButton:hover {
            background-color: #6495ed;
        }
        
        QLineEdit {
            background-color: #3d3d3d;
            color: #c8c8c8;
            border: 1px solid #4d4d4d;
            padding: 5px;
            border-radius: 2px;
            min-height: 25px;
        }
        
        QTableWidget {
            background-color: #2d2d2d;
            color: #c8c8c8;
            gridline-color: #3d3d3d;
            border: 1px solid #3d3d3d;
        }
        
        QTableWidget::item {
            padding: 5px;
        }
        
        QTableWidget::item:selected {
            background-color: #4682b4;
        }
        
        QHeaderView::section {
            background-color: #3d3d3d;
            color: #c8c8c8;
            padding: 5px;
            border: none;
            border-right: 1px solid #4d4d4d;
        }
        
        QCheckBox {
            color: #c8c8c8;
        }
        
        QCheckBox::indicator {
            width: 15px;
            height: 15px;
        }
        
        QProgressBar {
            border: 1px solid #4d4d4d;
            border-radius: 2px;
            background-color: #3d3d3d;
            text-align: center;
            color: white;
        }
        
        QProgressBar::chunk {
            background-color: #4682b4;
        }
        
        QLabel {
            color: #c8c8c8;
        }
        
        QGroupBox {
            border: 1px solid #3d3d3d;
            margin-top: 0.5em;
            color: #c8c8c8;
        }
        
        QGroupBox::title {
            color: #c8c8c8;
        }
    )";
}

#endif // UICONFIG_H 