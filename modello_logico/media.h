#ifndef MEDIACARD_H
#define MEDIACARD_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QPixmap>

class Media;

/**
 * @brief Widget card per visualizzare un media nella collezione
 * 
 * Ogni card mostra le informazioni principali del media
 * con un design responsivo e interattivo
 */
class MediaCard : public QFrame
{
    Q_OBJECT

public:
    explicit MediaCard(Media* media, QWidget *parent = nullptr);
    virtual ~MediaCard();
    
    // Accessori
    QString getId() const;
    Media* getMedia() const;
    
    // Gestione selezione
    void setSelected(bool selected);
    bool isSelected() const;
    
    // Aggiornamento contenuto
    void updateContent();

signals:
    void selezionato(const QString& id);
    void doppioClick(const QString& id);
    void contestualMenu(const QString& id, const QPoint& position);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onEditClicked();
    void onDeleteClicked();
    void onDetailsClicked();

private:
    void setupUI();
    void setupLayout();
    void updateStyleSheet();
    void setupTypeSpecificContent();
    
    // Creazione elementi UI specifici per tipo
    void setupLibroContent();
    void setupFilmContent();
    void setupArticoloContent();
    
    // Gestione immagini
    QPixmap getTypeIcon() const;
    QPixmap loadMediaImage() const;
    
    // Utility
    QString truncateText(const QString& text, int maxLength) const;
    QString formatDisplayInfo() const;
    
    // Membri privati
    Media* m_media;  // CORREZIONE: uso Media* invece di unique_ptr
    bool m_selected;
    bool m_hovered;
    
    // Widgets UI - inizializzati a nullptr automaticamente
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_headerLayout;
    QVBoxLayout* m_contentLayout;
    QHBoxLayout* m_buttonLayout;
    
    // Elementi UI comuni
    QLabel* m_typeLabel;
    QLabel* m_titleLabel;
    QLabel* m_yearLabel;
    QLabel* m_descriptionLabel;
    QLabel* m_imageLabel;
    QLabel* m_infoLabel;
    
    // Bottoni azione
    QPushButton* m_editButton;
    QPushButton* m_deleteButton;
    QPushButton* m_detailsButton;
    
    // Costanti per il design
    static const int CARD_WIDTH = 280;
    static const int CARD_HEIGHT = 200;
    static const int IMAGE_SIZE = 48;
    static const int BORDER_RADIUS = 8;
    static const int SHADOW_OFFSET = 2;
    
    // Colori per i tipi di media
    static const QString COLOR_LIBRO;
    static const QString COLOR_FILM;
    static const QString COLOR_ARTICOLO;
    static const QString COLOR_DEFAULT;
    
    // Stili CSS
    static const QString STYLE_NORMAL;
    static const QString STYLE_SELECTED;
    static const QString STYLE_HOVERED;
};

#endif // MEDIACARD_H