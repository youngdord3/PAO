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

void MainWindow::setupEditBaseForm()
{
    if (!m_editFormLayout) {
        qWarning() << "EditFormLayout non inizializzato";
        return;
    }
    
    try {
        m_editBaseGroup = new QGroupBox("Informazioni Base");
        if (!m_editBaseGroup) {
            throw std::runtime_error("Impossibile creare base group");
        }
        
        QFormLayout* baseLayout = new QFormLayout(m_editBaseGroup);
        if (!baseLayout) {
            throw std::runtime_error("Impossibile creare base layout");
        }
        
        baseLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
        
        // Tipo
        m_editTipoCombo = new QComboBox();
        if (m_editTipoCombo) {
            m_editTipoCombo->addItems({"Libro", "Film", "Articolo"});
            baseLayout->addRow("Tipo:", m_editTipoCombo);
        }
        
        // Titolo
        m_editTitoloEdit = new QLineEdit();
        if (m_editTitoloEdit) {
            m_editTitoloEdit->setMaxLength(200);
            m_editTitoloEdit->setPlaceholderText("Inserire il titolo...");
            baseLayout->addRow("Titolo*:", m_editTitoloEdit);
        }
        
        // Anno
        m_editAnnoSpin = new QSpinBox();
        if (m_editAnnoSpin) {
            m_editAnnoSpin->setRange(1000, QDate::currentDate().year() + 10);
            m_editAnnoSpin->setValue(QDate::currentDate().year());
            baseLayout->addRow("Anno*:", m_editAnnoSpin);
        }
        
        // Descrizione
        m_editDescrizioneEdit = new QTextEdit();
        if (m_editDescrizioneEdit) {
            m_editDescrizioneEdit->setMaximumHeight(80);
            m_editDescrizioneEdit->setPlaceholderText("Inserire una breve descrizione...");
            baseLayout->addRow("Descrizione:", m_editDescrizioneEdit);
        }
        
        m_editFormLayout->addWidget(m_editBaseGroup);
        
    } catch (const std::exception& e) {
        qWarning() << "Errore in setupEditBaseForm:" << e.what();
    }
}


void MainWindow::setupEditLibroForm()
{
    if (!m_editFormLayout) return;
    
    m_editLibroGroup = new QGroupBox("Dettagli Libro");
    if (!m_editLibroGroup) return;
    
    QFormLayout* libroLayout = new QFormLayout(m_editLibroGroup);
    
    // Autore
    m_editAutoreEdit = new QLineEdit();
    if (m_editAutoreEdit) {
        m_editAutoreEdit->setPlaceholderText("Nome dell'autore...");
        libroLayout->addRow("Autore*:", m_editAutoreEdit);
    }
    
    // Editore
    m_editEditoreEdit = new QLineEdit();
    if (m_editEditoreEdit) {
        m_editEditoreEdit->setPlaceholderText("Casa editrice...");
        libroLayout->addRow("Editore:", m_editEditoreEdit);
    }
    
    // Pagine
    m_editPagineSpin = new QSpinBox();
    if (m_editPagineSpin) {
        m_editPagineSpin->setRange(1, 10000);
        m_editPagineSpin->setValue(200);
        libroLayout->addRow("Pagine*:", m_editPagineSpin);
    }
    
    // ISBN
    m_editIsbnEdit = new QLineEdit();
    if (m_editIsbnEdit) {
        m_editIsbnEdit->setPlaceholderText("ISBN-10 o ISBN-13...");
        QRegularExpressionValidator* isbnValidator = 
            new QRegularExpressionValidator(QRegularExpression("^[0-9X-]{10,17}$"), this);
        m_editIsbnEdit->setValidator(isbnValidator);
        libroLayout->addRow("ISBN:", m_editIsbnEdit);
    }
    
    // Genere
    m_editGenereLibroCombo = new QComboBox();
    if (m_editGenereLibroCombo) {
        m_editGenereLibroCombo->addItems(Libro::getAllGeneri());
        libroLayout->addRow("Genere:", m_editGenereLibroCombo);
    }
    
    m_editFormLayout->addWidget(m_editLibroGroup);
}

void MainWindow::setupEditFilmForm()
{
    if (!m_editFormLayout) return;
    
    m_editFilmGroup = new QGroupBox("Dettagli Film");
    if (!m_editFilmGroup) return;
    
    QFormLayout* filmLayout = new QFormLayout(m_editFilmGroup);
    
    // Regista
    m_editRegistaEdit = new QLineEdit();
    if (m_editRegistaEdit) {
        m_editRegistaEdit->setPlaceholderText("Nome del regista...");
        filmLayout->addRow("Regista*:", m_editRegistaEdit);
    }
    
    // Attori
    QWidget* attoriWidget = new QWidget();
    if (attoriWidget) {
        QVBoxLayout* attoriLayout = new QVBoxLayout(attoriWidget);
        attoriLayout->setContentsMargins(0, 0, 0, 0);
        
        m_editAttoriList = new QListWidget();
        if (m_editAttoriList) {
            m_editAttoriList->setMaximumHeight(80);
            attoriLayout->addWidget(m_editAttoriList);
        }
        
        QHBoxLayout* attoriButtonLayout = new QHBoxLayout();
        if (attoriButtonLayout) {
            m_editNuovoAttoreEdit = new QLineEdit();
            if (m_editNuovoAttoreEdit) {
                m_editNuovoAttoreEdit->setPlaceholderText("Nome attore...");
                attoriButtonLayout->addWidget(m_editNuovoAttoreEdit);
            }
            
            m_editAggiungiAttoreBtn = new QPushButton("Aggiungi");
            if (m_editAggiungiAttoreBtn) {
                attoriButtonLayout->addWidget(m_editAggiungiAttoreBtn);
            }
            
            m_editRimuoviAttoreBtn = new QPushButton("Rimuovi");
            if (m_editRimuoviAttoreBtn) {
                attoriButtonLayout->addWidget(m_editRimuoviAttoreBtn);
            }
            
            attoriLayout->addLayout(attoriButtonLayout);
        }
        
        filmLayout->addRow("Attori*:", attoriWidget);
    }
    
    // Durata
    m_editDurataSpin = new QSpinBox();
    if (m_editDurataSpin) {
        m_editDurataSpin->setRange(1, 1000);
        m_editDurataSpin->setValue(90);
        m_editDurataSpin->setSuffix(" min");
        filmLayout->addRow("Durata*:", m_editDurataSpin);
    }
    
    // Genere
    m_editGenereFilmCombo = new QComboBox();
    if (m_editGenereFilmCombo) {
        m_editGenereFilmCombo->addItems(Film::getAllGeneri());
        filmLayout->addRow("Genere:", m_editGenereFilmCombo);
    }
    
    // Classificazione
    m_editClassificazioneCombo = new QComboBox();
    if (m_editClassificazioneCombo) {
        m_editClassificazioneCombo->addItems(Film::getAllClassificazioni());
        filmLayout->addRow("Classificazione:", m_editClassificazioneCombo);
    }
    
    // Casa di produzione
    m_editCasaProduzioneEdit = new QLineEdit();
    if (m_editCasaProduzioneEdit) {
        m_editCasaProduzioneEdit->setPlaceholderText("Nome casa di produzione...");
        filmLayout->addRow("Casa Produzione:", m_editCasaProduzioneEdit);
    }
    
    m_editFormLayout->addWidget(m_editFilmGroup);
}

void MainWindow::setupEditArticoloForm()
{
    if (!m_editFormLayout) return;
    
    m_editArticoloGroup = new QGroupBox("Dettagli Articolo");
    if (!m_editArticoloGroup) return;
    
    QFormLayout* articoloLayout = new QFormLayout(m_editArticoloGroup);
    
    // Autori
    QWidget* autoriWidget = new QWidget();
    if (autoriWidget) {
        QVBoxLayout* autoriLayout = new QVBoxLayout(autoriWidget);
        autoriLayout->setContentsMargins(0, 0, 0, 0);
        
        m_editAutoriList = new QListWidget();
        if (m_editAutoriList) {
            m_editAutoriList->setMaximumHeight(80);
            autoriLayout->addWidget(m_editAutoriList);
        }
        
        QHBoxLayout* autoriButtonLayout = new QHBoxLayout();
        if (autoriButtonLayout) {
            m_editNuovoAutoreEdit = new QLineEdit();
            if (m_editNuovoAutoreEdit) {
                m_editNuovoAutoreEdit->setPlaceholderText("Nome autore...");
                autoriButtonLayout->addWidget(m_editNuovoAutoreEdit);
            }
            
            m_editAggiungiAutoreBtn = new QPushButton("Aggiungi");
            if (m_editAggiungiAutoreBtn) {
                autoriButtonLayout->addWidget(m_editAggiungiAutoreBtn);
            }
            
            m_editRimuoviAutoreBtn = new QPushButton("Rimuovi");
            if (m_editRimuoviAutoreBtn) {
                autoriButtonLayout->addWidget(m_editRimuoviAutoreBtn);
            }
            
            autoriLayout->addLayout(autoriButtonLayout);
        }
        
        articoloLayout->addRow("Autori*:", autoriWidget);
    }
    
    // Rivista
    m_editRivisteEdit = new QLineEdit();
    if (m_editRivisteEdit) {
        m_editRivisteEdit->setPlaceholderText("Nome della rivista...");
        articoloLayout->addRow("Rivista*:", m_editRivisteEdit);
    }
    
    // Data pubblicazione
    m_editDataPubblicazioneEdit = new QDateEdit();
    if (m_editDataPubblicazioneEdit) {
        m_editDataPubblicazioneEdit->setDate(QDate::currentDate());
        m_editDataPubblicazioneEdit->setCalendarPopup(true);
        articoloLayout->addRow("Data Pubblicazione*:", m_editDataPubblicazioneEdit);
    }
    
    // Volume e Numero
    QWidget* volNumWidget = new QWidget();
    if (volNumWidget) {
        QHBoxLayout* volNumLayout = new QHBoxLayout(volNumWidget);
        volNumLayout->setContentsMargins(0, 0, 0, 0);
        
        m_editVolumeEdit = new QLineEdit();
        m_editNumeroEdit = new QLineEdit();
        
        if (m_editVolumeEdit && m_editNumeroEdit) {
            m_editVolumeEdit->setPlaceholderText("Vol.");
            m_editNumeroEdit->setPlaceholderText("Num.");
            volNumLayout->addWidget(new QLabel("Vol:"));
            volNumLayout->addWidget(m_editVolumeEdit);
            volNumLayout->addWidget(new QLabel("Num:"));
            volNumLayout->addWidget(m_editNumeroEdit);
        }
        
        articoloLayout->addRow("Volume/Numero:", volNumWidget);
    }
    
    // Pagine
    m_editPagineEdit = new QLineEdit();
    if (m_editPagineEdit) {
        m_editPagineEdit->setPlaceholderText("es. 123-130 o 45");
        articoloLayout->addRow("Pagine:", m_editPagineEdit);
    }
    
    // Categoria
    m_editCategoriaCombo = new QComboBox();
    if (m_editCategoriaCombo) {
        m_editCategoriaCombo->addItems(Articolo::getAllCategorie());
        articoloLayout->addRow("Categoria:", m_editCategoriaCombo);
    }
    
    // Tipo rivista
    m_editTipoRivistaCombo = new QComboBox();
    if (m_editTipoRivistaCombo) {
        m_editTipoRivistaCombo->addItems(Articolo::getAllTipiRivista());
        articoloLayout->addRow("Tipo Rivista:", m_editTipoRivistaCombo);
    }
    
    // DOI
    m_editDoiEdit = new QLineEdit();
    if (m_editDoiEdit) {
        m_editDoiEdit->setPlaceholderText("es. 10.1000/182");
        QRegularExpressionValidator* doiValidator = 
            new QRegularExpressionValidator(QRegularExpression("^10\\.\\d{4,}/\\S+$"), this);
        m_editDoiEdit->setValidator(doiValidator);
        articoloLayout->addRow("DOI:", m_editDoiEdit);
    }
    
    m_editFormLayout->addWidget(m_editArticoloGroup);
}

void MainWindow::setupEditConnections()
{
    try {
        if (m_editTitoloEdit) {
            connect(m_editTitoloEdit, &QLineEdit::textChanged, this, [this]() {
                scheduleValidation();
            });
        }
        
        if (m_editAnnoSpin) {
            connect(m_editAnnoSpin, QOverload<int>::of(&QSpinBox::valueChanged), 
                    this, [this]() {
                        scheduleValidation();
                    });
        }
        
        if (m_editDescrizioneEdit) {
            connect(m_editDescrizioneEdit, &QTextEdit::textChanged, this, [this]() {
                scheduleValidation();
            });
        }
        
        if (m_editTipoCombo) {
            connect(m_editTipoCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
                    this, &MainWindow::onEditTipoChanged);
        }
    } catch (const std::exception& e) {
        qWarning() << "Errore in setupEditConnections:" << e.what();
    }
}

void MainWindow::setupEditSpecificConnections()
{
    if (m_editTypeChanging) {
        qDebug() << "Skip connessioni specifiche - cambio tipo in corso";
        return;
    }
    
    QString tipo = m_editTipoCombo ? m_editTipoCombo->currentText() : "";
    qDebug() << "Setup connessioni specifiche per:" << tipo;
    
    if (tipo == "Libro") {
        if (m_editAutoreEdit) {
            connect(m_editAutoreEdit, &QLineEdit::textChanged, this, [this]() {
                scheduleValidation();
            });
        }
        if (m_editPagineSpin) {
            connect(m_editPagineSpin, QOverload<int>::of(&QSpinBox::valueChanged), 
                    this, [this]() {
                        scheduleValidation();
                    });
        }
        if (m_editEditoreEdit) {
            connect(m_editEditoreEdit, &QLineEdit::textChanged, this, [this]() {
                scheduleValidation();
            });
        }
        if (m_editIsbnEdit) {
            connect(m_editIsbnEdit, &QLineEdit::textChanged, this, [this]() {
                scheduleValidation();
            });
        }
        if (m_editGenereLibroCombo) {
            connect(m_editGenereLibroCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
                    this, [this]() {
                        scheduleValidation();
                    });
        }
        
    } else if (tipo == "Film") {
        if (m_editRegistaEdit) {
            connect(m_editRegistaEdit, &QLineEdit::textChanged, this, [this]() {
                scheduleValidation();
            });
        }
        if (m_editDurataSpin) {
            connect(m_editDurataSpin, QOverload<int>::of(&QSpinBox::valueChanged), 
                    this, [this]() {
                        scheduleValidation();
                    });
        }
        if (m_editCasaProduzioneEdit) {
            connect(m_editCasaProduzioneEdit, &QLineEdit::textChanged, this, [this]() {
                scheduleValidation();
            });
        }
        if (m_editGenereFilmCombo) {
            connect(m_editGenereFilmCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
                    this, [this]() {
                        scheduleValidation();
                    });
        }
        if (m_editClassificazioneCombo) {
            connect(m_editClassificazioneCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
                    this, [this]() {
                        scheduleValidation();
                    });
        }
        
        if (m_editAggiungiAttoreBtn) {
            qDebug() << "Connetto bottone aggiungi attore con Qt::UniqueConnection";
            connect(m_editAggiungiAttoreBtn, &QPushButton::clicked, 
                    this, &MainWindow::onEditAggiungiAttoreClicked, Qt::UniqueConnection);
        }
        if (m_editRimuoviAttoreBtn) {
            qDebug() << "Connetto bottone rimuovi attore con Qt::UniqueConnection";
            connect(m_editRimuoviAttoreBtn, &QPushButton::clicked, 
                    this, &MainWindow::onEditRimuoviAttoreClicked, Qt::UniqueConnection);
        }
        if (m_editNuovoAttoreEdit) {
            qDebug() << "Connetto enter su nuovo attore con Qt::UniqueConnection";
            connect(m_editNuovoAttoreEdit, &QLineEdit::returnPressed, 
                    this, &MainWindow::onEditAggiungiAttoreClicked, Qt::UniqueConnection);
        }
        
        // Connessione per validazione
        if (m_editAttoriList) {
            connect(m_editAttoriList, &QListWidget::itemChanged, this, [this]() {
                scheduleValidation();
            });
        }
        
    } else if (tipo == "Articolo") {
        if (m_editRivisteEdit) {
            connect(m_editRivisteEdit, &QLineEdit::textChanged, this, [this]() {
                scheduleValidation();
            });
        }
        if (m_editDataPubblicazioneEdit) {
            connect(m_editDataPubblicazioneEdit, &QDateEdit::dateChanged, 
                    this, [this]() {
                        scheduleValidation();
                    });
        }
        if (m_editVolumeEdit) {
            connect(m_editVolumeEdit, &QLineEdit::textChanged, this, [this]() {
                scheduleValidation();
            });
        }
        if (m_editNumeroEdit) {
            connect(m_editNumeroEdit, &QLineEdit::textChanged, this, [this]() {
                scheduleValidation();
            });
        }
        if (m_editPagineEdit) {
            connect(m_editPagineEdit, &QLineEdit::textChanged, this, [this]() {
                scheduleValidation();
            });
        }
        if (m_editCategoriaCombo) {
            connect(m_editCategoriaCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
                    this, [this]() {
                        scheduleValidation();
                    });
        }
        if (m_editTipoRivistaCombo) {
            connect(m_editTipoRivistaCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
                    this, [this]() {
                        scheduleValidation();
                    });
        }
        if (m_editDoiEdit) {
            connect(m_editDoiEdit, &QLineEdit::textChanged, this, [this]() {
                scheduleValidation();
            });
        }
        
        if (m_editAggiungiAutoreBtn) {
            qDebug() << "Connetto bottone aggiungi autore con Qt::UniqueConnection";
            connect(m_editAggiungiAutoreBtn, &QPushButton::clicked, 
                    this, &MainWindow::onEditAggiungiAutoreClicked, Qt::UniqueConnection);
        }
        if (m_editRimuoviAutoreBtn) {
            qDebug() << "Connetto bottone rimuovi autore con Qt::UniqueConnection";
            connect(m_editRimuoviAutoreBtn, &QPushButton::clicked, 
                    this, &MainWindow::onEditRimuoviAutoreClicked, Qt::UniqueConnection);
        }
        if (m_editNuovoAutoreEdit) {
            qDebug() << "Connetto enter su nuovo autore con Qt::UniqueConnection";
            connect(m_editNuovoAutoreEdit, &QLineEdit::returnPressed, 
                    this, &MainWindow::onEditAggiungiAutoreClicked, Qt::UniqueConnection);
        }
        
        // Connessione per validazione
        if (m_editAutoriList) {
            connect(m_editAutoriList, &QListWidget::itemChanged, this, [this]() {
                scheduleValidation();
            });
        }
    }
    
    qDebug() << "Connessioni specifiche completate per:" << tipo;
}

void MainWindow::clearEditSpecificForm()
{
    try {
        if (!m_editFormLayout) return;
        
        // Disconnetti tutti i widget prima di rimuoverli
        if (m_editLibroGroup) {
            disconnectEditGroupWidgets(m_editLibroGroup);
            m_editFormLayout->removeWidget(m_editLibroGroup);
            m_editLibroGroup->deleteLater();
            m_editLibroGroup = nullptr;
        }
        
        if (m_editFilmGroup) {
            disconnectEditGroupWidgets(m_editFilmGroup);
            m_editFormLayout->removeWidget(m_editFilmGroup);
            m_editFilmGroup->deleteLater();
            m_editFilmGroup = nullptr;
        }
        
        if (m_editArticoloGroup) {
            disconnectEditGroupWidgets(m_editArticoloGroup);
            m_editFormLayout->removeWidget(m_editArticoloGroup);
            m_editArticoloGroup->deleteLater();
            m_editArticoloGroup = nullptr;
        }
        
        // Reset di tutti puntatori specifici
        resetEditSpecificPointers();
        
        // Forza il processamento degli eventi per completare la cancellazione
        QApplication::processEvents();
        
    } catch (const std::exception& e) {
        qWarning() << "Errore in clearEditSpecificForm:" << e.what();
    }
}

void MainWindow::disconnectEditGroupWidgets(QGroupBox* group)
{
    if (!group) return;
    
    try {
        // Trova tutti i widget figli e disconnettili
        QList<QWidget*> widgets = group->findChildren<QWidget*>();
        for (QWidget* widget : widgets) {
            if (widget) {
                disconnect(widget, nullptr, this, nullptr);
            }
        }
    } catch (const std::exception& e) {
        qWarning() << "Errore in disconnectEditGroupWidgets:" << e.what();
    }
}

void MainWindow::resetEditSpecificPointers()
{
    // Reset puntatori libro
    m_editAutoreEdit = nullptr;
    m_editEditoreEdit = nullptr;
    m_editPagineSpin = nullptr;
    m_editIsbnEdit = nullptr;
    m_editGenereLibroCombo = nullptr;
    
    // Reset puntatori film
    m_editRegistaEdit = nullptr;
    m_editAttoriList = nullptr;
    m_editNuovoAttoreEdit = nullptr;
    m_editAggiungiAttoreBtn = nullptr;
    m_editRimuoviAttoreBtn = nullptr;
    m_editDurataSpin = nullptr;
    m_editGenereFilmCombo = nullptr;
    m_editClassificazioneCombo = nullptr;
    m_editCasaProduzioneEdit = nullptr;
    
    // Reset puntatori articolo
    m_editAutoriList = nullptr;
    m_editNuovoAutoreEdit = nullptr;
    m_editAggiungiAutoreBtn = nullptr;
    m_editRimuoviAutoreBtn = nullptr;
    m_editRivisteEdit = nullptr;
    m_editVolumeEdit = nullptr;
    m_editNumeroEdit = nullptr;
    m_editPagineEdit = nullptr;
    m_editCategoriaCombo = nullptr;
    m_editTipoRivistaCombo = nullptr;
    m_editDataPubblicazioneEdit = nullptr;
    m_editDoiEdit = nullptr;
}

void MainWindow::setupEditPanel()
{
    // Verifica che il container principale esista
    if (!m_editContentContainer) {
        qWarning() << "Container principale non inizializzato";
        return;
    }
    
    try {
        m_editPanel = new QWidget();
        if (!m_editPanel) {
            throw std::runtime_error("Impossibile creare editPanel");
        }
        
        m_editPanel->setVisible(false);
        m_editPanel->setObjectName("editPanel");
        
        QVBoxLayout* editLayout = new QVBoxLayout(m_editPanel);
        if (!editLayout) {
            throw std::runtime_error("Impossibile creare layout editPanel");
        }
        
        editLayout->setContentsMargins(10, 10, 10, 10);
        editLayout->setSpacing(10);
        
        // Header del pannello
        QHBoxLayout* headerLayout = new QHBoxLayout();
        m_editHeaderLabel = new QLabel("Modifica Media");
        if (!m_editHeaderLabel) {
            throw std::runtime_error("Impossibile creare header label");
        }
        
        m_editHeaderLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #1976D2; padding: 5px;");
        
        headerLayout->addWidget(m_editHeaderLabel);
        headerLayout->addStretch();
        
        editLayout->addLayout(headerLayout);
        
        // Scroll area per il form
        m_editScrollArea = new QScrollArea();
        if (!m_editScrollArea) {
            throw std::runtime_error("Impossibile creare scroll area");
        }
        
        m_editScrollArea->setWidgetResizable(true);
        m_editScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        m_editScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        
        QWidget* formWidget = new QWidget();
        if (!formWidget) {
            throw std::runtime_error("Impossibile creare form widget");
        }
        
        m_editFormLayout = new QVBoxLayout(formWidget);
        if (!m_editFormLayout) {
            throw std::runtime_error("Impossibile creare form layout");
        }
        
        m_editFormLayout->setContentsMargins(10, 10, 10, 10);
        m_editFormLayout->setSpacing(15);
        
        setupEditBaseForm();
        
        m_editScrollArea->setWidget(formWidget);
        editLayout->addWidget(m_editScrollArea);
        
        // Validation label
        m_editValidationLabel = new QLabel();
        if (m_editValidationLabel) {
            m_editValidationLabel->setWordWrap(true);
            m_editValidationLabel->setMinimumHeight(25);
            editLayout->addWidget(m_editValidationLabel);
        }
        
        // Bottoni del pannello
        QHBoxLayout* buttonLayout = new QHBoxLayout();
        
        m_editHelpButton = new QPushButton("Aiuto");
        m_editAnnullaButton = new QPushButton("Chiudi");
        m_editSalvaButton = new QPushButton("Salva");
        
        if (m_editSalvaButton) {
            m_editSalvaButton->setDefault(true);
        }
        
        // Connessioni sicure
        if (m_editHelpButton) {
            connect(m_editHelpButton, &QPushButton::clicked, [this]() {
                QMessageBox::information(this, "Aiuto", 
                    "I campi contrassegnati con * sono obbligatori.\n\n"
                    "Tipo: Seleziona il tipo di media da creare.\n"
                    "Titolo: Nome del media (obbligatorio).\n"
                    "Anno: Anno di pubblicazione (obbligatorio).\n"
                    "Descrizione: Breve descrizione del contenuto.\n\n"
                    "Ogni tipo di media ha campi specifici aggiuntivi.");
            });
        }
        
        if (m_editAnnullaButton) {
            connect(m_editAnnullaButton, &QPushButton::clicked, this, &MainWindow::onEditAnnullaClicked);
        }
        
        if (m_editSalvaButton) {
            connect(m_editSalvaButton, &QPushButton::clicked, this, &MainWindow::onEditSalvaClicked);
        }
        
        buttonLayout->addStretch();
        if (m_editHelpButton) buttonLayout->addWidget(m_editHelpButton);
        if (m_editAnnullaButton) buttonLayout->addWidget(m_editAnnullaButton);
        if (m_editSalvaButton) buttonLayout->addWidget(m_editSalvaButton);
        
        editLayout->addLayout(buttonLayout);
        
        setupEditConnections();
        
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore nella creazione pannello edit: %1").arg(e.what()));
        
        // Cleanup in caso di errore
        if (m_editPanel) {
            m_editPanel->deleteLater();
            m_editPanel = nullptr;
        }
    }
}

void MainWindow::showEditPanel(bool isNew, bool readOnly)
{
    try {
        if (!m_editPanel || !m_mediaScrollArea || !m_editContentContainer) {
            mostraErrore("Pannello di editing non inizializzato correttamente");
            return;
        }
        
        // Reset completo dello stato del pannello quando cambia modalità
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

void MainWindow::setupEditTypeSpecificForm()
{
    try {
        clearEditSpecificForm();
        
        if (!m_editTipoCombo) {
            qWarning() << "TipoCombo non inizializzato in setupEditTypeSpecificForm";
            return;
        }
        
        QString tipo = m_editTipoCombo->currentText();
        m_editTipoCorrente = tipo;
        
        qDebug() << "Setup form specifico per tipo:" << tipo;
        
        if (tipo == "Libro") {
            setupEditLibroForm();
        } else if (tipo == "Film") {
            setupEditFilmForm();
        } else if (tipo == "Articolo") {
            setupEditArticoloForm();
        } else {
            qWarning() << "Tipo non riconosciuto in setupEditTypeSpecificForm:" << tipo;
        }
        
        // Setup connessioni specifiche per il tipo
        setupEditSpecificConnections();
        
        qDebug() << "Form specifico completato per tipo:" << tipo;
        
    } catch (const std::exception& e) {
        mostraErrore(QString("Errore nel setup form specifico: %1").arg(e.what()));
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
        
        if (nuovoTipo != m_editTipoCorrente) {
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
        
        if (validateEditInput()) {
            auto media = createEditMedia();
            if (media && media->isValid()) {
                if (m_editIsNew) {
                    // Nuovo media
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
            } else {
                mostraErrore("Media non valido - verificare i dati inseriti");
            }
        } else {
            QStringList errors = getEditValidationErrors();
            QMessageBox::warning(this, "Errori di Validazione", 
                                "Correggere i seguenti errori:\n\n" + errors.join("\n"));
        }
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

        for (int i = 0; i < m_editAutoriList->count(); ++i) {
            if (m_editAutoriList->item(i)->text().trimmed() == autore) {
                QMessageBox::information(this, "Info", 
                    QString("L'autore '%1' è già presente nella lista").arg(autore));
                m_editNuovoAutoreEdit->clear();
                m_editNuovoAutoreEdit->setFocus();
                return;
            }
        }

        m_editAutoriList->addItem(autore);
        m_editNuovoAutoreEdit->clear();
        m_editNuovoAutoreEdit->setFocus();
        
        qDebug() << "Autore aggiunto:" << autore << "- Totale:" << m_editAutoriList->count();
        
        // Trigger validazione immediata
        QTimer::singleShot(100, this, [this]() {
            if (m_editValidationEnabled && !m_editTypeChanging) {
                scheduleValidation();
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
        
        for (int i = 0; i < m_editAttoriList->count(); ++i) {
            if (m_editAttoriList->item(i)->text().trimmed() == attore) {
                QMessageBox::information(this, "Info", 
                    QString("L'attore '%1' è già presente nella lista").arg(attore));
                m_editNuovoAttoreEdit->clear();
                m_editNuovoAttoreEdit->setFocus();
                return;
            }
        }
        
        m_editAttoriList->addItem(attore);
        m_editNuovoAttoreEdit->clear();
        m_editNuovoAttoreEdit->setFocus();
        
        qDebug() << "Attore aggiunto:" << attore << "- Totale:" << m_editAttoriList->count();
        
        // Trigger validazione immediata
        QTimer::singleShot(100, this, [this]() {
            if (m_editValidationEnabled && !m_editTypeChanging) {
                scheduleValidation();
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
        // Validazione base
        if (!m_editTitoloEdit || m_editTitoloEdit->text().trimmed().isEmpty()) {
            errors << "Il titolo è obbligatorio";
        }
        
        if (!m_editAnnoSpin || m_editAnnoSpin->value() <= 0) {
            errors << "L'anno deve essere positivo";
        }
        
        // Validazione specifica per tipo
        if (!m_editTipoCombo) {
            errors << "Tipo non selezionato";
            return errors;
        }
        
        QString tipo = m_editTipoCombo->currentText();
        
        if (tipo == "Libro") {
            if (!m_editAutoreEdit || m_editAutoreEdit->text().trimmed().isEmpty()) {
                errors << "L'autore è obbligatorio per i libri";
            }
            if (!m_editPagineSpin || m_editPagineSpin->value() <= 0) {
                errors << "Il numero di pagine deve essere positivo";
            }
        } else if (tipo == "Film") {
            if (!m_editRegistaEdit || m_editRegistaEdit->text().trimmed().isEmpty()) {
                errors << "Il regista è obbligatorio per i film";
            }
            if (!m_editAttoriList || m_editAttoriList->count() == 0) {
                errors << "Almeno un attore è richiesto";
            }
            if (!m_editDurataSpin || m_editDurataSpin->value() <= 0) {
                errors << "La durata deve essere positiva";
            }
        } else if (tipo == "Articolo") {
            if (!m_editAutoriList || m_editAutoriList->count() == 0) {
                errors << "Almeno un autore è richiesto";
            }
            if (!m_editRivisteEdit || m_editRivisteEdit->text().trimmed().isEmpty()) {
                errors << "Il nome della rivista è obbligatorio";
            }
            if (!m_editDataPubblicazioneEdit || !m_editDataPubblicazioneEdit->date().isValid()) {
                errors << "La data di pubblicazione non è valida";
            }
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