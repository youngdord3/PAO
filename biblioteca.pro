# Configurazione base del progetto Qt
QT += core widgets

# Standard C++ moderno richiesto per smart pointers e altre funzionalità
CONFIG += c++17

# Nome dell'eseguibile finale
TARGET = BibliotecaManager

# Tipo di progetto: applicazione eseguibile
TEMPLATE = app

# Definizione delle cartelle
INCLUDEPATH += modello_logico \
               interfaccia \
               json

# File sorgente del modello logico
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

# Cartelle delle risorse
RESOURCES += resources.qrc

# Opzionale: abilitare warning per API Qt deprecate
# Aiuta a mantenere il codice aggiornato con le versioni più recenti di Qt
# DEFINES += QT_DEPRECATED_WARNINGS

# Opzionale: disabilitare API Qt precedenti a una certa versione
# Forza l'uso delle API più moderne e sicure
# DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000