#include "jsonmanager.h"
#include "modello_logico/media.h"
#include "modello_logico/libro.h"
#include "modello_logico/film.h"
#include "modello_logico/articolo.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QDebug>

// Costanti statiche
const QString JsonManager::JSON_VERSION = "1.0";
const QString JsonManager::COLLECTION_KEY = "collezione";
const QString JsonManager::METADATA_KEY = "metadata";
const QString JsonManager::MEDIA_ARRAY_KEY = "media";

bool JsonManager::saveCollection(const std::vector<std::unique_ptr<Media>>& collection, 
                                const QString& filename) const
{
    clearError();
    
    try {
        QJsonObject rootObj = collectionToJson(collection);
        QJsonDocument doc(rootObj);
        
        return writeJsonToFile(doc, filename);
    } catch (const std::exception& e) {
        setError(QString("Errore durante il salvataggio: %1").arg(e.what()));
        return false;
    }
}

std::vector<std::unique_ptr<Media>> JsonManager::loadCollection(const QString& filename) const
{
    clearError();
    
    std::vector<std::unique_ptr<Media>> collection;
    
    if (!QFile::exists(filename)) {
        setError("File non trovato: " + filename);
        return collection;
    }
    
    try {
        QJsonDocument doc = readJsonFromFile(filename);
        if (doc.isNull()) {
            return collection;
        }
        
        QJsonObject rootObj = doc.object();
        
        // Validazione struttura
        QStringList errors = validateJsonStructure(doc);
        if (!errors.isEmpty()) {
            setError("Struttura JSON non valida: " + errors.join(", "));
            return collection;
        }
        
        collection = jsonToCollection(rootObj);
        
    } catch (const std::exception& e) {
        setError(QString("Errore durante il caricamento: %1").arg(e.what()));
        collection.clear();
    }
    
    return collection;
}

bool JsonManager::exportToJson(const std::vector<std::unique_ptr<Media>>& collection, 
                              const QString& filename, bool prettyFormat) const
{
    QJsonObject rootObj = collectionToJson(collection);
    QJsonDocument doc(rootObj);
    
    QJsonDocument::JsonFormat format = prettyFormat ? 
        QJsonDocument::Indented : QJsonDocument::Compact;
    
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        setError("Impossibile aprire il file per la scrittura: " + filename);
        return false;
    }
    
    file.write(doc.toJson(format));
    return true;
}

bool JsonManager::exportToCSV(const std::vector<std::unique_ptr<Media>>& collection, 
                             const QString& filename) const
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        setError("Impossibile aprire il file CSV per la scrittura: " + filename);
        return false;
    }
    
    QTextStream out(&file);
    
    // Header CSV
    out << "Tipo,Titolo,Anno,Descrizione,Info Specifiche\n";
    
    // Dati
    for (const auto& media : collection) {
        if (!media) continue;
        
        QString infoSpecifiche = media->getDisplayInfo().replace('\n', "; ");
        out << QString("%1,%2,%3,\"%4\",\"%5\"\n")
               .arg(media->getTypeDisplayName())
               .arg(media->getTitolo())
               .arg(media->getAnno())
               .arg(media->getDescrizione())
               .arg(infoSpecifiche);
    }
    
    return true;
}

std::vector<std::unique_ptr<Media>> JsonManager::importFromJson(const QString& filename) const
{
    return loadCollection(filename);
}

QStringList JsonManager::validateJsonStructure(const QJsonDocument& doc) const
{
    QStringList errors;
    
    if (!doc.isObject()) {
        errors << "Il documento non è un oggetto JSON";
        return errors;
    }
    
    QJsonObject rootObj = doc.object();
    
    // Verifica presenza delle chiavi obbligatorie
    if (!rootObj.contains(METADATA_KEY)) {
        errors << "Chiave 'metadata' mancante";
    }
    
    if (!rootObj.contains(MEDIA_ARRAY_KEY)) {
        errors << "Chiave 'media' mancante";
    }
    
    // Verifica struttura metadata
    if (rootObj.contains(METADATA_KEY)) {
        QJsonObject metadata = rootObj[METADATA_KEY].toObject();
        if (!metadata.contains("versione") || !metadata.contains("data_creazione")) {
            errors << "Metadata incompleti";
        }
    }
    
    // Verifica array media
    if (rootObj.contains(MEDIA_ARRAY_KEY)) {
        if (!rootObj[MEDIA_ARRAY_KEY].isArray()) {
            errors << "Il campo 'media' deve essere un array";
        }
    }
    
    return errors;
}

QString JsonManager::getLastError() const
{
    return m_lastError;
}

void JsonManager::clearError() const
{
    m_lastError.clear();
}

// Private methods
QJsonObject JsonManager::collectionToJson(const std::vector<std::unique_ptr<Media>>& collection) const
{
    QJsonObject rootObj;
    
    // Metadata
    QJsonObject metadata;
    metadata["versione"] = JSON_VERSION;
    metadata["data_creazione"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    metadata["numero_media"] = static_cast<int>(collection.size());
    metadata["generato_da"] = "Biblioteca Manager";
    
    rootObj[METADATA_KEY] = metadata;
    
    // Array dei media
    QJsonArray mediaArray;
    for (const auto& media : collection) {
        if (media) {
            mediaArray.append(media->toJson());
        }
    }
    
    rootObj[MEDIA_ARRAY_KEY] = mediaArray;
    
    return rootObj;
}

std::vector<std::unique_ptr<Media>> JsonManager::jsonToCollection(const QJsonObject& json) const
{
    std::vector<std::unique_ptr<Media>> collection;
    
    if (!json.contains(MEDIA_ARRAY_KEY)) {
        setError("Array media non trovato nel JSON");
        return collection;
    }
    
    QJsonArray mediaArray = json[MEDIA_ARRAY_KEY].toArray();
    
    for (const QJsonValue& value : mediaArray) {
        if (value.isObject()) {
            auto media = createMediaFromJson(value.toObject());
            if (media) {
                collection.push_back(std::move(media));
            }
        }
    }
    
    return collection;
}

std::unique_ptr<Media> JsonManager::createMediaFromJson(const QJsonObject& mediaJson) const
{
    if (!validateMediaJson(mediaJson)) {
        return nullptr;
    }
    
    QString type = mediaJson["type"].toString();
    
    try {
        if (type == "libro") {
            return std::make_unique<Libro>(mediaJson);
        } else if (type == "film") {
            return std::make_unique<Film>(mediaJson);
        } else if (type == "articolo") {
            return std::make_unique<Articolo>(mediaJson);
        } else {
            setError("Tipo di media non riconosciuto: " + type);
            return nullptr;
        }
    } catch (const std::exception& e) {
        setError(QString("Errore nella creazione del media: %1").arg(e.what()));
        return nullptr;
    }
}

bool JsonManager::writeJsonToFile(const QJsonDocument& doc, const QString& filename) const
{
    // Crea la directory se non esiste
    QFileInfo fileInfo(filename);
    QDir dir = fileInfo.dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        setError("Impossibile aprire il file per la scrittura: " + filename);
        return false;
    }
    
    qint64 bytesWritten = file.write(doc.toJson());
    bool success = bytesWritten != -1;
    
    if (!success) {
        setError("Errore durante la scrittura del file");
    }
    
    return success;
}

QJsonDocument JsonManager::readJsonFromFile(const QString& filename) const
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        setError("Impossibile aprire il file per la lettura: " + filename);
        return QJsonDocument();
    }
    
    QByteArray data = file.readAll();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        setError("Errore di parsing JSON: " + error.errorString());
        return QJsonDocument();
    }
    
    return doc;
}

bool JsonManager::validateMediaJson(const QJsonObject& mediaJson) const
{
    // Campi obbligatorie per tutti i media
    QStringList requiredFields = {"type", "id", "titolo", "anno", "descrizione"};
    
    for (const QString& field : requiredFields) {
        if (!mediaJson.contains(field)) {
            setError(QString("Campo obbligatorio mancante: %1").arg(field));
            return false;
        }
    }
    
    // Validazione specifica per tipo
    QString type = mediaJson["type"].toString();
    if (type != "libro" && type != "film" && type != "articolo") {
        setError("Tipo di media non valido: " + type);
        return false;
    }
    
    return true;
}

bool JsonManager::validateCollectionJson(const QJsonObject& collectionJson) const
{
    return collectionJson.contains(METADATA_KEY) && 
           collectionJson.contains(MEDIA_ARRAY_KEY);
}

void JsonManager::setError(const QString& error) const
{
    m_lastError = error;
    qWarning() << "JsonManager Error:" << error;
}