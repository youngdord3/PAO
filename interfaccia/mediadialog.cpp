#include "mediadialog.h"
#include "modello_logico/media.h"
#include "modello_logico/libro.h"
#include "modello_logico/film.h"
#include "modello_logico/articolo.h"
#include <QMessageBox>
#include <QShowEvent>
#include <QDate>
#include <QValidator>
#include <QRegularExpressionValidator>
#include <QScrollArea>
#include <QSplitter>

MediaDialog::MediaDialog(QWidget *parent)
    : QDialog(parent)
    , m_mediaOriginale(nullptr)
    , m_readOnly(false)
    , m_isEditing(false)
    , m_tipoCorrente("")
    , m_mainLayout(nullptr)
    , m_tabWidget(nullptr)
    , m_scrollArea(nullptr)
    , m_formWidget(nullptr)
    , m_formLayout(nullptr)
    , m_baseGroup(nullptr)
    , m_tipoCombo(nullptr)
    , m_titoloEdit(nullptr)
    , m_annoSpin(nullptr)
    , m_descrizioneEdit(nullptr)
    , m_libroGroup(nullptr)
    , m_filmGroup(nullptr)
    , m_articoloGroup(nullptr)
    , m_buttonLayout(nullptr)
    , m_okButton(nullptr)
    , m_cancelButton(nullptr)
    , m_helpButton(nullptr)
    , m_validationLabel(nullptr)
    , m_validationEnabled(true)
{
    setWindowTitle("Nuovo Media");
    setAttribute(Qt::WA_DeleteOnClose, false);
    
    try {
        setupUI();
        populateComboBoxes();
        onTipoChanged();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Errore", QString("Errore nell'inizializzazione: %1").arg(e.what()));
    }
}

MediaDialog::MediaDialog(Media* media, QWidget *parent, bool readOnly)
    : QDialog(parent)
    , m_mediaOriginale(media)
    , m_readOnly(readOnly)
    , m_isEditing(true)
    , m_tipoCorrente("")
    , m_mainLayout(nullptr)
    , m_tabWidget(nullptr)
    , m_scrollArea(nullptr)
    , m_formWidget(nullptr)
    , m_formLayout(nullptr)
    , m_baseGroup(nullptr)
    , m_tipoCombo(nullptr)
    , m_titoloEdit(nullptr)
    , m_annoSpin(nullptr)
    , m_descrizioneEdit(nullptr)
    , m_libroGroup(nullptr)
    , m_filmGroup(nullptr)
    , m_articoloGroup(nullptr)
    , m_buttonLayout(nullptr)
    , m_okButton(nullptr)
    , m_cancelButton(nullptr)
    , m_helpButton(nullptr)
    , m_validationLabel(nullptr)
    , m_validationEnabled(!readOnly)
{
    if (readOnly) {
        setWindowTitle("Dettagli Media");
    } else {
        setWindowTitle("Modifica Media");
    }
    
    setAttribute(Qt::WA_DeleteOnClose, false);
    
    try {
        setupUI();
        populateComboBoxes();
        loadMediaData();
        enableForm(!readOnly);
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Errore", QString("Errore nell'inizializzazione: %1").arg(e.what()));
    }
}

MediaDialog::~MediaDialog()
{
    m_mainLayout = nullptr;
    m_formLayout = nullptr;
    m_baseGroup = nullptr;
    m_libroGroup = nullptr;
    m_filmGroup = nullptr;
    m_articoloGroup = nullptr;
}

void MediaDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    
    // Focus sul primo campo editabile
    if (!m_readOnly && m_titoloEdit) {
        QTimer::singleShot(100, [this]() {
            if (m_titoloEdit) {
                m_titoloEdit->setFocus();
            }
        });
    }
}

std::unique_ptr<Media> MediaDialog::getMedia() const
{
    if (m_readOnly) {
        return nullptr;
    }
    
    try {
        QString tipo = m_tipoCombo->currentText();
        std::unique_ptr<Media> media;
        
        if (tipo == "Libro") {
            media = createLibro();
        } else if (tipo == "Film") {
            media = createFilm();
        } else if (tipo == "Articolo") {
            media = createArticolo();
        }
        
        if (media && m_isEditing && m_mediaOriginale) {
            media->setId(m_mediaOriginale->getId());
        }
        
        return media;
    } catch (const std::exception& e) {
        QMessageBox::critical(const_cast<MediaDialog*>(this), "Errore", 
                             QString("Errore nella creazione media: %1").arg(e.what()));
    }
    
    return nullptr;
}
void MediaDialog::onTipoChanged()
{
    try {
        if (!m_tipoCombo) return;
        
        QString nuovoTipo = m_tipoCombo->currentText();
        
        if (nuovoTipo != m_tipoCorrente) {
            m_tipoCorrente = nuovoTipo;
            setupTipoSpecificForm();
            updateFormVisibility();
        }
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Errore", QString("Errore nel cambio tipo: %1").arg(e.what()));
    }
}

void MediaDialog::onAccettaClicked()
{
    try {
        if (m_readOnly) {
            accept();
            return;
        }
        
        if (validateInput()) {
            accept();
        } else {
            QStringList errors = getValidationErrors();
            QMessageBox::warning(this, "Errori di Validazione", 
                                "Correggere i seguenti errori:\n\n" + errors.join("\n"));
        }
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Errore", QString("Errore nella validazione: %1").arg(e.what()));
    }
}

void MediaDialog::onAnnullaClicked()
{
    reject();
}

void MediaDialog::onAggiungiAutoreClicked()
{
    try {
        if (!m_nuovoAutoreEdit || !m_autoriList) return;
        
        QString autore = m_nuovoAutoreEdit->text().trimmed();
        if (!autore.isEmpty()) {
            for (int i = 0; i < m_autoriList->count(); ++i) {
                if (m_autoriList->item(i)->text() == autore) {
                    QMessageBox::information(this, "Info", "Autore già presente nella lista");
                    return;
                }
            }
            
            m_autoriList->addItem(autore);
            m_nuovoAutoreEdit->clear();
            m_nuovoAutoreEdit->setFocus();
            onValidazioneChanged();
        }
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Errore", QString("Errore nell'aggiunta autore: %1").arg(e.what()));
    }
}

void MediaDialog::onRimuoviAutoreClicked()
{
    try {
        if (!m_autoriList) return;
        
        int row = m_autoriList->currentRow();
        if (row >= 0) {
            QListWidgetItem* item = m_autoriList->takeItem(row);
            if (item) {
                delete item;
                onValidazioneChanged();
            }
        } else {
            QMessageBox::information(this, "Info", "Seleziona un autore da rimuovere");
        }
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Errore", QString("Errore nella rimozione autore: %1").arg(e.what()));
    }
}

void MediaDialog::onAggiungiAttoreClicked()
{
    try {
        if (!m_nuovoAttoreEdit || !m_attoriList) return;
        
        QString attore = m_nuovoAttoreEdit->text().trimmed();
        if (!attore.isEmpty()) {
            for (int i = 0; i < m_attoriList->count(); ++i) {
                if (m_attoriList->item(i)->text() == attore) {
                    QMessageBox::information(this, "Info", "Attore già presente nella lista");
                    return;
                }
            }
            
            m_attoriList->addItem(attore);
            m_nuovoAttoreEdit->clear();
            m_nuovoAttoreEdit->setFocus();
            onValidazioneChanged();
        }
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Errore", QString("Errore nell'aggiunta attore: %1").arg(e.what()));
    }
}

void MediaDialog::onRimuoviAttoreClicked()
{
    try {
        if (!m_attoriList) return;
        
        int row = m_attoriList->currentRow();
        if (row >= 0) {
            QListWidgetItem* item = m_attoriList->takeItem(row);
            if (item) {
                delete item;
                onValidazioneChanged();
            }
        } else {
            QMessageBox::information(this, "Info", "Seleziona un attore da rimuovere");
        }
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Errore", QString("Errore nella rimozione attore: %1").arg(e.what()));
    }
}

void MediaDialog::onValidazioneChanged()
{
    try {
        if (!m_validationEnabled || !m_okButton || !m_validationLabel) return;
        
        bool valid = validateInput();
        m_okButton->setEnabled(valid);
        
        if (valid) {
            m_validationLabel->setText("✓ Tutti i campi sono validi");
            m_validationLabel->setProperty("valid", true);
        } else {
            QStringList errors = getValidationErrors();
            m_validationLabel->setText("⚠ Errori: " + QString::number(errors.size()));
            m_validationLabel->setProperty("valid", false);
        }
        
        m_validationLabel->style()->unpolish(m_validationLabel);
        m_validationLabel->style()->polish(m_validationLabel);
        
        m_validationLabel->update();
        
    } catch (const std::exception& e) {
        qWarning() << "Errore nella validazione:" << e.what();
    }
}

void MediaDialog::setupUI()
{
    setFixedSize(DIALOG_WIDTH, DIALOG_HEIGHT);
    setModal(true);
    
    if (m_mainLayout) {
        delete m_mainLayout;
    }
    
    m_mainLayout = new QVBoxLayout(this);
    if (!m_mainLayout) {
        throw std::runtime_error("Impossibile creare layout principale");
    }
    
    // Scroll area per gestire form lunghi
    m_scrollArea = new QScrollArea();
    if (!m_scrollArea) {
        throw std::runtime_error("Impossibile creare scroll area");
    }
    
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    m_formWidget = new QWidget();
    if (!m_formWidget) {
        throw std::runtime_error("Impossibile creare form widget");
    }
    
    m_formLayout = new QFormLayout(m_formWidget);
    if (!m_formLayout) {
        throw std::runtime_error("Impossibile creare form layout");
    }
    
    m_formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    
    m_scrollArea->setWidget(m_formWidget);
    m_mainLayout->addWidget(m_scrollArea);
    
    setupBaseForm();
    setupButtons();
    
    m_validationLabel = new QLabel();
    if (m_validationLabel) {
        m_validationLabel->setWordWrap(true);
        m_mainLayout->addWidget(m_validationLabel);
    }
    
    setupConnections();
}

void MediaDialog::setupConnections()
{
    // Disconnetti tutto prima di riconnettere per evitare connessioni multiple
    if (m_titoloEdit) {
        disconnect(m_titoloEdit, nullptr, this, nullptr);
        connect(m_titoloEdit, &QLineEdit::textChanged, this, &MediaDialog::onValidazioneChanged);
    }
    
    if (m_annoSpin) {
        disconnect(m_annoSpin, nullptr, this, nullptr);
        connect(m_annoSpin, QOverload<int>::of(&QSpinBox::valueChanged), 
                this, &MediaDialog::onValidazioneChanged);
    }
    
    if (m_descrizioneEdit) {
        disconnect(m_descrizioneEdit, nullptr, this, nullptr);
        connect(m_descrizioneEdit, &QTextEdit::textChanged, this, &MediaDialog::onValidazioneChanged);
    }
    
    if (m_tipoCombo) {
        disconnect(m_tipoCombo, nullptr, this, nullptr);
        connect(m_tipoCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
                this, &MediaDialog::onTipoChanged);
    }
}

void MediaDialog::setupBaseForm()
{
    if (!m_formLayout) return;
    
    // Gruppo informazioni base
    m_baseGroup = new QGroupBox("Informazioni Base");
    if (!m_baseGroup) return;
    
    QFormLayout* baseLayout = new QFormLayout(m_baseGroup);
    if (!baseLayout) return;
    
    // Tipo
    m_tipoCombo = new QComboBox();
    if (m_tipoCombo) {
        m_tipoCombo->addItems({"Libro", "Film", "Articolo"});
        baseLayout->addRow("Tipo:", m_tipoCombo);
    }
    
    // Titolo
    m_titoloEdit = new QLineEdit();
    if (m_titoloEdit) {
        m_titoloEdit->setMaxLength(200);
        m_titoloEdit->setPlaceholderText("Inserire il titolo...");
        baseLayout->addRow("Titolo*:", m_titoloEdit);
    }
    
    // Anno
    m_annoSpin = new QSpinBox();
    if (m_annoSpin) {
        m_annoSpin->setRange(1000, QDate::currentDate().year() + 10);
        m_annoSpin->setValue(QDate::currentDate().year());
        baseLayout->addRow("Anno*:", m_annoSpin);
    }
    
    // Descrizione
    m_descrizioneEdit = new QTextEdit();
    if (m_descrizioneEdit) {
        m_descrizioneEdit->setMaximumHeight(80);
        m_descrizioneEdit->setPlaceholderText("Inserire una breve descrizione...");
        baseLayout->addRow("Descrizione:", m_descrizioneEdit);
    }
    
    m_formLayout->addRow(m_baseGroup);
}

void MediaDialog::setupTipoSpecificForm()
{
    try {
        clearSpecificForm();
        
        if (!m_tipoCombo) return;
        
        QString tipo = m_tipoCombo->currentText();
        
        if (tipo == "Libro") {
            setupLibroForm();
        } else if (tipo == "Film") {
            setupFilmForm();
        } else if (tipo == "Articolo") {
            setupArticoloForm();
        }
        
        // Riconnetti tutto dopo aver creato i nuovi widget
        setupConnections();
        
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Errore", QString("Errore nel setup form specifico: %1").arg(e.what()));
    }
}

void MediaDialog::setupLibroForm()
{
    if (!m_formLayout) return;
    
    m_libroGroup = new QGroupBox("Dettagli Libro");
    if (!m_libroGroup) return;
    
    QFormLayout* libroLayout = new QFormLayout(m_libroGroup);
    if (!libroLayout) return;
    
    // Autore
    m_autoreEdit = new QLineEdit();
    if (m_autoreEdit) {
        m_autoreEdit->setPlaceholderText("Nome dell'autore...");
        libroLayout->addRow("Autore*:", m_autoreEdit);
    }
    
    // Editore
    m_editoreEdit = new QLineEdit();
    if (m_editoreEdit) {
        m_editoreEdit->setPlaceholderText("Casa editrice...");
        libroLayout->addRow("Editore:", m_editoreEdit);
    }
    
    // Pagine
    m_pagineSpin = new QSpinBox();
    if (m_pagineSpin) {
        m_pagineSpin->setRange(1, 10000);
        m_pagineSpin->setValue(200);
        libroLayout->addRow("Pagine*:", m_pagineSpin);
    }
    
    // ISBN
    m_isbnEdit = new QLineEdit();
    if (m_isbnEdit) {
        m_isbnEdit->setPlaceholderText("ISBN-10 o ISBN-13...");
        QRegularExpressionValidator* isbnValidator = 
            new QRegularExpressionValidator(QRegularExpression("^[0-9X-]{10,17}$"), this);
        m_isbnEdit->setValidator(isbnValidator);
        libroLayout->addRow("ISBN:", m_isbnEdit);
    }
    
    // Genere
    m_genereLibroCombo = new QComboBox();
    if (m_genereLibroCombo) {
        m_genereLibroCombo->addItems(Libro::getAllGeneri());
        libroLayout->addRow("Genere:", m_genereLibroCombo);
    }
    
    m_formLayout->addRow(m_libroGroup);
}

void MediaDialog::setupFilmForm()
{
    if (!m_formLayout) return;
    
    m_filmGroup = new QGroupBox("Dettagli Film");
    if (!m_filmGroup) return;
    
    QFormLayout* filmLayout = new QFormLayout(m_filmGroup);
    if (!filmLayout) return;
    
    // Regista
    m_registaEdit = new QLineEdit();
    if (m_registaEdit) {
        m_registaEdit->setPlaceholderText("Nome del regista...");
        filmLayout->addRow("Regista*:", m_registaEdit);
    }
    
    // Attori
    QWidget* attoriWidget = new QWidget();
    if (attoriWidget) {
        QVBoxLayout* attoriLayout = new QVBoxLayout(attoriWidget);
        if (attoriLayout) {
            attoriLayout->setContentsMargins(0, 0, 0, 0);
            
            m_attoriList = new QListWidget();
            if (m_attoriList) {
                m_attoriList->setMaximumHeight(80);
                attoriLayout->addWidget(m_attoriList);
            }
            
            QHBoxLayout* attoriButtonLayout = new QHBoxLayout();
            if (attoriButtonLayout) {
                m_nuovoAttoreEdit = new QLineEdit();
                if (m_nuovoAttoreEdit) {
                    m_nuovoAttoreEdit->setPlaceholderText("Nome attore...");
                    attoriButtonLayout->addWidget(m_nuovoAttoreEdit);
                }
                
                m_aggiungiAttoreBtn = new QPushButton("Aggiungi");
                if (m_aggiungiAttoreBtn) {
                    attoriButtonLayout->addWidget(m_aggiungiAttoreBtn);
                }
                
                m_rimuoviAttoreBtn = new QPushButton("Rimuovi");
                if (m_rimuoviAttoreBtn) {
                    attoriButtonLayout->addWidget(m_rimuoviAttoreBtn);
                }
                
                attoriLayout->addLayout(attoriButtonLayout);
            }
            
            filmLayout->addRow("Attori*:", attoriWidget);
        }
    }
    
    // Durata
    m_durataSpin = new QSpinBox();
    if (m_durataSpin) {
        m_durataSpin->setRange(1, 1000);
        m_durataSpin->setValue(90);
        m_durataSpin->setSuffix(" min");
        filmLayout->addRow("Durata*:", m_durataSpin);
    }
    
    // Genere
    m_genereFilmCombo = new QComboBox();
    if (m_genereFilmCombo) {
        m_genereFilmCombo->addItems(Film::getAllGeneri());
        filmLayout->addRow("Genere:", m_genereFilmCombo);
    }
    
    // Classificazione
    m_classificazioneCombo = new QComboBox();
    if (m_classificazioneCombo) {
        m_classificazioneCombo->addItems(Film::getAllClassificazioni());
        filmLayout->addRow("Classificazione:", m_classificazioneCombo);
    }
    
    // Casa di produzione
    m_casaProduzioneEdit = new QLineEdit();
    if (m_casaProduzioneEdit) {
        m_casaProduzioneEdit->setPlaceholderText("Nome casa di produzione...");
        filmLayout->addRow("Casa Produzione:", m_casaProduzioneEdit);
    }
    
    m_formLayout->addRow(m_filmGroup);
}

void MediaDialog::setupArticoloForm()
{
    if (!m_formLayout) return;
    
    m_articoloGroup = new QGroupBox("Dettagli Articolo");
    if (!m_articoloGroup) return;
    
    QFormLayout* articoloLayout = new QFormLayout(m_articoloGroup);
    if (!articoloLayout) return;
    
    // Autori
    QWidget* autoriWidget = new QWidget();
    if (autoriWidget) {
        QVBoxLayout* autoriLayout = new QVBoxLayout(autoriWidget);
        if (autoriLayout) {
            autoriLayout->setContentsMargins(0, 0, 0, 0);
            
            m_autoriList = new QListWidget();
            if (m_autoriList) {
                m_autoriList->setMaximumHeight(80);
                autoriLayout->addWidget(m_autoriList);
            }
            
            QHBoxLayout* autoriButtonLayout = new QHBoxLayout();
            if (autoriButtonLayout) {
                m_nuovoAutoreEdit = new QLineEdit();
                if (m_nuovoAutoreEdit) {
                    m_nuovoAutoreEdit->setPlaceholderText("Nome autore...");
                    autoriButtonLayout->addWidget(m_nuovoAutoreEdit);
                }
                
                m_aggiungiAutoreBtn = new QPushButton("Aggiungi");
                if (m_aggiungiAutoreBtn) {
                    autoriButtonLayout->addWidget(m_aggiungiAutoreBtn);
                }
                
                m_rimuoviAutoreBtn = new QPushButton("Rimuovi");
                if (m_rimuoviAutoreBtn) {
                    autoriButtonLayout->addWidget(m_rimuoviAutoreBtn);
                }
                
                autoriLayout->addLayout(autoriButtonLayout);
            }
            
            articoloLayout->addRow("Autori*:", autoriWidget);
        }
    }
    
    // Rivista
    m_rivistaEdit = new QLineEdit();
    if (m_rivistaEdit) {
        m_rivistaEdit->setPlaceholderText("Nome della rivista...");
        articoloLayout->addRow("Rivista*:", m_rivistaEdit);
    }
    
    // Data pubblicazione
    m_dataPubblicazioneEdit = new QDateEdit();
    if (m_dataPubblicazioneEdit) {
        m_dataPubblicazioneEdit->setDate(QDate::currentDate());
        m_dataPubblicazioneEdit->setCalendarPopup(true);
        articoloLayout->addRow("Data Pubblicazione*:", m_dataPubblicazioneEdit);
    }
    
    // Volume e Numero
    QWidget* volNumWidget = new QWidget();
    if (volNumWidget) {
        QHBoxLayout* volNumLayout = new QHBoxLayout(volNumWidget);
        volNumLayout->setContentsMargins(0, 0, 0, 0);
        
        m_volumeEdit = new QLineEdit();
        m_numeroEdit = new QLineEdit();
        
        if (m_volumeEdit && m_numeroEdit) {
            m_volumeEdit->setPlaceholderText("Vol.");
            m_numeroEdit->setPlaceholderText("Num.");
            volNumLayout->addWidget(new QLabel("Vol:"));
            volNumLayout->addWidget(m_volumeEdit);
            volNumLayout->addWidget(new QLabel("Num:"));
            volNumLayout->addWidget(m_numeroEdit);
        }
        
        articoloLayout->addRow("Volume/Numero:", volNumWidget);
    }
    
    // Pagine
    m_pagineEdit = new QLineEdit();
    if (m_pagineEdit) {
        m_pagineEdit->setPlaceholderText("es. 123-130 o 45");
        articoloLayout->addRow("Pagine:", m_pagineEdit);
    }
    
    // Categoria
    m_categoriaCombo = new QComboBox();
    if (m_categoriaCombo) {
        m_categoriaCombo->addItems(Articolo::getAllCategorie());
        articoloLayout->addRow("Categoria:", m_categoriaCombo);
    }
    
    // Tipo rivista
    m_tipoRivistaCombo = new QComboBox();
    if (m_tipoRivistaCombo) {
        m_tipoRivistaCombo->addItems(Articolo::getAllTipiRivista());
        articoloLayout->addRow("Tipo Rivista:", m_tipoRivistaCombo);
    }
    
    // DOI
    m_doiEdit = new QLineEdit();
    if (m_doiEdit) {
        m_doiEdit->setPlaceholderText("es. 10.1000/182");
        QRegularExpressionValidator* doiValidator = 
            new QRegularExpressionValidator(QRegularExpression("^10\\.\\d{4,}/\\S+$"), this);
        m_doiEdit->setValidator(doiValidator);
        articoloLayout->addRow("DOI:", m_doiEdit);
    }
    
    m_formLayout->addRow(m_articoloGroup);
}

void MediaDialog::setupButtons()
{
    if (!m_mainLayout) return;
    
    m_buttonLayout = new QHBoxLayout();
    if (!m_buttonLayout) return;
    
    m_helpButton = new QPushButton("Aiuto");
    m_cancelButton = new QPushButton("Annulla");
    m_okButton = new QPushButton(m_readOnly ? "Chiudi" : (m_isEditing ? "Salva" : "Crea"));
    
    if (m_okButton) {
        m_okButton->setDefault(true);
    }
    
    if (m_okButton) {
        m_okButton->setObjectName("okButton");
    }
    if (m_cancelButton) {
        m_cancelButton->setObjectName("cancelButton");
    }
    if (m_helpButton) {
        m_helpButton->setObjectName("helpButton");
    }
    
    m_mainLayout->addLayout(m_buttonLayout);
    
    // Connessioni bottoni
    if (m_okButton) {
        connect(m_okButton, &QPushButton::clicked, this, &MediaDialog::onAccettaClicked);
    }
    if (m_cancelButton) {
        connect(m_cancelButton, &QPushButton::clicked, this, &MediaDialog::onAnnullaClicked);
    }
    if (m_helpButton) {
        connect(m_helpButton, &QPushButton::clicked, [this]() {
            QMessageBox::information(this, "Aiuto", 
                "I campi contrassegnati con * sono obbligatori.\n\n"
                "Tipo: Seleziona il tipo di media da creare.\n"
                "Titolo: Nome del media (obbligatorio).\n"
                "Anno: Anno di pubblicazione (obbligatorio).\n"
                "Descrizione: Breve descrizione del contenuto.\n\n"
                "Ogni tipo di media ha campi specifici aggiuntivi.");
        });
    }
}

void MediaDialog::loadMediaData()
{
    if (!m_mediaOriginale) return;
    
    try {
        // Carica dati base
        if (m_titoloEdit) {
            m_titoloEdit->setText(m_mediaOriginale->getTitolo());
        }
        if (m_annoSpin) {
            m_annoSpin->setValue(m_mediaOriginale->getAnno());
        }
        if (m_descrizioneEdit) {
            m_descrizioneEdit->setPlainText(m_mediaOriginale->getDescrizione());
        }
        
        // Imposta il tipo e carica dati specifici
        QString tipo = m_mediaOriginale->getTypeDisplayName();
        if (m_tipoCombo) {
            m_tipoCombo->setCurrentText(tipo);
        }
        m_tipoCorrente = tipo;
        
        setupTipoSpecificForm();
        
        // Carica dati specifici per tipo
        if (tipo == "Libro") {
            Libro* libro = dynamic_cast<Libro*>(m_mediaOriginale);
            if (libro) {
                if (m_autoreEdit) m_autoreEdit->setText(libro->getAutore());
                if (m_editoreEdit) m_editoreEdit->setText(libro->getEditore());
                if (m_pagineSpin) m_pagineSpin->setValue(libro->getPagine());
                if (m_isbnEdit) m_isbnEdit->setText(libro->getIsbn());
                if (m_genereLibroCombo) m_genereLibroCombo->setCurrentText(libro->getGenereString());
            }
        } else if (tipo == "Film") {
            Film* film = dynamic_cast<Film*>(m_mediaOriginale);
            if (film) {
                if (m_registaEdit) m_registaEdit->setText(film->getRegista());
                if (m_attoriList) m_attoriList->addItems(film->getAttori());
                if (m_durataSpin) m_durataSpin->setValue(film->getDurata());
                if (m_genereFilmCombo) m_genereFilmCombo->setCurrentText(film->getGenereString());
                if (m_classificazioneCombo) m_classificazioneCombo->setCurrentText(film->getClassificazioneString());
                if (m_casaProduzioneEdit) m_casaProduzioneEdit->setText(film->getCasaProduzione());
            }
        } else if (tipo == "Articolo") {
            Articolo* articolo = dynamic_cast<Articolo*>(m_mediaOriginale);
            if (articolo) {
                if (m_autoriList) m_autoriList->addItems(articolo->getAutori());
                if (m_rivistaEdit) m_rivistaEdit->setText(articolo->getRivista());
                if (m_volumeEdit) m_volumeEdit->setText(articolo->getVolume());
                if (m_numeroEdit) m_numeroEdit->setText(articolo->getNumero());
                if (m_pagineEdit) m_pagineEdit->setText(articolo->getPagine());
                if (m_categoriaCombo) m_categoriaCombo->setCurrentText(articolo->getCategoriaString());
                if (m_tipoRivistaCombo) m_tipoRivistaCombo->setCurrentText(articolo->getTipoRivistaString());
                if (m_dataPubblicazioneEdit) m_dataPubblicazioneEdit->setDate(articolo->getDataPubblicazione());
                if (m_doiEdit) m_doiEdit->setText(articolo->getDoi());
            }
        }
        
        updateFormVisibility();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Errore", QString("Errore nel caricamento dati: %1").arg(e.what()));
    }
}

bool MediaDialog::validateInput()
{
    try {
        QStringList errors = getValidationErrors();
        return errors.isEmpty();
    } catch (const std::exception&) {
        return false;
    }
}

QStringList MediaDialog::getValidationErrors()
{
    QStringList errors;
    
    try {
        // Validazione base
        if (!m_titoloEdit || m_titoloEdit->text().trimmed().isEmpty()) {
            errors << "Il titolo è obbligatorio";
        }
        
        if (!m_annoSpin || m_annoSpin->value() <= 0) {
            errors << "L'anno deve essere positivo";
        }
        
        // Validazione specifica per tipo
        if (!m_tipoCombo) {
            errors << "Tipo non selezionato";
            return errors;
        }
        
        QString tipo = m_tipoCombo->currentText();
        
        if (tipo == "Libro") {
            if (!m_autoreEdit || m_autoreEdit->text().trimmed().isEmpty()) {
                errors << "L'autore è obbligatorio per i libri";
            }
            if (!m_pagineSpin || m_pagineSpin->value() <= 0) {
                errors << "Il numero di pagine deve essere positivo";
            }
        } else if (tipo == "Film") {
            if (!m_registaEdit || m_registaEdit->text().trimmed().isEmpty()) {
                errors << "Il regista è obbligatorio per i film";
            }
            if (!m_attoriList || m_attoriList->count() == 0) {
                errors << "Almeno un attore è richiesto";
            }
            if (!m_durataSpin || m_durataSpin->value() <= 0) {
                errors << "La durata deve essere positiva";
            }
        } else if (tipo == "Articolo") {
            if (!m_autoriList || m_autoriList->count() == 0) {
                errors << "Almeno un autore è richiesto";
            }
            if (!m_rivistaEdit || m_rivistaEdit->text().trimmed().isEmpty()) {
                errors << "Il nome della rivista è obbligatorio";
            }
            if (!m_dataPubblicazioneEdit || !m_dataPubblicazioneEdit->date().isValid()) {
                errors << "La data di pubblicazione non è valida";
            }
        }
    } catch (const std::exception& e) {
        errors << QString("Errore nella validazione: %1").arg(e.what());
    }
    
    return errors;
}

void MediaDialog::clearSpecificForm()
{
    try {
        if (!m_formLayout) return;
        
        // Disconnetti e rimuovi i gruppi specifici esistenti in modo sicuro
        if (m_libroGroup) {
            // Disconnetti tutti i widget nel gruppo prima di cancellarlo
            disconnectGroupWidgets(m_libroGroup);
            m_formLayout->removeWidget(m_libroGroup);
            m_libroGroup->deleteLater();
            m_libroGroup = nullptr;
        }
        
        if (m_filmGroup) {
            disconnectGroupWidgets(m_filmGroup);
            m_formLayout->removeWidget(m_filmGroup);
            m_filmGroup->deleteLater();
            m_filmGroup = nullptr;
        }
        
        if (m_articoloGroup) {
            disconnectGroupWidgets(m_articoloGroup);
            m_formLayout->removeWidget(m_articoloGroup);
            m_articoloGroup->deleteLater();
            m_articoloGroup = nullptr;
        }
        
        // Reset tutti i puntatori specifici in modo sicuro
        resetSpecificPointers();
        
    } catch (const std::exception& e) {
        qWarning() << "Errore in clearSpecificForm:" << e.what();
    }
}

void MediaDialog::disconnectGroupWidgets(QGroupBox* group)
{
    if (!group) return;
    
    // Trova tutti i widget figli e disconnettili
    QList<QWidget*> widgets = group->findChildren<QWidget*>();
    for (QWidget* widget : widgets) {
        if (widget) {
            disconnect(widget, nullptr, this, nullptr);
        }
    }
}

void MediaDialog::resetSpecificPointers()
{
    // Reset puntatori libro
    m_autoreEdit = nullptr;
    m_editoreEdit = nullptr;
    m_pagineSpin = nullptr;
    m_isbnEdit = nullptr;
    m_genereLibroCombo = nullptr;
    
    // Reset puntatori film
    m_registaEdit = nullptr;
    m_attoriList = nullptr;
    m_nuovoAttoreEdit = nullptr;
    m_aggiungiAttoreBtn = nullptr;
    m_rimuoviAttoreBtn = nullptr;
    m_durataSpin = nullptr;
    m_genereFilmCombo = nullptr;
    m_classificazioneCombo = nullptr;
    m_casaProduzioneEdit = nullptr;
    
    // Reset puntatori articolo
    m_autoriList = nullptr;
    m_nuovoAutoreEdit = nullptr;
    m_aggiungiAutoreBtn = nullptr;
    m_rimuoviAutoreBtn = nullptr;
    m_rivistaEdit = nullptr;
    m_volumeEdit = nullptr;
    m_numeroEdit = nullptr;
    m_pagineEdit = nullptr;
    m_categoriaCombo = nullptr;
    m_tipoRivistaCombo = nullptr;
    m_dataPubblicazioneEdit = nullptr;
    m_doiEdit = nullptr;
}

void MediaDialog::updateFormVisibility()
{
    // Il form è già configurato correttamente dal setupTipoSpecificForm
    onValidazioneChanged();
}

void MediaDialog::populateComboBoxes()
{
    // I combo box vengono popolati durante la creazione dei form specifici
    // Questo metodo può essere usato per aggiornamenti futuri
}

void MediaDialog::enableForm(bool enabled)
{
    try {
        // Abilita/disabilita tutti i widget del form
        if (m_formWidget) {
            m_formWidget->setEnabled(enabled);
        }
        
        if (m_readOnly) {
            if (m_tipoCombo) m_tipoCombo->setEnabled(false);
            if (m_cancelButton) m_cancelButton->setVisible(false);
            if (m_helpButton) m_helpButton->setVisible(false);
        }
    } catch (const std::exception& e) {
        qWarning() << "Errore in enableForm:" << e.what();
    }
}

std::unique_ptr<Media> MediaDialog::createLibro() const
{
    try {
        if (!m_autoreEdit || !m_editoreEdit || !m_pagineSpin || 
            !m_isbnEdit || !m_genereLibroCombo || !m_titoloEdit || !m_annoSpin || !m_descrizioneEdit) {
            return nullptr;
        }
        
        QString titolo = m_titoloEdit->text().trimmed();
        int anno = m_annoSpin->value();
        QString descrizione = m_descrizioneEdit->toPlainText().trimmed();
        QString autore = m_autoreEdit->text().trimmed();
        QString editore = m_editoreEdit->text().trimmed();
        int pagine = m_pagineSpin->value();
        QString isbn = m_isbnEdit->text().trimmed();
        Libro::Genere genere = Libro::stringToGenere(m_genereLibroCombo->currentText());
        
        auto libro = std::make_unique<Libro>(titolo, anno, descrizione, autore, 
                                           editore, pagine, isbn, genere);
        
        if (m_isEditing && m_mediaOriginale) {
            libro->setId(m_mediaOriginale->getId());
        }
        
        return libro;
    } catch (const std::exception& e) {
        qWarning() << "Errore in createLibro:" << e.what();
        return nullptr;
    }
}

std::unique_ptr<Media> MediaDialog::createFilm() const
{
    try {
        if (!m_registaEdit || !m_attoriList || !m_durataSpin || 
            !m_genereFilmCombo || !m_classificazioneCombo || !m_casaProduzioneEdit ||
            !m_titoloEdit || !m_annoSpin || !m_descrizioneEdit) {
            return nullptr;
        }
        
        QString titolo = m_titoloEdit->text().trimmed();
        int anno = m_annoSpin->value();
        QString descrizione = m_descrizioneEdit->toPlainText().trimmed();
        QString regista = m_registaEdit->text().trimmed();
        
        QStringList attori;
        for (int i = 0; i < m_attoriList->count(); ++i) {
            QString attore = m_attoriList->item(i)->text().trimmed();
            if (!attore.isEmpty()) {
                attori << attore;
            }
        }
        
        int durata = m_durataSpin->value();
        Film::Genere genere = Film::stringToGenere(m_genereFilmCombo->currentText());
        Film::Classificazione classificazione = Film::stringToClassificazione(m_classificazioneCombo->currentText());
        QString casaProduzione = m_casaProduzioneEdit->text().trimmed();
        
        auto film = std::make_unique<Film>(titolo, anno, descrizione, regista, attori, 
                                         durata, genere, classificazione, casaProduzione);
        
        if (m_isEditing && m_mediaOriginale) {
            film->setId(m_mediaOriginale->getId());
        }
        
        return film;
    } catch (const std::exception& e) {
        qWarning() << "Errore in createFilm:" << e.what();
        return nullptr;
    }
}

std::unique_ptr<Media> MediaDialog::createArticolo() const
{
    try {
        if (!m_autoriList || !m_rivistaEdit || !m_volumeEdit || !m_numeroEdit ||
            !m_pagineEdit || !m_categoriaCombo || !m_tipoRivistaCombo || 
            !m_dataPubblicazioneEdit || !m_doiEdit ||
            !m_titoloEdit || !m_annoSpin || !m_descrizioneEdit) {
            return nullptr;
        }
        
        QString titolo = m_titoloEdit->text().trimmed();
        int anno = m_annoSpin->value();
        QString descrizione = m_descrizioneEdit->toPlainText().trimmed();
        
        QStringList autori;
        for (int i = 0; i < m_autoriList->count(); ++i) {
            autori << m_autoriList->item(i)->text();
        }
        
        QString rivista = m_rivistaEdit->text().trimmed();
        QString volume = m_volumeEdit->text().trimmed();
        QString numero = m_numeroEdit->text().trimmed();
        QString pagine = m_pagineEdit->text().trimmed();
        Articolo::Categoria categoria = Articolo::stringToCategoria(m_categoriaCombo->currentText());
        Articolo::TipoRivista tipoRivista = Articolo::stringToTipoRivista(m_tipoRivistaCombo->currentText());
        QDate dataPubblicazione = m_dataPubblicazioneEdit->date();
        QString doi = m_doiEdit->text().trimmed();
        
        auto articolo = std::make_unique<Articolo>(titolo, anno, descrizione, autori, rivista,
                                                 volume, numero, pagine, categoria, tipoRivista,
                                                 dataPubblicazione, doi);
        
        if (m_isEditing && m_mediaOriginale) {
            articolo->setId(m_mediaOriginale->getId());
        }
        
        return articolo;
    } catch (const std::exception& e) {
        qWarning() << "Errore in createArticolo:" << e.what();
        return nullptr;
    }
}