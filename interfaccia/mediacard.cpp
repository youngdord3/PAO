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
    return m_media;
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
        if (m_titleLabel) {
            m_titleLabel->setText(truncateText(m_media->getTitolo(), 25));
        }
        if (m_yearLabel) {
            m_yearLabel->setText(QString::number(m_media->getAnno()));
        }
        if (m_descriptionLabel) {
            m_descriptionLabel->setText(truncateText(m_media->getDescrizione(), 80));
        }
        if (m_typeLabel) {
            m_typeLabel->setText(m_media->getTypeDisplayName());
        }
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

void MediaCard::setupLayout()
{
    try {
        m_mainLayout = new QVBoxLayout(this);
        if (m_mainLayout) {
            m_mainLayout->setContentsMargins(8, 8, 8, 8);
            m_mainLayout->setSpacing(4);
        }
    } catch (const std::exception& e) {
        qWarning() << "Errore in setupLayout:" << e.what();
    }
}

void MediaCard::setupTypeSpecificContent()
{
    if (!m_media || !m_mainLayout) return;
    
    try {
        // Aggiungi widgets al layout se esistono
        if (m_headerLayout && m_typeLabel && m_imageLabel) {
            m_headerLayout->addWidget(m_typeLabel);
            m_headerLayout->addStretch();
            m_headerLayout->addWidget(m_imageLabel);
        }
        
        if (m_contentLayout) {
            if (m_titleLabel) m_contentLayout->addWidget(m_titleLabel);
            
            if (m_yearLabel) {
                QHBoxLayout* yearLayout = new QHBoxLayout();
                yearLayout->addWidget(new QLabel("Anno:", this));
                yearLayout->addWidget(m_yearLabel);
                yearLayout->addStretch();
                m_contentLayout->addLayout(yearLayout);
            }
            
            if (m_descriptionLabel) m_contentLayout->addWidget(m_descriptionLabel);
            if (m_infoLabel) m_contentLayout->addWidget(m_infoLabel);
        }
        
        if (m_buttonLayout) {
            m_buttonLayout->addStretch();
            if (m_detailsButton) m_buttonLayout->addWidget(m_detailsButton);
            if (m_editButton) m_buttonLayout->addWidget(m_editButton);
            if (m_deleteButton) m_buttonLayout->addWidget(m_deleteButton);
        }
        
        // Aggiungi contenuto specifico per tipo
        QString type = m_media->getTypeDisplayName();
        if (type == "Libro") {
            setupLibroContent();
        } else if (type == "Film") {
            setupFilmContent();
        } else if (type == "Articolo") {
            setupArticoloContent();
        }
        
    } catch (const std::exception& e) {
        qWarning() << "Errore in setupTypeSpecificContent:" << e.what();
    }
}

void MediaCard::setupLibroContent()
{
    // Implementazione base per libri
    // Per ora vuota, può essere estesa in futuro
}

void MediaCard::setupFilmContent()
{
    // Implementazione base per film
    // Per ora vuota, può essere estesa in futuro
}

void MediaCard::setupArticoloContent()
{
    // Implementazione base per articoli
    // Per ora vuota, può essere estesa in futuro
}

void MediaCard::setupUI()
{
    if (!m_media) return;
    
    try {
        setupLayout();
        
        m_typeLabel = new QLabel(m_media->getTypeDisplayName(), this);
        m_typeLabel->setStyleSheet("font-weight: bold; font-size: 10px; color: #666;");
        
        m_titleLabel = new QLabel(truncateText(m_media->getTitolo(), 25), this);
        m_titleLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #333;");
        m_titleLabel->setWordWrap(true);
        
        m_yearLabel = new QLabel(QString::number(m_media->getAnno()), this);
        m_yearLabel->setStyleSheet("font-size: 12px; color: #666;");
        
        m_descriptionLabel = new QLabel(truncateText(m_media->getDescrizione(), 80), this);
        m_descriptionLabel->setStyleSheet("font-size: 11px; color: #555;");
        m_descriptionLabel->setWordWrap(true);
        
        m_imageLabel = new QLabel(this);
        m_imageLabel->setFixedSize(IMAGE_SIZE, IMAGE_SIZE);
        m_imageLabel->setScaledContents(true);
        m_imageLabel->setPixmap(getTypeIcon());
        
        // Layout semplificato
        if (m_mainLayout) {
            QHBoxLayout* headerLayout = new QHBoxLayout();
            headerLayout->addWidget(m_typeLabel);
            headerLayout->addStretch();
            headerLayout->addWidget(m_imageLabel);
            
            m_mainLayout->addLayout(headerLayout);
            m_mainLayout->addWidget(m_titleLabel);
            m_mainLayout->addWidget(m_yearLabel);
            m_mainLayout->addWidget(m_descriptionLabel);
            m_mainLayout->addStretch();
        }
        
    } catch (const std::exception& e) {
        qWarning() << "Errore in setupUI:" << e.what();
    }
}

void MediaCard::updateStyleSheet()
{
    try {
        QString style;
        if (m_selected) {
            style = "MediaCard { background-color: #E3F2FD; border: 2px solid #2196F3; border-radius: 8px; }";
        } else if (m_hovered) {
            style = "MediaCard { background-color: #F5F5F5; border: 2px solid #2196F3; border-radius: 8px; }";
        } else {
            style = "MediaCard { background-color: white; border: 1px solid #E0E0E0; border-radius: 8px; }";
        }
        setStyleSheet(style);
    } catch (const std::exception& e) {
        qWarning() << "Errore in updateStyleSheet:" << e.what();
    }
}

QPixmap MediaCard::getTypeIcon() const
{
    if (!m_media) {
        return QPixmap();
    }
    
    QString type = m_media->getTypeDisplayName();
    QString color = COLOR_DEFAULT;
    
    if (type == "Libro") {
        color = COLOR_LIBRO;
    } else if (type == "Film") {
        color = COLOR_FILM;
    } else if (type == "Articolo") {
        color = COLOR_ARTICOLO;
    }
    
    QPixmap pixmap(IMAGE_SIZE, IMAGE_SIZE);
    pixmap.fill(QColor(color));
    
    QPainter painter(&pixmap);
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 16, QFont::Bold));
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
        QStringList lines = info.split('\n');
        if (lines.size() > 2) {
            return lines.mid(0, 2).join('\n') + "...";
        }
        return info;
    } catch (const std::exception& e) {
        qWarning() << "Errore in formatDisplayInfo:" << e.what();
        return "Errore nel formato";
    }
}

void MediaCard::onEditClicked()
{
    if (m_media) {
        QString id = getId();
        if (!id.isEmpty()) {
            qDebug() << "Edit button clicked per:" << m_media->getTitolo();
            emit selezionato(id); // Seleziona prima
            // Il MainWindow gestirà l'apertura del dialog di modifica
        }
    }
}

void MediaCard::onDeleteClicked()
{
    if (m_media) {
        QString id = getId();
        if (!id.isEmpty()) {
            qDebug() << "Delete button clicked per:" << m_media->getTitolo();
            emit selezionato(id); // Seleziona prima
            // Il MainWindow gestirà la rimozione
        }
    }
}

void MediaCard::onDetailsClicked()
{
    if (m_media) {
        QString id = getId();
        if (!id.isEmpty()) {
            qDebug() << "Details button clicked per:" << m_media->getTitolo();
            emit doppioClick(id); // Usa doppio click per i dettagli
        }
    }
}