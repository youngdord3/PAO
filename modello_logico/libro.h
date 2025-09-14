#ifndef LIBRO_H
#define LIBRO_H

#include "media.h"
#include <QStringList>

class MediaCard;
class QWidget;

/**
 * @brief Classe per rappresentare un libro nella collezione
 * 
 * Estende Media con attributi specifici per i libri
 */
class Libro : public Media
{
public:
    enum Genere {
        Romanzo,
        Saggio,
        Biografia,
        Fantascienza,
        Fantasy,
        Giallo,
        Horror,
        Storico,
        Tecnico,
        Altro
    };
    
    Libro(const QString& titolo, int anno, const QString& descrizione,
          const QString& autore, const QString& editore, int pagine, 
          const QString& isbn, Genere genere);
    
    Libro(const QJsonObject& json);
    
    // Accessori specifici
    QString getAutore() const;
    QString getEditore() const;
    int getPagine() const;
    QString getIsbn() const;
    Genere getGenere() const;
    QString getGenereString() const;
    
    void setAutore(const QString& autore);
    void setEditore(const QString& editore);
    void setPagine(int pagine);
    void setIsbn(const QString& isbn);
    void setGenere(Genere genere);
    
    // Metodi virtuali da Media
    std::unique_ptr<Media> clone() const override;
    QJsonObject toJson() const override;
    void fromJson(const QJsonObject& json) override;
    QString getDisplayInfo() const override;
    QString getTypeDisplayName() const override;
    bool matchesCriteria(const QString& criteria, const QString& value) const override;
    
    // Utility statiche
    static QString genereToString(Genere genere);
    static Genere stringToGenere(const QString& str);
    static QStringList getAllGeneri();

protected:
    bool validateSpecificFields() const override;
    QString getSearchableText() const override;

private:
    QString m_autore;
    QString m_editore;
    int m_pagine;
    QString m_isbn;
    Genere m_genere;
    
    bool isValidIsbn(const QString& isbn) const;
};

#endif // LIBRO_H