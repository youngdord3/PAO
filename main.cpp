#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include "interfaccia/mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Impostazione di uno stile moderno
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // Setup di percorsi e directory di lavoro
    QString appDir = QApplication::applicationDirPath();
    QDir::setCurrent(appDir);
    
    // Log delle informazioni di avvio
    qDebug() << "Avvio Biblioteca Manager";
    qDebug() << "Directory dell'applicazione:" << appDir;
    qDebug() << "Stile utilizzato:" << app.style()->objectName();
    
    try {
        // Creazione e visualizzazione della finestra principale
        MainWindow window;
        window.show();
        
        qDebug() << "Finestra principale creata e mostrata";
        
        // Avvio del ciclo degli eventi
        int result = app.exec();
        
        qDebug() << "Applicazione terminata con codice:" << result;
        return result;
        
    } catch (const std::exception& e) {
        qCritical() << "Errore critico durante l'avvio:" << e.what();
        return -1;
    } catch (...) {
        qCritical() << "Errore critico sconosciuto durante l'avvio";
        return -2;
    }
}