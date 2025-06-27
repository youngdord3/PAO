#include "mediacard.h"
#include "modello_logico/media.h"
#include "modello_logico/libro.h"
#include "modello_logico/film.h"
#include "modello_logico/articolo.h"
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QStyleOption>
#include <QApplication>
#include <QStyle>
#include <QPixmap>
#include <QFileInfo>

// Costanti statiche
const QString MediaCard::COLOR_LIBRO = "#4CAF50";     // Verde
const QString MediaCard::COLOR_FILM = "#2196F3";      // Blu
const QString MediaCard::COLOR_ARTICOLO = "#FF9800";  // Arancione
const QString MediaCard::COLOR_DEFAULT = "#9E9E9E";   // Grigio

const QString MediaCard::STYLE_NORMAL = 
    "MediaCard {"
    "    background-color: white;"
    "    border: 1px solid #E0E0E0;"
    "    border-radius: 8px;"
    "    margin: 2px;"
    "}"
    "MediaCard:hover {"
    "    border: 2px solid #2196F3;"
    "    box-shadow: 0 2px 8px rgba(0,0,0,0.1);"
    "}";

const QString MediaCard::STYLE_SELECTED = 
    "MediaCard {"
    "    background-color: #E3F2FD;"
    "    border: 2px solid #2196F3;"
    "    border-radius: 8px;"
    "    margin: 2px;"
    "    box-shadow: 0 2px 8px rgba(33,150,243,0.3);"
    "}";

const QString MediaCard::STYLE_HOVERED = 
    "MediaCard {"
    "    background-color: #F5F5F5;"
    "    border: 2px solid #2196F3;"
    "    border-radius: 8px;"
    "    margin: 2px;"
    "    box-shadow: 0 4px 12px rgba(0,0,0,0.15);"
    "}";

MediaCard::MediaCard(std::unique_ptr<Media> media, QWidget *parent)
    : QFrame(parent)
    , m_media(std::move(media))
    , m_selected(false)
    , m_hovered(false)
{
    setFixedSize(CARD_WIDTH, CARD_HEIGHT);
    setFrameStyle(QFrame::StyledPanel);
    setupUI();
    updateStyleSheet();
}

QString MediaCard::getId() const
{
    return m_media ? m_media->getId() : QString();
}

Media* MediaCard::getMedia() const
{
    return m_media.get();
}

void MediaCard::setSelected(bool selected)
{
    if (m_selected != selected) {
        m_selected = selected;
        updateStyleSheet();
        update();
    }
}

bool MediaCard::isSelected() const
{
    return m_selected;
}

void MediaCard::updateContent()
{
    if (!m_media) return;
    
    // Aggiorna i contenuti delle label
    m_titleLabel->setText(truncateText(m_media->getTitolo(), 25));
    m_yearLabel->setText(QString::number(m_media->getAnno()));
    m_descriptionLabel->setText(truncateText(m_media->getDescrizione(), 80));
    m_typeLabel->setText(m_media->getTypeDisplayName());
    m_infoLabel->setText(formatDisplayInfo());
    
    // Aggiorna l'icona del tipo
    m_imageLabel->setPixmap(getTypeIcon());
    
    setupTypeSpecificContent();
}

void MediaCard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        setSelected(true);
        emit selezionato(getId());
    }
    QFrame::mousePressEvent(event);
}

void MediaCard::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit doppioClick(getId());
    }
    QFrame::mouseDoubleClickEvent(event);
}

void MediaCard::contextMenuEvent(QContextMenuEvent *event)
{
    emit contestualMenu(getId(), event->globalPos());
    QFrame::contextMenuEvent(event);
}

void MediaCard::enterEvent(QEnterEvent *event)
{
    m_hovered = true;
    updateStyleSheet();
    QFrame::enterEvent(event);
}

void MediaCard::leaveEvent(QEvent *event)
{
    m_hovered = false;
    updateStyleSheet();
    QFrame::leaveEvent(event);
}

void MediaCard::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);
    
    // Disegna una barra colorata in alto per identificare il tipo
    if (m_media) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        
        QString color = COLOR_DEFAULT;
        QString type = m_media->getTypeDisplayName();
        
        if (type == "Libro") {
            color = COLOR_LIBRO;
        } else if (type == "Film") {
            color = COLOR_FILM;
        } else if (type == "Articolo") {
            color = COLOR_ARTICOLO;
        }
        
        painter.fillRect(0, 0, width(), 4, QColor(color));
    }
}

void MediaCard::onEditClicked()
{
    // Questo sarà gestito dal parent
    emit selezionato(getId());
}

void MediaCard::onDeleteClicked()
{
    // Questo sarà gestito dal parent
    emit selezionato(getId());
}

void MediaCard::onDetailsClicked()
{
    emit doppioClick(getId());
}

void MediaCard::setupUI()
{
    if (!m_media) return;
    
    setupLayout();
    
    // Configurazione delle label
    m_typeLabel = new QLabel(m_media->getTypeDisplayName());
    m_typeLabel->setStyleSheet("font-weight: bold; font-size: 10px; color: #666;");
    
    m_titleLabel = new QLabel(truncateText(m_media->getTitolo(), 25));
    m_titleLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #333;");
    m_titleLabel->setWordWrap(true);
    
    m_yearLabel = new QLabel(QString::number(m_media->getAnno()));
    m_yearLabel->setStyleSheet("font-size: 12px; color: #666;");
    
    m_descriptionLabel = new QLabel(truncateText(m_media->getDescrizione(), 80));
    m_descriptionLabel->setStyleSheet("font-size: 11px; color: #555;");
    m_descriptionLabel->setWordWrap(true);
    
    m_imageLabel = new QLabel();
    m_imageLabel->setFixedSize(IMAGE_SIZE, IMAGE_SIZE);
    m_imageLabel->setScaledContents(true);
    m_imageLabel->setPixmap(getTypeIcon());
    
    m_infoLabel = new QLabel(formatDisplayInfo());
    m_infoLabel->setStyleSheet("font-size: 10px; color: #777;");
    m_infoLabel->setWordWrap(true);
    
    // Configurazione bottoni (nascosti per default, mostrati solo al hover)
    m_editButton = new QPushButton("✏");
    m_deleteButton = new QPushButton("🗑");
    m_detailsButton = new QPushButton("👁");
    
    m_editButton->setFixedSize(24, 24);
    m_deleteButton->setFixedSize(24, 24);
    m_detailsButton->setFixedSize(24, 24);
    
    m_editButton->setToolTip("Modifica");
    m_deleteButton->setToolTip("Elimina");
    m_detailsButton->setToolTip("Dettagli");
    
    QString buttonStyle = 
        "QPushButton {"
        "    border: none;"
        "    border-radius: 12px;"
        "    background-color: rgba(255,255,255,180);"
        "    font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgba(33,150,243,200);"
        "    color: white;"
        "}";
    
    m_editButton->setStyleSheet(buttonStyle);
    m_deleteButton->setStyleSheet(buttonStyle);
    m_detailsButton->setStyleSheet(buttonStyle);
    
    // I bottoni sono inizialmente nascosti
    m_editButton->setVisible(false);
    m_deleteButton->setVisible(false);
    m_detailsButton->setVisible(false);
    
    connect(m_editButton, &QPushButton::clicked, this, &MediaCard::onEditClicked);
    connect(m_deleteButton, &QPushButton::clicked, this, &MediaCard::onDeleteClicked);
    connect(m_detailsButton, &QPushButton::clicked, this, &MediaCard::onDetailsClicked);
    
    setupTypeSpecificContent();
}

void MediaCard::setupLayout()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(8, 8, 8, 8);
    m_mainLayout->setSpacing(4);
    
    // Header con tipo e immagine
    m_headerLayout = new QHBoxLayout();
    m_headerLayout->setContentsMargins(0, 0, 0, 0);
    
    // Content area
    m_contentLayout = new QVBoxLayout();
    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    m_contentLayout->setSpacing(2);
    
    // Area bottoni (in overlay)
    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->setContentsMargins(0, 0, 0, 0);
    m_buttonLayout->addStretch();
}

void MediaCard::updateStyleSheet()
{
    if (m_selected) {
        setStyleSheet(STYLE_SELECTED);
    } else if (m_hovered) {
        setStyleSheet(STYLE_HOVERED);
        // Mostra i bottoni quando si fa hover
        m_editButton->setVisible(true);
        m_deleteButton->setVisible(true);
        m_detailsButton->setVisible(true);
    } else {
        setStyleSheet(STYLE_NORMAL);
        // Nascondi i bottoni quando non c'è hover
        m_editButton->setVisible(false);
        m_deleteButton->setVisible(false);
        m_detailsButton->setVisible(false);
    }
}

void MediaCard::setupTypeSpecificContent()
{
    if (!m_media) return;
    
    // Rimuovi elementi precedenti
    QLayoutItem* item;
    while ((item = m_mainLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    
    // Header
    m_headerLayout = new QHBoxLayout();
    m_headerLayout->addWidget(m_typeLabel);
    m_headerLayout->addStretch();
    m_headerLayout->addWidget(m_imageLabel);
    
    // Content
    m_contentLayout = new QVBoxLayout();
    m_contentLayout->addWidget(m_titleLabel);
    
    QHBoxLayout* yearLayout = new QHBoxLayout();
    yearLayout->addWidget(new QLabel("Anno:"));
    yearLayout->addWidget(m_yearLabel);
    yearLayout->addStretch();
    m_contentLayout->addLayout(yearLayout);
    
    m_contentLayout->addWidget(m_descriptionLabel);
    m_contentLayout->addWidget(m_infoLabel);
    
    // Bottoni
    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_detailsButton);
    m_buttonLayout->addWidget(m_editButton);
    m_buttonLayout->addWidget(m_deleteButton);
    
    // Aggiungi al layout principale
    m_mainLayout->addLayout(m_headerLayout);
    m_mainLayout->addLayout(m_contentLayout);
    m_mainLayout->addStretch();
    m_mainLayout->addLayout(m_buttonLayout);
    
    QString type = m_media->getTypeDisplayName();
    if (type == "Libro") {
        setupLibroContent();
    } else if (type == "Film") {
        setupFilmContent();
    } else if (type == "Articolo") {
        setupArticoloContent();
    }
}

void MediaCard::setupLibroContent()
{
    Libro* libro = dynamic_cast<Libro*>(m_media.get());
    if (!libro) return;
    
    // Aggiungi informazioni specifiche del libro
    QLabel* autoreLabel = new QLabel(QString("Autore: %1").arg(
        truncateText(libro->getAutore(), 20)));
    autoreLabel->setStyleSheet("font-size: 10px; color: #666;");
    
    QLabel* pagesLabel = new QLabel(QString("%1 pagine").arg(libro->getPagine()));
    pagesLabel->setStyleSheet("font-size: 10px; color: #666;");
    
    m_contentLayout->insertWidget(m_contentLayout->count() - 1, autoreLabel);
    m_contentLayout->insertWidget(m_contentLayout->count() - 1, pagesLabel);
}

void MediaCard::setupFilmContent()
{
    Film* film = dynamic_cast<Film*>(m_media.get());
    if (!film) return;
    
    // Aggiungi informazioni specifiche del film
    QLabel* registaLabel = new QLabel(QString("Regista: %1").arg(
        truncateText(film->getRegista(), 20)));
    registaLabel->setStyleSheet("font-size: 10px; color: #666;");
    
    QLabel* durataLabel = new QLabel(film->getDurataFormatted());
    durataLabel->setStyleSheet("font-size: 10px; color: #666;");
    
    m_contentLayout->insertWidget(m_contentLayout->count() - 1, registaLabel);
    m_contentLayout->insertWidget(m_contentLayout->count() - 1, durataLabel);
}

void MediaCard::setupArticoloContent()
{
    Articolo* articolo = dynamic_cast<Articolo*>(m_media.get());
    if (!articolo) return;
    
    // Aggiungi informazioni specifiche dell'articolo
    QLabel* rivistaLabel = new QLabel(QString("Rivista: %1").arg(
        truncateText(articolo->getRivista(), 20)));
    rivistaLabel->setStyleSheet("font-size: 10px; color: #666;");
    
    QLabel* autoriLabel = new QLabel(QString("Autori: %1").arg(
        truncateText(articolo->getAutori().join(", "), 25)));
    autoriLabel->setStyleSheet("font-size: 10px; color: #666;");
    
    m_contentLayout->insertWidget(m_contentLayout->count() - 1, rivistaLabel);
    m_contentLayout->insertWidget(m_contentLayout->count() - 1, autoriLabel);
}

QPixmap MediaCard::getTypeIcon() const
{
    if (!m_media) {
        return QPixmap();
    }
    
    QString type = m_media->getTypeDisplayName();
    QString iconPath;
    
    if (type == "Libro") {
        iconPath = ":/icons/libro.png";
    } else if (type == "Film") {
        iconPath = ":/icons/film.png";
    } else if (type == "Articolo") {
        iconPath = ":/icons/articolo.png";
    }
    
    QPixmap pixmap(iconPath);
    if (pixmap.isNull()) {
        // Crea un'icona di default se il file non esiste
        pixmap = QPixmap(IMAGE_SIZE, IMAGE_SIZE);
        pixmap.fill(QColor(COLOR_DEFAULT));
        
        QPainter painter(&pixmap);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 16, QFont::Bold));
        painter.drawText(pixmap.rect(), Qt::AlignCenter, 
                        type.left(1).toUpper());
    }
    
    return pixmap.scaled(IMAGE_SIZE, IMAGE_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QPixmap MediaCard::loadMediaImage() const
{
    // Implementazione futura per caricare immagini specifiche del media
    return getTypeIcon();
}

QString MediaCard::truncateText(const QString& text, int maxLength) const
{
    if (text.length() <= maxLength) {
        return text;
    }
    return text.left(maxLength - 3) + "...";
}

QString MediaCard::formatDisplayInfo() const
{
    if (!m_media) return QString();
    
    QString info = m_media->getDisplayInfo();
    
    // Prendi solo le prime due righe delle informazioni
    QStringList lines = info.split('\n');
    if (lines.size() > 2) {
        return lines.mid(0, 2).join('\n') + "...";
    }
    
    return info;
}