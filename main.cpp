#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "interfaccia/mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Setup applicazione base
    app.setApplicationName("Biblioteca Manager");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("Library Systems");
    
    // Imposta stile moderno se disponibile
    QStringList availableStyles = QStyleFactory::keys();
    if (availableStyles.contains("Fusion")) {
        app.setStyle(QStyleFactory::create("Fusion"));
    }
    
    // Setup directory di lavoro
    QString appDir = QApplication::applicationDirPath();
    QDir::setCurrent(appDir);
    
    // CSS BASE MINIMO - EMBEDDED
    QString cssContent = R"(
/* CSS BASE MINIMALE - Biblioteca Manager */

/* TUTTO BIANCO SU NERO */
QMainWindow, QWidget {
    background-color: black;
    color: white;
}

/* CONTAINER */
QGroupBox {
    border: 1px solid gray;
    margin-top: 10px;
    padding-top: 5px;
}

QGroupBox::title {
    color: white;
    padding: 2px;
}

/* FORM CONTROLS */
QLineEdit, QTextEdit, QSpinBox, QComboBox, QDateEdit {
    border: 1px solid gray;
    background-color: black;
    color: white;
    padding: 5px;
}

QLineEdit:focus, QTextEdit:focus, QSpinBox:focus, QComboBox:focus, QDateEdit:focus {
    border: 1px solid white;
}

/* BUTTONS */
QPushButton {
    background-color: gray;
    border: 1px solid white;
    color: white;
    padding: 5px 10px;
}

QPushButton:hover {
    background-color: white;
    color: black;
}

QPushButton:disabled {
    background-color: darkgray;
    color: gray;
}

/* TOOLBAR */
QToolBar {
    background-color: black;
    border: 1px solid gray;
}

QToolButton {
    background-color: black;
    color: white;
    border: none;
    padding: 5px;
}

QToolButton:hover {
    background-color: gray;
}

/* LISTS */
QListWidget {
    border: 1px solid gray;
    background-color: black;
    color: white;
}

QListWidget::item:selected {
    background-color: gray;
}

/* SCROLLBARS */
QScrollBar {
    background-color: black;
}

QScrollBar::handle {
    background-color: gray;
}

/* MEDIA CARDS */
MediaCard {
    background-color: black;
    border: 1px solid gray;
    color: white;
}

MediaCard[selected="true"] {
    border: 2px solid white;
}

/* COMBO DROPDOWN */
QComboBox QAbstractItemView {
    background-color: black;
    color: white;
    border: 1px solid gray;
}

/* PROGRESS */
QProgressBar {
    border: 1px solid gray;
    background-color: black;
}

QProgressBar::chunk {
    background-color: white;
}

/* MENUS */
QMenuBar {
    background-color: black;
    color: white;
}

QMenu {
    background-color: black;
    color: white;
    border: 1px solid gray;
}

QMenu::item:selected {
    background-color: gray;
}
    )";
    
    // Applica gli stili CSS base embedded
    app.setStyleSheet(cssContent);
    qDebug() << "CSS Base applicato";
    
    try {
        // Crea e mostra finestra principale
        MainWindow window;
        
        // Imposta icona solo se disponibile
        QIcon appIcon(":/icons/app_icon.png");
        if (appIcon.isNull()) {
            // Fallback per icona
            appIcon = app.style()->standardIcon(QStyle::SP_ComputerIcon);
        }
        
        if (!appIcon.isNull()) {
            window.setWindowIcon(appIcon);
            app.setWindowIcon(appIcon);
        }
        
        window.show();
        return app.exec();
        
    } catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "Errore Critico", 
                             QString("Errore durante l'avvio: %1").arg(e.what()));
        return -1;
    }
}