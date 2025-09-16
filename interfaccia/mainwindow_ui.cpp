#include "mainwindow.h"
#include "mediacard.h"
#include "modello_logico/collezione.h"
#include "modello_logico/filtrostrategy.h"
#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
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
#include <QDate>

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
    
    // Icone caricate dal resources.qrc
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
    
    QHBoxLayout* mainLayout = new QHBoxLayout(m_centralWidget);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    
    m_splitter = new QSplitter(Qt::Horizontal);
    mainLayout->addWidget(m_splitter);
    
    setupFilterArea();
    
    // Container per contenuto principale (media + edit panel)
    m_editContentContainer = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(m_editContentContainer);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);
    
    setupMediaArea();
    setupEditPanel();
    
    // Aggiungi entrambe le aree al container
    contentLayout->addWidget(m_mediaScrollArea);
    contentLayout->addWidget(m_editPanel);
    
    m_splitter->addWidget(m_editContentContainer);
    
    // Imposta proporzioni iniziali
    m_splitter->setSizes({FILTER_WIDTH, width() - FILTER_WIDTH});
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 1);
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
    
    // Pulsanti con icone dal resources.qrc
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
}