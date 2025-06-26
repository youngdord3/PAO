#ifndef JSONMANAGER_H
#define JSONMANAGER_H

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <vector>
#include <memory>

class Media;

/**
 * @brief Classe per gestire la serializzazione/deserializzazione JSON
 * 
 * Gestisce il salvataggio e caricamento della collezione di media
 * in formato JSON strutturato
 */
class JsonManager
{
public:
    JsonManager() = default;
    ~JsonManager() = default;
    
    // Salvataggio e caricamento della collezione
    bool saveCollection(const std::vector<std::unique_ptr<Media>>& collection, 
                       const QString& filename) const;
    std::vector<std::unique_ptr<Media>> loadCollection(const QString& filename) const;
    
    // Salvataggio e caricamento di singoli media
    bool saveMedia(const Media* media, const QString& filename) const;
    std::unique_ptr<Media> loadMedia(const QString& filename) const;
    
    // Esportazione in diversi formati
    bool exportToJson(const std::vector<std::unique_ptr<Media>>& collection, 
                     const QString& filename, bool prettyFormat = true) const;
    bool exportToCSV(const std::vector<std::unique_ptr<Media>>& collection, 
                    const QString& filename) const;
    
    // Importazione da JSON esterno
    std::vector<std::unique_ptr<Media>> importFromJson(const QString& filename) const;
    
    // Validazione
    bool isValidJsonFile(const QString& filename) const;
    QStringList validateJsonStructure(const QJsonDocument& doc) const;
    
    // Utility
    QString getLastError() const;
    void clearError();
    
    // Backup e recovery
    bool createBackup(const QString& originalFile) const;
    bool restoreFromBackup(const QString& backupFile, const QString& targetFile) const;

private:
    mutable QString m_lastError;
    
    // Helper methods per serializzazione
    QJsonObject collectionToJson(const std::vector<std::unique_ptr<Media>>& collection) const;
    std::vector<std::unique_ptr<Media>> jsonToCollection(const QJsonObject& json) const;
    
    // Factory method per creare media da JSON
    std::unique_ptr<Media> createMediaFromJson(const QJsonObject& mediaJson) const;
    
    // Utility per file I/O
    bool writeJsonToFile(const QJsonDocument& doc, const QString& filename) const;
    QJsonDocument readJsonFromFile(const QString& filename) const;
    
    // Validazione specifica
    bool validateMediaJson(const QJsonObject& mediaJson) const;
    bool validateCollectionJson(const QJsonObject& collectionJson) const;
    
    // Error handling
    void setError(const QString& error) const;
    
    // Costanti per la struttura JSON
    static const QString JSON_VERSION;
    static const QString COLLECTION_KEY;
    static const QString METADATA_KEY;
    static const QString MEDIA_ARRAY_KEY;
};

#endif // JSONMANAGER_H