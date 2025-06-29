#include "film.h"
#include <QJsonObject>
#include <QJsonArray>

Film::Film(const QString& titolo, int anno, const QString& descrizione,
           const QString& regista, const QStringList& attori, int durata,
           Genere genere, Classificazione classificazione, const QString& casa_produzione)
    : Media(titolo, anno, descrizione), m_regista(regista), m_attori(attori),
      m_durata(durata), m_genere(genere), m_classificazione(classificazione),
      m_casa_produzione(casa_produzione)
{
}

Film::Film(const QJsonObject& json)
    : Media("", 0, "")
{
    fromJson(json);
}

QString Film::getRegista() const
{
    return m_regista;
}

QStringList Film::getAttori() const
{
    return m_attori;
}

int Film::getDurata() const
{
    return m_durata;
}

Film::Genere Film::getGenere() const
{
    return m_genere;
}

QString Film::getGenereString() const
{
    return genereToString(m_genere);
}

Film::Classificazione Film::getClassificazione() const
{
    return m_classificazione;
}

QString Film::getClassificazioneString() const
{
    return classificazioneToString(m_classificazione);
}

QString Film::getCasaProduzione() const
{
    return m_casa_produzione;
}

void Film::setRegista(const QString& regista)
{
    m_regista = regista;
}

void Film::setAttori(const QStringList& attori)
{
    m_attori = attori;
}

void Film::setDurata(int durata)
{
    m_durata = durata;
}

void Film::setGenere(Genere genere)
{
    m_genere = genere;
}

void Film::setClassificazione(Classificazione classificazione)
{
    m_classificazione = classificazione;
}

void Film::setCasaProduzione(const QString& casa_produzione)
{
    m_casa_produzione = casa_produzione;
}

std::unique_ptr<Media> Film::clone() const
{
    return std::make_unique<Film>(m_titolo, m_anno, m_descrizione, m_regista, m_attori, m_durata, m_genere, m_classificazione, m_casa_produzione);
}

QJsonObject Film::toJson() const
{
    QJsonObject json;
    json["type"] = "film";
    json["id"] = m_id;
    json["titolo"] = m_titolo;
    json["anno"] = m_anno;
    json["descrizione"] = m_descrizione;
    json["regista"] = m_regista;
    
    QJsonArray attoriArray;
    for (const QString& attore : m_attori) {
        attoriArray.append(attore);
    }
    json["attori"] = attoriArray;
    
    json["durata"] = m_durata;
    json["genere"] = static_cast<int>(m_genere);
    json["classificazione"] = static_cast<int>(m_classificazione);
    json["casa_produzione"] = m_casa_produzione;
    
    return json;
}

void Film::fromJson(const QJsonObject& json)
{
    m_id = json["id"].toString();
    m_titolo = json["titolo"].toString();
    m_anno = json["anno"].toInt();
    m_descrizione = json["descrizione"].toString();
    m_regista = json["regista"].toString();
    
    QJsonArray attoriArray = json["attori"].toArray();
    m_attori.clear();
    for (const QJsonValue& value : attoriArray) {
        m_attori.append(value.toString());
    }
    
    m_durata = json["durata"].toInt();
    m_genere = static_cast<Genere>(json["genere"].toInt());
    m_classificazione = static_cast<Classificazione>(json["classificazione"].toInt());
    m_casa_produzione = json["casa_produzione"].toString();
}

QString Film::getDisplayInfo() const
{
    return QString("Regista: %1\nAttori: %2\nDurata: %3\nGenere: %4\nClassificazione: %5\nCasa di Produzione: %6")
           .arg(m_regista)
           .arg(m_attori.join(", "))
           .arg(getDurataFormatted())
           .arg(getGenereString())
           .arg(getClassificazioneString())
           .arg(m_casa_produzione);
}

QString Film::getTypeDisplayName() const
{
    return "Film";
}

bool Film::matchesCriteria(const QString& criteria, const QString& value) const
{
    if (criteria == "regista") {
        return m_regista.toLower().contains(value.toLower());
    } else if (criteria == "attore") {
        for (const QString& attore : m_attori) {
            if (attore.toLower().contains(value.toLower())) {
                return true;
            }
        }
        return false;
    } else if (criteria == "genere") {
        return getGenereString().toLower().contains(value.toLower());
    } else if (criteria == "casa_produzione") {
        return m_casa_produzione.toLower().contains(value.toLower());
    }
    return false;
}

bool Film::isLongMovie() const
{
    return m_durata > 150; // Film lunghi sopra le 2.5 ore
}

QString Film::getDurataFormatted() const
{
    int ore = m_durata / 60;
    int minuti = m_durata % 60;
    
    if (ore > 0) {
        return QString("%1h %2min").arg(ore).arg(minuti);
    } else {
        return QString("%1min").arg(minuti);
    }
}

bool Film::isAdultContent() const
{
    return m_classificazione == R || m_classificazione == NC17;
}

QString Film::getRating() const
{
    // Sistema di rating basato su genere e classificazione
    int rating = 5; // Base rating
    
    if (m_genere == Documentario || m_genere == Dramma) rating += 1;
    if (m_classificazione == G || m_classificazione == PG) rating += 1;
    if (m_durata > 90 && m_durata < 150) rating += 1; // Durata ottimale
    
    return QString("%1/10").arg(qMin(rating, 10));
}

QString Film::genereToString(Genere genere)
{
    switch (genere) {
        case Azione: return "Azione";
        case Commedia: return "Commedia";
        case Dramma: return "Dramma";
        case Horror: return "Horror";
        case Fantascienza: return "Fantascienza";
        case Fantasy: return "Fantasy";
        case Thriller: return "Thriller";
        case Romantico: return "Romantico";
        case Documentario: return "Documentario";
        case Animazione: return "Animazione";
        case Guerra: return "Guerra";
        case Western: return "Western";
        case Musicale: return "Musicale";
        case Altro: return "Altro";
        default: return "Altro";
    }
}

Film::Genere Film::stringToGenere(const QString& str)
{
    if (str == "Azione") return Azione;
    if (str == "Commedia") return Commedia;
    if (str == "Dramma") return Dramma;
    if (str == "Horror") return Horror;
    if (str == "Fantascienza") return Fantascienza;
    if (str == "Fantasy") return Fantasy;
    if (str == "Thriller") return Thriller;
    if (str == "Romantico") return Romantico;
    if (str == "Documentario") return Documentario;
    if (str == "Animazione") return Animazione;
    if (str == "Guerra") return Guerra;
    if (str == "Western") return Western;
    if (str == "Musicale") return Musicale;
    return Altro;
}

QString Film::classificazioneToString(Classificazione classificazione)
{
    switch (classificazione) {
        case G: return "G (Tutti)";
        case PG: return "PG (Supervisione Genitori)";
        case PG13: return "PG-13 (Sconsigliato sotto i 13)";
        case R: return "R (Vietato ai minori)";
        case NC17: return "NC-17 (Solo adulti)";
        default: return "G (Tutti)";
    }
}

Film::Classificazione Film::stringToClassificazione(const QString& str)
{
    if (str.contains("PG-13")) return PG13;
    if (str.contains("PG")) return PG;
    if (str.contains("NC-17")) return NC17;
    if (str.contains("R")) return R;
    return G;
}

QStringList Film::getAllGeneri()
{
    return {"Azione", "Commedia", "Dramma", "Horror", "Fantascienza", 
            "Fantasy", "Thriller", "Romantico", "Documentario", 
            "Animazione", "Guerra", "Western", "Musicale", "Altro"};
}

QStringList Film::getAllClassificazioni()
{
    return {"G (Tutti)", "PG (Supervisione Genitori)", "PG-13 (Sconsigliato sotto i 13)", 
            "R (Vietato ai minori)", "NC-17 (Solo adulti)"};
}

bool Film::validateSpecificFields() const
{
    return !m_regista.isEmpty() && 
           m_durata > 0 && 
           !m_attori.isEmpty();
}

QString Film::getSearchableText() const
{
    return QString("%1 %2 %3 %4 %5 %6")
           .arg(m_titolo, m_descrizione, m_regista)
           .arg(m_attori.join(" "))
           .arg(getGenereString(), m_casa_produzione);
}