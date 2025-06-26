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
    , m_validationEnabled(true)
{
    setWindowTitle("Nuovo Media");
    setupUI();
    populateComboBoxes();
    onTipoChanged(); // Inizializza il form per il primo tipo
}

MediaDialog::MediaDialog(Media* media, QWidget *parent, bool readOnly)
    : QDialog(parent)
    , m_mediaOriginale(media)
    , m_readOnly(readOnly)
    , m_isEditing(true)
    , m_validationEnabled(!readOnly)
{
    if (readOnly) {
        setWindowTitle("Dettagli Media");
    } else {
        setWindowTitle("Modifica Media");
    }
    
    setupUI();
    populateComboBoxes();
    loadMediaData();
    enableForm(!readOnly);
}

std::unique_ptr<Media> MediaDialog::getMedia() const
{
    if (m_readOnly) {
        return nullptr;
    }
    
    QString tipo = m_tipoCombo->currentText();
    
    if (tipo == "Libro") {
        return createLibro();
    } else if (tipo == "Film") {
        return createFilm();
    } else if (tipo == "Articolo") {
        return createArticolo();
    }
    
    return nullptr;
}

void MediaDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    
    // Focus sul primo campo editabile
    if (!m_readOnly && m_titoloEdit) {
        m_titoloEdit->setFocus();
    }
}

void MediaDialog::onTipoChanged()
{
    QString nuovoTipo = m_tipoCombo->currentText();
    
    if (nuovoTipo != m_tipoCorrente) {
        m_tipoCorrente = nuovoTipo;
        setupTipoSpecificForm();
        updateFormVisibility();
    }
}

void MediaDialog::onAccettaClicked()
{
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
}

void MediaDialog::onAnnullaClicked()
{
    reject();
}

void MediaDialog::onAggiungiAutoreClicked()
{
    QString autore = m_nuovoAutoreEdit->text().trimmed();
    if (!autore.isEmpty()) {
        m_autoriList->addItem(autore);
        m_nuovoAutoreEdit->clear();
        onValidazioneChanged();
    }
}

void MediaDialog::onRimuoviAutoreClicked()
{
    int row = m_autoriList->currentRow();
    if (row >= 0) {
        delete m_autoriList->takeItem(row);
        onValidazioneChanged();
    }
}

void MediaDialog::onAggiungiAttoreClicked()
{
    QString attore = m_nuovoAttoreEdit->text().trimmed();
    if (!attore.isEmpty()) {
        m_attoriList->addItem(attore);
        m_nuovoAttoreEdit->clear();
        onValidazioneChanged();
    }
}

void MediaDialog::onRimuoviAttoreClicked()
{
    int row = m_attoriList->currentRow();
    if (row >= 0) {
        delete m_attoriList->takeItem(row);
        onValidazioneChanged();
    }
}

void MediaDialog::onValidazioneChanged()
{
    if (!m_validationEnabled) return;
    
    bool valid = validateInput();
    m_okButton->setEnabled(valid);
    
    if (valid) {
        m_validationLabel->setText("✓ Tutti i campi sono validi");
        m_validationLabel->setStyleSheet("color: green;");
    } else {
        QStringList errors = getValidationErrors();
        m_validationLabel->setText("⚠ Errori: " + QString::number(errors.size()));
        m_validationLabel->setStyleSheet("color: red;");
    }
}

void MediaDialog::setupUI()
{
    setFixedSize(DIALOG_WIDTH, DIALOG_HEIGHT);
    setModal(true);
    
    m_mainLayout = new QVBoxLayout(this);
    
    // Scroll area per gestire form lunghi
    m_scrollArea = new QScrollArea();
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    m_formWidget = new QWidget();
    m_formLayout = new QFormLayout(m_formWidget);
    m_formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    
    m_scrollArea->setWidget(m_formWidget);
    m_mainLayout->addWidget(m_scrollArea);
    
    setupBaseForm();
    setupButtons();
    
    // Validation label
    m_validationLabel = new QLabel();
    m_validationLabel->setWordWrap(true);
    m_mainLayout->addWidget(m_validationLabel);
    
    // Connessioni per validazione in tempo reale
    connect(m_titoloEdit, &QLineEdit::textChanged, this, &MediaDialog::onValidazioneChanged);
    connect(m_annoSpin, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &MediaDialog::onValidazioneChanged);
    connect(m_descrizioneEdit, &QTextEdit::textChanged, this, &MediaDialog::onValidazioneChanged);
}

void MediaDialog::setupBaseForm()
{
    // Gruppo informazioni base
    m_baseGroup = new QGroupBox("Informazioni Base");
    QFormLayout* baseLayout = new QFormLayout(m_baseGroup);
    
    // Tipo
    m_tipoCombo = new QComboBox();
    m_tipoCombo->addItems({"Libro", "Film", "Articolo"});
    connect(m_tipoCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MediaDialog::onTipoChanged);
    baseLayout->addRow("Tipo:", m_tipoCombo);
    
    // Titolo
    m_titoloEdit = new QLineEdit();
    m_titoloEdit->setMaxLength(200);
    m_titoloEdit->setPlaceholderText("Inserire il titolo...");
    baseLayout->addRow("Titolo*:", m_titoloEdit);
    
    // Anno
    m_annoSpin = new QSpinBox();
    m_annoSpin->setRange(1000, QDate::currentDate().year() + 10);
    m_annoSpin->setValue(QDate::currentDate().year());
    baseLayout->addRow("Anno*:", m_annoSpin);
    
    // Descrizione
    m_descrizioneEdit = new QTextEdit();
    m_descrizioneEdit->setMaximumHeight(80);
    m_descrizioneEdit->setPlaceholderText("Inserire una breve descrizione...");
    baseLayout->addRow("Descrizione:", m_descrizioneEdit);
    
    m_formLayout->addRow(m_baseGroup);
}

void MediaDialog::setupTipoSpecificForm()
{
    clearSpecificForm();
    
    QString tipo = m_tipoCombo->currentText();
    
    if (tipo == "Libro") {
        setupLibroForm();
    } else if (tipo == "Film") {
        setupFilmForm();
    } else if (tipo == "Articolo") {
        setupArticoloForm();
    }
}

void MediaDialog::setupLibroForm()
{
    m_libroGroup = new QGroupBox("Dettagli Libro");
    QFormLayout* libroLayout = new QFormLayout(m_libroGroup);
    
    // Autore
    m_autoreEdit = new QLineEdit();
    m_autoreEdit->setPlaceholderText("Nome dell'autore...");
    libroLayout->addRow("Autore*:", m_autoreEdit);
    
    // Editore
    m_editoreEdit = new QLineEdit();
    m_editoreEdit->setPlaceholderText("Casa editrice...");
    libroLayout->addRow("Editore:", m_editoreEdit);
    
    // Pagine
    m_pagineSpin = new QSpinBox();
    m_pagineSpin->setRange(1, 10000);
    m_pagineSpin->setValue(200);
    libroLayout->addRow("Pagine*:", m_pagineSpin);
    
    // ISBN
    m_isbnEdit = new QLineEdit();
    m_isbnEdit->setPlaceholderText("ISBN-10 o ISBN-13...");
    // Validator per ISBN
    QRegularExpressionValidator* isbnValidator = 
        new QRegularExpressionValidator(QRegularExpression("^[0-9X-]{10,17}$"), this);
    m_isbnEdit->setValidator(isbnValidator);
    libroLayout->addRow("ISBN:", m_isbnEdit);
    
    // Genere
    m_genereLibroCombo = new QComboBox();
    m_genereLibroCombo->addItems(Libro::getAllGeneri());
    libroLayout->addRow("Genere:", m_genereLibroCombo);
    
    m_formLayout->addRow(m_libroGroup);
    
    // Connessioni per validazione
    connect(m_autoreEdit, &QLineEdit::textChanged, this, &MediaDialog::onValidazioneChanged);
    connect(m_pagineSpin, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &MediaDialog::onValidazioneChanged);
    connect(m_isbnEdit, &QLineEdit::textChanged, this, &MediaDialog::onValidazioneChanged);
}

void MediaDialog::setupFilmForm()
{
    m_filmGroup = new QGroupBox("Dettagli Film");
    QFormLayout* filmLayout = new QFormLayout(m_filmGroup);
    
    // Regista
    m_registaEdit = new QLineEdit();
    m_registaEdit->setPlaceholderText("Nome del regista...");
    filmLayout->addRow("Regista*:", m_registaEdit);
    
    // Attori
    QWidget* attoriWidget = new QWidget();
    QVBoxLayout* attoriLayout = new QVBoxLayout(attoriWidget);
    attoriLayout->setContentsMargins(0, 0, 0, 0);
    
    m_attoriList = new QListWidget();
    m_attoriList->setMaximumHeight(80);
    attoriLayout->addWidget(m_attoriList);
    
    QHBoxLayout* attoriButtonLayout = new QHBoxLayout();
    m_nuovoAttoreEdit = new QLineEdit();
    m_nuovoAttoreEdit->setPlaceholderText("Nome attore...");
    m_aggiungiAttoreBtn = new QPushButton("Aggiungi");
    m_rimuoviAttoreBtn = new QPushButton("Rimuovi");
    
    attoriButtonLayout->addWidget(m_nuovoAttoreEdit);
    attoriButtonLayout->addWidget(m_aggiungiAttoreBtn);
    attoriButtonLayout->addWidget(m_rimuoviAttoreBtn);
    attoriLayout->addLayout(attoriButtonLayout);
    
    filmLayout->addRow("Attori*:", attoriWidget);
    
    // Durata
    m_durataSpin = new QSpinBox();
    m_durataSpin->setRange(1, 1000);
    m_durataSpin->setValue(90);
    m_durataSpin->setSuffix(" min");
    filmLayout->addRow("Durata*:", m_durataSpin);
    
    // Genere
    m_genereFilmCombo = new QComboBox();
    m_genereFilmCombo->addItems(Film::getAllGeneri());
    filmLayout->addRow("Genere:", m_genereFilmCombo);
    
    // Classificazione
    m_classificazioneCombo = new QComboBox();
    m_classificazioneCombo->addItems(Film::getAllClassificazioni());
    filmLayout->addRow("Classificazione:", m_classificazioneCombo);
    
    // Casa di produzione
    m_casaProduzioneEdit = new QLineEdit();
    m_casaProduzioneEdit->setPlaceholderText("Nome casa di produzione...");
    filmLayout->addRow("Casa Produzione:", m_casaProduzioneEdit);
    
    m_formLayout->addRow(m_filmGroup);
    
    // Connessioni
    connect(m_aggiungiAttoreBtn, &QPushButton::clicked, this, &MediaDialog::onAggiungiAttoreClicked);
    connect(m_rimuoviAttoreBtn, &QPushButton::clicked, this, &MediaDialog::onRimuoviAttoreClicked);
    connect(m_nuovoAttoreEdit, &QLineEdit::returnPressed, this, &MediaDialog::onAggiungiAttoreClicked);
    connect(m_registaEdit, &QLineEdit::textChanged, this, &MediaDialog::onValidazioneChanged);
    connect(m_durataSpin, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &MediaDialog::onValidazioneChanged);
}

void MediaDialog::setupArticoloForm()
{
    m_articoloGroup = new QGroupBox("Dettagli Articolo");
    QFormLayout* articoloLayout = new QFormLayout(m_articoloGroup);
    
    // Autori
    QWidget* autoriWidget = new QWidget();
    QVBoxLayout* autoriLayout = new QVBoxLayout(autoriWidget);
    autoriLayout->setContentsMargins(0, 0, 0, 0);
    
    m_autoriList = new QListWidget();
    m_autoriList->setMaximumHeight(80);
    autoriLayout->addWidget(m_autoriList);
    
    QHBoxLayout* autoriButtonLayout = new QHBoxLayout();
    m_nuovoAutoreEdit = new QLineEdit();
    m_nuovoAutoreEdit->setPlaceholderText("Nome autore...");
    m_aggiungiAutoreBtn = new QPushButton("Aggiungi");
    m_rimuoviAutoreBtn = new QPushButton("Rimuovi");
    
    autoriButtonLayout->addWidget(m_nuovoAutoreEdit);
    autoriButtonLayout->addWidget(m_aggiungiAutoreBtn);
    autoriButtonLayout->addWidget(m_rimuoviAutoreBtn);
    autoriLayout->addLayout(autoriButtonLayout);
    
    articoloLayout->addRow("Autori*:", autoriWidget);
    
    // Rivista
    m_rivistaEdit = new QLineEdit();
    m_rivistaEdit->setPlaceholderText("Nome della rivista...");
    articoloLayout->addRow("Rivista*:", m_rivistaEdit);
    
    // Volume e Numero
    QHBoxLayout* volNumLayout = new QHBoxLayout();
    m_volumeEdit = new QLineEdit();
    m_volumeEdit->setPlaceholderText("Vol.");
    m_numeroEdit = new QLineEdit();
    m_numeroEdit->setPlaceholderText("Num.");
    volNumLayout->addWidget(new QLabel("Volume:"));
    volNumLayout->addWidget(m_volumeEdit);
    volNumLayout->addWidget(new QLabel("Numero:"));
    volNumLayout->addWidget(m_numeroEdit);
    articoloLayout->addRow("Volume/Numero:", volNumLayout);
    
    // Pagine
    m_pagineEdit = new QLineEdit();
    m_pagineEdit->setPlaceholderText("es. 123-130 o 45");
    articoloLayout->addRow("Pagine:", m_pagineEdit);
    
    // Categoria
    m_categoriaCombo = new QComboBox();
    m_categoriaCombo->addItems(Articolo::getAllCategorie());
    articoloLayout->addRow("Categoria:", m_categoriaCombo);
    
    // Tipo rivista
    m_tipoRivistaCombo = new QComboBox();
    m_tipoRivistaCombo->addItems(Articolo::getAllTipiRivista());
    articoloLayout->addRow("Tipo Rivista:", m_tipoRivistaCombo);
    
    // Data pubblicazione
    m_dataPubblicazioneEdit = new QDateEdit();
    m_dataPubblicazioneEdit->setDate(QDate::currentDate());
    m_dataPubblicazioneEdit->setCalendarPopup(true);
    articoloLayout->addRow("Data Pubblicazione*:", m_dataPubblicazioneEdit);
    
    // DOI
    m_doiEdit = new QLineEdit();
    m_doiEdit->setPlaceholderText("es. 10.1000/182");
    QRegularExpressionValidator* doiValidator = 
        new QRegularExpressionValidator(QRegularExpression("^10\\.\\d{4,}/\\S+$"), this);
    m_doiEdit->setValidator(doiValidator);
    articoloLayout->addRow("DOI:", m_doiEdit);
    
    m_formLayout->addRow(m_articoloGroup);
    
    // Connessioni
    connect(m_aggiungiAutoreBtn, &QPushButton::clicked, this, &MediaDialog::onAggiungiAutoreClicked);
    connect(m_rimuoviAutoreBtn, &QPushButton::clicked, this, &MediaDialog::onRimuoviAutoreClicked);
    connect(m_nuovoAutoreEdit, &QLineEdit::returnPressed, this, &MediaDialog::onAggiungiAutoreClicked);
    connect(m_rivistaEdit, &QLineEdit::textChanged, this, &MediaDialog::onValidazioneChanged);
    connect(m_dataPubblicazioneEdit, &QDateEdit::dateChanged, this, &MediaDialog::onValidazioneChanged);
}

void MediaDialog::setupButtons()
{
    m_buttonLayout = new QHBoxLayout();
    
    m_helpButton = new QPushButton("Aiuto");
    m_cancelButton = new QPushButton("Annulla");
    m_okButton = new QPushButton(m_readOnly ? "Chiudi" : (m_isEditing ? "Salva" : "Crea"));
    
    m_okButton->setDefault(true);
    
    m_buttonLayout->addWidget(m_helpButton);
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_cancelButton);
    m_buttonLayout->addWidget(m_okButton);
    
    m_mainLayout->addLayout(m_buttonLayout);
    
    connect(m_okButton, &QPushButton::clicked, this, &MediaDialog::onAccettaClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &MediaDialog::onAnnullaClicked);
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

void MediaDialog::loadMediaData()
{
    if (!m_mediaOriginale) return;
    
    // Carica dati base
    m_titoloEdit->setText(m_mediaOriginale->getTitolo());
    m_annoSpin->setValue(m_mediaOriginale->getAnno());
    m_descrizioneEdit->setPlainText(m_mediaOriginale->getDescrizione());
    
    // Imposta il tipo e carica dati specifici
    QString tipo = m_mediaOriginale->getTypeDisplayName();
    m_tipoCombo->setCurrentText(tipo);
    m_tipoCorrente = tipo;
    
    setupTipoSpecificForm();
    
    if (tipo == "Libro") {
        Libro* libro = dynamic_cast<Libro*>(m_mediaOriginale);
        if (libro) {
            m_autoreEdit->setText(libro->getAutore());
            m_editoreEdit->setText(libro->getEditore());
            m_pagineSpin->setValue(libro->getPagine());
            m_isbnEdit->setText(libro->getIsbn());
            m_genereLibroCombo->setCurrentText(libro->getGenereString());
        }
    } else if (tipo == "Film") {
        Film* film = dynamic_cast<Film*>(m_mediaOriginale);
        if (film) {
            m_registaEdit->setText(film->getRegista());
            m_attoriList->addItems(film->getAttori());
            m_durataSpin->setValue(film->getDurata());
            m_genereFilmCombo->setCurrentText(film->getGenereString());
            m_classificazioneCombo->setCurrentText(film->getClassificazioneString());
            m_casaProduzioneEdit->setText(film->getCasaProduzione());
        }
    } else if (tipo == "Articolo") {
        Articolo* articolo = dynamic_cast<Articolo*>(m_mediaOriginale);
        if (articolo) {
            m_autoriList->addItems(articolo->getAutori());
            m_rivistaEdit->setText(articolo->getRivista());
            m_volumeEdit->setText(articolo->getVolume());
            m_numeroEdit->setText(articolo->getNumero());
            m_pagineEdit->setText(articolo->getPagine());
            m_categoriaCombo->setCurrentText(articolo->getCategoriaString());
            m_tipoRivistaCombo->setCurrentText(articolo->getTipoRivistaString());
            m_dataPubblicazioneEdit->setDate(articolo->getDataPubblicazione());
            m_doiEdit->setText(articolo->getDoi());
        }
    }
    
    updateFormVisibility();
}

bool MediaDialog::validateInput()
{
    QStringList errors = getValidationErrors();
    return errors.isEmpty();
}

QStringList MediaDialog::getValidationErrors()
{
    QStringList errors;
    
    // Validazione base
    if (m_titoloEdit->text().trimmed().isEmpty()) {
        errors << "Il titolo è obbligatorio";
    }
    
    if (m_annoSpin->value() <= 0) {
        errors << "L'anno deve essere positivo";
    }
    
    // Validazione specifica per tipo
    QString tipo = m_tipoCombo->currentText();
    
    if (tipo == "Libro") {
        if (m_autoreEdit && m_autoreEdit->text().trimmed().isEmpty()) {
            errors << "L'autore è obbligatorio per i libri";
        }
        if (m_pagineSpin && m_pagineSpin->value() <= 0) {
            errors << "Il numero di pagine deve essere positivo";
        }
    } else if (tipo == "Film") {
        if (m_registaEdit && m_registaEdit->text().trimmed().isEmpty()) {
            errors << "Il regista è obbligatorio per i film";
        }
        if (m_attoriList && m_attoriList->count() == 0) {
            errors << "Almeno un attore è richiesto";
        }
        if (m_durataSpin && m_durataSpin->value() <= 0) {
            errors << "La durata deve essere positiva";
        }
    } else if (tipo == "Articolo") {
        if (m_autoriList && m_autoriList->count() == 0) {
            errors << "Almeno un autore è richiesto";
        }
        if (m_rivistaEdit && m_rivistaEdit->text().trimmed().isEmpty()) {
            errors << "Il nome della rivista è obbligatorio";
        }
        if (m_dataPubblicazioneEdit && !m_dataPubblicazioneEdit->date().isValid()) {
            errors << "La data di pubblicazione non è valida";
        }
    }
    
    return errors;
}

void MediaDialog::clearSpecificForm()
{
    // Rimuovi i gruppi specifici esistenti
    if (m_libroGroup) {
        m_formLayout->removeWidget(m_libroGroup);
        m_libroGroup->deleteLater();
        m_libroGroup = nullptr;
    }
    
    if (m_filmGroup) {
        m_formLayout->removeWidget(m_filmGroup);
        m_filmGroup->deleteLater();
        m_filmGroup = nullptr;
    }
    
    if (m_articoloGroup) {
        m_formLayout->removeWidget(m_articoloGroup);
        m_articoloGroup->deleteLater();
        m_articoloGroup = nullptr;
    }
}

void MediaDialog::updateFormVisibility()
{
    // Il form è già configurato correttamente dal setupTipoSpecificForm
    onValidazioneChanged();
}

void MediaDialog::populateComboBoxes()
{
    // I combo box vengono popolati durante la creazione dei form specifici
}

void MediaDialog::enableForm(bool enabled)
{
    // Disabilita/abilita tutti i widget del form
    setEnabled(enabled);
    
    if (m_readOnly) {
        m_tipoCombo->setEnabled(false);
        m_cancelButton->setText("Chiudi");
        m_helpButton->setVisible(false);
    }
}

std::unique_ptr<Media> MediaDialog::createLibro() const
{
    if (!m_autoreEdit || !m_editoreEdit || !m_pagineSpin || 
        !m_isbnEdit || !m_genereLibroCombo) {
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
    
    return std::make_unique<Libro>(titolo, anno, descrizione, autore, 
                                  editore, pagine, isbn, genere);
}

std::unique_ptr<Media> MediaDialog::createFilm() const
{
    if (!m_registaEdit || !m_attoriList || !m_durataSpin || 
        !m_genereFilmCombo || !m_classificazioneCombo || !m_casaProduzioneEdit) {
        return nullptr;
    }
    
    QString titolo = m_titoloEdit->text().trimmed();
    int anno = m_annoSpin->value();
    QString descrizione = m_descrizioneEdit->toPlainText().trimmed();
    QString regista = m_registaEdit->text().trimmed();
    
    QStringList attori;
    for (int i = 0; i < m_attoriList->count(); ++i) {
        attori << m_attoriList->item(i)->text();
    }
    
    int durata = m_durataSpin->value();
    Film::Genere genere = Film::stringToGenere(m_genereFilmCombo->currentText());
    Film::Classificazione classificazione = Film::stringToClassificazione(m_classificazioneCombo->currentText());
    QString casaProduzione = m_casaProduzioneEdit->text().trimmed();
    
    return std::make_unique<Film>(titolo, anno, descrizione, regista, attori, 
                                 durata, genere, classificazione, casaProduzione);
}

std::unique_ptr<Media> MediaDialog::createArticolo() const
{
    if (!m_autoriList || !m_rivistaEdit || !m_volumeEdit || !m_numeroEdit ||
        !m_pagineEdit || !m_categoriaCombo || !m_tipoRivistaCombo || 
        !m_dataPubblicazioneEdit || !m_doiEdit) {
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
    
    return std::make_unique<Articolo>(titolo, anno, descrizione, autori, rivista,
                                     volume, numero, pagine, categoria, tipoRivista,
                                     dataPubblicazione, doi);
}