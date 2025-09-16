#include "mainwindow.h"
#include "modello_logico/collezione.h"
#include "modello_logico/libro.h"
#include "modello_logico/film.h"
#include "modello_logico/articolo.h"
#include <QApplication>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>
#include <QDate>

void MainWindow::showEditPanel(bool isNew, bool readOnly)
{
    try {
        // Reset completo necessario quando si cambia modalità  
        bool wasVisible = m_editPanelVisible;
        if (wasVisible && ((m_editIsNew != isNew) || (m_editReadOnly != readOnly))) {
            resetEditPanelState();
        }
        
        m_editIsNew = isNew;
        m_editReadOnly = readOnly;
        
        m_editValidationEnabled = false;
        
        // Nascondi l'area media e mostra il pannello edit
        m_mediaScrollArea->setVisible(false);
        m_editPanel->setVisible(true);
        m_editPanelVisible = true;
        
        // Aggiorna il titolo
        QString title = readOnly ? "Dettagli Media" : (isNew ? "Nuovo Media" : "Modifica Media");
        if (m_editHeaderLabel) {
            m_editHeaderLabel->setText(title);
        }
        
        // Reset stato validazione
        if (m_editValidationLabel) {
            m_editValidationLabel->setText("Preparazione interfaccia...");
            m_editValidationLabel->setStyleSheet("color: #757575;");
            m_editValidationLabel->setVisible(true);
        }
        
        // Aggiorna il testo dei pulsanti
        if (m_editSalvaButton) {
            m_editSalvaButton->setText(readOnly ? "Chiudi" : (isNew ? "Crea" : "Salva"));
            m_editSalvaButton->setEnabled(readOnly);
        }
        
        if (m_editAnnullaButton) {
            if (readOnly) {
                m_editAnnullaButton->setVisible(false);
            } else {
                m_editAnnullaButton->setVisible(true);
                m_editAnnullaButton->setText("Annulla");
                m_editAnnullaButton->setEnabled(true);
            }
        }
        
        if (isNew) {
            clearEditSpecificForm();
            if (m_editTipoCombo) {
                m_editTipoCombo->blockSignals(true);
                m_editTipoCombo->setCurrentIndex(0);
                m_editTipoCombo->blockSignals(false);
            }
            
            // Reset campi base
            if (m_editTitoloEdit) {
                m_editTitoloEdit->blockSignals(true);
                m_editTitoloEdit->clear();
                m_editTitoloEdit->blockSignals(false);
            }
            if (m_editAnnoSpin) {
                m_editAnnoSpin->blockSignals(true);
                m_editAnnoSpin->setValue(QDate::currentDate().year());
                m_editAnnoSpin->blockSignals(false);
            }
            if (m_editDescrizioneEdit) {
                m_editDescrizioneEdit->blockSignals(true);
                m_editDescrizioneEdit->clear();
                m_editDescrizioneEdit->blockSignals(false);
            }
            
            // Trigger cambio tipo
            QTimer::singleShot(10, this, [this, readOnly]() {
                onEditTipoChanged();
                
                QTimer::singleShot(800, this, [this, readOnly]() {
                    if (!readOnly) {
                        m_editValidationEnabled = true;
                        enableEditForm(true);
                        
                        // Prima validazione
                        QTimer::singleShot(200, this, [this]() {
                            if (m_editValidationEnabled && !m_editTypeChanging) {
                                onEditValidationChanged();
                            }
                        });
                    } else {
                        m_editValidationEnabled = false;
                        enableEditForm(false);
                    }
                });
            });
        } else {
            if (!readOnly) {
                // Modalità modifica
                QTimer::singleShot(300, this, [this]() {
                    m_editValidationEnabled = true;
                    QTimer::singleShot(100, this, [this]() {
                        if (m_editValidationEnabled && !m_editTypeChanging) {
                            onEditValidationChanged();
                        }
                    });
                });
            } else {
                // Modalità read-only
                m_editValidationEnabled = false;
                if (m_editValidationLabel) {
                    m_editValidationLabel->setVisible(false);
                }
            }
        }

        if (!readOnly && m_editTitoloEdit) {
            QTimer::singleShot(1000, [this]() {
                if (m_editTitoloEdit && m_editPanel && m_editPanel->isVisible()) {
                    m_editTitoloEdit->setFocus();
                }
            });
        }
        
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore nell'apertura pannello: %1").arg(e.what()));
    }
}

void MainWindow::hideEditPanel()
{
    try {
        if (!m_editPanel || !m_mediaScrollArea) {
            return;
        }

        m_editPanel->setVisible(false);
        m_mediaScrollArea->setVisible(true);
        m_editPanelVisible = false;
        m_editingMediaId.clear();
        m_editIsNew = false;
        m_editReadOnly = false;
        
        // Reset stato validazione
        m_editValidationEnabled = true;
        
        // Refresh delle card per aggiornare eventuali modifiche
        QTimer::singleShot(50, this, &MainWindow::refreshMediaCards);
        
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore nella chiusura pannello: %1").arg(e.what()));
    }
}

void MainWindow::resetEditPanelState()
{
    try {
        // Disabilità validazione
        m_editValidationEnabled = false;
        
        // Reset stato bottoni
        if (m_editSalvaButton) {
            m_editSalvaButton->setEnabled(true);
            m_editSalvaButton->setText("Salva");
        }
        
        if (m_editAnnullaButton) {
            m_editAnnullaButton->setEnabled(true);
            m_editAnnullaButton->setVisible(true);
            m_editAnnullaButton->setText("Annulla");
        }
        
        // Reset label validazione
        if (m_editValidationLabel) {
            m_editValidationLabel->setText("");
            m_editValidationLabel->setVisible(false);
            m_editValidationLabel->setStyleSheet("");
        }
        
        // Reset ID editing
        m_editingMediaId.clear();
        
        // Reset flags
        m_editIsNew = false;
        m_editReadOnly = false;
        
    } catch (const std::exception& e) {
        qWarning() << "Errore in resetEditPanelState:" << e.what();
    }
}

void MainWindow::loadEditMediaData(Media* media)
{
    if (!media) return;
    
    try {
        if (m_editAttoriList) {
            m_editAttoriList->clear();
        }
        if (m_editAutoriList) {
            m_editAutoriList->clear();
        }
        
        // Dati base
        if (m_editTitoloEdit) {
            m_editTitoloEdit->setText(media->getTitolo());
        }
        if (m_editAnnoSpin) {
            m_editAnnoSpin->setValue(media->getAnno());
        }
        if (m_editDescrizioneEdit) {
            m_editDescrizioneEdit->setPlainText(media->getDescrizione());
        }
        
        // Dati specifici
        QString tipo = media->getTypeDisplayName();
        if (m_editTipoCombo) {
            m_editTipoCombo->setCurrentText(tipo);
        }
        m_editTipoCorrente = tipo;
        
        setupEditTypeSpecificForm();
        
        // Dati specifici per tipo
        if (tipo == "Libro") {
            Libro* libro = dynamic_cast<Libro*>(media);
            if (libro) {
                if (m_editAutoreEdit) m_editAutoreEdit->setText(libro->getAutore());
                if (m_editEditoreEdit) m_editEditoreEdit->setText(libro->getEditore());
                if (m_editPagineSpin) m_editPagineSpin->setValue(libro->getPagine());
                if (m_editIsbnEdit) m_editIsbnEdit->setText(libro->getIsbn());
                if (m_editGenereLibroCombo) m_editGenereLibroCombo->setCurrentText(libro->getGenereString());
            }
        } else if (tipo == "Film") {
            Film* film = dynamic_cast<Film*>(media);
            if (film) {
                if (m_editRegistaEdit) m_editRegistaEdit->setText(film->getRegista());
                if (m_editAttoriList) m_editAttoriList->addItems(film->getAttori());
                if (m_editDurataSpin) m_editDurataSpin->setValue(film->getDurata());
                if (m_editGenereFilmCombo) m_editGenereFilmCombo->setCurrentText(film->getGenereString());
                if (m_editClassificazioneCombo) m_editClassificazioneCombo->setCurrentText(film->getClassificazioneString());
                if (m_editCasaProduzioneEdit) m_editCasaProduzioneEdit->setText(film->getCasaProduzione());
            }
        } else if (tipo == "Articolo") {
            Articolo* articolo = dynamic_cast<Articolo*>(media);
            if (articolo) {
                if (m_editAutoriList) m_editAutoriList->addItems(articolo->getAutori());
                if (m_editRivisteEdit) m_editRivisteEdit->setText(articolo->getRivista());
                if (m_editVolumeEdit) m_editVolumeEdit->setText(articolo->getVolume());
                if (m_editNumeroEdit) m_editNumeroEdit->setText(articolo->getNumero());
                if (m_editPagineEdit) m_editPagineEdit->setText(articolo->getPagine());
                if (m_editCategoriaCombo) m_editCategoriaCombo->setCurrentText(articolo->getCategoriaString());
                if (m_editTipoRivistaCombo) m_editTipoRivistaCombo->setCurrentText(articolo->getTipoRivistaString());
                if (m_editDataPubblicazioneEdit) m_editDataPubblicazioneEdit->setDate(articolo->getDataPubblicazione());
                if (m_editDoiEdit) m_editDoiEdit->setText(articolo->getDoi());
            }
        }
        
        updateEditFormVisibility();
        
        QTimer::singleShot(10, this, [this]() {
            enableEditForm(!m_editReadOnly);
        });
        
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore nel caricamento dati: %1").arg(e.what()));
    }
}

void MainWindow::onEditTipoChanged()
{
    try {
        if (!m_editTipoCombo || m_editTypeChanging) {
            return;
        }
        
        QString nuovoTipo = m_editTipoCombo->currentText();
        qDebug() << "Cambio tipo da" << m_editTipoCorrente << "a" << nuovoTipo;
        
        if (nuovoTipo != m_editTipoCorrente || m_editIsNew) {
            // Previeni loop
            m_editTypeChanging = true;
            
            // Disabilita validazione temporaneamente
            m_editValidationEnabled = false;
            
            // Reset stato interfaccia
            if (m_editValidationLabel) {
                m_editValidationLabel->setText("Caricamento...");
                m_editValidationLabel->setStyleSheet("color: #757575;");
                m_editValidationLabel->setVisible(true);
            }
            
            if (m_editSalvaButton) {
                m_editSalvaButton->setEnabled(false);
            }
            
            m_editTipoCorrente = nuovoTipo;
            
            // Ricrea il form specifico
            setupEditTypeSpecificForm();
            updateEditFormVisibility();
            
            // Riabilita tutto dopo un delay
            QTimer::singleShot(300, this, [this]() {
                m_editTypeChanging = false;
                
                if (!m_editReadOnly) {
                    m_editValidationEnabled = true;
                    setupEditSpecificConnections();
                    
                    // Trigger validazione iniziale dopo delay
                    QTimer::singleShot(200, this, [this]() {
                        if (m_editValidationEnabled && !m_editTypeChanging) {
                            onEditValidationChanged();
                        }
                    });
                } else {
                    m_editValidationEnabled = false;
                    if (m_editValidationLabel) {
                        m_editValidationLabel->setVisible(false);
                    }
                }
            });
        }
    } catch (const std::exception& e) {
        m_editTypeChanging = false;
        mostraErrore(QString("Errore nel cambio tipo: %1").arg(e.what()));
    }
}

void MainWindow::onEditSalvaClicked()
{
    try {
        if (m_editReadOnly) {
            hideEditPanel();
            return;
        }
        
        // Prima controlla la validazione custom del form
        if (!validateEditInput()) {
            QStringList errors = getEditValidationErrors();
            QMessageBox::warning(this, "Errori di Validazione", 
                                "Correggere i seguenti errori:\n\n" + errors.join("\n"));
            return;
        }
        
        // Poi crea il media e controlla se è completo e valido
        auto media = createEditMedia();
        if (!media) {
            mostraErrore("Impossibile creare il media - errore interno");
            return;
        }
        
        if (!media->isCompleteAndValid()) {
            mostraErrore("Media non valido - verificare i dati inseriti");
            return;
        }
        
        if (m_editIsNew) {
            m_collezione->addMedia(std::move(media));
            m_modificato = true;
            mostraInfo("Media aggiunto con successo");
        } else {
            // Modifica media esistente
            if (m_collezione->updateMedia(m_editingMediaId, std::move(media))) {
                m_modificato = true;
                mostraInfo("Media modificato con successo");
            } else {
                mostraErrore("Impossibile aggiornare il media");
                return;
            }
        }
        
        aggiornaStatusBar();
        hideEditPanel();
        
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore nel salvataggio: %1").arg(e.what()));
    }
}

void MainWindow::onEditAnnullaClicked()
{
    hideEditPanel();
}

void MainWindow::onEditValidationChanged()
{
    try {
        if (!m_editValidationEnabled || 
            m_editReadOnly || 
            m_editTypeChanging ||
            !m_editSalvaButton || 
            !m_editValidationLabel ||
            !m_editTipoCombo ||
            !m_editPanel ||
            !m_editPanel->isVisible()) {
            qDebug() << "Skip validazione - condizioni non soddisfatte";
            return;
        }
        
        // Verifica che i widget siano pronti
        if (!areEditCurrentTypeWidgetsReady()) {
            qDebug() << "Widget non pronti per validazione";
            if (m_editSalvaButton) m_editSalvaButton->setEnabled(false);
            if (m_editValidationLabel) {
                m_editValidationLabel->setText("Caricamento interfaccia...");
                m_editValidationLabel->setStyleSheet("color: #757575;");
                m_editValidationLabel->setVisible(true);
            }
            
            if (m_editIsNew) {
                QTimer::singleShot(500, this, [this]() {
                    if (m_editValidationEnabled && !m_editTypeChanging && !m_editReadOnly) {
                        onEditValidationChanged();
                    }
                });
            }
            return;
        }
        
        qDebug() << "Eseguo validazione per tipo:" << m_editTipoCombo->currentText();
        
        // Esegui validazione
        bool valid = validateEditInput();
        QStringList errors = getEditValidationErrors();
        
        qDebug() << "Validazione completata - Valido:" << valid << "Errori:" << errors.size();
        if (!errors.isEmpty()) {
            qDebug() << "Errori trovati:" << errors.join("; ");
        }

        m_editSalvaButton->setEnabled(valid);
        m_editValidationLabel->setVisible(true);
        
        if (valid) {
            m_editValidationLabel->setText("✓ Tutti i campi sono validi");
            m_editValidationLabel->setStyleSheet("color: #4CAF50; font-weight: bold;");
        } else {
            QString errorText = QString("⚠ Errori (%1):").arg(errors.size());
            if (!errors.isEmpty()) {
                QStringList mainErrors = errors.mid(0, 3);
                errorText += "\n• " + mainErrors.join("\n• ");
                if (errors.size() > 3) {
                    errorText += QString("\n• ... e altri %1 errori").arg(errors.size() - 3);
                }
            }
            m_editValidationLabel->setText(errorText);
            m_editValidationLabel->setStyleSheet("color: #F44336; font-weight: bold;");
        }
        
    } catch (const std::exception& e) {
        qWarning() << "Errore nella validazione:" << e.what();
        if (m_editSalvaButton) m_editSalvaButton->setEnabled(false);
        if (m_editValidationLabel) {
            m_editValidationLabel->setText("Errore nella validazione: " + QString(e.what()));
            m_editValidationLabel->setStyleSheet("color: #F44336;");
        }
    }
}

void MainWindow::scheduleValidation()
{
    if (!m_editValidationEnabled || 
        m_editTypeChanging || 
        m_editReadOnly ||
        !m_editPanel ||
        !m_editPanel->isVisible()) {
        qDebug() << "Skip schedule validation - condizioni non soddisfatte";
        return;
    }
    
    if (!m_editTitoloEdit || !m_editAnnoSpin || !m_editTipoCombo) {
        qDebug() << "Widget base non pronti per validazione";
        return;
    }
    
    if (!m_validationTimer) {
        qDebug() << "Creo validation timer";
        m_validationTimer = new QTimer(this);
        m_validationTimer->setSingleShot(true);
        m_validationTimer->setInterval(100);
        connect(m_validationTimer, &QTimer::timeout, this, [this]() {
            qDebug() << "Timer validazione scaduto";
            if (m_editValidationEnabled && 
                !m_editTypeChanging && 
                !m_editReadOnly &&
                m_editPanel && 
                m_editPanel->isVisible()) {
                onEditValidationChanged();
            }
        });
    }
    
    qDebug() << "Schedule validazione tra 100ms";
    m_validationTimer->start();
}

void MainWindow::onEditAggiungiAutoreClicked()
{
    try {
        qDebug() << "onEditAggiungiAutoreClicked chiamato";
        
        if (!m_editNuovoAutoreEdit || !m_editAutoriList) {
            qWarning() << "Widget autori non inizializzati";
            QMessageBox::warning(this, "Errore", "Interfaccia non pronta. Riprova tra qualche secondo.");
            return;
        }
        
        QString autore = m_editNuovoAutoreEdit->text().trimmed();
        if (autore.isEmpty()) {
            QMessageBox::information(this, "Info", "Inserire il nome dell'autore");
            m_editNuovoAutoreEdit->setFocus();
            return;
        }

        // Controlla duplicati
        for (int i = 0; i < m_editAutoriList->count(); ++i) {
            if (m_editAutoriList->item(i)->text().trimmed() == autore) {
                QMessageBox::information(this, "Info", 
                    QString("L'autore '%1' è già presente nella lista").arg(autore));
                m_editNuovoAutoreEdit->clear();
                m_editNuovoAutoreEdit->setFocus();
                return;
            }
        }

        // Aggiungi l'autore
        m_editAutoriList->addItem(autore);
        m_editNuovoAutoreEdit->clear();
        m_editNuovoAutoreEdit->setFocus();
        
        qDebug() << "Autore aggiunto:" << autore << "- Totale:" << m_editAutoriList->count();
        
        // Trigger validazione immediata
        QTimer::singleShot(50, this, [this]() {
            if (m_editPanel && m_editPanel->isVisible() && m_editValidationEnabled) {
                qDebug() << "Trigger validazione dopo aggiunta autore";
                onEditValidationChanged();
            }
        });
        
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore nell'aggiunta autore: %1").arg(e.what()));
    }
}

void MainWindow::onEditRimuoviAutoreClicked()
{
    try {
        if (!m_editAutoriList) {
            qWarning() << "Widget lista autori non inizializzato";
            return;
        }
        
        int row = m_editAutoriList->currentRow();
        
        // Verifica che ci sia una selezione
        if (row < 0) {
            QMessageBox::information(this, "Info", 
                "Seleziona un autore dalla lista per rimuoverlo");
            return;
        }
        
        // Verifica che ci sia almeno un elemento nella lista
        if (m_editAutoriList->count() == 0) {
            QMessageBox::information(this, "Info", "La lista degli autori è vuota");
            return;
        }
        
        // Ottieni il nome dell'autore da rimuovere
        QListWidgetItem* item = m_editAutoriList->item(row);
        QString nomeAutore = item ? item->text() : "Sconosciuto";
        
        QListWidgetItem* itemDaRimuovere = m_editAutoriList->takeItem(row);
        if (itemDaRimuovere) {
            delete itemDaRimuovere;
            qDebug() << "Autore rimosso:" << nomeAutore << "- Autori rimanenti:" << m_editAutoriList->count();
            
            // Seleziona l'elemento successivo o precedente se disponibile
            if (m_editAutoriList->count() > 0) {
                int nuovaRow = qMin(row, m_editAutoriList->count() - 1);
                m_editAutoriList->setCurrentRow(nuovaRow);
            }

            // Trigger validazione
            if (m_editValidationEnabled && !m_editTypeChanging) {
                QTimer::singleShot(100, this, [this]() {
                    if (m_editValidationEnabled && !m_editTypeChanging && m_editAutoriList) {
                        qDebug() << "Trigger validazione dopo rimozione autore";
                        onEditValidationChanged();
                    }
                });
            }
        } else {
            qWarning() << "Impossibile rimuovere l'autore dalla lista";
        }
        
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore nella rimozione autore: %1").arg(e.what()));
    }
}

void MainWindow::onEditAggiungiAttoreClicked()
{
    try {
        qDebug() << "onEditAggiungiAttoreClicked chiamato";
        
        if (!m_editNuovoAttoreEdit || !m_editAttoriList) {
            qWarning() << "Widget attori non inizializzati";
            QMessageBox::warning(this, "Errore", "Interfaccia non pronta. Riprova tra qualche secondo.");
            return;
        }
        
        QString attore = m_editNuovoAttoreEdit->text().trimmed();
        if (attore.isEmpty()) {
            QMessageBox::information(this, "Info", "Inserire il nome dell'attore");
            m_editNuovoAttoreEdit->setFocus();
            return;
        }
        
        // Controlla duplicati
        for (int i = 0; i < m_editAttoriList->count(); ++i) {
            if (m_editAttoriList->item(i)->text().trimmed() == attore) {
                QMessageBox::information(this, "Info", 
                    QString("L'attore '%1' è già presente nella lista").arg(attore));
                m_editNuovoAttoreEdit->clear();
                m_editNuovoAttoreEdit->setFocus();
                return;
            }
        }
        
        // Aggiungi l'attore
        m_editAttoriList->addItem(attore);
        m_editNuovoAttoreEdit->clear();
        m_editNuovoAttoreEdit->setFocus();
        
        qDebug() << "Attore aggiunto:" << attore << "- Totale:" << m_editAttoriList->count();
        
        // Trigger validazione immediata
        QTimer::singleShot(50, this, [this]() {
            if (m_editPanel && m_editPanel->isVisible() && m_editValidationEnabled) {
                qDebug() << "Trigger validazione dopo aggiunta attore";
                onEditValidationChanged();
            }
        });
        
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore nell'aggiunta attore: %1").arg(e.what()));
    }
}

void MainWindow::onEditRimuoviAttoreClicked()
{
    try {
        if (!m_editAttoriList) return;
        
        int row = m_editAttoriList->currentRow();
        if (row >= 0) {
            QListWidgetItem* item = m_editAttoriList->takeItem(row);
            if (item) {
                delete item;
                
                // Trigger validazione
                if (m_editValidationEnabled && !m_editTypeChanging) {
                    QTimer::singleShot(100, this, [this]() {
                        if (m_editValidationEnabled && !m_editTypeChanging) {
                            onEditValidationChanged();
                        }
                    });
                }
            }
        } else {
            QMessageBox::information(this, "Info", "Seleziona un attore da rimuovere");
        }
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore nella rimozione attore: %1").arg(e.what()));
    }
}

void MainWindow::updateEditFormVisibility()
{
    onEditValidationChanged();
}

void MainWindow::enableEditForm(bool enabled)
{
    try {
        if (m_editScrollArea) {
            m_editScrollArea->setEnabled(enabled);
        }
        
        if (m_editReadOnly) {
            if (m_editTipoCombo) m_editTipoCombo->setEnabled(false);
            if (m_editAnnullaButton) m_editAnnullaButton->setVisible(false);
            if (m_editHelpButton) m_editHelpButton->setVisible(false);
            if (m_editSalvaButton) {
                m_editSalvaButton->setText("Chiudi");
                m_editSalvaButton->setEnabled(true);
            }
            
            hideManagementControls();
            
            if (m_editValidationLabel) {
                m_editValidationLabel->setText("");
                m_editValidationLabel->setVisible(false);
            }
        } else {
            if (m_editTipoCombo) {
                m_editTipoCombo->setEnabled(enabled);
            }
            
            showManagementControls();
        }
    } catch (const std::exception& e) {
        qWarning() << "Errore in enableEditForm:" << e.what();
    }
}

void MainWindow::hideManagementControls()
{
    // Film
    if (m_editNuovoAttoreEdit) m_editNuovoAttoreEdit->setVisible(false);
    if (m_editAggiungiAttoreBtn) m_editAggiungiAttoreBtn->setVisible(false);
    if (m_editRimuoviAttoreBtn) m_editRimuoviAttoreBtn->setVisible(false);
    
    // Articolo
    if (m_editNuovoAutoreEdit) m_editNuovoAutoreEdit->setVisible(false);
    if (m_editAggiungiAutoreBtn) m_editAggiungiAutoreBtn->setVisible(false);
    if (m_editRimuoviAutoreBtn) m_editRimuoviAutoreBtn->setVisible(false);
}

void MainWindow::showManagementControls()
{
    QString tipo = m_editTipoCombo ? m_editTipoCombo->currentText() : "";
    
    if (tipo == "Film") {
        if (m_editNuovoAttoreEdit) m_editNuovoAttoreEdit->setVisible(true);
        if (m_editAggiungiAttoreBtn) m_editAggiungiAttoreBtn->setVisible(true);
        if (m_editRimuoviAttoreBtn) m_editRimuoviAttoreBtn->setVisible(true);
    } else if (tipo == "Articolo") {
        if (m_editNuovoAutoreEdit) m_editNuovoAutoreEdit->setVisible(true);
        if (m_editAggiungiAutoreBtn) m_editAggiungiAutoreBtn->setVisible(true);
        if (m_editRimuoviAutoreBtn) m_editRimuoviAutoreBtn->setVisible(true);
    }
}