#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>
#include <QFile>
#include <QTextStream>
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
    
    // Carica CSS tema scuro - OBBLIGATORIO
    QFile styleFile(":/styles/styles.css");
    if (!styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::critical(nullptr, "Errore CSS", 
                             "File CSS tema scuro non trovato!\nVerificare styles/styles.css nelle risorse.");
        return -1;
    }
    
    QTextStream stream(&styleFile);
    QString cssContent = stream.readAll();
    app.setStyleSheet(cssContent);
    
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