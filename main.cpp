#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include "interfaccia/mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Impostazione di uno stile moderno
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // Setup di percorsi e directory di lavoro
    QString appDir = QApplication::applicationDirPath();
    QDir::setCurrent(appDir);
    
    // Carica gli stili CSS dalle risorse
    QFile styleFile(":/styles/styles.css");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&styleFile);
        QString styleSheet = stream.readAll();
        app.setStyleSheet(styleSheet);
    } else {
        qWarning() << "Impossibile caricare il file CSS dalle risorse";
        // Prova a caricare dal filesystem come fallback
        QFile fallbackFile("styles/styles.css");
        if (fallbackFile.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream stream(&fallbackFile);
            QString styleSheet = stream.readAll();
            app.setStyleSheet(styleSheet);
        } else {
            qWarning() << "Nessun file CSS trovato - usando stili di default";
        }
    }
    
    try {
        // Creazione e visualizzazione della finestra principale
        MainWindow window;
        
        // Imposta l'icona dell'applicazione se disponibile
        QIcon appIcon(":/icons/app_icon.png");
        if (!appIcon.isNull()) {
            window.setWindowIcon(appIcon);
            app.setWindowIcon(appIcon);
        }
        
        window.show();
        
        // Avvio del ciclo degli eventi
        int result = app.exec();
        
        return result;
        
    } catch (const std::exception& e) {
        qCritical() << "Errore critico durante l'avvio:" << e.what();
        
        // Mostra un messaggio di errore all'utente
        QMessageBox::critical(nullptr, "Errore Critico", 
                             QString("Si è verificato un errore critico:\n%1").arg(e.what()));
        
        return -1;
    } catch (...) {
        qCritical() << "Errore critico sconosciuto durante l'avvio";
        
        QMessageBox::critical(nullptr, "Errore Critico", 
                             "Si è verificato un errore critico sconosciuto durante l'avvio dell'applicazione.");
        
        return -2;
    }
}