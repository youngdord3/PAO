#include "libro.h"
#include "interfaccia/mediacard.h"
#include <QJsonObject>
#include <QRegularExpression>

Libro::Libro(const QString& titolo, int anno, const QString& descrizione,
             const QString& autore, const QString& editore, int pagine, 
             const QString& isbn, Genere genere)
    : Media(titolo, anno, descrizione), m_autore(autore), m_editore(editore),
      m_pagine(pagine), m_isbn(isbn), m_genere(genere)
{
    // Genera ID semplice per nuovo libro
    if (m_id.isEmpty()) {
        m_id = generateSimpleId("libro");
    }
}

Libro::Libro(const QJsonObject& json)
    : Media("", 0, "")
{
    fromJson(json);
}

QString Libro::getAutore() const
{
    return m_autore;
}

QString Libro::getEditore() const
{
    return m_editore;
}

int Libro::getPagine() const
{
    return m_pagine;
}

QString Libro::getIsbn() const
{
    return m_isbn;
}

Libro::Genere Libro::getGenere() const
{
    return m_genere;
}

QString Libro::getGenereString() const
{
    return genereToString(m_genere);
}

void Libro::setAutore(const QString& autore)
{
    m_autore = autore;
}

void Libro::setEditore(const QString& editore)
{
    m_editore = editore;
}

void Libro::setPagine(int pagine)
{
    m_pagine = pagine;
}

void Libro::setIsbn(const QString& isbn)
{
    m_isbn = isbn;
}

void Libro::setGenere(Genere genere)
{
    m_genere = genere;
}

std::unique_ptr<Media> Libro::clone() const
{
    // Crea una nuova istanza con tutti i dati correnti
    auto cloned = std::make_unique<Libro>(m_titolo, m_anno, m_descrizione, m_autore, 
                                         m_editore, m_pagine, m_isbn, m_genere);
    
    // IMPORTANTE: Mantiene l'ID originale per il clone
    cloned->setId(this->getId());
    
    return cloned;
}

QJsonObject Libro::toJson() const
{
    QJsonObject json;
    json["type"] = "libro";
    json["id"] = m_id;
    json["titolo"] = m_titolo;
    json["anno"] = m_anno;
    json["descrizione"] = m_descrizione;
    json["autore"] = m_autore;
    json["editore"] = m_editore;
    json["pagine"] = m_pagine;
    json["isbn"] = m_isbn;
    json["genere"] = static_cast<int>(m_genere);
    return json;
}

void Libro::fromJson(const QJsonObject& json)
{
    m_id = json["id"].toString();
    m_titolo = json["titolo"].toString();
    m_anno = json["anno"].toInt();
    m_descrizione = json["descrizione"].toString();
    m_autore = json["autore"].toString();
    m_editore = json["editore"].toString();
    m_pagine = json["pagine"].toInt();
    m_isbn = json["isbn"].toString();
    m_genere = static_cast<Genere>(json["genere"].toInt());
}

QString Libro::getDisplayInfo() const
{
    return QString("Autore: %1\nEditore: %2\nPagine: %3\nGenere: %4\nISBN: %5")
           .arg(m_autore, m_editore)
           .arg(m_pagine)
           .arg(getGenereString(), m_isbn);
}

QString Libro::getTypeDisplayName() const
{
    return "Libro";
}

bool Libro::matchesCriteria(const QString& criteria, const QString& value) const
{
    if (criteria == "autore") {
        return m_autore.toLower().contains(value.toLower());
    } else if (criteria == "editore") {
        return m_editore.toLower().contains(value.toLower());
    } else if (criteria == "genere") {
        return getGenereString().toLower().contains(value.toLower());
    } else if (criteria == "isbn") {
        return m_isbn.contains(value);
    }
    return false;
}

QString Libro::genereToString(Genere genere)
{
    switch (genere) {
        case Romanzo: return "Romanzo";
        case Saggio: return "Saggio";
        case Biografia: return "Biografia";
        case Fantascienza: return "Fantascienza";
        case Fantasy: return "Fantasy";
        case Giallo: return "Giallo";
        case Horror: return "Horror";
        case Storico: return "Storico";
        case Tecnico: return "Tecnico";
        case Altro: return "Altro";
        default: return "Altro";
    }
}

Libro::Genere Libro::stringToGenere(const QString& str)
{
    if (str == "Romanzo") return Romanzo;
    if (str == "Saggio") return Saggio;
    if (str == "Biografia") return Biografia;
    if (str == "Fantascienza") return Fantascienza;
    if (str == "Fantasy") return Fantasy;
    if (str == "Giallo") return Giallo;
    if (str == "Horror") return Horror;
    if (str == "Storico") return Storico;
    if (str == "Tecnico") return Tecnico;
    return Altro;
}

QStringList Libro::getAllGeneri()
{
    return {"Romanzo", "Saggio", "Biografia", "Fantascienza", 
            "Fantasy", "Giallo", "Horror", "Storico", "Tecnico", "Altro"};
}

bool Libro::validateSpecificFields() const
{
    return !m_autore.isEmpty() && 
           m_pagine > 0 && 
           (m_isbn.isEmpty() || isValidIsbn(m_isbn));
}

QString Libro::getSearchableText() const
{
    return QString("%1 %2 %3 %4 %5 %6")
           .arg(m_titolo, m_descrizione, m_autore, m_editore, getGenereString(), m_isbn);
}

bool Libro::isValidIsbn(const QString& isbn) const
{
    if (isbn.isEmpty()) return true; // ISBN opzionale
    
    QRegularExpression isbn10("^\\d{9}[\\dX]$");
    QRegularExpression isbn13("^\\d{13}$");
    
    QString cleanIsbn = isbn;
    cleanIsbn.remove(QRegularExpression("[\\s-]"));
    
    return isbn10.match(cleanIsbn).hasMatch() || isbn13.match(cleanIsbn).hasMatch();
}