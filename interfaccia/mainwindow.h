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
#include <QStackedWidget>
#include <QTextEdit>
#include <QDateEdit>
#include <QListWidget>
#include <QFormLayout>
#include <vector>
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
    void tornaACollezione();
    
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
    
    // Form handling
    void onTipoChanged();
    void onSalvaMediaClicked();
    void onAnnullaClicked();
    void onAggiungiAutoreClicked();
    void onRimuoviAutoreClicked();
    void onAggiungiAttoreClicked();
    void onRimuoviAttoreClicked();
    void onValidazioneChanged();
    
    // About
    void informazioniSu();

private:
    enum ViewMode {
        COLLECTION_VIEW,
        DETAILS_VIEW,
        EDIT_VIEW,
        ADD_VIEW
    };
    
    // Setup interfaccia
    void setupUI();
    void setupToolBar();
    void setupStatusBar();
    void setupMainArea();
    void setupFilterArea();
    void setupMediaArea();
    void setupDetailsArea();
    void setupFormArea();
    
    // Gestione viste
    void switchToView(ViewMode mode);
    void setupCollectionView();
    void setupDetailsView();
    void setupFormView();
    
    // Gestione layout
    void updateLayout();
    void refreshMediaCards();
    void clearMediaCards();
    
    // Form specifici
    void setupBaseForm();
    void setupLibroForm();
    void setupFilmForm();
    void setupArticoloForm();
    void clearSpecificForm();
    void updateFormVisibility();
    void populateComboBoxes();
    
    // Gestione dati form
    void loadMediaData(Media* media);
    bool validateInput();
    QStringList getValidationErrors();
    std::unique_ptr<Media> createMediaFromForm();
    std::unique_ptr<Media> createLibro() const;
    std::unique_ptr<Media> createFilm() const;
    std::unique_ptr<Media> createArticolo() const;
    
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
    ViewMode m_currentView;
    bool m_isEditing;
    QString m_tipoCorrente;
    
    // Widgets principali
    QWidget* m_centralWidget;
    QSplitter* m_splitter;
    QWidget* m_filterWidget;
    QStackedWidget* m_stackedWidget;
    
    // Vista collezione
    QWidget* m_collectionWidget;
    QScrollArea* m_mediaScrollArea;
    QWidget* m_mediaContainer;
    QGridLayout* m_mediaLayout;
    
    // Vista dettagli
    QWidget* m_detailsWidget;
    QScrollArea* m_detailsScrollArea;
    QWidget* m_detailsContainer;
    QVBoxLayout* m_detailsLayout;
    QLabel* m_detailsTitleLabel;
    QLabel* m_detailsInfoLabel;
    QPushButton* m_tornaButton;
    QPushButton* m_modificaDettagliButton;
    
    // Vista form (aggiunta/modifica)
    QWidget* m_formWidget;
    QScrollArea* m_formScrollArea;
    QWidget* m_formContainer;
    QVBoxLayout* m_formMainLayout;
    QFormLayout* m_formLayout;
    
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
    
    // Form widgets
    QGroupBox* m_baseGroup;
    QComboBox* m_tipoFormCombo;
    QLineEdit* m_titoloEdit;
    QSpinBox* m_annoSpin;
    QTextEdit* m_descrizioneEdit;
    
    // Form libro
    QGroupBox* m_libroGroup;
    QLineEdit* m_autoreLibroEdit;
    QLineEdit* m_editoreEdit;
    QSpinBox* m_pagineSpin;
    QLineEdit* m_isbnEdit;
    QComboBox* m_genereLibroCombo;
    
    // Form film
    QGroupBox* m_filmGroup;
    QLineEdit* m_registaFormEdit;
    QListWidget* m_attoriList;
    QLineEdit* m_nuovoAttoreEdit;
    QPushButton* m_aggiungiAttoreBtn;
    QPushButton* m_rimuoviAttoreBtn;
    QSpinBox* m_durataSpin;
    QComboBox* m_genereFilmCombo;
    QComboBox* m_classificazioneCombo;
    QLineEdit* m_casaProduzioneEdit;
    
    // Form articolo
    QGroupBox* m_articoloGroup;
    QListWidget* m_autoriList;
    QLineEdit* m_nuovoAutoreEdit;
    QPushButton* m_aggiungiAutoreBtn;
    QPushButton* m_rimuoviAutoreBtn;
    QLineEdit* m_rivistaFormEdit;
    QLineEdit* m_volumeEdit;
    QLineEdit* m_numeroEdit;
    QLineEdit* m_pagineEdit;
    QComboBox* m_categoriaCombo;
    QComboBox* m_tipoRivistaCombo;
    QDateEdit* m_dataPubblicazioneEdit;
    QLineEdit* m_doiEdit;
    
    // Bottoni form
    QHBoxLayout* m_formButtonLayout;
    QPushButton* m_salvaButton;
    QPushButton* m_annullaButton;
    QLabel* m_validationLabel;
    
    // Status bar
    QLabel* m_statusLabel;
    QLabel* m_countLabel;
    QProgressBar* m_progressBar;
    
    std::vector<MediaCard*> m_mediaCards;
    
    // Dimensioni e layout
    static const int CARD_WIDTH = 280;
    static const int CARD_HEIGHT = 200;
    static const int CARD_MARGIN = 10;
    static const int FILTER_WIDTH = 250;
};

#endif // MAINWINDOW_H