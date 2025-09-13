#ifndef COLLEZIONE_H
#define COLLEZIONE_H

#include "media.h"
#include "filtrostrategy.h"
#include <QObject>
#include <vector>
#include <memory>
#include <functional>

class JsonManager;

/**
 * @brief Classe per gestire la collezione di media
 * 
 * Implementa il pattern Observer per notificare le modifiche
 * e il pattern Strategy per i filtri di ricerca
 */
class Collezione : public QObject
{
    Q_OBJECT
    
public:
    explicit Collezione(QObject* parent = nullptr);
    ~Collezione();
    
    // Gestione dei media
    void addMedia(std::unique_ptr<Media> media);
    bool removeMedia(const QString& id);
    bool updateMedia(const QString& id, std::unique_ptr<Media> updatedMedia);
    Media* findMedia(const QString& id) const;
    
    // Accesso alla collezione
    const std::vector<std::unique_ptr<Media>>& getAllMedia() const;
    std::vector<Media*> getMediaByType(const QString& type) const;
    std::vector<Media*> searchMedia(const QString& searchText) const;
    std::vector<Media*> filterMedia(std::unique_ptr<FiltroStrategy> strategy) const;
    
    // Statistiche
    size_t size() const;
    bool isEmpty() const;
    size_t countByType(const QString& type) const;
    
    // Persistenza
    bool saveToFile(const QString& filename) const;
    bool loadFromFile(const QString& filename);
    void clear();
    
    // Validazione
    bool isValidCollection() const;
    QStringList getValidationErrors() const;
    
    // Iterator pattern
    class Iterator {
    public:
        Iterator(const std::vector<std::unique_ptr<Media>>& collection, size_t index);
        Media* operator*() const;
        Iterator& operator++();
        bool operator!=(const Iterator& other) const;
        
    private:
        const std::vector<std::unique_ptr<Media>>& m_collection;
        size_t m_index;
    };
    
    Iterator begin() const;
    Iterator end() const;

signals:
    void mediaAdded(const QString& id);
    void mediaRemoved(const QString& id);
    void mediaUpdated(const QString& id);
    void collectionCleared();
    void collectionLoaded(int count);

private:
    std::vector<std::unique_ptr<Media>> m_media;
    std::unique_ptr<JsonManager> m_jsonManager;
    
    // Helper methods
    bool isIdUnique(const QString& id) const;
    void notifyChange();
    std::vector<std::unique_ptr<Media>>::iterator findMediaIterator(const QString& id);
};

#endif // COLLEZIONE_H