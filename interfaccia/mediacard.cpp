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
        // Non lanciare eccezione, ma gestisci gracefully
        setVisible(false);
        return;
    }
    
    setFixedSize(CARD_WIDTH, CARD_HEIGHT);
    setFrameStyle(QFrame::StyledPanel);
    
    try {
        setupUI();
        updateContent(); // Chiamiamo updateContent() invece di updateStyleSheet()
        updateStyleSheet();
        
        qDebug() << "MediaCard creata per:" << m_media->getTitolo() << "ID:" << m_media->getId();
    } catch (const std::exception& e) {
        qWarning() << "Errore nella creazione MediaCard:" << e.what();
        setVisible(false);
    }
}

MediaCard::~MediaCard()
{
    // Il distruttore è già implicito per unique_ptr
}

QString MediaCard::getId() const
{
    if (m_media) {
        return m_media->getId();
    }
    qWarning() << "Tentativo di ottenere ID da MediaCard con media nullo";
    return QString();
}

Media* MediaCard::getMedia() const
{
    if (!m_media) {
        qWarning() << "Tentativo di ottenere Media da MediaCard con media nullo";
    }
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
    if (!m_media) {
        qWarning() << "updateContent chiamato su MediaCard con media nullo";
        return;
    }
    
    try {
        // Aggiorna i contenuti delle label con controlli di sicurezza
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
        if (m_infoLabel) {
            m_infoLabel->setText(formatDisplayInfo());
        }
        
        // Aggiorna l'icona del tipo
        if (m_imageLabel) {
            m_imageLabel->setPixmap(getTypeIcon());
        }
        
        setupTypeSpecificContent();
    } catch (const std::exception& e) {
        qWarning() << "Errore nell'aggiornamento contenuto MediaCard:" << e.what();
    }
}

void MediaCard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_media) {
        setSelected(true);
        QString id = getId();
        if (!id.isEmpty()) {
            qDebug() << "Card selezionata:" << m_media->getTitolo() << "ID:" << id;
            emit selezionato(id);
        } else {
            qWarning() << "ID vuoto nella selezione card";
        }
    }
    QFrame::mousePressEvent(event);
}

void MediaCard::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_media) {
        QString id = getId();
        if (!id.isEmpty()) {
            qDebug() << "Doppio click su card:" << m_media->getTitolo() << "ID:" << id;
            emit doppioClick(id);
        }
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

void MediaCard::setupUI()
{
    if (!m_media) {
        qWarning() << "setupUI chiamato su MediaCard con media nullo";
        return;
    }
    
    try {
        setupLayout();
        
        // Configurazione delle label con controlli di sicurezza più rigorosi
        if (!m_typeLabel) {
            m_typeLabel = new QLabel(m_media->getTypeDisplayName(), this);
            if (m_typeLabel) {
                m_typeLabel->setStyleSheet("font-weight: bold; font-size: 10px; color: #666;");
            }
        }
        
        if (!m_titleLabel) {
            m_titleLabel = new QLabel(truncateText(m_media->getTitolo(), 25), this);
            if (m_titleLabel) {
                m_titleLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #333;");
                m_titleLabel->setWordWrap(true);
            }
        }
        
        if (!m_yearLabel) {
            m_yearLabel = new QLabel(QString::number(m_media->getAnno()), this);
            if (m_yearLabel) {
                m_yearLabel->setStyleSheet("font-size: 12px; color: #666;");
            }
        }
        
        if (!m_descriptionLabel) {
            m_descriptionLabel = new QLabel(truncateText(m_media->getDescrizione(), 80), this);
            if (m_descriptionLabel) {
                m_descriptionLabel->setStyleSheet("font-size: 11px; color: #555;");
                m_descriptionLabel->setWordWrap(true);
            }
        }
        
        if (!m_imageLabel) {
            m_imageLabel = new QLabel(this);
            if (m_imageLabel) {
                m_imageLabel->setFixedSize(IMAGE_SIZE, IMAGE_SIZE);
                m_imageLabel->setScaledContents(true);
                m_imageLabel->setPixmap(getTypeIcon());
            }
        }
        
        if (!m_infoLabel) {
            m_infoLabel = new QLabel(formatDisplayInfo(), this);
            if (m_infoLabel) {
                m_infoLabel->setStyleSheet("font-size: 10px; color: #777;");
                m_infoLabel->setWordWrap(true);
            }
        }
        
        // Configurazione bottoni (nascosti per default, mostrati solo al hover)
        if (!m_editButton) {
            m_editButton = new QPushButton("✏", this);
            if (m_editButton) {
                m_editButton->setFixedSize(24, 24);
                m_editButton->setToolTip("Modifica");
                m_editButton->setVisible(false);
                connect(m_editButton, &QPushButton::clicked, this, &MediaCard::onEditClicked);
            }
        }
        
        if (!m_deleteButton) {
            m_deleteButton = new QPushButton("🗑", this);
            if (m_deleteButton) {
                m_deleteButton->setFixedSize(24, 24);
                m_deleteButton->setToolTip("Elimina");
                m_deleteButton->setVisible(false);
                connect(m_deleteButton, &QPushButton::clicked, this, &MediaCard::onDeleteClicked);
            }
        }
        
        if (!m_detailsButton) {
            m_detailsButton = new QPushButton("👁", this);
            if (m_detailsButton) {
                m_detailsButton->setFixedSize(24, 24);
                m_detailsButton->setToolTip("Dettagli");
                m_detailsButton->setVisible(false);
                connect(m_detailsButton, &QPushButton::clicked, this, &MediaCard::onDetailsClicked);
            }
        }
        
        // Applica stile ai bottoni se esistono
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
        
        if (m_editButton) m_editButton->setStyleSheet(buttonStyle);
        if (m_deleteButton) m_deleteButton->setStyleSheet(buttonStyle);
        if (m_detailsButton) m_detailsButton->setStyleSheet(buttonStyle);
        
        setupTypeSpecificContent();
        
    } catch (const std::exception& e) {
        qWarning() << "Errore in setupUI:" << e.what();
    }
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
    if (!m_media) {
        qWarning() << "formatDisplayInfo chiamato su MediaCard con media nullo";
        return "Errore: media non disponibile";
    }
    
    try {
        QString info = m_media->getDisplayInfo();
        
        // Prendi solo le prime due righe delle informazioni
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