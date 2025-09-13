#include "mediafactory.h"
#include "modello_logico/media.h"
#include "modello_logico/libro.h"
#include "modello_logico/film.h"
#include "modello_logico/articolo.h"
#include <QJsonObject>
#include <QDate>

std::unique_ptr<Media> MediaFactory::createMedia(const QString& tipo)
{
    QString normalizedType = normalizeTypeName(tipo);
    
    if (normalizedType == "libro") {
        return LibroFactory::createTemplate();
    } else if (normalizedType == "film") {
        return FilmFactory::createTemplate();
    } else if (normalizedType == "articolo") {
        return ArticoloFactory::createTemplate();
    }
    
    return nullptr;
}

std::unique_ptr<Media> MediaFactory::createMediaFromJson(const QJsonObject& json)
{
    if (!json.contains("type")) {
        return nullptr;
    }
    
    QString tipo = json["type"].toString();
    QString normalizedType = normalizeTypeName(tipo);
    
    QString error;
    if (!validateMediaData(json, error)) {
        return nullptr;
    }
    
    if (normalizedType == "libro") {
        return LibroFactory::createFromJson(json);
    } else if (normalizedType == "film") {
        return FilmFactory::createFromJson(json);
    } else if (normalizedType == "articolo") {
        return ArticoloFactory::createFromJson(json);
    }
    
    return nullptr;
}

std::unique_ptr<Media> MediaFactory::createDefaultMedia(const QString& tipo)
{
    return createMedia(tipo);
}

std::unique_ptr<Media> MediaFactory::createLibroTemplate()
{
    return LibroFactory::createTemplate();
}

std::unique_ptr<Media> MediaFactory::createFilmTemplate()
{
    return FilmFactory::createTemplate();
}

std::unique_ptr<Media> MediaFactory::createArticoloTemplate()
{
    return ArticoloFactory::createTemplate();
}

QStringList MediaFactory::getSupportedTypes()
{
    return {"Libro", "Film", "Articolo"};
}

bool MediaFactory::isValidType(const QString& tipo)
{
    QStringList supportedTypes = getSupportedTypes();
    return supportedTypes.contains(tipo, Qt::CaseInsensitive);
}

QString MediaFactory::getTypeDisplayName(const QString& tipo)
{
    QString normalizedType = normalizeTypeName(tipo);
    
    if (normalizedType == "libro") return "Libro";
    if (normalizedType == "film") return "Film";
    if (normalizedType == "articolo") return "Articolo";
    
    return "Sconosciuto";
}

QString MediaFactory::getTypeDescription(const QString& tipo)
{
    QString normalizedType = normalizeTypeName(tipo);
    
    if (normalizedType == "libro") {
        return "Opere letterarie, saggi, manuali e pubblicazioni cartacee";
    } else if (normalizedType == "film") {
        return "Pellicole cinematografiche, documentari e opere audiovisive";
    } else if (normalizedType == "articolo") {
        return "Articoli di riviste, pubblicazioni accademiche e scientifiche";
    }
    
    return "Tipo di media non riconosciuto";
}

bool MediaFactory::validateMediaData(const QJsonObject& json, QString& error)
{
    // Validazione base
    if (!json.contains("type")) {
        error = "Campo 'type' mancante";
        return false;
    }
    
    QString tipo = json["type"].toString();
    QString normalizedType = normalizeTypeName(tipo);
    
    QStringList requiredFields = getRequiredFields(normalizedType);
    
    if (!validateJsonStructure(json, requiredFields)) {
        error = "Campi obbligatori mancanti";
        return false;
    }
    
    // Validazione specifica per tipo
    if (normalizedType == "libro") {
        if (json.contains("pagine") && json["pagine"].toInt() <= 0) {
            error = "Il numero di pagine deve essere positivo";
            return false;
        }
    } else if (normalizedType == "film") {
        if (json.contains("durata") && json["durata"].toInt() <= 0) {
            error = "La durata deve essere positiva";
            return false;
        }
    } else if (normalizedType == "articolo") {
        if (json.contains("data_pubblicazione")) {
            QString dateStr = json["data_pubblicazione"].toString();
            QDate date = QDate::fromString(dateStr, Qt::ISODate);
            if (!date.isValid()) {
                error = "Data di pubblicazione non valida";
                return false;
            }
        }
    }
    
    return true;
}

QStringList MediaFactory::getRequiredFields(const QString& tipo)
{
    QString normalizedType = normalizeTypeName(tipo);
    
    QStringList baseFields = {"type", "id", "titolo", "anno", "descrizione"};
    
    if (normalizedType == "libro") {
        baseFields << LibroFactory::getRequiredFields();
    } else if (normalizedType == "film") {
        baseFields << FilmFactory::getRequiredFields();
    } else if (normalizedType == "articolo") {
        baseFields << ArticoloFactory::getRequiredFields();
    }
    
    return baseFields;
}

QStringList MediaFactory::getOptionalFields(const QString& tipo)
{
    QString normalizedType = normalizeTypeName(tipo);
    
    if (normalizedType == "libro") {
        return LibroFactory::getOptionalFields();
    } else if (normalizedType == "film") {
        return FilmFactory::getOptionalFields();
    } else if (normalizedType == "articolo") {
        return ArticoloFactory::getOptionalFields();
    }
    
    return QStringList();
}

QString MediaFactory::normalizeTypeName(const QString& tipo)
{
    return tipo.toLower().trimmed();
}

bool MediaFactory::validateJsonStructure(const QJsonObject& json, const QStringList& requiredFields)
{
    for (const QString& field : requiredFields) {
        if (!json.contains(field)) {
            return false;
        }
    }
    return true;
}

// LibroFactory
std::unique_ptr<Media> LibroFactory::create(const QString& titolo, int anno, 
                                          const QString& descrizione, const QString& autore)
{
    return std::make_unique<Libro>(titolo, anno, descrizione, autore, 
                                  "", 100, "", Libro::Altro);
}

std::unique_ptr<Media> LibroFactory::createFromJson(const QJsonObject& json)
{
    try {
        return std::make_unique<Libro>(json);
    } catch (const std::exception&) {
        return nullptr;
    }
}

std::unique_ptr<Media> LibroFactory::createTemplate()
{
    int currentYear = QDate::currentDate().year();
    return std::make_unique<Libro>("Nuovo Libro", currentYear, 
                                  "Descrizione del libro", "Autore", 
                                  "Editore", 200, "", Libro::Romanzo);
}

QStringList LibroFactory::getRequiredFields()
{
    return {"autore", "pagine"};
}

QStringList LibroFactory::getOptionalFields()
{
    return {"editore", "isbn", "genere"};
}

// FilmFactory
std::unique_ptr<Media> FilmFactory::create(const QString& titolo, int anno, 
                                         const QString& descrizione, const QString& regista)
{
    QStringList attori = {"Attore principale"};
    return std::make_unique<Film>(titolo, anno, descrizione, regista, attori, 
                                 90, Film::Dramma, Film::PG, "Casa di Produzione");
}

std::unique_ptr<Media> FilmFactory::createFromJson(const QJsonObject& json)
{
    try {
        return std::make_unique<Film>(json);
    } catch (const std::exception&) {
        return nullptr;
    }
}

std::unique_ptr<Media> FilmFactory::createTemplate()
{
    int currentYear = QDate::currentDate().year();
    QStringList attori = {"Attore Principale", "Attore Secondario"};
    return std::make_unique<Film>("Nuovo Film", currentYear, 
                                 "Descrizione del film", "Regista", attori,
                                 120, Film::Dramma, Film::PG, "Casa di Produzione");
}

QStringList FilmFactory::getRequiredFields()
{
    return {"regista", "attori", "durata"};
}

QStringList FilmFactory::getOptionalFields()
{
    return {"genere", "classificazione", "casa_produzione"};
}

// ArticoloFactory
std::unique_ptr<Media> ArticoloFactory::create(const QString& titolo, int anno, 
                                              const QString& descrizione, const QStringList& autori)
{
    QDate currentDate = QDate::currentDate();
    return std::make_unique<Articolo>(titolo, anno, descrizione, autori,
                                     "Rivista", "1", "1", "1-10",
                                     Articolo::Scientifico, Articolo::Accademica,
                                     currentDate);
}

std::unique_ptr<Media> ArticoloFactory::createFromJson(const QJsonObject& json)
{
    try {
        return std::make_unique<Articolo>(json);
    } catch (const std::exception&) {
        return nullptr;
    }
}

std::unique_ptr<Media> ArticoloFactory::createTemplate()
{
    int currentYear = QDate::currentDate().year();
    QStringList autori = {"Primo Autore", "Secondo Autore"};
    QDate currentDate = QDate::currentDate();
    
    return std::make_unique<Articolo>("Nuovo Articolo", currentYear,
                                     "Abstract dell'articolo", autori,
                                     "Nome Rivista", "1", "1", "1-15",
                                     Articolo::Scientifico, Articolo::Accademica,
                                     currentDate);
}

QStringList ArticoloFactory::getRequiredFields()
{
    return {"autori", "rivista", "data_pubblicazione"};
}

QStringList ArticoloFactory::getOptionalFields()
{
    return {"volume", "numero", "pagine", "categoria", "tipo_rivista", "doi"};
}