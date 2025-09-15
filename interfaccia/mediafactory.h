#ifndef MEDIAFACTORY_H
#define MEDIAFACTORY_H

#include <QString>
#include <QJsonObject>
#include <memory>

class Media;
class QWidget;

/**
 * @brief Factory per la creazione di media e componenti UI correlati
 * 
 * Implementa il pattern Abstract Factory per gestire la creazione
 * di diversi tipi di media e delle loro rappresentazioni UI
 */
class MediaFactory
{
public:
    MediaFactory() = default;
    virtual ~MediaFactory() = default;
    
    // Factory methods per la creazione di media
    static std::unique_ptr<Media> createMedia(const QString& tipo);
    static std::unique_ptr<Media> createMediaFromJson(const QJsonObject& json);
    static std::unique_ptr<Media> createDefaultMedia(const QString& tipo);
    
    // Utility per la gestione dei tipi
    static QStringList getSupportedTypes();
    static bool isValidType(const QString& tipo);
    static QString getTypeDisplayName(const QString& tipo);
    
    // Validazione e diagnostica
    static bool validateMediaData(const QJsonObject& json, QString& error);
    static QStringList getRequiredFields(const QString& tipo);
    static QStringList getOptionalFields(const QString& tipo);

private:
    // Helper methods
    static QString normalizeTypeName(const QString& tipo);
    static bool validateJsonStructure(const QJsonObject& json, const QStringList& requiredFields);
};

/**
 * @brief Abstract Factory specifica per ogni tipo di media
 */
class LibroFactory
{
public:
    static std::unique_ptr<Media> create(const QString& titolo, int anno, 
                                       const QString& descrizione, const QString& autore);
    static std::unique_ptr<Media> createFromJson(const QJsonObject& json);
    static std::unique_ptr<Media> createTemplate();
    static QStringList getRequiredFields();
    static QStringList getOptionalFields();
};

class FilmFactory
{
public:
    static std::unique_ptr<Media> create(const QString& titolo, int anno, 
                                       const QString& descrizione, const QString& regista);
    static std::unique_ptr<Media> createFromJson(const QJsonObject& json);
    static std::unique_ptr<Media> createTemplate();
    static QStringList getRequiredFields();
    static QStringList getOptionalFields();
};

class ArticoloFactory
{
public:
    static std::unique_ptr<Media> create(const QString& titolo, int anno, 
                                       const QString& descrizione, const QStringList& autori);
    static std::unique_ptr<Media> createFromJson(const QJsonObject& json);
    static std::unique_ptr<Media> createTemplate();
    static QStringList getRequiredFields();
    static QStringList getOptionalFields();
};

#endif