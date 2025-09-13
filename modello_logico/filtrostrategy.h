#ifndef FILTROSTRATEGY_H
#define FILTROSTRATEGY_H

#include <QString>
#include <memory>

class Media;

/**
 * @brief Pattern Strategy per i filtri di ricerca
 * 
 * Permette di implementare diversi algoritmi di filtro
 */
class FiltroStrategy
{
public:
    virtual ~FiltroStrategy() = default;
    virtual bool matches(const Media* media) const = 0;
    virtual QString getDescription() const = 0;
    virtual std::unique_ptr<FiltroStrategy> clone() const = 0;
};

/**
 * @brief Filtro per tipo di media
 */
class FiltroTipo : public FiltroStrategy
{
public:
    explicit FiltroTipo(const QString& tipo);
    bool matches(const Media* media) const override;
    QString getDescription() const override;
    std::unique_ptr<FiltroStrategy> clone() const override;

private:
    QString m_tipo;
};

/**
 * @brief Filtro per anno di pubblicazione
 */
class FiltroAnno : public FiltroStrategy
{
public:
    FiltroAnno(int annoMin, int annoMax);
    bool matches(const Media* media) const override;
    QString getDescription() const override;
    std::unique_ptr<FiltroStrategy> clone() const override;

private:
    int m_annoMin;
    int m_annoMax;
};

/**
 * @brief Filtro per criterio specifico (autore, regista, ecc.)
 */
class FiltroCriterio : public FiltroStrategy
{
public:
    FiltroCriterio(const QString& criterio, const QString& valore);
    bool matches(const Media* media) const override;
    QString getDescription() const override;
    std::unique_ptr<FiltroStrategy> clone() const override;

private:
    QString m_criterio;
    QString m_valore;
};

/**
 * @brief Filtro composto che combina più filtri
 */
class FiltroComposto : public FiltroStrategy
{
public:
    FiltroComposto();
    void addFiltro(std::unique_ptr<FiltroStrategy> filtro);
    bool matches(const Media* media) const override;
    QString getDescription() const override;
    std::unique_ptr<FiltroStrategy> clone() const override;
    
    size_t size() const;
    bool isEmpty() const;
    void clear();

private:
    std::vector<std::unique_ptr<FiltroStrategy>> m_filtri;
};

/**
 * @brief Filtro che nega un altro filtro (NOT)
 */
class FiltroNegato : public FiltroStrategy
{
public:
    explicit FiltroNegato(std::unique_ptr<FiltroStrategy> filtro);
    bool matches(const Media* media) const override;
    QString getDescription() const override;
    std::unique_ptr<FiltroStrategy> clone() const override;

private:
    std::unique_ptr<FiltroStrategy> m_filtro;
};

/**
 * @brief Factory per creare filtri comuni
 */
class FiltroFactory
{
public:
    static std::unique_ptr<FiltroStrategy> createTipoFiltro(const QString& tipo);
    static std::unique_ptr<FiltroStrategy> createAnnoFiltro(int annoMin, int annoMax);
    static std::unique_ptr<FiltroStrategy> createAutoreFiltro(const QString& autore);
    static std::unique_ptr<FiltroStrategy> createRegistaFiltro(const QString& regista);
    static std::unique_ptr<FiltroStrategy> createRivistaFiltro(const QString& rivista);
    static std::unique_ptr<FiltroStrategy> createGenereLibroFiltro(const QString& genere);
    static std::unique_ptr<FiltroStrategy> createGenereFilmFiltro(const QString& genere);
    static std::unique_ptr<FiltroStrategy> createCategoriaArticoloFiltro(const QString& categoria);
    
    // Filtri composti comuni
    static std::unique_ptr<FiltroStrategy> createLibriRecentiFiltro(int anni = 5);
    static std::unique_ptr<FiltroStrategy> createFilmLunghiFiltro();
    static std::unique_ptr<FiltroStrategy> createArticoliPeerReviewedFiltro();
};

#endif // FILTROSTRATEGY_H