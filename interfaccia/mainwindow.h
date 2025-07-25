#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QMenuBar>
#include <QStatusBar>
#include <QProgressBar>
#include <QSplitter>
#include <QGroupBox>
#include <QSpinBox>
#include <QCheckBox>
#include <vector>  // AGGIUNTO: per std::vector
#include <memory>

class Collezione;
class Media;
class MediaCard;
class FiltroStrategy;

/**
 * @brief Finestra principale dell'applicazione
 * 
 * Implementa il pattern MVC separando completamente 
 * l'interfaccia dal modello logico
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    // Gestione file
    void nuovaCollezione();
    void apriCollezione();
    void salvaCollezione();
    void salvaCollezioneCome();
    void esportaCSV();
    void esci();
    
    // Gestione media
    void aggiungiMedia();
    void rimuoviMedia();
    void modificaMedia();
    void visualizzaDettagli();
    
    // Ricerca e filtri
    void cercaMedia();
    void applicaFiltri();
    void resetFiltri();
    void filtraPerTipo();
    void filtraPerAnno();
    
    // Slots per notifiche dalla collezione
    void onMediaAggiunto(const QString& id);
    void onMediaRimosso(const QString& id);
    void onMediaModificato(const QString& id);
    void onCollezioneCaricata(int count);
    
    // Gestione card
    void onCardSelezionata(const QString& id);
    void onCardDoubleClic(const QString& id);
    
    // RIMOSSO: void informazioniSu();

private:
    // Setup interfaccia
    void setupUI();
    void setupToolBar();
    void setupStatusBar();
    void setupMainArea();
    void setupFilterArea();
    void setupMediaArea();
    
    // Gestione layout
    void updateLayout();
    void refreshMediaCards();
    void clearMediaCards();
    
    // Filtri e ricerca
    void applicaRicercaCorrente();
    std::unique_ptr<FiltroStrategy> creaFiltroCorrente();
    
    // Utility
    void aggiornaStatistiche();
    void aggiornaStatusBar();
    void salvaImpostazioni();
    void caricaImpostazioni();
    bool verificaModifiche();
    void resetInterfaccia();
    
    // Gestione errori
    void mostraErrore(const QString& errore);
    void mostraInfo(const QString& info);
    bool confermaAzione(const QString& messaggio);
    
    // Membri privati
    std::unique_ptr<Collezione> m_collezione;
    QString m_fileCorrente;
    bool m_modificato;
    QString m_selezionato_id;
    
    // Widgets principali
    QWidget* m_centralWidget;
    QSplitter* m_splitter;
    QWidget* m_filterWidget;
    QScrollArea* m_mediaScrollArea;
    QWidget* m_mediaContainer;
    QGridLayout* m_mediaLayout;
    
    // Area filtri
    QGroupBox* m_searchGroup;
    QLineEdit* m_searchEdit;
    QPushButton* m_searchButton;
    QPushButton* m_clearSearchButton;
    
    QGroupBox* m_filterGroup;
    QComboBox* m_tipoCombo;
    QSpinBox* m_annoMinSpin;
    QSpinBox* m_annoMaxSpin;
    QLineEdit* m_autoreEdit;
    QLineEdit* m_registaEdit;
    QLineEdit* m_rivistaEdit;
    QPushButton* m_applyFilterButton;
    QPushButton* m_resetFilterButton;
    
    QGroupBox* m_statisticheGroup;
    QLabel* m_totalLabel;
    QLabel* m_libriLabel;
    QLabel* m_filmLabel;
    QLabel* m_articoliLabel;
    
    // Bottoni azioni
    QGroupBox* m_actionsGroup;
    QPushButton* m_addButton;
    QPushButton* m_editButton;
    QPushButton* m_removeButton;
    QPushButton* m_detailsButton;
    
    // Status bar
    QLabel* m_statusLabel;
    QLabel* m_countLabel;
    QProgressBar* m_progressBar;
    
    // CORREZIONE: Usa std::vector invece di QVector per evitare problemi di compilazione
    std::vector<MediaCard*> m_mediaCards;
    
    // Dimensioni e layout
    static const int CARD_WIDTH = 280;
    static const int CARD_HEIGHT = 200;
    static const int CARD_MARGIN = 10;
    static const int FILTER_WIDTH = 250;
};

#endif // MAINWINDOW_H