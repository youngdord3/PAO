#ifndef MEDIA_H
#define MEDIA_H

#include <QString>
#include <QJsonObject>
#include <QDate>
#include <memory>

// Forward declarations per evitare dipendenze circolari
class QWidget;
class MediaCard;

/**
 * @brief Classe base astratta per tutti i tipi di media
 * 
 * Implementa il pattern Template Method per le operazioni comuni
 * e definisce l'interfaccia per il polimorfismo
 */
class Media
{
public:
    Media(const QString& titolo, int anno, const QString& descrizione);
    virtual ~Media() = default;
    
    // Rimuovi copy constructor e assignment operator per evitare problemi con unique_ptr
    Media(const Media&) = delete;
    Media& operator=(const Media&) = delete;
    
    // Metodi accessori comuni
    QString getTitolo() const;
    int getAnno() const;
    QString getDescrizione() const;
    QString getId() const;
    
    void setTitolo(const QString& titolo);
    void setAnno(int anno);
    void setDescrizione(const QString& descrizione);
    
    // Metodi virtuali puri per il polimorfismo non banale
    virtual std::unique_ptr<Media> clone() const = 0;
    virtual QJsonObject toJson() const = 0;
    virtual void fromJson(const QJsonObject& json) = 0;
    virtual QString getDisplayInfo() const = 0;
    virtual QString getTypeDisplayName() const = 0;
    
    // Pattern Template Method per la validazione
    bool isValid() const;
    
    // Metodi per la ricerca e filtri
    virtual bool matchesFilter(const QString& searchText) const;
    virtual bool matchesCriteria(const QString& criteria, const QString& value) const = 0;

    // RIMOSSO: createCard() - non più necessario

protected:
    // Template method steps - da implementare nelle classi derivate
    virtual bool validateSpecificFields() const = 0;
    virtual QString getSearchableText() const = 0;
    
    // Attributi comuni protetti
    QString m_id;
    QString m_titolo;
    int m_anno;
    QString m_descrizione;
    
private:
    static QString generateId();
};

#endif // MEDIA_H