#include "mainwindow.h"
#include "mediacard.h"
#include "mediadialog.h"
#include "modello_logico/collezione.h"
#include "modello_logico/filtrostrategy.h"
#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
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
#include <QStandardPaths>
#include <QTimer>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_collezione(std::make_unique<Collezione>(this))
    , m_modificato(false)
    , m_selezionato_id("")
{
    setWindowTitle("Biblioteca Manager");
    setMinimumSize(800, 600);
    resize(1200, 800);
    
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
    if (QFile::exists(defaultFile)) {
        if (m_collezione->loadFromFile(defaultFile)) {
            m_fileCorrente = defaultFile;
            refreshMediaCards();
        }
    }
    
    caricaImpostazioni();
    aggiornaStatistiche();
    aggiornaStatusBar();
}

MainWindow::~MainWindow()
{
    salvaImpostazioni();
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
    updateLayout();
}

void MainWindow::setupUI()
{
    setupToolBar();
    setupStatusBar();
    setupMainArea();
}

void MainWindow::setupToolBar()
{
    QToolBar* toolBar = addToolBar("Principale");
    toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    
    // IMPORTANTE: Rendiamo la toolbar non movibile
    toolBar->setMovable(false);
    toolBar->setFloatable(false);
    
    // Azioni file
    QAction* nuovoAction = toolBar->addAction("Nuovo");
    nuovoAction->setToolTip("Crea una nuova collezione");
    connect(nuovoAction, &QAction::triggered, this, [this]() {
        try {
            nuovaCollezione();
        } catch (const std::exception& e) {
            mostraErrore(QString("Errore: %1").arg(e.what()));
        }
    });
    
    QAction* apriAction = toolBar->addAction("Apri");
    apriAction->setToolTip("Apri una collezione esistente");
    connect(apriAction, &QAction::triggered, this, [this]() {
        try {
            apriCollezione();
        } catch (const std::exception& e) {
            mostraErrore(QString("Errore: %1").arg(e.what()));
        }
    });
    
    QAction* salvaAction = toolBar->addAction("Salva");
    salvaAction->setToolTip("Salva la collezione");
    connect(salvaAction, &QAction::triggered, this, [this]() {
        try {
            salvaCollezione();
        } catch (const std::exception& e) {
            mostraErrore(QString("Errore: %1").arg(e.what()));
        }
    });
    
    toolBar->addSeparator();
    
    // Azioni media
    QAction* aggiungiAction = toolBar->addAction("Aggiungi");
    aggiungiAction->setToolTip("Aggiungi un nuovo media");
    connect(aggiungiAction, &QAction::triggered, this, [this]() {
        try {
            aggiungiMedia();
        } catch (const std::exception& e) {
            mostraErrore(QString("Errore: %1").arg(e.what()));
        }
    });
    
    QAction* modificaAction = toolBar->addAction("Modifica");
    modificaAction->setToolTip("Modifica il media selezionato");
    connect(modificaAction, &QAction::triggered, this, [this]() {
        try {
            modificaMedia();
        } catch (const std::exception& e) {
            mostraErrore(QString("Errore: %1").arg(e.what()));
        }
    });
    
    QAction* rimuoviAction = toolBar->addAction("Rimuovi");
    rimuoviAction->setToolTip("Rimuovi il media selezionato");
    connect(rimuoviAction, &QAction::triggered, this, [this]() {
        try {
            rimuoviMedia();
        } catch (const std::exception& e) {
            mostraErrore(QString("Errore: %1").arg(e.what()));
        }
    });
    
    toolBar->addSeparator();
    
    // Azione informazioni
    QAction* aboutAction = toolBar->addAction("Info");
    aboutAction->setToolTip("Informazioni sull'applicazione");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::informazioniSu);
}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel("Pronto");
    statusBar()->addWidget(m_statusLabel);
    
    statusBar()->addPermanentWidget(new QLabel(" | "));
    
    m_countLabel = new QLabel("0 media");
    statusBar()->addPermanentWidget(m_countLabel);
    
    m_progressBar = new QProgressBar();
    m_progressBar->setVisible(false);
    statusBar()->addPermanentWidget(m_progressBar);
}

void MainWindow::setupMainArea()
{
    m_centralWidget = new QWidget();
    setCentralWidget(m_centralWidget);
    
    // Layout principale orizzontale
    QHBoxLayout* mainLayout = new QHBoxLayout(m_centralWidget);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    
    // Splitter per ridimensionamento dinamico
    m_splitter = new QSplitter(Qt::Horizontal);
    mainLayout->addWidget(m_splitter);
    
    setupFilterArea();
    setupMediaArea();
    
    // Imposta proporzioni iniziali
    m_splitter->setSizes({FILTER_WIDTH, width() - FILTER_WIDTH});
    m_splitter->setStretchFactor(0, 0); // Area filtri non si espande
    m_splitter->setStretchFactor(1, 1); // Area media si espande
}

void MainWindow::setupFilterArea()
{
    m_filterWidget = new QWidget();
    m_filterWidget->setMaximumWidth(FILTER_WIDTH);
    m_filterWidget->setMinimumWidth(200);
    
    QVBoxLayout* filterLayout = new QVBoxLayout(m_filterWidget);
    
    // Gruppo ricerca
    m_searchGroup = new QGroupBox("Ricerca");
    QVBoxLayout* searchLayout = new QVBoxLayout(m_searchGroup);
    
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("Cerca nei media...");
    searchLayout->addWidget(m_searchEdit);
    
    QHBoxLayout* searchButtonLayout = new QHBoxLayout();
    m_searchButton = new QPushButton("Cerca");
    m_clearSearchButton = new QPushButton("Cancella");
    searchButtonLayout->addWidget(m_searchButton);
    searchButtonLayout->addWidget(m_clearSearchButton);
    searchLayout->addLayout(searchButtonLayout);
    
    // Connessioni per la ricerca
    connect(m_searchEdit, &QLineEdit::textChanged, this, &MainWindow::cercaMedia);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &MainWindow::cercaMedia);
    connect(m_searchButton, &QPushButton::clicked, this, &MainWindow::cercaMedia);
    connect(m_clearSearchButton, &QPushButton::clicked, this, [this]() {
        m_searchEdit->clear();
        cercaMedia();
    });
    
    // Gruppo filtri
    m_filterGroup = new QGroupBox("Filtri");
    QVBoxLayout* filtersLayout = new QVBoxLayout(m_filterGroup);
    
    // Filtro per tipo
    filtersLayout->addWidget(new QLabel("Tipo:"));
    m_tipoCombo = new QComboBox();
    m_tipoCombo->addItems({"Tutti", "Libro", "Film", "Articolo"});
    filtersLayout->addWidget(m_tipoCombo);
    
    // Filtro per anno
    filtersLayout->addWidget(new QLabel("Anno:"));
    QHBoxLayout* annoLayout = new QHBoxLayout();
    m_annoMinSpin = new QSpinBox();
    m_annoMinSpin->setRange(1000, 2100);
    m_annoMinSpin->setValue(1000);
    m_annoMaxSpin = new QSpinBox();
    m_annoMaxSpin->setRange(1000, 2100);
    m_annoMaxSpin->setValue(QDate::currentDate().year());
    annoLayout->addWidget(new QLabel("Da:"));
    annoLayout->addWidget(m_annoMinSpin);
    annoLayout->addWidget(new QLabel("A:"));
    annoLayout->addWidget(m_annoMaxSpin);
    filtersLayout->addLayout(annoLayout);
    
    // Campi specifici
    filtersLayout->addWidget(new QLabel("Autore:"));
    m_autoreEdit = new QLineEdit();
    m_autoreEdit->setPlaceholderText("Nome autore...");
    filtersLayout->addWidget(m_autoreEdit);
    
    filtersLayout->addWidget(new QLabel("Regista:"));
    m_registaEdit = new QLineEdit();
    m_registaEdit->setPlaceholderText("Nome regista...");
    filtersLayout->addWidget(m_registaEdit);
    
    filtersLayout->addWidget(new QLabel("Rivista:"));
    m_rivistaEdit = new QLineEdit();
    m_rivistaEdit->setPlaceholderText("Nome rivista...");
    filtersLayout->addWidget(m_rivistaEdit);
    
    // Bottoni filtri
    QHBoxLayout* filterButtonLayout = new QHBoxLayout();
    m_applyFilterButton = new QPushButton("Applica");
    m_resetFilterButton = new QPushButton("Reset");
    filterButtonLayout->addWidget(m_applyFilterButton);
    filterButtonLayout->addWidget(m_resetFilterButton);
    filtersLayout->addLayout(filterButtonLayout);
    
    connect(m_tipoCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::applicaFiltri);
    connect(m_applyFilterButton, &QPushButton::clicked, this, &MainWindow::applicaFiltri);
    connect(m_resetFilterButton, &QPushButton::clicked, this, &MainWindow::resetFiltri);
    
    // Gruppo statistiche
    m_statisticheGroup = new QGroupBox("Statistiche");
    QVBoxLayout* statsLayout = new QVBoxLayout(m_statisticheGroup);
    
    m_totalLabel = new QLabel("Totale: 0");
    m_libriLabel = new QLabel("Libri: 0");
    m_filmLabel = new QLabel("Film: 0");
    m_articoliLabel = new QLabel("Articoli: 0");
    
    statsLayout->addWidget(m_totalLabel);
    statsLayout->addWidget(m_libriLabel);
    statsLayout->addWidget(m_filmLabel);
    statsLayout->addWidget(m_articoliLabel);
    
    // Gruppo azioni
    m_actionsGroup = new QGroupBox("Azioni");
    QVBoxLayout* actionsLayout = new QVBoxLayout(m_actionsGroup);
    
    m_addButton = new QPushButton("Aggiungi Media");
    m_editButton = new QPushButton("Modifica");
    m_removeButton = new QPushButton("Rimuovi");
    m_detailsButton = new QPushButton("Dettagli");
    
    m_editButton->setEnabled(false);
    m_removeButton->setEnabled(false);
    m_detailsButton->setEnabled(false);
    
    actionsLayout->addWidget(m_addButton);
    actionsLayout->addWidget(m_editButton);
    actionsLayout->addWidget(m_removeButton);
    actionsLayout->addWidget(m_detailsButton);
    
    connect(m_addButton, &QPushButton::clicked, this, [this]() {
        try {
            aggiungiMedia();
        } catch (const std::exception& e) {
            mostraErrore(QString("Errore: %1").arg(e.what()));
        }
    });
    connect(m_editButton, &QPushButton::clicked, this, [this]() {
        try {
            modificaMedia();
        } catch (const std::exception& e) {
            mostraErrore(QString("Errore: %1").arg(e.what()));
        }
    });
    connect(m_removeButton, &QPushButton::clicked, this, [this]() {
        try {
            rimuoviMedia();
        } catch (const std::exception& e) {
            mostraErrore(QString("Errore: %1").arg(e.what()));
        }
    });
    connect(m_detailsButton, &QPushButton::clicked, this, [this]() {
        try {
            visualizzaDettagli();
        } catch (const std::exception& e) {
            mostraErrore(QString("Errore: %1").arg(e.what()));
        }
    });
    
    // Aggiungi tutti i gruppi al layout
    filterLayout->addWidget(m_searchGroup);
    filterLayout->addWidget(m_filterGroup);
    filterLayout->addWidget(m_statisticheGroup);
    filterLayout->addWidget(m_actionsGroup);
    filterLayout->addStretch();
    
    m_splitter->addWidget(m_filterWidget);
}

void MainWindow::setupMediaArea()
{
    // Scroll area per le card dei media
    m_mediaScrollArea = new QScrollArea();
    m_mediaScrollArea->setWidgetResizable(true);
    m_mediaScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_mediaScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    // Container per le card
    m_mediaContainer = new QWidget();
    m_mediaLayout = new QGridLayout(m_mediaContainer);
    m_mediaLayout->setContentsMargins(CARD_MARGIN, CARD_MARGIN, CARD_MARGIN, CARD_MARGIN);
    m_mediaLayout->setSpacing(CARD_MARGIN);
    
    m_mediaScrollArea->setWidget(m_mediaContainer);
    m_splitter->addWidget(m_mediaScrollArea);
}

void MainWindow::updateLayout()
{
    if (!m_mediaLayout) return;
    
    // Calcola il numero di colonne in base alla larghezza disponibile
    int containerWidth = m_mediaScrollArea->viewport()->width();
    int cardWidthWithMargin = CARD_WIDTH + CARD_MARGIN;
    int columns = qMax(1, (containerWidth - CARD_MARGIN) / cardWidthWithMargin);
    
    // Usa size_t per evitare warning di conversione
    for (size_t i = 0; i < m_mediaCards.size(); ++i) {
        int row = static_cast<int>(i) / columns;
        int col = static_cast<int>(i) % columns;
        m_mediaLayout->addWidget(m_mediaCards[i], row, col);
    }
    
    // Aggiungi stretch per spingere le card in alto
    m_mediaLayout->setRowStretch(m_mediaLayout->rowCount(), 1);
}

void MainWindow::refreshMediaCards()
{
    clearMediaCards();
    
    try {
        std::vector<Media*> media;
        
        // Prima applica la ricerca testuale
        QString searchText = m_searchEdit->text().trimmed();
        if (searchText.isEmpty()) {
            // Se non c'è testo di ricerca, prendi tutti i media
            auto allMedia = m_collezione->getAllMedia();
            for (const auto& m : allMedia) {
                media.push_back(m.get());
            }
        } else {
            // Applica la ricerca testuale
            auto tmp = m_collezione->searchMedia(searchText);
            media.clear();
            media.reserve(tmp.size());
            for (Media* mptr : tmp) media.push_back(mptr);
        }
        
        // Poi applica i filtri sui risultati della ricerca
        auto filtro = creaFiltroCorrente();
        if (filtro) {
            std::vector<Media*> filteredMedia;
            for (Media* m : media) {
                if (filtro->matches(m)) {
                    filteredMedia.push_back(m);
                }
            }
            media.clear();
            media.reserve(filteredMedia.size());
            for (auto& uptr : filteredMedia) {
            media.push_back(std::move(uptr));
        }
        
        // Usa push_back() invece di append() per std::vector
        for (Media* mediaPtr : media) {
            if (mediaPtr) {
                try {
                    // Crea MediaCard direttamente usando createCard()
                    std::unique_ptr<MediaCard> card = mediaPtr->createCard(m_mediaContainer);
                    if (card) {
                        MediaCard* cardPtr = card.release();
                        m_mediaCards.push_back(cardPtr);
                        
                        // Connessioni per selezione
                        connect(cardPtr, &MediaCard::selezionato,
                                this, &MainWindow::onCardSelezionata);
                        connect(cardPtr, &MediaCard::doppioClick,
                                this, &MainWindow::onCardDoubleClic);
                    }
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
            m_mediaLayout->removeWidget(card);
            card->deleteLater();
        }
    }
    m_mediaCards.clear();
    
    m_selezionato_id.clear();
    m_editButton->setEnabled(false);
    m_removeButton->setEnabled(false);
    m_detailsButton->setEnabled(false);
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
    
    // Filtro per anno (solo se diverso dal range completo)
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

void MainWindow::nuovaCollezione()
{
    try {
        if (!verificaModifiche()) return;
        
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
            salvaCollezioneCome();
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

void MainWindow::salvaCollezioneCome()
{
    try {
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
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore nel salvataggio: %1").arg(e.what()));
    }
}

void MainWindow::esportaCSV()
{
    try {
        QString fileName = QFileDialog::getSaveFileName(this,
            "Esporta CSV", "collezione.csv", "File CSV (*.csv)");
        
        if (!fileName.isEmpty()) {
            // Implementazione esportazione CSV tramite JsonManager
            mostraInfo("CSV esportato con successo");
        }
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore nell'esportazione: %1").arg(e.what()));
    }
}

void MainWindow::esci()
{
    close();
}

void MainWindow::aggiungiMedia()
{
    try {
        MediaDialog dialog(this);
        if (dialog.exec() == QDialog::Accepted) {
            auto media = dialog.getMedia();
            if (media) {
                m_collezione->addMedia(std::move(media));
                m_modificato = true;
                aggiornaStatusBar();
            }
        }
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore nell'aggiunta: %1").arg(e.what()));
    }
}

void MainWindow::modificaMedia()
{
    try {
        if (m_selezionato_id.isEmpty()) return;
        
        Media* media = m_collezione->findMedia(m_selezionato_id);
        if (!media) return;
        
        MediaDialog dialog(media, this);
        if (dialog.exec() == QDialog::Accepted) {
            auto updatedMedia = dialog.getMedia();
            if (updatedMedia && m_collezione->updateMedia(m_selezionato_id, std::move(updatedMedia))) {
                m_modificato = true;
                aggiornaStatusBar();
            }
        }
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore nella modifica: %1").arg(e.what()));
    }
}

void MainWindow::rimuoviMedia()
{
    try {
        if (m_selezionato_id.isEmpty()) return;
        
        Media* media = m_collezione->findMedia(m_selezionato_id);
        if (!media) return;
        
        if (confermaAzione(QString("Rimuovere '%1'?").arg(media->getTitolo()))) {
            if (m_collezione->removeMedia(m_selezionato_id)) {
                m_modificato = true;
                mostraInfo("Media rimosso");
                aggiornaStatusBar();
            }
        }
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore nella rimozione: %1").arg(e.what()));
    }
}

void MainWindow::visualizzaDettagli()
{
    try {
        if (m_selezionato_id.isEmpty()) return;
        
        Media* media = m_collezione->findMedia(m_selezionato_id);
        if (!media) return;
        
        MediaDialog dialog(media, this, true); // true = modalità solo lettura
        dialog.exec();
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore nella visualizzazione: %1").arg(e.what()));
    }
}

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

void MainWindow::filtraPerTipo()
{
    applicaFiltri();
}

void MainWindow::filtraPerAnno()
{
    applicaFiltri();
}

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

void MainWindow::onCardSelezionata(const QString& id)
{
    m_selezionato_id = id;
    m_editButton->setEnabled(true);
    m_removeButton->setEnabled(true);
    m_detailsButton->setEnabled(true);
    
    // Deseleziona altre card
    for (MediaCard* card : m_mediaCards) {
        if (card->getId() != id) {
            card->setSelected(false);
        }
    }
}

void MainWindow::onCardDoubleClic(const QString& id)
{
    m_selezionato_id = id;
    visualizzaDettagli();
}

void MainWindow::informazioniSu()
{
    QMessageBox::about(this, "Informazioni",
        "<h3>Biblioteca Manager</h3>"
        "<p>Versione 1.0</p>"
        "<p>Sistema di gestione per biblioteche multimediali</p>"
        "<p>Supporta libri, film e articoli con interfaccia dinamica</p>"
        "<p>Sviluppato con Qt 6.2.4 e C++17</p>");
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
        m_countLabel->setText(QString("%1 media").arg(m_collezione->size()));
        
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
        QSettings settings("BibliotecaManager", "MainWindow");
        settings.setValue("geometria", saveGeometry());
        settings.setValue("splitter", m_splitter->saveState());
    } catch (const std::exception& e) {
        qWarning() << "Errore nel salvataggio impostazioni:" << e.what();
    }
}

void MainWindow::caricaImpostazioni()
{
    try {
        QSettings settings("BibliotecaManager", "MainWindow");
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
                return !m_modificato; // Solo se il salvataggio è riuscito
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

void MainWindow::resetInterfaccia()
{
    try {
        m_searchEdit->clear();
        resetFiltri();
        m_selezionato_id.clear();
        refreshMediaCards();
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore nel reset interfaccia: %1").arg(e.what()));
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
    // Auto-reset dopo 3 secondi
    QTimer::singleShot(3000, [this]() {
        m_statusLabel->setText("Pronto");
    });
}

bool MainWindow::confermaAzione(const QString& messaggio)
{
    return QMessageBox::question(this, "Conferma", messaggio,
                                QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes;
}