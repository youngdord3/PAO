#ifndef ARTICOLO_H
#define ARTICOLO_H

#include "media.h"
#include <QDate>
#include <QStringList>

/**
 * @brief Classe per rappresentare un articolo di rivista nella collezione
 * 
 * Estende Media con attributi specifici per gli articoli accademici/giornalistici
 */
class Articolo : public Media
{
public:
    enum Categoria {
        Scientifico,
        Tecnologico,
        Medico,
        Economico,
        Politico,
        Culturale,
        Sportivo,
        Ambiente,
        Sociale,
        Arte,
        Storia,
        Filosofia,
        Altro
    };
    
    enum TipoRivista {
        Accademica,
        Divulgativa,
        Specialistica,
        Quotidiano,
        Settimanale,
        Mensile,
        Online
    };
    
    Articolo(const QString& titolo, int anno, const QString& descrizione,
             const QStringList& autori, const QString& rivista, 
             const QString& volume, const QString& numero, 
             const QString& pagine, Categoria categoria, TipoRivista tipo_rivista,
             const QDate& data_pubblicazione, const QString& doi = "");
    
    Articolo(const QJsonObject& json);
    
    // Accessori specifici
    QStringList getAutori() const;
    QString getRivista() const;
    QString getVolume() const;
    QString getNumero() const;
    QString getPagine() const;
    Categoria getCategoria() const;
    QString getCategoriaString() const;
    TipoRivista getTipoRivista() const;
    QString getTipoRivistaString() const;
    QDate getDataPubblicazione() const;
    QString getDoi() const;
    
    void setAutori(const QStringList& autori);
    void setRivista(const QString& rivista);
    void setVolume(const QString& volume);
    void setNumero(const QString& numero);
    void setPagine(const QString& pagine);
    void setCategoria(Categoria categoria);
    void setTipoRivista(TipoRivista tipo_rivista);
    void setDataPubblicazione(const QDate& data);
    void setDoi(const QString& doi);
    
    // Metodi virtuali da Media
    std::unique_ptr<Media> clone() const override;
    QJsonObject toJson() const override;
    void fromJson(const QJsonObject& json) override;
    QString getDisplayInfo() const override;
    QString getTypeDisplayName() const override;
    bool matchesCriteria(const QString& criteria, const QString& value) const override;
    
    // Metodi specifici per articolo
    bool isPeerReviewed() const;
    QString getCitationFormat() const;
    bool isRecent() const; // Pubblicato negli ultimi 5 anni
    QString getImpactInfo() const;
    int getEstimatedReadingTime() const; // Basato sul numero di pagine
    
    // Utility statiche
    static QString categoriaToString(Categoria categoria);
    static Categoria stringToCategoria(const QString& str);
    static QString tipoRivistaToString(TipoRivista tipo);
    static TipoRivista stringToTipoRivista(const QString& str);
    static QStringList getAllCategorie();
    static QStringList getAllTipiRivista();

protected:
    bool validateSpecificFields() const override;
    QString getSearchableText() const override;

private:
    QStringList m_autori;
    QString m_rivista;
    QString m_volume;
    QString m_numero;
    QString m_pagine;
    Categoria m_categoria;
    TipoRivista m_tipo_rivista;
    QDate m_data_pubblicazione;
    QString m_doi; // Digital Object Identifier
    
    bool isValidDoi(const QString& doi) const;
    int calculatePageCount() const;
};

#endif // ARTICOLO_H