#include "mainwindow.h"
#include "mediacard.h"
#include "modello_logico/collezione.h"
#include "modello_logico/filtrostrategy.h"
#include "modello_logico/libro.h"
#include "modello_logico/film.h"
#include "modello_logico/articolo.h"
#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressBar>
#include <QSplitter>
#include <QGroupBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QCloseEvent>
#include <QResizeEvent>
#include <QSettings>
#include <QTimer>
#include <QDebug>
#include <QFormLayout>
#include <QTextEdit> 
#include <QListWidget>
#include <QDateEdit>
#include <QDate>
#include <QFile> 
#include <QFileInfo>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_collezione(std::make_unique<Collezione>(this))
    , m_modificato(false)
    , m_selezionato_id("")
    , m_centralWidget(nullptr)
    , m_splitter(nullptr)
    , m_filterWidget(nullptr)
    , m_mediaScrollArea(nullptr)
    , m_mediaContainer(nullptr)
    , m_mediaLayout(nullptr)
    , m_editPanel(nullptr)
    , m_editContentContainer(nullptr)
    , m_editScrollArea(nullptr)
    , m_editHeaderLabel(nullptr)
    , m_editPanelVisible(false)
    , m_editingMediaId("")
    , m_editReadOnly(false)
    , m_editIsNew(false)
    , m_editTipoCorrente("")
    , m_editTypeChanging(false)
    , m_editFormLayout(nullptr)
    , m_editBaseGroup(nullptr)
    , m_editTipoCombo(nullptr)
    , m_editTitoloEdit(nullptr)
    , m_editAnnoSpin(nullptr)
    , m_editDescrizioneEdit(nullptr)
    , m_editLibroGroup(nullptr)
    , m_editAutoreEdit(nullptr)
    , m_editEditoreEdit(nullptr)
    , m_editPagineSpin(nullptr)
    , m_editIsbnEdit(nullptr)
    , m_editGenereLibroCombo(nullptr)
    , m_editFilmGroup(nullptr)
    , m_editRegistaEdit(nullptr)
    , m_editAttoriList(nullptr)
    , m_editNuovoAttoreEdit(nullptr)
    , m_editAggiungiAttoreBtn(nullptr)
    , m_editRimuoviAttoreBtn(nullptr)
    , m_editDurataSpin(nullptr)
    , m_editGenereFilmCombo(nullptr)
    , m_editClassificazioneCombo(nullptr)
    , m_editCasaProduzioneEdit(nullptr)
    , m_editArticoloGroup(nullptr)
    , m_editAutoriList(nullptr)
    , m_editNuovoAutoreEdit(nullptr)
    , m_editAggiungiAutoreBtn(nullptr)
    , m_editRimuoviAutoreBtn(nullptr)
    , m_editRivisteEdit(nullptr)
    , m_editVolumeEdit(nullptr)
    , m_editNumeroEdit(nullptr)
    , m_editPagineEdit(nullptr)
    , m_editCategoriaCombo(nullptr)
    , m_editTipoRivistaCombo(nullptr)
    , m_editDataPubblicazioneEdit(nullptr)
    , m_editDoiEdit(nullptr)
    , m_editSalvaButton(nullptr)
    , m_editAnnullaButton(nullptr)
    , m_editHelpButton(nullptr)
    , m_editValidationLabel(nullptr)
    , m_editValidationEnabled(true)
    , m_validationTimer(nullptr)
    , m_validationPending(false)
{
    setWindowTitle("Biblioteca Manager");
    setMinimumSize(800, 600);
    resize(1200, 800);
    
    try {
        setupUI();
        
        // Connessioni con la collezione
        connect(m_collezione.get(), &Collezione::mediaAdded,
                this, &MainWindow::onMediaAggiunto);
        connect(m_collezione.get(), &Collezione::mediaRemoved,
                this, &MainWindow::onMediaRimosso);
        connect(m_collezione.get(), &Collezione::mediaUpdated,
                this, &MainWindow::onMediaModificato);
        connect(m_collezione.get(), &Collezione::collectionLoaded,
                this, &MainWindow::onCollezioneCaricata);
        
        // Carica file di default se esiste
        QString defaultFile = "data.json";
        if (!QFile::exists(defaultFile)) {
            defaultFile = "../data.json";
        }
        
        if (QFile::exists(defaultFile)) {
            if (m_collezione->loadFromFile(defaultFile)) {
                m_fileCorrente = defaultFile;
                refreshMediaCards();
            }
        }
        
        caricaImpostazioni();
        aggiornaStatistiche();
        aggiornaStatusBar();
        
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Errore Inizializzazione", 
                             QString("Errore durante l'inizializzazione: %1").arg(e.what()));
    }
}

MainWindow::~MainWindow()
{
    try {
        // Pulisci le connessioni prima della distruzione
        if (m_collezione) {
            disconnect(m_collezione.get(), nullptr, this, nullptr);
        }
        
        // Pulisci le card prima di distruggere il layout
        clearMediaCards();
        
        salvaImpostazioni();
    } catch (...) {}
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (verificaModifiche()) {
        salvaImpostazioni();
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    
    // Timer per evitare troppi aggiornamenti durante il resize
    static QTimer* resizeTimer = nullptr;
    if (!resizeTimer) {
        resizeTimer = new QTimer(this);
        resizeTimer->setSingleShot(true);
        resizeTimer->setInterval(100); // 100ms delay
        connect(resizeTimer, &QTimer::timeout, [this]() {
            updateLayout();
        });
    }
    
    resizeTimer->start();
}

void MainWindow::updateLayout()
{
    if (!m_mediaLayout || !m_mediaScrollArea) return;
    
    // Calcola il numero di colonne in base alla larghezza disponibile
    int containerWidth = m_mediaScrollArea->viewport()->width();
    int cardWidthWithMargin = CARD_WIDTH + CARD_MARGIN;
    int columns = qMax(1, (containerWidth - CARD_MARGIN) / cardWidthWithMargin);

    for (MediaCard* card : m_mediaCards) {
        if (card) {
            m_mediaLayout->removeWidget(card);
        }
    }
    
    QLayoutItem* item;
    while ((item = m_mediaLayout->takeAt(0)) != nullptr) {
        delete item;
    }
    
    // Riposiziona tutte le card nel layout
    for (int i = 0; i < m_mediaCards.size(); ++i) {
        MediaCard* card = m_mediaCards[i];
        if (card) {
            int row = i / columns;
            int col = i % columns;
            m_mediaLayout->addWidget(card, row, col);
        }
    }
    
    // Aggiungi stretch finale per spingere le card in alto
    int totalRows = (m_mediaCards.size() + columns - 1) / columns;
    m_mediaLayout->setRowStretch(totalRows, 1);
    
    // Forza un update del container
    m_mediaContainer->updateGeometry();
    m_mediaScrollArea->update();
}

void MainWindow::refreshMediaCards() {
    if (!m_mediaLayout) return;
    
    try {
        clearMediaCards();
        
        std::vector<Media*> media;
        
        // Applica ricerca
        QString searchText = m_searchEdit->text().trimmed();
        if (searchText.isEmpty()) {
            const auto& allMedia = m_collezione->getAllMedia();
            for (const auto& m : allMedia) {
                media.push_back(m.get());
            }
        } else {
            media = m_collezione->searchMedia(searchText);
        }
        
        // Applica filtri
        auto filtro = creaFiltroCorrente();
        if (filtro) {
            std::vector<Media*> filteredMedia;
            for (Media* m : media) {
                if (filtro->matches(m)) {
                    filteredMedia.push_back(m);
                }
            }
            media = filteredMedia;
        }
        
        // Crea nuove card per tutti i media
        for (Media* mediaPtr : media) {
            if (mediaPtr) {
                try {
                    MediaCard* card = new MediaCard(mediaPtr, m_mediaContainer);
                    m_mediaCards.push_back(card);
                    
                    // Connessioni per selezione
                    connect(card, &MediaCard::selezionato,
                            this, &MainWindow::onCardSelezionata);
                    connect(card, &MediaCard::doppioClick,
                            this, &MainWindow::onCardDoubleClic);
                    
                } catch (const std::exception& e) {
                    qWarning() << "Errore nella creazione MediaCard:" << e.what();
                }
            }
        }
        
        updateLayout();
        aggiornaStatistiche();
        
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore durante l'aggiornamento: %1").arg(e.what()));
    }
}

void MainWindow::clearMediaCards()
{
    for (MediaCard* card : m_mediaCards) {
        if (card) {
            disconnect(card, nullptr, this, nullptr);
            m_mediaLayout->removeWidget(card);
            card->deleteLater();
        }
    }
    
    m_mediaCards.clear();
    
    // Rimuovi eventuali elementi residui dal layout
    QLayoutItem* item;
    while ((item = m_mediaLayout->takeAt(0)) != nullptr) {
        delete item;
    }
    
    m_selezionato_id.clear();
    aggiornaStatoBottoni();
}

void MainWindow::applicaRicercaCorrente()
{
    refreshMediaCards();
}

std::unique_ptr<FiltroStrategy> MainWindow::creaFiltroCorrente()
{
    auto filtroComposto = std::make_unique<FiltroComposto>();
    bool hasFiltri = false;
    
    // Filtro per tipo
    QString tipo = m_tipoCombo->currentText();
    if (tipo != "Tutti") {
        filtroComposto->addFiltro(FiltroFactory::createTipoFiltro(tipo));
        hasFiltri = true;
    }
    
    // Filtro per anno
    if (m_annoMinSpin->value() > 1000 || m_annoMaxSpin->value() < QDate::currentDate().year()) {
        filtroComposto->addFiltro(FiltroFactory::createAnnoFiltro(
            m_annoMinSpin->value(), m_annoMaxSpin->value()));
        hasFiltri = true;
    }
    
    // Filtri per campi specifici
    if (!m_autoreEdit->text().isEmpty()) {
        filtroComposto->addFiltro(FiltroFactory::createAutoreFiltro(m_autoreEdit->text()));
        hasFiltri = true;
    }
    
    if (!m_registaEdit->text().isEmpty()) {
        filtroComposto->addFiltro(FiltroFactory::createRegistaFiltro(m_registaEdit->text()));
        hasFiltri = true;
    }
    
    if (!m_rivistaEdit->text().isEmpty()) {
        filtroComposto->addFiltro(FiltroFactory::createRivistaFiltro(m_rivistaEdit->text()));
        hasFiltri = true;
    }
    
    return hasFiltri ? std::move(filtroComposto) : nullptr;
}

// Gestione file
void MainWindow::nuovaCollezione()
{
    try {
        if (!verificaModifiche()) return;
        
        Media::resetCounters();
        m_collezione->clear();
        
        m_fileCorrente.clear();
        m_modificato = false;
        setWindowTitle("Biblioteca Manager - [Nuova Collezione]");
        refreshMediaCards();
        mostraInfo("Nuova collezione creata");
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore nella creazione: %1").arg(e.what()));
    }
}

void MainWindow::apriCollezione()
{
    try {
        if (!verificaModifiche()) return;
        
        QString fileName = QFileDialog::getOpenFileName(this,
            "Apri Collezione", "", "File JSON (*.json);;Tutti i file (*.*)");
        
        if (!fileName.isEmpty()) {
            if (m_collezione->loadFromFile(fileName)) {
                m_fileCorrente = fileName;
                m_modificato = false;
                setWindowTitle(QString("Biblioteca Manager - [%1]").arg(QFileInfo(fileName).baseName()));
                refreshMediaCards();
                mostraInfo(QString("Collezione caricata: %1 media").arg(m_collezione->size()));
            } else {
                mostraErrore("Impossibile caricare il file");
            }
        }
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore nell'apertura: %1").arg(e.what()));
    }
}

void MainWindow::salvaCollezione()
{
    try {
        if (m_fileCorrente.isEmpty()) {
            QString fileName = QFileDialog::getSaveFileName(this,
                "Salva Collezione", "collezione.json", "File JSON (*.json)");
            
            if (!fileName.isEmpty()) {
                if (m_collezione->saveToFile(fileName)) {
                    m_fileCorrente = fileName;
                    m_modificato = false;
                    setWindowTitle(QString("Biblioteca Manager - [%1]").arg(QFileInfo(fileName).baseName()));
                    mostraInfo("Collezione salvata");
                } else {
                    mostraErrore("Impossibile salvare il file");
                }
            }
        } else {
            if (m_collezione->saveToFile(m_fileCorrente)) {
                m_modificato = false;
                mostraInfo("Collezione salvata");
            } else {
                mostraErrore("Impossibile salvare il file");
            }
        }
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore nel salvataggio: %1").arg(e.what()));
    }
}

// Gestione media
void MainWindow::aggiungiMedia()
{
    try {
        qDebug() << "Apertura pannello per nuovo media";
        m_editingMediaId.clear();
        showEditPanel(true, false);
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore nell'apertura form: %1").arg(e.what()));
    }
}

void MainWindow::modificaMedia()
{
    try {
        qDebug() << "Apertura pannello per modifica media:" << m_selezionato_id;
        
        if (m_selezionato_id.isEmpty()) {
            mostraInfo("Seleziona un media da modificare");
            return;
        }
        
        Media* media = m_collezione->findMedia(m_selezionato_id);
        if (!media) {
            mostraErrore("Media selezionato non trovato");
            return;
        }
        
        m_editingMediaId = m_selezionato_id;
        showEditPanel(false, false);
        
        // Carica i dati dopo aver mostrato il pannello
        QTimer::singleShot(100, [this, media]() {
            loadEditMediaData(media);
        });
        
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore nella modifica: %1").arg(e.what()));
    }
}

void MainWindow::rimuoviMedia()
{
    try {
        if (m_selezionato_id.isEmpty()) {
            mostraInfo("Seleziona un media da rimuovere");
            return;
        }
        
        Media* media = m_collezione->findMedia(m_selezionato_id);
        if (!media) {
            mostraErrore("Media selezionato non trovato");
            m_selezionato_id.clear();
            aggiornaStatoBottoni();
            return;
        }
        
        QString messaggio = QString("Sei sicuro di voler rimuovere il media:\n\n"
                                   "Titolo: %1\n"
                                   "Tipo: %2\n"
                                   "Anno: %3\n\n"
                                   "Questa azione non può essere annullata.")
                           .arg(media->getTitolo())
                           .arg(media->getTypeDisplayName())
                           .arg(media->getAnno());
        
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Conferma Rimozione");
        msgBox.setText(messaggio);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        
        msgBox.button(QMessageBox::Yes)->setText("Sì");
        msgBox.button(QMessageBox::No)->setText("No");
        
        if (msgBox.exec() == QMessageBox::Yes) {
            QString titoloRimosso = media->getTitolo();
            
            if (m_collezione->removeMedia(m_selezionato_id)) {
                m_selezionato_id.clear();
                m_modificato = true;
                aggiornaStatusBar();
                aggiornaStatoBottoni();
                
                refreshMediaCards();
                
                mostraInfo(QString("Media '%1' rimosso con successo").arg(titoloRimosso));
            } else {
                mostraErrore("Impossibile rimuovere il media dalla collezione");
            }
        }
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore nella rimozione: %1").arg(e.what()));
    }
}

void MainWindow::visualizzaDettagli()
{
    try {
        qDebug() << "Apertura pannello per dettagli media:" << m_selezionato_id;
        
        if (m_selezionato_id.isEmpty()) {
            mostraInfo("Seleziona un media per visualizzare i dettagli");
            return;
        }
        
        Media* media = m_collezione->findMedia(m_selezionato_id);
        if (!media) {
            mostraErrore("Media selezionato non trovato");
            return;
        }
        
        m_editingMediaId = m_selezionato_id;
        showEditPanel(false, true);
        
        // Carica i dati dopo aver mostrato il pannello
        QTimer::singleShot(100, [this, media]() {
            loadEditMediaData(media);
        });
        
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore nella visualizzazione: %1").arg(e.what()));
    }
}

// Ricerca e filtri
void MainWindow::cercaMedia()
{
    try {
        applicaRicercaCorrente();
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore nella ricerca: %1").arg(e.what()));
    }
}

void MainWindow::applicaFiltri()
{
    try {
        refreshMediaCards();
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore nell'applicazione filtri: %1").arg(e.what()));
    }
}

void MainWindow::resetFiltri()
{
    try {
        m_tipoCombo->setCurrentIndex(0);
        m_annoMinSpin->setValue(1000);
        m_annoMaxSpin->setValue(QDate::currentDate().year());
        m_autoreEdit->clear();
        m_registaEdit->clear();
        m_rivistaEdit->clear();
        refreshMediaCards();
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore nel reset filtri: %1").arg(e.what()));
    }
}

// Slots per notifiche dalla collezione
void MainWindow::onMediaAggiunto(const QString& id)
{
    Q_UNUSED(id)
    refreshMediaCards();
}

void MainWindow::onMediaRimosso(const QString& id)
{
    Q_UNUSED(id)
    refreshMediaCards();
}

void MainWindow::onMediaModificato(const QString& id)
{
    Q_UNUSED(id)
    refreshMediaCards();
}

void MainWindow::onCollezioneCaricata(int count)
{
    refreshMediaCards();
    mostraInfo(QString("Caricati %1 media").arg(count));
}

// Gestione card
void MainWindow::onCardSelezionata(const QString& id)
{
    m_selezionato_id = id;
    
    // Deseleziona altre card
    for (MediaCard* card : m_mediaCards) {
        if (card && card->getId() != id) {
            card->setSelected(false);
        }
    }
    
    // Trova e seleziona la card corretta
    for (MediaCard* card : m_mediaCards) {
        if (card && card->getId() == id) {
            card->setSelected(true);
            break;
        }
    }
    
    aggiornaStatoBottoni();
}

void MainWindow::onCardDoubleClic(const QString& id)
{
    try {
        if (!m_collezione->findMedia(id)) {
            mostraErrore("Media non trovato");
            return;
        }
        
        m_selezionato_id = id;
        aggiornaStatoBottoni();
        
        visualizzaDettagli();
        
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore nell'apertura dettagli: %1").arg(e.what()));
    }
}

void MainWindow::aggiornaStatoBottoni()
{
    bool hasSelection = !m_selezionato_id.isEmpty();
    
    if (m_editButton) m_editButton->setEnabled(hasSelection);
    if (m_removeButton) m_removeButton->setEnabled(hasSelection);
    if (m_detailsButton) m_detailsButton->setEnabled(hasSelection);
}

void MainWindow::aggiornaStatistiche()
{
    try {
        size_t total = m_collezione->size();
        size_t libri = m_collezione->countByType("Libro");
        size_t film = m_collezione->countByType("Film");
        size_t articoli = m_collezione->countByType("Articolo");
        
        m_totalLabel->setText(QString("Totale: %1").arg(total));
        m_libriLabel->setText(QString("Libri: %1").arg(libri));
        m_filmLabel->setText(QString("Film: %1").arg(film));
        m_articoliLabel->setText(QString("Articoli: %1").arg(articoli));
    } catch (const std::exception& e) {
        qWarning() << "Errore nell'aggiornamento statistiche:" << e.what();
    }
}

void MainWindow::aggiornaStatusBar()
{
    try {
        if (m_modificato) {
            m_statusLabel->setText("Modificato");
        } else {
            m_statusLabel->setText("Pronto");
        }
    } catch (const std::exception& e) {
        qWarning() << "Errore nell'aggiornamento status bar:" << e.what();
    }
}

void MainWindow::salvaImpostazioni()
{
    try {
        QSettings settings;
        settings.setValue("geometria", saveGeometry());
        settings.setValue("splitter", m_splitter->saveState());
    } catch (const std::exception& e) {
        qWarning() << "Errore nel salvataggio impostazioni:" << e.what();
    }
}

void MainWindow::caricaImpostazioni()
{
    try {
        QSettings settings;
        restoreGeometry(settings.value("geometria").toByteArray());
        m_splitter->restoreState(settings.value("splitter").toByteArray());
    } catch (const std::exception& e) {
        qWarning() << "Errore nel caricamento impostazioni:" << e.what();
    }
}

bool MainWindow::verificaModifiche()
{
    try {
        if (m_modificato) {
            QMessageBox::StandardButton ret = QMessageBox::question(this,
                "Modifiche non salvate",
                "Ci sono modifiche non salvate. Salvare ora?",
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
            
            if (ret == QMessageBox::Save) {
                salvaCollezione();
                return !m_modificato;
            } else if (ret == QMessageBox::Cancel) {
                return false;
            }
        }
        return true;
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore nella verifica modifiche: %1").arg(e.what()));
        return false;
    }
}

void MainWindow::mostraErrore(const QString& errore)
{
    QMessageBox::critical(this, "Errore", errore);
    m_statusLabel->setText("Errore: " + errore);
    qWarning() << "ERRORE:" << errore;
}

void MainWindow::mostraInfo(const QString& info)
{
    m_statusLabel->setText(info);
    QTimer::singleShot(3000, [this]() {
        m_statusLabel->setText("Pronto");
    });
}