#ifndef FILM_H
#define FILM_H

#include "media.h"
#include <QStringList>

class MediaCard;
class QWidget;

/**
 * @brief Classe per rappresentare un film nella collezione
 * 
 * Estende Media con attributi specifici per i film
 */
class Film : public Media
{
public:
    enum Genere {
        Azione,
        Commedia,
        Dramma,
        Horror,
        Fantascienza,
        Fantasy,
        Thriller,
        Romantico,
        Documentario,
        Animazione,
        Guerra,
        Western,
        Musicale,
        Altro
    };
    
    enum Classificazione {
        G,      // General Audiences
        PG,     // Parental Guidance
        PG13,   // Parents Strongly Cautioned
        R,      // Restricted
        NC17    // Adults Only
    };
    
    Film(const QString& titolo, int anno, const QString& descrizione,
         const QString& regista, const QStringList& attori, int durata,
         Genere genere, Classificazione classificazione, const QString& casa_produzione);
    
    Film(const QJsonObject& json);
    
    // Accessori specifici
    QString getRegista() const;
    QStringList getAttori() const;
    int getDurata() const; // in minuti
    Genere getGenere() const;
    QString getGenereString() const;
    Classificazione getClassificazione() const;
    QString getClassificazioneString() const;
    QString getCasaProduzione() const;
    
    void setRegista(const QString& regista);
    void setAttori(const QStringList& attori);
    void setDurata(int durata);
    void setGenere(Genere genere);
    void setClassificazione(Classificazione classificazione);
    void setCasaProduzione(const QString& casa_produzione);
    
    // Metodi virtuali da Media
    std::unique_ptr<Media> clone() const override;
    QJsonObject toJson() const override;
    void fromJson(const QJsonObject& json) override;
    QString getDisplayInfo() const override;
    QString getTypeDisplayName() const override;
    bool matchesCriteria(const QString& criteria, const QString& value) const override;
    
    // Metodi specifici per film
    QString getDurataFormatted() const;

    // Utility statiche
    static QString genereToString(Genere genere);
    static Genere stringToGenere(const QString& str);
    static QString classificazioneToString(Classificazione classificazione);
    static Classificazione stringToClassificazione(const QString& str);
    static QStringList getAllGeneri();
    static QStringList getAllClassificazioni();

protected:
    bool validateSpecificFields() const override;
    QString getSearchableText() const override;

private:
    QString m_regista;
    QStringList m_attori;
    int m_durata; // in minuti
    Genere m_genere;
    Classificazione m_classificazione;
    QString m_casa_produzione;
};

#endif // FILM_H