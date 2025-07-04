# Configurazione base del progetto Qt
QT += core widgets

# Standard C++ moderno richiesto per smart pointers e altre funzionalità
CONFIG += c++17

# Nome dell'eseguibile finale
TARGET = BibliotecaManager

# Tipo di progetto: applicazione eseguibile
TEMPLATE = app

# Definizione delle cartelle per gli include
INCLUDEPATH += modello_logico \
               interfaccia \
               json

# File sorgente
SOURCES += main.cpp \
           modello_logico/media.cpp \
           modello_logico/libro.cpp \
           modello_logico/film.cpp \
           modello_logico/articolo.cpp \
           modello_logico/collezione.cpp \
           modello_logico/filtrostrategy.cpp \
           interfaccia/mainwindow.cpp \
           interfaccia/mediacard.cpp \
           interfaccia/mediafactory.cpp \
           interfaccia/mediadialog.cpp \
           json/jsonmanager.cpp

# File header
HEADERS += modello_logico/media.h \
           modello_logico/libro.h \
           modello_logico/film.h \
           modello_logico/articolo.h \
           modello_logico/collezione.h \
           modello_logico/filtrostrategy.h \
           interfaccia/mainwindow.h \
           interfaccia/mediacard.h \
           interfaccia/mediafactory.h \
           interfaccia/mediadialog.h \
           json/jsonmanager.h

# File di risorse - ABILITATO
RESOURCES += resources.qrc

# Abilitare warning per API Qt deprecate
DEFINES += QT_DEPRECATED_WARNINGS

# Disabilitare API Qt precedenti a una certa versione
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000
