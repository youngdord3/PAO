#ifndef MEDIACARD_H
#define MEDIACARD_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QPixmap>
#include <memory>

class Media;

/**
 * @brief Widget card semplificato per visualizzare un media
 */
class MediaCard : public QFrame
{
    Q_OBJECT

public:
    // CORREZIONE: Costruttore semplificato che prende puntatore raw
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

private:
    void setupUI();
    void setupLayout();
    void updateStyleSheet();
    
    QString truncateText(const QString& text, int maxLength) const;
    QString formatDisplayInfo() const;
    QPixmap getTypeIcon() const;
    
    // CORREZIONE: Usa puntatore raw invece di unique_ptr
    Media* m_media;  // NON possiede il media, solo riferimento
    bool m_selected;
    bool m_hovered;
    
    // Widgets UI - semplificati
    QVBoxLayout* m_mainLayout;
    QLabel* m_typeLabel;
    QLabel* m_titleLabel;
    QLabel* m_yearLabel;
    QLabel* m_descriptionLabel;
    QLabel* m_imageLabel;
    
    // Costanti per il design
    static const int CARD_WIDTH = 280;
    static const int CARD_HEIGHT = 200;
    static const int IMAGE_SIZE = 48;
    
    // Colori per i tipi di media
    static const QString COLOR_LIBRO;
    static const QString COLOR_FILM;
    static const QString COLOR_ARTICOLO;
    static const QString COLOR_DEFAULT;
};

#endif // MEDIACARD_H