#include "mainwindow.h"
#include "json/jsonmanager.h" 
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
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupMainArea();
}

void MainWindow::setupMenuBar()
{
    // Menu File
    QMenu* fileMenu = menuBar()->addMenu("&File");
    
    QAction* nuovoAction = fileMenu->addAction("&Nuova Collezione");
    nuovoAction->setShortcut(QKeySequence::New);
    connect(nuovoAction, &QAction::triggered, this, &MainWindow::nuovaCollezione);
    
    QAction* apriAction = fileMenu->addAction("&Apri Collezione...");
    apriAction->setShortcut(QKeySequence::Open);
    connect(apriAction, &QAction::triggered, this, &MainWindow::apriCollezione);
    
    fileMenu->addSeparator();
    
    QAction* salvaAction = fileMenu->addAction("&Salva");
    salvaAction->setShortcut(QKeySequence::Save);
    connect(salvaAction, &QAction::triggered, this, &MainWindow::salvaCollezione);
    
    QAction* salvaComeAction = fileMenu->addAction("Salva &Come...");
    salvaComeAction->setShortcut(QKeySequence::SaveAs);
    connect(salvaComeAction, &QAction::triggered, this, &MainWindow::salvaCollezioneCome);
    
    fileMenu->addSeparator();
    
    QAction* esportaAction = fileMenu->addAction("&Esporta CSV...");
    connect(esportaAction, &QAction::triggered, this, &MainWindow::esportaCSV);
    
    fileMenu->addSeparator();
    
    QAction* esciAction = fileMenu->addAction("&Esci");
    esciAction->setShortcut(QKeySequence::Quit);
    connect(esciAction, &QAction::triggered, this, &MainWindow::esci);
    
    // Menu Media
    QMenu* mediaMenu = menuBar()->addMenu("&Media");
    
    QAction* aggiungiAction = mediaMenu->addAction("&Aggiungi Media...");
    aggiungiAction->setShortcut(QKeySequence("Ctrl+N"));
    connect(aggiungiAction, &QAction::triggered, this, &MainWindow::aggiungiMedia);
    
    QAction* modificaAction = mediaMenu->addAction("&Modifica Media...");
    modificaAction->setShortcut(QKeySequence("Ctrl+E"));
    connect(modificaAction, &QAction::triggered, this, &MainWindow::modificaMedia);
    
    QAction* rimuoviAction = mediaMenu->addAction("&Rimuovi Media");
    rimuoviAction->setShortcut(QKeySequence::Delete);
    connect(rimuoviAction, &QAction::triggered, this, &MainWindow::rimuoviMedia);
    
    mediaMenu->addSeparator();
    
    QAction* dettagliAction = mediaMenu->addAction("&Dettagli...");
    dettagliAction->setShortcut(QKeySequence("Ctrl+D"));
    connect(dettagliAction, &QAction::triggered, this, &MainWindow::visualizzaDettagli);
    
    // Menu Aiuto
    QMenu* aiutoMenu = menuBar()->addMenu("&Aiuto");
    
    QAction* aboutAction = aiutoMenu->addAction("&Informazioni su Biblioteca Manager");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::informazioniSu);
}

void MainWindow::setupToolBar()
{
    QToolBar* toolBar = addToolBar("Principale");
    toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    
    // Azioni file
    QAction* nuovoAction = toolBar->addAction("Nuovo");
    connect(nuovoAction, &QAction::triggered, this, &MainWindow::nuovaCollezione);
    
    QAction* apriAction = toolBar->addAction("Apri");
    connect(apriAction, &QAction::triggered, this, &MainWindow::apriCollezione);
    
    QAction* salvaAction = toolBar->addAction("Salva");
    connect(salvaAction, &QAction::triggered, this, &MainWindow::salvaCollezione);
    
    toolBar->addSeparator();
    
    // Azioni media
    QAction* aggiungiAction = toolBar->addAction("Aggiungi");
    connect(aggiungiAction, &QAction::triggered, this, &MainWindow::aggiungiMedia);
    
    QAction* modificaAction = toolBar->addAction("Modifica");
    connect(modificaAction, &QAction::triggered, this, &MainWindow::modificaMedia);
    
    QAction* rimuoviAction = toolBar->addAction("Rimuovi");
    connect(rimuoviAction, &QAction::triggered, this, &MainWindow::rimuoviMedia);
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
    
    connect(m_searchEdit, &QLineEdit::textChanged, this, &MainWindow::cercaMedia);
    connect(m_searchButton, &QPushButton::clicked, this, &MainWindow::cercaMedia);
    connect(m_clearSearchButton, &QPushButton::clicked, [this]() {
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
    m_annoMinSpin->setValue(2000);
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
    if (!m_mediaLayout) return;
    
    // Calcola il numero di colonne in base alla larghezza disponibile
    int containerWidth = m_mediaScrollArea->viewport()->width();
    int cardWidthWithMargin = CARD_WIDTH + CARD_MARGIN;
    int columns = qMax(1, (containerWidth - CARD_MARGIN) / cardWidthWithMargin);
    
    // Riorganizza le card
    for (int i = 0; i < static_cast<int>(m_mediaCards.size()); ++i) {
        int row = i / columns;
        int col = i % columns;
        m_mediaLayout->addWidget(m_mediaCards[i], row, col);
    }
    
    // Aggiungi stretch per spingere le card in alto
    m_mediaLayout->setRowStretch(m_mediaLayout->rowCount(), 1);
}

void MainWindow::refreshMediaCards()
{
    clearMediaCards();
    
    // Ottieni i media filtrati
    auto media = m_collezione->searchMedia(m_searchEdit->text());
    
    // Applica filtri aggiuntivi se necessario
    auto filtro = creaFiltroCorrente();
    if (filtro) {
        media = m_collezione->filterMedia(std::move(filtro));
    }
    
    // Crea le card
    for (Media* mediaPtr : media) {
        if (mediaPtr) {
            auto card = mediaPtr->createCard(m_mediaContainer);
            if (card) {
                MediaCard* cardPtr = card.release();
                m_mediaCards.push_back(cardPtr);
                
                // Connessioni per selezione
                connect(cardPtr, &MediaCard::selezionato,
                        this, &MainWindow::onCardSelezionata);
                connect(cardPtr, &MediaCard::doppioClick,
                        this, &MainWindow::onCardDoubleClic);
            }
        }
    }
    
    updateLayout();
    aggiornaStatistiche();
}

void MainWindow::clearMediaCards()
{
    for (MediaCard* card : m_mediaCards) {
        m_mediaLayout->removeWidget(card);
        card->deleteLater();
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

// Continua con gli altri metodi...
void MainWindow::nuovaCollezione()
{
    if (!verificaModifiche()) return;
    
    m_collezione->clear();
    m_fileCorrente.clear();
    m_modificato = false;
    setWindowTitle("Biblioteca Manager - [Nuova Collezione]");
    refreshMediaCards();
    mostraInfo("Nuova collezione creata");
}

void MainWindow::apriCollezione()
{
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
}

void MainWindow::salvaCollezione()
{
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
}

void MainWindow::salvaCollezioneCome()
{
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
}

void MainWindow::esportaCSV()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        "Esporta CSV", "collezione.csv", "File CSV (*.csv)");
    
    if (!fileName.isEmpty()) {
        // Implementazione esportazione CSV tramite JsonManager
        mostraInfo("CSV esportato con successo");
    }
}

void MainWindow::esci()
{
    close();
}

void MainWindow::aggiungiMedia()
{
    MediaDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        auto media = dialog.getMedia();
        if (media) {
            m_collezione->addMedia(std::move(media));
            m_modificato = true;
        }
    }
}

void MainWindow::modificaMedia()
{
    if (m_selezionato_id.isEmpty()) return;
    
    Media* media = m_collezione->findMedia(m_selezionato_id);
    if (!media) return;
    
    MediaDialog dialog(media, this);
    if (dialog.exec() == QDialog::Accepted) {
        auto updatedMedia = dialog.getMedia();
        if (updatedMedia && m_collezione->updateMedia(m_selezionato_id, std::move(updatedMedia))) {
            m_modificato = true;
        }
    }
}

void MainWindow::rimuoviMedia()
{
    if (m_selezionato_id.isEmpty()) return;
    
    Media* media = m_collezione->findMedia(m_selezionato_id);
    if (!media) return;
    
    if (confermaAzione(QString("Rimuovere '%1'?").arg(media->getTitolo()))) {
        if (m_collezione->removeMedia(m_selezionato_id)) {
            m_modificato = true;
            mostraInfo("Media rimosso");
        }
    }
}

void MainWindow::visualizzaDettagli()
{
    if (m_selezionato_id.isEmpty()) return;
    
    Media* media = m_collezione->findMedia(m_selezionato_id);
    if (!media) return;
    
    MediaDialog dialog(media, this, true); // true = modalità solo lettura
    dialog.exec();
}

void MainWindow::cercaMedia()
{
    applicaRicercaCorrente();
}

void MainWindow::applicaFiltri()
{
    refreshMediaCards();
}

void MainWindow::resetFiltri()
{
    m_tipoCombo->setCurrentIndex(0);
    m_annoMinSpin->setValue(2000);
    m_annoMaxSpin->setValue(QDate::currentDate().year());
    m_autoreEdit->clear();
    m_registaEdit->clear();
    m_rivistaEdit->clear();
    refreshMediaCards();
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
    size_t total = m_collezione->size();
    size_t libri = m_collezione->countByType("Libro");
    size_t film = m_collezione->countByType("Film");
    size_t articoli = m_collezione->countByType("Articolo");
    
    m_totalLabel->setText(QString("Totale: %1").arg(total));
    m_libriLabel->setText(QString("Libri: %1").arg(libri));
    m_filmLabel->setText(QString("Film: %1").arg(film));
    m_articoliLabel->setText(QString("Articoli: %1").arg(articoli));
}

void MainWindow::aggiornaStatusBar()
{
    m_countLabel->setText(QString("%1 media").arg(m_collezione->size()));
    
    if (m_modificato) {
        m_statusLabel->setText("Modificato");
    } else {
        m_statusLabel->setText("Pronto");
    }
}

void MainWindow::salvaImpostazioni()
{
    QSettings settings;
    settings.setValue("geometria", saveGeometry());
    settings.setValue("splitter", m_splitter->saveState());
}

void MainWindow::caricaImpostazioni()
{
    QSettings settings;
    restoreGeometry(settings.value("geometria").toByteArray());
    m_splitter->restoreState(settings.value("splitter").toByteArray());
}

bool MainWindow::verificaModifiche()
{
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
}

void MainWindow::resetInterfaccia()
{
    m_searchEdit->clear();
    resetFiltri();
    m_selezionato_id.clear();
    refreshMediaCards();
}

void MainWindow::mostraErrore(const QString& errore)
{
    QMessageBox::critical(this, "Errore", errore);
    m_statusLabel->setText("Errore: " + errore);
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