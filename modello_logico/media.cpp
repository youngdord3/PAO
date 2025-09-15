#include "media.h"
#include <QUuid>
#include <QDateTime>

// Contatori statici per ogni tipo di media
static int s_libroCounter = 1;
static int s_filmCounter = 1;
static int s_articoloCounter = 1;

Media::Media(const QString& titolo, int anno, const QString& descrizione)
    : m_id(""), m_titolo(titolo), m_anno(anno), m_descrizione(descrizione)
{
    // L'ID verrà impostato dalle classi derivate
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

QString Media::generateSimpleId(const QString& type)
{
    if (type.toLower() == "libro") {
        return QString("libro-%1").arg(s_libroCounter++, 3, 10, QChar('0'));
    } else if (type.toLower() == "film") {
        return QString("film-%1").arg(s_filmCounter++, 3, 10, QChar('0'));
    } else if (type.toLower() == "articolo") {
        return QString("articolo-%1").arg(s_articoloCounter++, 3, 10, QChar('0'));
    }
    
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void Media::updateCountersFromExistingIds(const std::vector<QString>& existingIds)
{
    int maxLibro = 0;
    int maxFilm = 0;
    int maxArticolo = 0;
    
    // Se non ci sono ID esistenti (nuova collezione vuota), resetta i contatori
    if (existingIds.empty()) {
        s_libroCounter = 1;
        s_filmCounter = 1;
        s_articoloCounter = 1;
        return;
    }
    
    for (const QString& id : existingIds) {
        if (id.startsWith("libro-")) {
            QString numStr = id.mid(6);
            bool ok;
            int num = numStr.toInt(&ok);
            if (ok && num > maxLibro) {
                maxLibro = num;
            }
        } else if (id.startsWith("film-")) {
            QString numStr = id.mid(5);
            bool ok;
            int num = numStr.toInt(&ok);
            if (ok && num > maxFilm) {
                maxFilm = num;
            }
        } else if (id.startsWith("articolo-")) {
            QString numStr = id.mid(9);
            bool ok;
            int num = numStr.toInt(&ok);
            if (ok && num > maxArticolo) {
                maxArticolo = num;
            }
        }
    }
    
    // Imposta i contatori al valore massimo + 1
    s_libroCounter = maxLibro + 1;
    s_filmCounter = maxFilm + 1;
    s_articoloCounter = maxArticolo + 1;
}

void Media::resetCounters()
{
    s_libroCounter = 1;
    s_filmCounter = 1;
    s_articoloCounter = 1;
}