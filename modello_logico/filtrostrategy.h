#ifndef FILTROSTRATEGY_H
#define FILTROSTRATEGY_H

#include <QString>
#include <memory>
#include <vector>

class Media;

/**
 * @brief Pattern Strategy per i filtri di ricerca
 */
class FiltroStrategy
{
public:
    virtual ~FiltroStrategy() = default;
    virtual bool matches(const Media* media) const = 0;
    virtual QString getDescription() const = 0;
    virtual std::unique_ptr<FiltroStrategy> clone() const = 0;
};

/* Filtro per tipo di media*/
class FiltroTipo : public FiltroStrategy
{
public:
    explicit FiltroTipo(const QString& tipo) : m_tipo(tipo) {}
    bool matches(const Media* media) const override;
    QString getDescription() const override;
    std::unique_ptr<FiltroStrategy> clone() const override;

private:
    QString m_tipo;
};

/*Filtro per anno di pubblicazione*/
class FiltroAnno : public FiltroStrategy
{
public:
    FiltroAnno(int annoMin, int annoMax) : m_annoMin(annoMin), m_annoMax(annoMax) {}
    bool matches(const Media* media) const override;
    QString getDescription() const override;
    std::unique_ptr<FiltroStrategy> clone() const override;

private:
    int m_annoMin;
    int m_annoMax;
};

/*Filtro per criterio specifico (autore, regista, ecc.)*/
class FiltroCriterio : public FiltroStrategy
{
public:
    FiltroCriterio(const QString& criterio, const QString& valore) 
        : m_criterio(criterio), m_valore(valore) {}
    bool matches(const Media* media) const override;
    QString getDescription() const override;
    std::unique_ptr<FiltroStrategy> clone() const override;

private:
    QString m_criterio;
    QString m_valore;
};

/* Filtro composto che combina più filtri*/
class FiltroComposto : public FiltroStrategy
{
public:
    FiltroComposto() = default;
    void addFiltro(std::unique_ptr<FiltroStrategy> filtro);
    bool matches(const Media* media) const override;
    QString getDescription() const override;
    std::unique_ptr<FiltroStrategy> clone() const override;
    
    size_t size() const { return m_filtri.size(); }
    bool isEmpty() const { return m_filtri.empty(); }
    void clear() { m_filtri.clear(); }

private:
    std::vector<std::unique_ptr<FiltroStrategy>> m_filtri;
};

/* Filtro che nega un altro filtro (NOT)*/
class FiltroNegato : public FiltroStrategy
{
public:
    explicit FiltroNegato(std::unique_ptr<FiltroStrategy> filtro) 
        : m_filtro(std::move(filtro)) {}
    bool matches(const Media* media) const override;
    QString getDescription() const override;
    std::unique_ptr<FiltroStrategy> clone() const override;

private:
    std::unique_ptr<FiltroStrategy> m_filtro;
};

/* Factory per creare filtri comuni*/
class FiltroFactory
{
public:
    static std::unique_ptr<FiltroStrategy> createTipoFiltro(const QString& tipo);
    static std::unique_ptr<FiltroStrategy> createAnnoFiltro(int annoMin, int annoMax);
    static std::unique_ptr<FiltroStrategy> createAutoreFiltro(const QString& autore);
    static std::unique_ptr<FiltroStrategy> createRegistaFiltro(const QString& regista);
    static std::unique_ptr<FiltroStrategy> createRivistaFiltro(const QString& rivista);
};

#endif