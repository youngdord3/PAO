// interfaccia/mediacard.h - Rimuovere costanti colori hardcoded

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
 * @brief Widget card per visualizzare un media nella collezione
 * 
 * Ogni card mostra le informazioni principali del media
 * con un design responsivo e interattivo utilizzando solo CSS
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

private:
    void setupUI();
    void setupLayout();
    void setupTypeSpecificContent();
    
    // Gestione immagini
    QPixmap getTypeIcon() const;
    
    // Utility
    QString truncateText(const QString& text, int maxLength) const;
    QString formatDisplayInfo() const;
    
    // Puntatore al media
    Media* m_media;
    bool m_selected;
    bool m_hovered;
    
    // Widgets UI per layout
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_headerLayout;
    QVBoxLayout* m_contentLayout;
    
    // Elementi UI per visualizzazione dati - con objectName per CSS
    QLabel* m_typeLabel;
    QLabel* m_titleLabel;
    QLabel* m_yearLabel;
    QLabel* m_descriptionLabel;
    QLabel* m_imageLabel;
    QLabel* m_infoLabel;
    
    // Costanti per le dimensioni
    static const int CARD_WIDTH = 280;
    static const int CARD_HEIGHT = 200;
    static const int IMAGE_SIZE = 48;
};

#endif // MEDIACARD_H