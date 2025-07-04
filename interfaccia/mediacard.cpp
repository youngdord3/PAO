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
    "    padding: 8px;"
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
    "    padding: 8px;"
    "    box-shadow: 0 2px 8px rgba(33,150,243,0.3);"
    "}";

const QString MediaCard::STYLE_HOVERED = 
    "MediaCard {"
    "    background-color: #F5F5F5;"
    "    border: 2px solid #2196F3;"
    "    border-radius: 8px;"
    "    margin: 2px;"
    "    padding: 8px;"
    "    box-shadow: 0 4px 12px rgba(0,0,0,0.15);"
    "}";

MediaCard::MediaCard(std::unique_ptr<Media> media, QWidget *parent)
    : QFrame(parent)
    , m_media(std::move(media))
    , m_selected(false)
    , m_hovered(false)
    , m_mainLayout(nullptr)
    , m_headerLayout(nullptr)
    , m_contentLayout(nullptr)
    , m_buttonLayout(nullptr)
    , m_typeLabel(nullptr)
    , m_titleLabel(nullptr)
    , m_yearLabel(nullptr)
    , m_descriptionLabel(nullptr)
    , m_imageLabel(nullptr)
    , m_infoLabel(nullptr)
    , m_editButton(nullptr)
    , m_deleteButton(nullptr)
    , m_detailsButton(nullptr)
{
    if (!m_media) {
        qWarning() << "MediaCard creata con media nullo!";
        return;
    }
    
    setFixedSize(CARD_WIDTH, CARD_HEIGHT);
    setFrameStyle(QFrame::StyledPanel);
    setCursor(Qt::PointingHandCursor); // Indica che è cliccabile
    
    try {
        setupUI();
        updateStyleSheet();
    } catch (const std::exception& e) {
        qWarning() << "Errore nella creazione MediaCard:" << e.what();
    }
}

MediaCard::~MediaCard()
{
    // Il distruttore è già implicito per unique_ptr
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
    
    try {
        // Aggiorna i contenuti delle label con controlli di sicurezza
        if (m_titleLabel) {
            m_titleLabel->setText(truncateText(m_media->getTitolo(), 30));
        }
        if (m_yearLabel) {
            m_yearLabel->setText(QString::number(m_media->getAnno()));
        }
        if (m_descriptionLabel) {
            m_descriptionLabel->setText(truncateText(m_media->getDescrizione(), 100));
        }
        if (m_typeLabel) {
            m_typeLabel->setText(m_media->getTypeDisplayName());
        }
        if (m_infoLabel) {
            m_infoLabel->setText(formatDisplayInfo());
        }
        
        // Aggiorna l'icona del tipo
        if (m_imageLabel) {
            m_imageLabel->setPixmap(getTypeIcon());
        }
        
    } catch (const std::exception& e) {
        qWarning() << "Errore nell'aggiornamento contenuto MediaCard:" << e.what();
    }
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

// Metodi vuoti per compatibilità
void MediaCard::onEditClicked()
{
    // Rimosso - ora si usa solo il box azioni
}

void MediaCard::onDeleteClicked()
{
    // Rimosso - ora si usa solo il box azioni
}

void MediaCard::onDetailsClicked()
{
    // Rimosso - ora si usa solo il box azioni
}

void MediaCard::setupUI()
{
    if (!m_media) return;
    
    try {
        // Layout principale semplificato
        m_mainLayout = new QVBoxLayout(this);
        m_mainLayout->setContentsMargins(12, 8, 12, 8);
        m_mainLayout->setSpacing(6);
        
        // Header con tipo e icona
        m_headerLayout = new QHBoxLayout();
        m_headerLayout->setContentsMargins(0, 0, 0, 0);
        
        m_typeLabel = new QLabel(m_media->getTypeDisplayName(), this);
        m_typeLabel->setStyleSheet("font-weight: bold; font-size: 11px; color: #666; text-transform: uppercase;");
        
        m_imageLabel = new QLabel(this);
        m_imageLabel->setFixedSize(IMAGE_SIZE, IMAGE_SIZE);
        m_imageLabel->setScaledContents(true);
        m_imageLabel->setPixmap(getTypeIcon());
        
        m_headerLayout->addWidget(m_typeLabel);
        m_headerLayout->addStretch();
        m_headerLayout->addWidget(m_imageLabel);
        
        // Contenuto principale
        m_titleLabel = new QLabel(truncateText(m_media->getTitolo(), 30), this);
        m_titleLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #333;");
        m_titleLabel->setWordWrap(true);
        m_titleLabel->setMaximumHeight(40); // Limita l'altezza
        
        // Anno con label
        QHBoxLayout* yearLayout = new QHBoxLayout();
        yearLayout->setContentsMargins(0, 0, 0, 0);
        QLabel* yearLabelTitle = new QLabel("Anno:", this);
        yearLabelTitle->setStyleSheet("font-size: 11px; color: #666; font-weight: bold;");
        m_yearLabel = new QLabel(QString::number(m_media->getAnno()), this);
        m_yearLabel->setStyleSheet("font-size: 11px; color: #333;");
        yearLayout->addWidget(yearLabelTitle);
        yearLayout->addWidget(m_yearLabel);
        yearLayout->addStretch();
        
        // Descrizione
        m_descriptionLabel = new QLabel(truncateText(m_media->getDescrizione(), 100), this);
        m_descriptionLabel->setStyleSheet("font-size: 10px; color: #555; line-height: 1.3;");
        m_descriptionLabel->setWordWrap(true);
        m_descriptionLabel->setMaximumHeight(45);
        
        // Info aggiuntive
        m_infoLabel = new QLabel(formatDisplayInfo(), this);
        m_infoLabel->setStyleSheet("font-size: 9px; color: #777; font-style: italic;");
        m_infoLabel->setWordWrap(true);
        m_infoLabel->setMaximumHeight(30);
        
        // Assemblaggio layout
        m_mainLayout->addLayout(m_headerLayout);
        m_mainLayout->addWidget(m_titleLabel);
        m_mainLayout->addLayout(yearLayout);
        m_mainLayout->addWidget(m_descriptionLabel);
        m_mainLayout->addStretch();
        m_mainLayout->addWidget(m_infoLabel);
        
    } catch (const std::exception& e) {
        qWarning() << "Errore in setupUI:" << e.what();
    }
}

void MediaCard::setupLayout()
{
    // Metodo vuoto - ora tutto è in setupUI()
}

void MediaCard::updateStyleSheet()
{
    try {
        if (m_selected) {
            setStyleSheet(STYLE_SELECTED);
        } else if (m_hovered) {
            setStyleSheet(STYLE_HOVERED);
        } else {
            setStyleSheet(STYLE_NORMAL);
        }
    } catch (const std::exception& e) {
        qWarning() << "Errore in updateStyleSheet:" << e.what();
    }
}

void MediaCard::setupTypeSpecificContent()
{
    // Semplificato - ora non serve più
}

void MediaCard::setupLibroContent()
{
    // Rimosso - ora non serve più
}

void MediaCard::setupFilmContent()
{
    // Rimosso - ora non serve più
}

void MediaCard::setupArticoloContent()
{
    // Rimosso - ora non serve più
}

QPixmap MediaCard::getTypeIcon() const
{
    if (!m_media) {
        return QPixmap();
    }
    
    QString type = m_media->getTypeDisplayName();
    
    // Prova prima a caricare l'icona dalle risorse
    QString iconPath;
    if (type == "Libro") {
        iconPath = ":/icons/libro.png";
    } else if (type == "Film") {
        iconPath = ":/icons/film.png";
    } else if (type == "Articolo") {
        iconPath = ":/icons/articolo.png";
    }
    
    // Carica l'icona se esiste
    QPixmap pixmap(iconPath);
    if (!pixmap.isNull()) {
        return pixmap.scaled(IMAGE_SIZE, IMAGE_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    
    // Fallback: crea un'icona di default semplice
    pixmap = QPixmap(IMAGE_SIZE, IMAGE_SIZE);
    QString color = COLOR_DEFAULT;
    
    if (type == "Libro") {
        color = COLOR_LIBRO;
    } else if (type == "Film") {
        color = COLOR_FILM;
    } else if (type == "Articolo") {
        color = COLOR_ARTICOLO;
    }
    
    pixmap.fill(QColor(color));
    
    QPainter painter(&pixmap);
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 14, QFont::Bold));
    painter.drawText(pixmap.rect(), Qt::AlignCenter, type.left(1).toUpper());
    
    return pixmap;
}

QPixmap MediaCard::loadMediaImage() const
{
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
    
    try {
        QString info = m_media->getDisplayInfo();
        
        // Prendi solo le prime due righe delle informazioni
        QStringList lines = info.split('\n');
        if (lines.size() > 2) {
            return lines.mid(0, 2).join(" • ") + "...";
        }
        
        return lines.join(" • ");
    } catch (const std::exception& e) {
        qWarning() << "Errore in formatDisplayInfo:" << e.what();
        return "Errore nel formato";
    }
}