#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include "interfaccia/mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Impostazione di uno stile moderno
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // RIMOSSA: Creazione automatica della cartella immagini
    // Le immagini sono ora gestite come risorse nei file .qrc
    
    // Creazione e visualizzazione della finestra principale
    MainWindow window;
    window.show();
    
    return app.exec();
}