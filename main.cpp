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
    
    // Carica CSS tema scuro - Prova prima dalle risorse, poi dal file system
    QString cssContent;
    QFile styleFile(":/styles/styles.css");
    
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        // Caricamento dalle risorse riuscito
        QTextStream stream(&styleFile);
        cssContent = stream.readAll();
        qDebug() << "CSS caricato dalle risorse Qt";
    } else {
        // Fallback: prova a caricare dal file system
        qWarning() << "Risorse Qt non disponibili, tento caricamento da file system";
        
        QFile fallbackStyleFile("styles/styles.css");
        if (fallbackStyleFile.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream stream(&fallbackStyleFile);
            cssContent = stream.readAll();
            qDebug() << "CSS caricato dal file system";
        } else {
            // Ultimo tentativo: CSS di base embedded
            qWarning() << "File CSS non trovato, uso stili di base";
            cssContent = R"(
                QMainWindow {
                    background-color: #121212;
                    color: #E1E1E1;
                }
                QWidget {
                    background-color: #121212;
                    color: #E1E1E1;
                }
                QGroupBox {
                    border: 2px solid #404040;
                    border-radius: 8px;
                    margin-top: 10px;
                    padding-top: 10px;
                    background-color: #1E1E1E;
                    color: #E1E1E1;
                }
                QGroupBox::title {
                    color: #BB86FC;
                    padding: 0 5px;
                    background-color: #121212;
                }
                QPushButton {
                    background-color: #BB86FC;
                    border: none;
                    border-radius: 6px;
                    color: #121212;
                    padding: 8px 16px;
                    font-weight: bold;
                }
                QPushButton:hover {
                    background-color: #D4B3FF;
                }
                QPushButton:disabled {
                    background-color: #404040;
                    color: #777777;
                }
                QLineEdit, QTextEdit, QSpinBox, QComboBox {
                    border: 2px solid #404040;
                    border-radius: 6px;
                    padding: 8px;
                    background-color: #2D2D2D;
                    color: #E1E1E1;
                }
                QLineEdit:focus, QTextEdit:focus, QSpinBox:focus, QComboBox:focus {
                    border-color: #BB86FC;
                }
                MediaCard {
                    background-color: #1E1E1E;
                    border: 2px solid #404040;
                    border-radius: 8px;
                    color: #E1E1E1;
                }
                MediaCard[selected="true"] {
                    background-color: #2A1B3D;
                    border: 2px solid #BB86FC;
                }
                MediaCard[mediaType="libro"] { border-left: 4px solid #4CAF50; }
                MediaCard[mediaType="film"] { border-left: 4px solid #03DAC6; }
                MediaCard[mediaType="articolo"] { border-left: 4px solid #FF9800; }
            )";
        }
    }
    
    // Applica gli stili
    if (!cssContent.isEmpty()) {
        app.setStyleSheet(cssContent);
        qDebug() << "Stili CSS applicati con successo";
    }
    
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