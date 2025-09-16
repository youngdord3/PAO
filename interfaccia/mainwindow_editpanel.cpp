#include "mainwindow.h"
#include "modello_logico/collezione.h"
#include "modello_logico/libro.h"
#include "modello_logico/film.h"
#include "modello_logico/articolo.h"
#include <QApplication>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QScrollArea>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QListWidget>
#include <QDateEdit>
#include <QTimer>
#include <QDebug>
#include <QDate>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

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
        // Connessioni per validazione
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
        // Connessioni per validazione
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
        
        // Connessioni per i bottoni
        if (m_editAggiungiAttoreBtn) {
            qDebug() << "Connetto bottone aggiungi attore";
            connect(m_editAggiungiAttoreBtn, &QPushButton::clicked, 
                    this, &MainWindow::onEditAggiungiAttoreClicked);
        }
        if (m_editRimuoviAttoreBtn) {
            qDebug() << "Connetto bottone rimuovi attore";
            connect(m_editRimuoviAttoreBtn, &QPushButton::clicked, 
                    this, &MainWindow::onEditRimuoviAttoreClicked);
        }
        if (m_editNuovoAttoreEdit) {
            qDebug() << "Connetto enter su nuovo attore";
            connect(m_editNuovoAttoreEdit, &QLineEdit::returnPressed, 
                    this, &MainWindow::onEditAggiungiAttoreClicked);
        }
        
    } else if (tipo == "Articolo") {
        // Connessioni per validazione
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
        
        // Connessioni per i bottoni
        if (m_editAggiungiAutoreBtn) {
            qDebug() << "Connetto bottone aggiungi autore";
            connect(m_editAggiungiAutoreBtn, &QPushButton::clicked, 
                    this, &MainWindow::onEditAggiungiAutoreClicked);
        }
        if (m_editRimuoviAutoreBtn) {
            qDebug() << "Connetto bottone rimuovi autore";
            connect(m_editRimuoviAutoreBtn, &QPushButton::clicked, 
                    this, &MainWindow::onEditRimuoviAutoreClicked);
        }
        if (m_editNuovoAutoreEdit) {
            qDebug() << "Connetto enter su nuovo autore";
            connect(m_editNuovoAutoreEdit, &QLineEdit::returnPressed, 
                    this, &MainWindow::onEditAggiungiAutoreClicked);
        }
    }
    
    qDebug() << "Connessioni specifiche completate per:" << tipo;
}

void MainWindow::clearEditSpecificForm()
{
    try {
        if (!m_editFormLayout) return;
        
        if (m_editLibroGroup) {
            // Disconnetti i widget specifici del libro
            if (m_editAutoreEdit) disconnect(m_editAutoreEdit, nullptr, this, nullptr);
            if (m_editEditoreEdit) disconnect(m_editEditoreEdit, nullptr, this, nullptr);
            if (m_editPagineSpin) disconnect(m_editPagineSpin, nullptr, this, nullptr);
            if (m_editIsbnEdit) disconnect(m_editIsbnEdit, nullptr, this, nullptr);
            if (m_editGenereLibroCombo) disconnect(m_editGenereLibroCombo, nullptr, this, nullptr);
            
            m_editFormLayout->removeWidget(m_editLibroGroup);
            m_editLibroGroup->deleteLater();
            m_editLibroGroup = nullptr;
        }
        
        if (m_editFilmGroup) {
            // Disconnetti i widget specifici del film
            if (m_editRegistaEdit) disconnect(m_editRegistaEdit, nullptr, this, nullptr);
            if (m_editDurataSpin) disconnect(m_editDurataSpin, nullptr, this, nullptr);
            if (m_editCasaProduzioneEdit) disconnect(m_editCasaProduzioneEdit, nullptr, this, nullptr);
            if (m_editGenereFilmCombo) disconnect(m_editGenereFilmCombo, nullptr, this, nullptr);
            if (m_editClassificazioneCombo) disconnect(m_editClassificazioneCombo, nullptr, this, nullptr);
            if (m_editAttoriList) disconnect(m_editAttoriList, nullptr, this, nullptr);
            
            // Disconnetti i bottoni per evitare connessioni doppie
            if (m_editAggiungiAttoreBtn) {
                disconnect(m_editAggiungiAttoreBtn, nullptr, this, nullptr);
            }
            if (m_editRimuoviAttoreBtn) {
                disconnect(m_editRimuoviAttoreBtn, nullptr, this, nullptr);
            }
            if (m_editNuovoAttoreEdit) {
                disconnect(m_editNuovoAttoreEdit, nullptr, this, nullptr);
            }
            
            m_editFormLayout->removeWidget(m_editFilmGroup);
            m_editFilmGroup->deleteLater();
            m_editFilmGroup = nullptr;
        }
        
        if (m_editArticoloGroup) {
            // Disconnetti i widget specifici dell'articolo
            if (m_editRivisteEdit) disconnect(m_editRivisteEdit, nullptr, this, nullptr);
            if (m_editDataPubblicazioneEdit) disconnect(m_editDataPubblicazioneEdit, nullptr, this, nullptr);
            if (m_editVolumeEdit) disconnect(m_editVolumeEdit, nullptr, this, nullptr);
            if (m_editNumeroEdit) disconnect(m_editNumeroEdit, nullptr, this, nullptr);
            if (m_editPagineEdit) disconnect(m_editPagineEdit, nullptr, this, nullptr);
            if (m_editCategoriaCombo) disconnect(m_editCategoriaCombo, nullptr, this, nullptr);
            if (m_editTipoRivistaCombo) disconnect(m_editTipoRivistaCombo, nullptr, this, nullptr);
            if (m_editDoiEdit) disconnect(m_editDoiEdit, nullptr, this, nullptr);
            if (m_editAutoriList) disconnect(m_editAutoriList, nullptr, this, nullptr);
            
            // Disconnetti i bottoni per evitare connessioni doppie
            if (m_editAggiungiAutoreBtn) {
                disconnect(m_editAggiungiAutoreBtn, nullptr, this, nullptr);
            }
            if (m_editRimuoviAutoreBtn) {
                disconnect(m_editRimuoviAutoreBtn, nullptr, this, nullptr);
            }
            if (m_editNuovoAutoreEdit) {
                disconnect(m_editNuovoAutoreEdit, nullptr, this, nullptr);
            }
            
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