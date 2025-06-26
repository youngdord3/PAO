QT += core widgets

CONFIG += c++17

TARGET = BibliotecaManager
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