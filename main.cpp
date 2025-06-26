#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include "interfaccia/mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Impostazione di uno stile moderno
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // Creazione delle cartelle necessarie se non esistono
    QDir dir;
    if (!dir.exists("immagini")) {
        dir.mkdir("immagini");
    }
    
    // Creazione e visualizzazione della finestra principale
    MainWindow window;
    window.show();
    
    return app.exec();
}