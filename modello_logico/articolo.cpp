#include "articolo.h"
#include "interfaccia/mediacard.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>

Articolo::Articolo(const QString& titolo, int anno, const QString& descrizione,
                   const QStringList& autori, const QString& rivista, 
                   const QString& volume, const QString& numero, 
                   const QString& pagine, Categoria categoria, TipoRivista tipo_rivista,
                   const QDate& data_pubblicazione, const QString& doi)
    : Media(titolo, anno, descrizione), m_autori(autori), m_rivista(rivista),
      m_volume(volume), m_numero(numero), m_pagine(pagine), m_categoria(categoria),
      m_tipo_rivista(tipo_rivista), m_data_pubblicazione(data_pubblicazione), m_doi(doi)
{
}

Articolo::Articolo(const QJsonObject& json)
    : Media("", 0, "")
{
    fromJson(json);
}

QStringList Articolo::getAutori() const
{
    return m_autori;
}

QString Articolo::getRivista() const
{
    return m_rivista;
}

QString Articolo::getVolume() const
{
    return m_volume;
}

QString Articolo::getNumero() const
{
    return m_numero;
}

QString Articolo::getPagine() const
{
    return m_pagine;
}

Articolo::Categoria Articolo::getCategoria() const
{
    return m_categoria;
}

QString Articolo::getCategoriaString() const
{
    return categoriaToString(m_categoria);
}

Articolo::TipoRivista Articolo::getTipoRivista() const
{
    return m_tipo_rivista;
}

QString Articolo::getTipoRivistaString() const
{
    return tipoRivistaToString(m_tipo_rivista);
}

QDate Articolo::getDataPubblicazione() const
{
    return m_data_pubblicazione;
}

QString Articolo::getDoi() const
{
    return m_doi;
}

void Articolo::setAutori(const QStringList& autori)
{
    m_autori = autori;
}

void Articolo::setRivista(const QString& rivista)
{
    m_rivista = rivista;
}

void Articolo::setVolume(const QString& volume)
{
    m_volume = volume;
}

void Articolo::setNumero(const QString& numero)
{
    m_numero = numero;
}

void Articolo::setPagine(const QString& pagine)
{
    m_pagine = pagine;
}

void Articolo::setCategoria(Categoria categoria)
{
    m_categoria = categoria;
}

void Articolo::setTipoRivista(TipoRivista tipo_rivista)
{
    m_tipo_rivista = tipo_rivista;
}

void Articolo::setDataPubblicazione(const QDate& data)
{
    m_data_pubblicazione = data;
}

void Articolo::setDoi(const QString& doi)
{
    m_doi = doi;
}

std::unique_ptr<Media> Articolo::clone() const
{
    // Crea una nuova istanza con tutti i dati correnti
    auto cloned = std::make_unique<Articolo>(m_titolo, m_anno, m_descrizione, m_autori, 
                                            m_rivista, m_volume, m_numero, m_pagine, 
                                            m_categoria, m_tipo_rivista, m_data_pubblicazione, m_doi);
    
    // IMPORTANTE: Copia l'ID originale per mantenere la corrispondenza
    cloned->m_id = this->m_id;
    
    return cloned;
}

QJsonObject Articolo::toJson() const
{
    QJsonObject json;
    json["type"] = "articolo";
    json["id"] = m_id;
    json["titolo"] = m_titolo;
    json["anno"] = m_anno;
    json["descrizione"] = m_descrizione;
    
    QJsonArray autoriArray;
    for (const QString& autore : m_autori) {
        autoriArray.append(autore);
    }
    json["autori"] = autoriArray;
    
    json["rivista"] = m_rivista;
    json["volume"] = m_volume;
    json["numero"] = m_numero;
    json["pagine"] = m_pagine;
    json["categoria"] = static_cast<int>(m_categoria);
    json["tipo_rivista"] = static_cast<int>(m_tipo_rivista);
    json["data_pubblicazione"] = m_data_pubblicazione.toString(Qt::ISODate);
    json["doi"] = m_doi;
    
    return json;
}

void Articolo::fromJson(const QJsonObject& json)
{
    m_id = json["id"].toString();
    m_titolo = json["titolo"].toString();
    m_anno = json["anno"].toInt();
    m_descrizione = json["descrizione"].toString();
    
    QJsonArray autoriArray = json["autori"].toArray();
    m_autori.clear();
    for (const QJsonValue& value : autoriArray) {
        m_autori.append(value.toString());
    }
    
    m_rivista = json["rivista"].toString();
    m_volume = json["volume"].toString();
    m_numero = json["numero"].toString();
    m_pagine = json["pagine"].toString();
    m_categoria = static_cast<Categoria>(json["categoria"].toInt());
    m_tipo_rivista = static_cast<TipoRivista>(json["tipo_rivista"].toInt());
    m_data_pubblicazione = QDate::fromString(json["data_pubblicazione"].toString(), Qt::ISODate);
    m_doi = json["doi"].toString();
}

QString Articolo::getDisplayInfo() const
{
    return QString("Autori: %1\nRivista: %2\nVolume: %3, Numero: %4\nPagine: %5\nCategoria: %6\nTipo: %7\nData: %8\nDOI: %9")
           .arg(m_autori.join(", "))
           .arg(m_rivista, m_volume, m_numero, m_pagine)
           .arg(getCategoriaString())
           .arg(getTipoRivistaString())
           .arg(m_data_pubblicazione.toString("dd/MM/yyyy"))
           .arg(m_doi.isEmpty() ? "N/A" : m_doi);
}

QString Articolo::getTypeDisplayName() const
{
    return "Articolo";
}

bool Articolo::matchesCriteria(const QString& criteria, const QString& value) const
{
    if (criteria == "autore") {
        for (const QString& autore : m_autori) {
            if (autore.toLower().contains(value.toLower())) {
                return true;
            }
        }
        return false;
    } else if (criteria == "rivista") {
        return m_rivista.toLower().contains(value.toLower());
    } else if (criteria == "categoria") {
        return getCategoriaString().toLower().contains(value.toLower());
    } else if (criteria == "doi") {
        return m_doi.contains(value);
    }
    return false;
}

bool Articolo::isPeerReviewed() const
{
    return m_tipo_rivista == Accademica || m_tipo_rivista == Specialistica;
}

QString Articolo::getCitationFormat() const
{
    QString citation = m_autori.join(", ");
    citation += QString(" (%1). ").arg(m_anno);
    citation += QString("%1. ").arg(m_titolo);
    citation += QString("%1").arg(m_rivista);
    
    if (!m_volume.isEmpty()) {
        citation += QString(", %1").arg(m_volume);
        if (!m_numero.isEmpty()) {
            citation += QString("(%1)").arg(m_numero);
        }
    }
    
    if (!m_pagine.isEmpty()) {
        citation += QString(", %1").arg(m_pagine);
    }
    
    if (!m_doi.isEmpty()) {
        citation += QString(". DOI: %1").arg(m_doi);
    }
    
    return citation;
}

bool Articolo::isRecent() const
{
    return QDate::currentDate().year() - m_anno <= 5;
}

QString Articolo::getImpactInfo() const
{
    QString impact = "Impatto: ";
    
    if (isPeerReviewed()) {
        impact += "Alto (Peer-reviewed)";
    } else if (m_tipo_rivista == Divulgativa) {
        impact += "Medio (Divulgativo)";
    } else {
        impact += "Variabile";
    }
    
    if (isRecent()) {
        impact += " - Recente";
    }
    
    return impact;
}

int Articolo::getEstimatedReadingTime() const
{
    int pageCount = calculatePageCount();
    // Stima: 2-3 minuti per pagina per articoli accademici
    return pageCount * 3;
}

QString Articolo::categoriaToString(Categoria categoria)
{
    switch (categoria) {
        case Scientifico: return "Scientifico";
        case Tecnologico: return "Tecnologico";
        case Medico: return "Medico";
        case Economico: return "Economico";
        case Politico: return "Politico";
        case Culturale: return "Culturale";
        case Sportivo: return "Sportivo";
        case Ambiente: return "Ambiente";
        case Sociale: return "Sociale";
        case Arte: return "Arte";
        case Storia: return "Storia";
        case Filosofia: return "Filosofia";
        case Altro: return "Altro";
        default: return "Altro";
    }
}

Articolo::Categoria Articolo::stringToCategoria(const QString& str)
{
    if (str == "Scientifico") return Scientifico;
    if (str == "Tecnologico") return Tecnologico;
    if (str == "Medico") return Medico;
    if (str == "Economico") return Economico;
    if (str == "Politico") return Politico;
    if (str == "Culturale") return Culturale;
    if (str == "Sportivo") return Sportivo;
    if (str == "Ambiente") return Ambiente;
    if (str == "Sociale") return Sociale;
    if (str == "Arte") return Arte;
    if (str == "Storia") return Storia;
    if (str == "Filosofia") return Filosofia;
    return Altro;
}

QString Articolo::tipoRivistaToString(TipoRivista tipo)
{
    switch (tipo) {
        case Accademica: return "Accademica";
        case Divulgativa: return "Divulgativa";
        case Specialistica: return "Specialistica";
        case Quotidiano: return "Quotidiano";
        case Settimanale: return "Settimanale";
        case Mensile: return "Mensile";
        case Online: return "Online";
        default: return "Divulgativa";
    }
}

Articolo::TipoRivista Articolo::stringToTipoRivista(const QString& str)
{
    if (str == "Accademica") return Accademica;
    if (str == "Divulgativa") return Divulgativa;
    if (str == "Specialistica") return Specialistica;
    if (str == "Quotidiano") return Quotidiano;
    if (str == "Settimanale") return Settimanale;
    if (str == "Mensile") return Mensile;
    if (str == "Online") return Online;
    return Divulgativa;
}

QStringList Articolo::getAllCategorie()
{
    return {"Scientifico", "Tecnologico", "Medico", "Economico", "Politico", 
            "Culturale", "Sportivo", "Ambiente", "Sociale", "Arte", 
            "Storia", "Filosofia", "Altro"};
}

QStringList Articolo::getAllTipiRivista()
{
    return {"Accademica", "Divulgativa", "Specialistica", "Quotidiano", 
            "Settimanale", "Mensile", "Online"};
}

bool Articolo::validateSpecificFields() const
{
    return !m_autori.isEmpty() && 
           !m_rivista.isEmpty() && 
           m_data_pubblicazione.isValid() &&
           (m_doi.isEmpty() || isValidDoi(m_doi));
}

QString Articolo::getSearchableText() const
{
    return QString("%1 %2 %3 %4 %5 %6 %7")
           .arg(m_titolo, m_descrizione)
           .arg(m_autori.join(" "))
           .arg(m_rivista)
           .arg(getCategoriaString())
           .arg(getTipoRivistaString())
           .arg(m_doi);
}

bool Articolo::isValidDoi(const QString& doi) const
{
    if (doi.isEmpty()) return true; // DOI opzionale
    
    // Pattern base per DOI: 10.xxxx/xxxxx
    QRegularExpression doiPattern("^10\\.\\d{4,}/\\S+$");
    return doiPattern.match(doi).hasMatch();
}

int Articolo::calculatePageCount() const
{
    // Estrae il numero di pagine da stringhe come "123-130" o "45"
    QRegularExpression pageRange("(\\d+)-(\\d+)");
    QRegularExpressionMatch match = pageRange.match(m_pagine);
    
    if (match.hasMatch()) {
        int start = match.captured(1).toInt();
        int end = match.captured(2).toInt();
        return end - start + 1;
    }
    
    // Se è solo un numero, assume 1 pagina
    bool ok;
    m_pagine.toInt(&ok);
    return ok ? 1 : 10; // Default per stime
}