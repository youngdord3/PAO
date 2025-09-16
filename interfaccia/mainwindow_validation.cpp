#include "mainwindow.h"
#include "modello_logico/collezione.h"
#include "modello_logico/libro.h"
#include "modello_logico/film.h"
#include "modello_logico/articolo.h"
#include <QDate>
#include <QDebug>

bool MainWindow::validateEditInput()
{
    try {
        QStringList errors = getEditValidationErrors();
        return errors.isEmpty();
    } catch (const std::exception&) {
        return false;
    }
}

QStringList MainWindow::getEditValidationErrors()
{
    QStringList errors;
    
    try {
        // Verifica che i widget base esistano prima di controllarli
        if (!m_editTitoloEdit || !m_editAnnoSpin || !m_editTipoCombo) {
            errors << "Interfaccia non completamente inizializzata";
            return errors;
        }
        
        // Validazione base
        if (m_editTitoloEdit->text().trimmed().isEmpty()) {
            errors << "Il titolo è obbligatorio";
        }
        
        if (m_editAnnoSpin->value() <= 0) {
            errors << "L'anno deve essere positivo";
        }
        
        // Validazione specifica per tipo - solo se i widget esistono
        QString tipo = m_editTipoCombo->currentText();
        
        if (tipo == "Libro") {
            // Controlla che tutti i widget libro esistano prima di validarli
            if (!m_editLibroGroup || !m_editAutoreEdit || !m_editPagineSpin) {
                errors << "Form libro non completamente caricato";
                return errors;
            }
            
            if (m_editAutoreEdit->text().trimmed().isEmpty()) {
                errors << "L'autore è obbligatorio per i libri";
            }
            if (m_editPagineSpin->value() <= 0) {
                errors << "Il numero di pagine deve essere positivo";
            }
            
        } else if (tipo == "Film") {
            // Controlla che tutti i widget film esistano prima di validarli
            if (!m_editFilmGroup || !m_editRegistaEdit || !m_editAttoriList || !m_editDurataSpin) {
                errors << "Form film non completamente caricato";
                return errors;
            }
            
            if (m_editRegistaEdit->text().trimmed().isEmpty()) {
                errors << "Il regista è obbligatorio per i film";
            }
            if (m_editAttoriList->count() == 0) {
                errors << "Almeno un attore è richiesto";
            }
            if (m_editDurataSpin->value() <= 0) {
                errors << "La durata deve essere positiva";
            }
            
        } else if (tipo == "Articolo") {
            // Controlla che tutti i widget articolo esistano prima di validarli
            if (!m_editArticoloGroup || !m_editAutoriList || !m_editRivisteEdit || !m_editDataPubblicazioneEdit) {
                errors << "Form articolo non completamente caricato";
                return errors;
            }
            
            if (m_editAutoriList->count() == 0) {
                errors << "Almeno un autore è richiesto";
            }
            if (m_editRivisteEdit->text().trimmed().isEmpty()) {
                errors << "Il nome della rivista è obbligatorio";
            }
            if (!m_editDataPubblicazioneEdit->date().isValid()) {
                errors << "La data di pubblicazione non è valida";
            }
        } else {
            errors << "Tipo di media non riconosciuto: " + tipo;
        }
        
    } catch (const std::exception& e) {
        errors << QString("Errore nella validazione: %1").arg(e.what());
    }
    
    return errors;
}

bool MainWindow::areEditCurrentTypeWidgetsReady() const
{
    if (!m_editTipoCombo || m_editTypeChanging) {
        qDebug() << "TipoCombo non pronto o cambio tipo in corso";
        return false;
    }
    
    if (!m_editTitoloEdit || !m_editAnnoSpin || !m_editDescrizioneEdit) {
        qDebug() << "Widget base non pronti";
        return false;
    }
    
    QString tipo = m_editTipoCombo->currentText();
    qDebug() << "Controllo readiness per tipo:" << tipo;
    
    if (tipo == "Libro") {
        bool ready = m_editLibroGroup != nullptr && 
                    m_editAutoreEdit != nullptr && 
                    m_editPagineSpin != nullptr &&
                    m_editEditoreEdit != nullptr &&
                    m_editGenereLibroCombo != nullptr &&
                    m_editIsbnEdit != nullptr;
        qDebug() << "Libro ready:" << ready;
        return ready;
        
    } else if (tipo == "Film") {
        bool ready = m_editFilmGroup != nullptr && 
                    m_editRegistaEdit != nullptr && 
                    m_editAttoriList != nullptr && 
                    m_editDurataSpin != nullptr &&
                    m_editGenereFilmCombo != nullptr &&
                    m_editClassificazioneCombo != nullptr &&
                    m_editCasaProduzioneEdit != nullptr &&
                    m_editNuovoAttoreEdit != nullptr &&
                    m_editAggiungiAttoreBtn != nullptr &&
                    m_editRimuoviAttoreBtn != nullptr;
        qDebug() << "Film ready:" << ready;
        return ready;
        
    } else if (tipo == "Articolo") {
        bool ready = m_editArticoloGroup != nullptr && 
                    m_editAutoriList != nullptr && 
                    m_editRivisteEdit != nullptr && 
                    m_editDataPubblicazioneEdit != nullptr &&
                    m_editVolumeEdit != nullptr &&
                    m_editNumeroEdit != nullptr &&
                    m_editCategoriaCombo != nullptr &&
                    m_editTipoRivistaCombo != nullptr &&
                    m_editNuovoAutoreEdit != nullptr &&
                    m_editAggiungiAutoreBtn != nullptr &&
                    m_editRimuoviAutoreBtn != nullptr;
        qDebug() << "Articolo ready:" << ready;
        return ready;
    }
    
    qDebug() << "Tipo non riconosciuto:" << tipo;
    return false;
}

std::unique_ptr<Media> MainWindow::createEditMedia()
{
    if (m_editReadOnly) {
        return nullptr;
    }
    
    try {
        if (!m_editTipoCombo) return nullptr;
        
        QString tipo = m_editTipoCombo->currentText();
        std::unique_ptr<Media> media;
        
        if (tipo == "Libro") {
            media = createEditLibro();
        } else if (tipo == "Film") {
            media = createEditFilm();
        } else if (tipo == "Articolo") {
            media = createEditArticolo();
        }
        
        if (media && !m_editIsNew && !m_editingMediaId.isEmpty()) {
            media->setId(m_editingMediaId);
        }
        
        return media;
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore nella creazione media: %1").arg(e.what()));
    }
    
    return nullptr;
}

std::unique_ptr<Media> MainWindow::createEditLibro()
{
    try {
        if (!m_editAutoreEdit || !m_editEditoreEdit || !m_editPagineSpin || 
            !m_editIsbnEdit || !m_editGenereLibroCombo || !m_editTitoloEdit || 
            !m_editAnnoSpin || !m_editDescrizioneEdit) {
            return nullptr;
        }
        
        QString titolo = m_editTitoloEdit->text().trimmed();
        int anno = m_editAnnoSpin->value();
        QString descrizione = m_editDescrizioneEdit->toPlainText().trimmed();
        QString autore = m_editAutoreEdit->text().trimmed();
        QString editore = m_editEditoreEdit->text().trimmed();
        int pagine = m_editPagineSpin->value();
        QString isbn = m_editIsbnEdit->text().trimmed();
        Libro::Genere genere = Libro::stringToGenere(m_editGenereLibroCombo->currentText());
        
        auto libro = std::make_unique<Libro>(titolo, anno, descrizione, autore, 
                                           editore, pagine, isbn, genere);
        
        return libro;
    } catch (const std::exception& e) {
        qWarning() << "Errore in createEditLibro:" << e.what();
        return nullptr;
    }
}

std::unique_ptr<Media> MainWindow::createEditFilm()
{
    try {
        if (!m_editRegistaEdit || !m_editAttoriList || !m_editDurataSpin || 
            !m_editGenereFilmCombo || !m_editClassificazioneCombo || !m_editCasaProduzioneEdit ||
            !m_editTitoloEdit || !m_editAnnoSpin || !m_editDescrizioneEdit) {
            return nullptr;
        }
        
        QString titolo = m_editTitoloEdit->text().trimmed();
        int anno = m_editAnnoSpin->value();
        QString descrizione = m_editDescrizioneEdit->toPlainText().trimmed();
        QString regista = m_editRegistaEdit->text().trimmed();
        
        QStringList attori;
        for (int i = 0; i < m_editAttoriList->count(); ++i) {
            QString attore = m_editAttoriList->item(i)->text().trimmed();
            if (!attore.isEmpty()) {
                attori << attore;
            }
        }
        
        int durata = m_editDurataSpin->value();
        Film::Genere genere = Film::stringToGenere(m_editGenereFilmCombo->currentText());
        Film::Classificazione classificazione = Film::stringToClassificazione(m_editClassificazioneCombo->currentText());
        QString casaProduzione = m_editCasaProduzioneEdit->text().trimmed();
        
        auto film = std::make_unique<Film>(titolo, anno, descrizione, regista, attori, 
                                         durata, genere, classificazione, casaProduzione);
        
        return film;
    } catch (const std::exception& e) {
        qWarning() << "Errore in createEditFilm:" << e.what();
        return nullptr;
    }
}

std::unique_ptr<Media> MainWindow::createEditArticolo()
{
    try {
        if (!m_editAutoriList || !m_editRivisteEdit || !m_editVolumeEdit || !m_editNumeroEdit ||
            !m_editPagineEdit || !m_editCategoriaCombo || !m_editTipoRivistaCombo || 
            !m_editDataPubblicazioneEdit || !m_editDoiEdit ||
            !m_editTitoloEdit || !m_editAnnoSpin || !m_editDescrizioneEdit) {
            return nullptr;
        }
        
        QString titolo = m_editTitoloEdit->text().trimmed();
        int anno = m_editAnnoSpin->value();
        QString descrizione = m_editDescrizioneEdit->toPlainText().trimmed();
        
        QStringList autori;
        for (int i = 0; i < m_editAutoriList->count(); ++i) {
            autori << m_editAutoriList->item(i)->text();
        }
        
        QString rivista = m_editRivisteEdit->text().trimmed();
        QString volume = m_editVolumeEdit->text().trimmed();
        QString numero = m_editNumeroEdit->text().trimmed();
        QString pagine = m_editPagineEdit->text().trimmed();
        Articolo::Categoria categoria = Articolo::stringToCategoria(m_editCategoriaCombo->currentText());
        Articolo::TipoRivista tipoRivista = Articolo::stringToTipoRivista(m_editTipoRivistaCombo->currentText());
        QDate dataPubblicazione = m_editDataPubblicazioneEdit->date();
        QString doi = m_editDoiEdit->text().trimmed();
        
        auto articolo = std::make_unique<Articolo>(titolo, anno, descrizione, autori, rivista,
                                                 volume, numero, pagine, categoria, tipoRivista,
                                                 dataPubblicazione, doi);
        
        return articolo;
    } catch (const std::exception& e) {
        qWarning() << "Errore in createEditArticolo:" << e.what();
        return nullptr;
    }
}