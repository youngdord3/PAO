#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QMessageBox>
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
    
    // Carica CSS
    QFile cssFile(":/styles/styles/styles.css");
    if (cssFile.open(QIODevice::ReadOnly)) {
        QTextStream stream(&cssFile);
        QString cssContent = stream.readAll();
        app.setStyleSheet(cssContent);
        qDebug() << "CSS caricato da resources.qrc";
    } else {
        qWarning() << "Impossibile caricare il CSS da resources.qrc";
    }
    
    try {
        // Crea e configura la finestra principale
        MainWindow window;
        
        QIcon appIcon(":/icons/app_icon.png");
        if (!appIcon.isNull()) {
            window.setWindowIcon(appIcon);
            app.setWindowIcon(appIcon);
        }
        
        window.show();
        
        qDebug() << "Biblioteca Manager Avviato";
        
        return app.exec();
        
    } catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "Errore Critico", 
                             QString("Errore durante l'avvio: %1").arg(e.what()));
        return -1;
    }
}