#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>
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
    
    // Carica CSS solo se esistente
    QString cssContent;
    QFile styleFile(":/styles/styles.css");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&styleFile);
        cssContent = stream.readAll();
        app.setStyleSheet(cssContent);
    } else {
        // Fallback CSS inline minimo
        cssContent = R"(
            QMainWindow { background-color: #f5f5f5; }
            QPushButton { 
                background-color: #2196f3; 
                color: white; 
                border: none; 
                padding: 8px 16px; 
                border-radius: 4px; 
            }
            QPushButton:hover { background-color: #1976d2; }
            QGroupBox { 
                font-weight: bold; 
                border: 2px solid #cccccc; 
                border-radius: 5px; 
                margin-top: 10px; 
            }
        )";
        app.setStyleSheet(cssContent);
    }
    
    try {
        // Crea e mostra finestra principale
        MainWindow window;
        
        // Imposta icona solo se disponibile
        QIcon appIcon(":/icons/app_icon.png");
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