#include "collezione.h"
#include "libro.h"
#include "film.h"
#include "articolo.h"
#include "json/jsonmanager.h"
#include <algorithm>
#include <QDebug>
#include <set>

Collezione::Collezione(QObject* parent)
    : QObject(parent), m_jsonManager(std::make_unique<JsonManager>())
{
}

void Collezione::addMedia(std::unique_ptr<Media> media)
{
    if (!media || !media->isValid()) {
        qWarning() << "Tentativo di aggiungere media non valido";
        return;
    }
    
    if (!isIdUnique(media->getId())) {
        qWarning() << "ID non unico:" << media->getId();
        return;
    }
    
    QString id = media->getId();
    m_media.push_back(std::move(media));
    
    emit mediaAdded(id);
    notifyChange();
}

bool Collezione::removeMedia(const QString& id)
{
    auto it = findMediaIterator(id);
    if (it != m_media.end()) {
        m_media.erase(it);
        emit mediaRemoved(id);
        notifyChange();
        return true;
    }
    return false;
}

bool Collezione::updateMedia(const QString& id, std::unique_ptr<Media> updatedMedia)
{
    if (!updatedMedia || !updatedMedia->isValid()) {
        return false;
    }
    
    auto it = findMediaIterator(id);
    if (it != m_media.end()) {
        *it = std::move(updatedMedia);
        emit mediaUpdated(id);
        notifyChange();
        return true;
    }
    return false;
}

Media* Collezione::findMedia(const QString& id) const
{
    auto it = std::find_if(m_media.begin(), m_media.end(),
                          [&id](const std::unique_ptr<Media>& media) {
                              return media->getId() == id;
                          });
    
    return (it != m_media.end()) ? it->get() : nullptr;
}

const std::vector<std::unique_ptr<Media>>& Collezione::getAllMedia() const
{
    return m_media;
}

std::vector<Media*> Collezione::getMediaByType(const QString& type) const
{
    std::vector<Media*> result;
    
    for (const auto& media : m_media) {
        if (media->getTypeDisplayName().toLower() == type.toLower()) {
            result.push_back(media.get());
        }
    }
    
    return result;
}

std::vector<Media*> Collezione::searchMedia(const QString& searchText) const
{
    std::vector<Media*> result;
    
    if (searchText.isEmpty()) {
        // Se la ricerca è vuota, restituisci tutti i media
        for (const auto& media : m_media) {
            result.push_back(media.get());
        }
        return result;
    }
    
    for (const auto& media : m_media) {
        if (media->matchesFilter(searchText)) {
            result.push_back(media.get());
        }
    }
    
    return result;
}

std::vector<Media*> Collezione::filterMedia(std::unique_ptr<FiltroStrategy> strategy) const
{
    std::vector<Media*> result;
    
    if (!strategy) {
        return result;
    }
    
    for (const auto& media : m_media) {
        if (strategy->matches(media.get())) {
            result.push_back(media.get());
        }
    }
    
    return result;
}

size_t Collezione::size() const
{
    return m_media.size();
}

bool Collezione::isEmpty() const
{
    return m_media.empty();
}

size_t Collezione::countByType(const QString& type) const
{
    return static_cast<size_t>(
        std::count_if(m_media.begin(), m_media.end(),
                     [&type](const std::unique_ptr<Media>& media) {
                         return media->getTypeDisplayName().toLower() == type.toLower();
                     })
    );
}

bool Collezione::saveToFile(const QString& filename) const
{
    return m_jsonManager->saveCollection(m_media, filename);
}

bool Collezione::loadFromFile(const QString& filename)
{
    auto loadedMedia = m_jsonManager->loadCollection(filename);
    if (!loadedMedia.empty()) {
        clear();
        m_media = std::move(loadedMedia);
        emit collectionLoaded(static_cast<int>(m_media.size()));
        notifyChange();
        return true;
    }
    return false;
}

void Collezione::clear()
{
    m_media.clear();
    emit collectionCleared();
    notifyChange();
}

bool Collezione::isValidCollection() const
{
    // Verifica che tutti i media siano validi e abbiano ID unici
    std::set<QString> ids;
    
    for (const auto& media : m_media) {
        if (!media || !media->isValid()) {
            return false;
        }
        
        if (ids.find(media->getId()) != ids.end()) {
            return false; // ID duplicato
        }
        
        ids.insert(media->getId());
    }
    
    return true;
}

QStringList Collezione::getValidationErrors() const
{
    QStringList errors;
    std::set<QString> ids;
    
    for (size_t i = 0; i < m_media.size(); ++i) {
        const auto& media = m_media[i];
        
        if (!media) {
            errors << QString("Media #%1: Puntatore nullo").arg(i);
            continue;
        }
        
        if (!media->isValid()) {
            errors << QString("Media #%1 (%2): Dati non validi").arg(i).arg(media->getTitolo());
        }
        
        const QString& id = media->getId();
        if (ids.find(id) != ids.end()) {
            errors << QString("Media #%1 (%2): ID duplicato").arg(i).arg(media->getTitolo());
        } else {
            ids.insert(id);
        }
    }
    
    return errors;
}

// Iterator implementation
Collezione::Iterator::Iterator(const std::vector<std::unique_ptr<Media>>& collection, size_t index)
    : m_collection(collection), m_index(index)
{
}

Media* Collezione::Iterator::operator*() const
{
    if (m_index < m_collection.size()) {
        return m_collection[m_index].get();
    }
    return nullptr;
}

Collezione::Iterator& Collezione::Iterator::operator++()
{
    ++m_index;
    return *this;
}

bool Collezione::Iterator::operator!=(const Iterator& other) const
{
    return m_index != other.m_index;
}

Collezione::Iterator Collezione::begin() const
{
    return Iterator(m_media, 0);
}

Collezione::Iterator Collezione::end() const
{
    return Iterator(m_media, m_media.size());
}

// Private methods
bool Collezione::isIdUnique(const QString& id) const
{
    return std::none_of(m_media.begin(), m_media.end(),
                       [&id](const std::unique_ptr<Media>& media) {
                           return media->getId() == id;
                       });
}

void Collezione::notifyChange()
{
    // Qui si potrebbero aggiungere altre notifiche o validazioni
    // Ad esempio, salvare automaticamente in un file di backup
}

std::vector<std::unique_ptr<Media>>::iterator Collezione::findMediaIterator(const QString& id)
{
    return std::find_if(m_media.begin(), m_media.end(),
                       [&id](const std::unique_ptr<Media>& media) {
                           return media->getId() == id;
                       });
}