#ifndef MEDIA_H
#define MEDIA_H

#include <QString>
#include <QJsonObject>
#include <QDate>
#include <memory>
#include <vector>

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
    void setId(const QString& id) { m_id = id; }
    
    // Metodi virtuali puri
    virtual std::unique_ptr<Media> clone() const = 0;
    virtual QJsonObject toJson() const = 0;
    virtual void fromJson(const QJsonObject& json) = 0;
    virtual QString getDisplayInfo() const = 0;
    virtual QString getTypeDisplayName() const = 0;
    virtual bool matchesCriteria(const QString& criteria, const QString& value) const = 0;
    
    // Template Method per la validazione
    bool isValid() const;
    bool isCompleteAndValid() const;
    
    // Metodi per la ricerca e filtri
    bool matchesFilter(const QString& searchText) const;
    
    // Gestione ID semplici con contatori
    static QString generateSimpleId(const QString& type);
    static void updateCountersFromExistingIds(const std::vector<QString>& existingIds);
    static void resetCounters();

protected:
    // Template method steps
    virtual bool validateSpecificFields() const = 0;
    virtual QString getSearchableText() const = 0;
    
    // Attributi comuni protetti
    QString m_id;
    QString m_titolo;
    int m_anno;
    QString m_descrizione;
};

#endif