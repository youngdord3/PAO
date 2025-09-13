#include "mediacard.h"
#include "modello_logico/media.h"
#include "modello_logico/libro.h"
#include "modello_logico/film.h"
#include "modello_logico/articolo.h"
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QPainter>
#include <QStyleOption>
#include <QApplication>
#include <QStyle>
#include <QPixmap>

MediaCard::MediaCard(Media* media, QWidget *parent)
    : QFrame(parent)
    , m_media(media)
    , m_selected(false)
    , m_hovered(false)
    , m_mainLayout(nullptr)
    , m_headerLayout(nullptr)
    , m_contentLayout(nullptr)
    , m_typeLabel(nullptr)
    , m_titleLabel(nullptr)
    , m_yearLabel(nullptr)
    , m_descriptionLabel(nullptr)
    , m_imageLabel(nullptr)
    , m_infoLabel(nullptr)
{
    if (!m_media) {
        qWarning() << "MediaCard creata con media nullo!";
        return;
    }
    
    setFixedSize(CARD_WIDTH, CARD_HEIGHT);
    setFrameStyle(QFrame::StyledPanel);
    
    // Imposta il tipo di media come proprietà per il CSS
    QString mediaType = m_media->getTypeDisplayName().toLower();
    setProperty("mediaType", mediaType);
    
    // Imposta stato iniziale
    setProperty("selected", false);
    
    try {
        setupUI();
    } catch (const std::exception& e) {
        qWarning() << "Errore nella creazione MediaCard:" << e.what();
    }
}

MediaCard::~MediaCard()
{
    // Non cancellare il media pointer, poiché non lo possediamo
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
        setProperty("selected", selected);
        
        // Forza il refresh dello stile
        style()->unpolish(this);
        style()->polish(this);
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
    QFrame::enterEvent(event);
}

void MediaCard::leaveEvent(QEvent *event)
{
    m_hovered = false;
    QFrame::leaveEvent(event);
}

void MediaCard::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);
    
    // Il colore della barra viene gestito dal CSS tramite border-left
    // Non serve più disegnare manualmente
}

void MediaCard::setupUI()
{
    if (!m_media) return;
    
    try {
        setupLayout();
        
        // Configurazione delle label con objectName per il CSS
        m_typeLabel = new QLabel(m_media->getTypeDisplayName(), this);
        m_typeLabel->setObjectName("typeLabel");  // Per il CSS
        
        m_titleLabel = new QLabel(truncateText(m_media->getTitolo(), 25), this);
        m_titleLabel->setObjectName("titleLabel");  // Per il CSS
        m_titleLabel->setWordWrap(true);
        
        m_yearLabel = new QLabel(QString::number(m_media->getAnno()), this);
        m_yearLabel->setObjectName("yearLabel");  // Per il CSS
        
        m_descriptionLabel = new QLabel(truncateText(m_media->getDescrizione(), 80), this);
        m_descriptionLabel->setObjectName("descriptionLabel");  // Per il CSS
        m_descriptionLabel->setWordWrap(true);
        
        m_imageLabel = new QLabel(this);
        m_imageLabel->setFixedSize(IMAGE_SIZE, IMAGE_SIZE);
        m_imageLabel->setScaledContents(true);
        m_imageLabel->setPixmap(getTypeIcon());
        
        m_infoLabel = new QLabel(formatDisplayInfo(), this);
        m_infoLabel->setObjectName("infoLabel");  // Per il CSS
        m_infoLabel->setWordWrap(true);
        
        setupTypeSpecificContent();
        
    } catch (const std::exception& e) {
        qWarning() << "Errore in setupUI:" << e.what();
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
        
        // Header con tipo e immagine
        m_headerLayout = new QHBoxLayout();
        if (m_headerLayout) {
            m_headerLayout->setContentsMargins(0, 0, 0, 0);
        }
        
        // Content area
        m_contentLayout = new QVBoxLayout();
        if (m_contentLayout) {
            m_contentLayout->setContentsMargins(0, 0, 0, 0);
            m_contentLayout->setSpacing(2);
        }
    } catch (const std::exception& e) {
        qWarning() << "Errore in setupLayout:" << e.what();
    }
}

void MediaCard::setupTypeSpecificContent()
{
    if (!m_media || !m_mainLayout) return;
    
    try {
        if (!m_headerLayout) {
            m_headerLayout = new QHBoxLayout();
        }
        if (!m_contentLayout) {
            m_contentLayout = new QVBoxLayout();
        }
        
        // Pulisci solo se necessario
        if (m_mainLayout->count() == 0) {
            // Header: tipo + icona
            if (m_typeLabel && m_imageLabel) {
                m_headerLayout->addWidget(m_typeLabel);
                m_headerLayout->addStretch();
                m_headerLayout->addWidget(m_imageLabel);
            }
            
            // Informazioni principali
            if (m_titleLabel) m_contentLayout->addWidget(m_titleLabel);
            if (m_yearLabel) {
                QHBoxLayout* yearLayout = new QHBoxLayout();
                QLabel* yearLabelText = new QLabel("Anno:", this);
                yearLabelText->setObjectName("yearLabelText");
                yearLayout->addWidget(yearLabelText);
                yearLayout->addWidget(m_yearLabel);
                yearLayout->addStretch();
                m_contentLayout->addLayout(yearLayout);
            }
            if (m_descriptionLabel) m_contentLayout->addWidget(m_descriptionLabel);
            if (m_infoLabel) m_contentLayout->addWidget(m_infoLabel);
            
            // Assembla layout finale
            m_mainLayout->addLayout(m_headerLayout);
            m_mainLayout->addLayout(m_contentLayout);
            m_mainLayout->addStretch();
        }
        
    } catch (const std::exception& e) {
        qWarning() << "Errore in setupTypeSpecificContent:" << e.what();
    }
}

QPixmap MediaCard::getTypeIcon() const
{
    if (!m_media) {
        return QPixmap();
    }
    
    QString type = m_media->getTypeDisplayName().toLower();
    
    // Carica l'icona appropriata dal resources.qrc
    QIcon icon;
    if (type == "libro") {
        icon = QIcon(":/icons/libro.png");
    } else if (type == "film") {
        icon = QIcon(":/icons/film.png");
    } else if (type == "articolo") {
        icon = QIcon(":/icons/articolo.png");
    }
    
    // Se l'icona esiste nel resources.qrc, convertila in QPixmap
    if (!icon.isNull()) {
        return icon.pixmap(IMAGE_SIZE, IMAGE_SIZE);
    }
    
    // Se non esiste l'icona nel resources.qrc, restituisci un pixmap vuoto
    return QPixmap();
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