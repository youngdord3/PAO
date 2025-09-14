#include "filtrostrategy.h"
#include "media.h"
#include "libro.h"
#include "film.h"
#include "articolo.h"
#include <QDate>

// FiltroTipo
FiltroTipo::FiltroTipo(const QString& tipo) : m_tipo(tipo)
{
}

bool FiltroTipo::matches(const Media* media) const
{
    if (!media) return false;
    return media->getTypeDisplayName().toLower() == m_tipo.toLower();
}

QString FiltroTipo::getDescription() const
{
    return QString("Tipo: %1").arg(m_tipo);
}

std::unique_ptr<FiltroStrategy> FiltroTipo::clone() const
{
    return std::make_unique<FiltroTipo>(m_tipo);
}

// FiltroAnno
FiltroAnno::FiltroAnno(int annoMin, int annoMax) : m_annoMin(annoMin), m_annoMax(annoMax)
{
}

bool FiltroAnno::matches(const Media* media) const
{
    if (!media) return false;
    int anno = media->getAnno();
    return anno >= m_annoMin && anno <= m_annoMax;
}

QString FiltroAnno::getDescription() const
{
    if (m_annoMin == m_annoMax) {
        return QString("Anno: %1").arg(m_annoMin);
    }
    return QString("Anno: %1-%2").arg(m_annoMin).arg(m_annoMax);
}

std::unique_ptr<FiltroStrategy> FiltroAnno::clone() const
{
    return std::make_unique<FiltroAnno>(m_annoMin, m_annoMax);
}

// FiltroCriterio
FiltroCriterio::FiltroCriterio(const QString& criterio, const QString& valore)
    : m_criterio(criterio), m_valore(valore)
{
}

bool FiltroCriterio::matches(const Media* media) const
{
    if (!media) return false;
    return media->matchesCriteria(m_criterio, m_valore);
}

QString FiltroCriterio::getDescription() const
{
    return QString("%1: %2").arg(m_criterio).arg(m_valore);
}

std::unique_ptr<FiltroStrategy> FiltroCriterio::clone() const
{
    return std::make_unique<FiltroCriterio>(m_criterio, m_valore);
}

// FiltroComposto
FiltroComposto::FiltroComposto()
{
}

void FiltroComposto::addFiltro(std::unique_ptr<FiltroStrategy> filtro)
{
    if (filtro) {
        m_filtri.push_back(std::move(filtro));
    }
}

bool FiltroComposto::matches(const Media* media) const
{
    if (!media || m_filtri.empty()) return true;
    
    for (const auto& filtro : m_filtri) {
        if (!filtro->matches(media)) {
            return false;
        }
    }
    return true;
}

QString FiltroComposto::getDescription() const
{
    if (m_filtri.empty()) {
        return "Nessun filtro";
    }
    
    QStringList descrizioni;
    for (const auto& filtro : m_filtri) {
        descrizioni << filtro->getDescription();
    }
    
    return descrizioni.join(" AND ");
}

std::unique_ptr<FiltroStrategy> FiltroComposto::clone() const
{
    auto copia = std::make_unique<FiltroComposto>();
    for (const auto& filtro : m_filtri) {
        copia->addFiltro(filtro->clone());
    }
    return copia;
}

size_t FiltroComposto::size() const
{
    return m_filtri.size();
}

bool FiltroComposto::isEmpty() const
{
    return m_filtri.empty();
}

void FiltroComposto::clear()
{
    m_filtri.clear();
}

// FiltroNegato
FiltroNegato::FiltroNegato(std::unique_ptr<FiltroStrategy> filtro)
    : m_filtro(std::move(filtro))
{
}

bool FiltroNegato::matches(const Media* media) const
{
    if (!media || !m_filtro) return false;
    return !m_filtro->matches(media);
}

QString FiltroNegato::getDescription() const
{
    if (!m_filtro) return "NOT (vuoto)";
    return QString("NOT (%1)").arg(m_filtro->getDescription());
}

std::unique_ptr<FiltroStrategy> FiltroNegato::clone() const
{
    if (!m_filtro) return nullptr;
    return std::make_unique<FiltroNegato>(m_filtro->clone());
}

// FiltroFactory
std::unique_ptr<FiltroStrategy> FiltroFactory::createTipoFiltro(const QString& tipo)
{
    return std::make_unique<FiltroTipo>(tipo);
}

std::unique_ptr<FiltroStrategy> FiltroFactory::createAnnoFiltro(int annoMin, int annoMax)
{
    return std::make_unique<FiltroAnno>(annoMin, annoMax);
}

std::unique_ptr<FiltroStrategy> FiltroFactory::createAutoreFiltro(const QString& autore)
{
    return std::make_unique<FiltroCriterio>("autore", autore);
}

std::unique_ptr<FiltroStrategy> FiltroFactory::createRegistaFiltro(const QString& regista)
{
    return std::make_unique<FiltroCriterio>("regista", regista);
}

std::unique_ptr<FiltroStrategy> FiltroFactory::createRivistaFiltro(const QString& rivista)
{
    return std::make_unique<FiltroCriterio>("rivista", rivista);
}