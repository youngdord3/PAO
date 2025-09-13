#include "media.h"
#include <QUuid>
#include <QDateTime>

Media::Media(const QString& titolo, int anno, const QString& descrizione)
    : m_id(generateId()), m_titolo(titolo), m_anno(anno), m_descrizione(descrizione)
{
}

QString Media::getTitolo() const
{
    return m_titolo;
}

int Media::getAnno() const
{
    return m_anno;
}

QString Media::getDescrizione() const
{
    return m_descrizione;
}

QString Media::getId() const
{
    return m_id;
}

void Media::setTitolo(const QString& titolo)
{
    m_titolo = titolo;
}

void Media::setAnno(int anno)
{
    m_anno = anno;
}

void Media::setDescrizione(const QString& descrizione)
{
    m_descrizione = descrizione;
}

bool Media::isValid() const
{
    bool basicValid = !m_titolo.isEmpty() && 
                     m_anno > 0 && 
                     m_anno <= QDate::currentDate().year();
    
    return basicValid && validateSpecificFields();
}

bool Media::matchesFilter(const QString& searchText) const
{
    if (searchText.isEmpty()) {
        return true;
    }
    
    QString searchLower = searchText.toLower();
    QString searchableText = getSearchableText().toLower();
    
    return searchableText.contains(searchLower);
}

QString Media::generateId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}