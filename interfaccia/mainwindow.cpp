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
    if (!QFile::exists(defaultFile)) {
        defaultFile = "../data.json";  // Prova un livello sopra (cartella principale)
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
    
    // Aggiungi un timer per evitare troppi aggiornamenti durante il resize
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
    toolBar->setMovable(false);
    toolBar->setFloatable(false);
    
    // Tutte le icone caricate dal resources.qrc
    QAction* nuovoAction = toolBar->addAction(QIcon(":/icons/new_icon.png"), "Nuovo");
    nuovoAction->setToolTip("Crea una nuova collezione");
    connect(nuovoAction, &QAction::triggered, this, [this]() {
        try {
            nuovaCollezione();
        } catch (const std::exception& e) {
            mostraErrore(QString("Errore: %1").arg(e.what()));
        }
    });
    
    QAction* apriAction = toolBar->addAction(QIcon(":/icons/open_icon.png"), "Apri");
    apriAction->setToolTip("Apri una collezione esistente");
    connect(apriAction, &QAction::triggered, this, [this]() {
        try {
            apriCollezione();
        } catch (const std::exception& e) {
            mostraErrore(QString("Errore: %1").arg(e.what()));
        }
    });
    
    QAction* salvaAction = toolBar->addAction(QIcon(":/icons/save_icon.png"), "Salva");
    salvaAction->setToolTip("Salva la collezione");
    connect(salvaAction, &QAction::triggered, this, [this]() {
        try {
            salvaCollezione();
        } catch (const std::exception& e) {
            mostraErrore(QString("Errore: %1").arg(e.what()));
        }
    });
}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel("Pronto");
    statusBar()->addWidget(m_statusLabel);
    
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
    m_filterWidget->setMinimumHeight(700); 
    
    QVBoxLayout* filterLayout = new QVBoxLayout(m_filterWidget);
    
    // Gruppo ricerca
    m_searchGroup = new QGroupBox("Ricerca");
    m_searchGroup->setMinimumHeight(80);
    QVBoxLayout* searchLayout = new QVBoxLayout(m_searchGroup);

    QHBoxLayout* searchInputLayout = new QHBoxLayout();

    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("Cerca nei media...");
    m_searchEdit->setMinimumHeight(25);
    searchInputLayout->addWidget(m_searchEdit);

    m_clearSearchButton = new QPushButton("Cancella");
    m_clearSearchButton->setToolTip("Cancella il testo di ricerca e mostra tutti i media");
    m_clearSearchButton->setMaximumWidth(80);
    m_clearSearchButton->setMinimumHeight(25);
    m_clearSearchButton->setEnabled(false);
    searchInputLayout->addWidget(m_clearSearchButton);

    searchLayout->addLayout(searchInputLayout);

    // Connessioni
    connect(m_searchEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        m_clearSearchButton->setEnabled(!text.isEmpty());
        cercaMedia();
    });

    connect(m_searchEdit, &QLineEdit::returnPressed, this, &MainWindow::cercaMedia);

    connect(m_clearSearchButton, &QPushButton::clicked, this, [this]() {
        m_searchEdit->clear();
        m_clearSearchButton->setEnabled(false);
        cercaMedia();
    });
    
    // Gruppo filtri
    m_filterGroup = new QGroupBox("Filtri");
    m_filterGroup->setMinimumHeight(280);
    m_filterGroup->setMaximumHeight(280);
    QVBoxLayout* filtersLayout = new QVBoxLayout(m_filterGroup);
    filtersLayout->setSpacing(4);
    
    // Filtro per tipo
    filtersLayout->addWidget(new QLabel("Tipo:"));
    m_tipoCombo = new QComboBox();
    m_tipoCombo->addItems({"Tutti", "Libro", "Film", "Articolo"});
    m_tipoCombo->setMaximumHeight(30);
    m_tipoCombo->setToolTip("Filtra i media per tipo");
    filtersLayout->addWidget(m_tipoCombo);
    
    // Filtro per anno
    filtersLayout->addWidget(new QLabel("Anno:"));
    QHBoxLayout* annoLayout = new QHBoxLayout();
    m_annoMinSpin = new QSpinBox();
    m_annoMinSpin->setRange(1000, 2100);
    m_annoMinSpin->setValue(1000);
    m_annoMinSpin->setMaximumHeight(30);
    m_annoMinSpin->setToolTip("Anno minimo di pubblicazione");
    m_annoMaxSpin = new QSpinBox();
    m_annoMaxSpin->setRange(1000, 2100);
    m_annoMaxSpin->setValue(QDate::currentDate().year());
    m_annoMaxSpin->setMaximumHeight(30);
    m_annoMaxSpin->setToolTip("Anno massimo di pubblicazione");
    annoLayout->addWidget(new QLabel("Da:"));
    annoLayout->addWidget(m_annoMinSpin);
    annoLayout->addWidget(new QLabel("A:"));
    annoLayout->addWidget(m_annoMaxSpin);
    filtersLayout->addLayout(annoLayout);
    
    // Campi specifici
    filtersLayout->addWidget(new QLabel("Autore:"));
    m_autoreEdit = new QLineEdit();
    m_autoreEdit->setPlaceholderText("Nome autore...");
    m_autoreEdit->setMaximumHeight(30);
    m_autoreEdit->setToolTip("Filtra per nome dell'autore (solo libri e articoli)");
    filtersLayout->addWidget(m_autoreEdit);
    
    filtersLayout->addWidget(new QLabel("Regista:"));
    m_registaEdit = new QLineEdit();
    m_registaEdit->setPlaceholderText("Nome regista...");
    m_registaEdit->setMaximumHeight(30);
    m_registaEdit->setToolTip("Filtra per nome del regista (solo film)");
    filtersLayout->addWidget(m_registaEdit);
    
    filtersLayout->addWidget(new QLabel("Rivista:"));
    m_rivistaEdit = new QLineEdit();
    m_rivistaEdit->setPlaceholderText("Nome rivista...");
    m_rivistaEdit->setMaximumHeight(30);
    m_rivistaEdit->setToolTip("Filtra per nome della rivista (solo articoli)");
    filtersLayout->addWidget(m_rivistaEdit);
    
    // Bottoni filtri
    QHBoxLayout* filterButtonLayout = new QHBoxLayout();
    m_applyFilterButton = new QPushButton("Applica"); 
    m_applyFilterButton->setMaximumHeight(30);
    m_applyFilterButton->setToolTip("Applica tutti i filtri impostati");
    m_resetFilterButton = new QPushButton("Reset");
    m_resetFilterButton->setMaximumHeight(30);
    m_resetFilterButton->setToolTip("Cancella tutti i filtri e mostra tutti i media");
    filterButtonLayout->addWidget(m_applyFilterButton);
    filterButtonLayout->addWidget(m_resetFilterButton);
    filtersLayout->addLayout(filterButtonLayout);
    
    connect(m_tipoCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::applicaFiltri);
    connect(m_applyFilterButton, &QPushButton::clicked, this, &MainWindow::applicaFiltri);
    connect(m_resetFilterButton, &QPushButton::clicked, this, &MainWindow::resetFiltri);
    
    // Gruppo statistiche
    m_statisticheGroup = new QGroupBox("Statistiche");
    m_statisticheGroup->setMinimumHeight(120);
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
    m_actionsGroup->setMinimumHeight(180);
    QVBoxLayout* actionsLayout = new QVBoxLayout(m_actionsGroup);
    
    // Tutti i pulsanti con icone dal resources.qrc e tooltip migliorati
    m_addButton = new QPushButton(QIcon(":/icons/add_icon.png"), "Aggiungi Media");
    m_addButton->setToolTip("Aggiungi un nuovo media alla collezione");
    
    m_editButton = new QPushButton(QIcon(":/icons/edit_icon.png"), "Modifica");
    m_editButton->setEnabled(false);
    m_editButton->setToolTip("Modifica il media selezionato");
    
    m_removeButton = new QPushButton(QIcon(":/icons/delete_icon.png"), "Rimuovi");
    m_removeButton->setEnabled(false);
    m_removeButton->setToolTip("Rimuovi il media selezionato dalla collezione");
    
    m_detailsButton = new QPushButton(QIcon(":/icons/details_icon.png"), "Dettagli");
    m_detailsButton->setEnabled(false);
    m_detailsButton->setToolTip("Visualizza i dettagli completi del media selezionato");
    
    actionsLayout->addWidget(m_addButton);
    actionsLayout->addWidget(m_editButton);
    actionsLayout->addWidget(m_removeButton);
    actionsLayout->addWidget(m_detailsButton);
    
    // Connessioni dirette alle azioni
    connect(m_addButton, &QPushButton::clicked, this, &MainWindow::aggiungiMedia);
    connect(m_editButton, &QPushButton::clicked, this, &MainWindow::modificaMedia);
    connect(m_removeButton, &QPushButton::clicked, this, &MainWindow::rimuoviMedia);
    connect(m_detailsButton, &QPushButton::clicked, this, &MainWindow::visualizzaDettagli);
    
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
    if (!m_mediaLayout || !m_mediaScrollArea) return;
    
    // Calcola il numero di colonne in base alla larghezza disponibile
    int containerWidth = m_mediaScrollArea->viewport()->width();
    int cardWidthWithMargin = CARD_WIDTH + CARD_MARGIN;
    int columns = qMax(1, (containerWidth - CARD_MARGIN) / cardWidthWithMargin);
    
    // IMPORTANTE: Rimuovi tutti i widget dal layout prima di riorganizzarli
    for (MediaCard* card : m_mediaCards) {
        if (card) {
            m_mediaLayout->removeWidget(card);
        }
    }
    
    // Rimuovi eventuali stretch esistenti
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
        // Prima pulisci completamente le card esistenti
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
        
        // Applica il layout DOPO aver creato tutte le card
        updateLayout();
        aggiornaStatistiche();
        
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore durante l'aggiornamento: %1").arg(e.what()));
    }
}

void MainWindow::clearMediaCards()
{
    // IMPORTANTE: Prima disconnetti tutti i segnali
    for (MediaCard* card : m_mediaCards) {
        if (card) {
            disconnect(card, nullptr, this, nullptr);
            m_mediaLayout->removeWidget(card);
            card->deleteLater();
        }
    }
    
    // Pulisci il vettore
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
        MediaDialog* dialog = new MediaDialog(this);
        
        dialog->setAttribute(Qt::WA_DeleteOnClose, true);
        
        connect(dialog, &QDialog::finished, this, [this, dialog](int result) {
            if (result == QDialog::Accepted) {
                try {
                    auto media = dialog->getMedia();
                    if (media && media->isValid()) {
                        m_collezione->addMedia(std::move(media));
                        m_modificato = true;
                        aggiornaStatusBar();
                        mostraInfo("Media aggiunto con successo");
                    } else {
                        mostraErrore("Media non valido - non è stato aggiunto");
                    }
                } catch (const std::exception& e) {
                    mostraErrore(QString("Errore nell'aggiunta del media: %1").arg(e.what()));
                }
            }
        });
        
        dialog->show();
        
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore nella creazione del dialog: %1").arg(e.what()));
    }
}

void MainWindow::modificaMedia()
{
    try {
        if (m_selezionato_id.isEmpty()) {
            mostraInfo("Seleziona un media da modificare");
            return;
        }
        
        Media* media = m_collezione->findMedia(m_selezionato_id);
        if (!media) {
            mostraErrore("Media selezionato non trovato");
            return;
        }
        
        MediaDialog* dialog = new MediaDialog(media, this, false);
        dialog->setAttribute(Qt::WA_DeleteOnClose, true);
        
        QString mediaId = m_selezionato_id;
        
        connect(dialog, &QDialog::finished, this, [this, dialog, mediaId](int result) {
            if (result == QDialog::Accepted) {
                try {
                    auto updatedMedia = dialog->getMedia();
                    if (updatedMedia && updatedMedia->isValid()) {
                        if (m_collezione->updateMedia(mediaId, std::move(updatedMedia))) {
                            m_modificato = true;
                            aggiornaStatusBar();
                            mostraInfo("Media modificato con successo");
                            refreshMediaCards();
                        } else {
                            mostraErrore("Impossibile aggiornare il media");
                        }
                    } else {
                        mostraErrore("Media modificato non valido");
                    }
                } catch (const std::exception& e) {
                    mostraErrore(QString("Errore nella modifica: %1").arg(e.what()));
                }
            }
        });
        
        dialog->show();
        
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
        if (m_selezionato_id.isEmpty()) {
            mostraInfo("Seleziona un media per visualizzare i dettagli");
            return;
        }
        
        Media* media = m_collezione->findMedia(m_selezionato_id);
        if (!media) {
            mostraErrore("Media selezionato non trovato");
            return;
        }
        
        MediaDialog* dialog = new MediaDialog(media, this, true); // true = modalità solo lettura
        dialog->setAttribute(Qt::WA_DeleteOnClose, true);
        
        dialog->show();
        
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore nella visualizzazione: %1").arg(e.what()));
    }
}

void MainWindow::aggiornaStatoBottoni()
{
    bool hasSelection = !m_selezionato_id.isEmpty();
    
    if (m_editButton) m_editButton->setEnabled(hasSelection);
    if (m_removeButton) m_removeButton->setEnabled(hasSelection);
    if (m_detailsButton) m_detailsButton->setEnabled(hasSelection);
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
        // Verifica che il media esista
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
    QTimer::singleShot(3000, [this]() {
        m_statusLabel->setText("Pronto");
    });
}

bool MainWindow::confermaAzione(const QString& messaggio)
{
    return QMessageBox::question(this, "Conferma", messaggio,
                                QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes;
}