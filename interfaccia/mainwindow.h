#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QScrollArea>
#include <QLineEdit>
#include <QTextEdit>
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
#include <QListWidget>
#include <QDateEdit>
#include <QTimer>
#include <QMessageBox>
#include <QDate>
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
    
    // Gestione media
    void aggiungiMedia();
    void rimuoviMedia();
    void modificaMedia();
    void visualizzaDettagli();
    
    // Ricerca e filtri
    void cercaMedia();
    void applicaFiltri();
    void resetFiltri();
    
    // Slots per notifiche dalla collezione
    void onMediaAggiunto(const QString& id);
    void onMediaRimosso(const QString& id);
    void onMediaModificato(const QString& id);
    void onCollezioneCaricata(int count);
    
    // Gestione card
    void onCardSelezionata(const QString& id);
    void onCardDoubleClic(const QString& id);

    // Nuovi slots per il pannello integrato
    void showEditPanel(bool isNew = false, bool readOnly = false);
    void hideEditPanel();
    void onEditTipoChanged();
    void onEditSalvaClicked();
    void onEditAnnullaClicked();
    void onEditValidationChanged();
    void onEditAggiungiAutoreClicked();
    void onEditRimuoviAutoreClicked();
    void onEditAggiungiAttoreClicked();
    void onEditRimuoviAttoreClicked();

private:
    // Setup interfaccia
    void setupUI();
    void setupToolBar();
    void setupStatusBar();
    void setupMainArea();
    void setupFilterArea();
    void setupMediaArea();
    void setupEditPanel();
    
    // Gestione layout
    void updateLayout();
    void refreshMediaCards();
    void clearMediaCards();

    // Metodi helper per gestire i controlli di gestione
    void hideManagementControls();
    void showManagementControls();
    
    // Filtri e ricerca
    void applicaRicercaCorrente();
    std::unique_ptr<FiltroStrategy> creaFiltroCorrente();
    
    // Setup pannello edit
    void setupEditBaseForm();
    void setupEditLibroForm();
    void setupEditFilmForm();
    void setupEditArticoloForm();
    void clearEditSpecificForm();
    void setupEditTypeSpecificForm();
    void setupEditConnections();
    void setupEditSpecificConnections();
    void disconnectEditGroupWidgets(QGroupBox* group);
    void resetEditSpecificPointers();
    void resetEditPanelState();
    
    // Gestione dati pannello edit
    void loadEditMediaData(Media* media);
    void updateEditFormVisibility();
    bool validateEditInput();
    QStringList getEditValidationErrors();
    std::unique_ptr<Media> createEditMedia();
    std::unique_ptr<Media> createEditLibro();
    std::unique_ptr<Media> createEditFilm();
    std::unique_ptr<Media> createEditArticolo();
    void enableEditForm(bool enabled);
    bool areEditCurrentTypeWidgetsReady() const;
    
    // Utility
    void aggiornaStatistiche();
    void aggiornaStatusBar();
    void aggiornaStatoBottoni();
    void salvaImpostazioni();
    void caricaImpostazioni();
    bool verificaModifiche();
    
    // Gestione errori
    void mostraErrore(const QString& errore);
    void mostraInfo(const QString& info);
    
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
    QProgressBar* m_progressBar;
    
    QList<MediaCard*> m_mediaCards;
    
    // Pannello di modifica integrato
    QWidget* m_editPanel;
    QWidget* m_editContentContainer;
    QScrollArea* m_editScrollArea;
    QLabel* m_editHeaderLabel;
    bool m_editPanelVisible;
    QString m_editingMediaId;
    bool m_editReadOnly;
    bool m_editIsNew;
    QString m_editTipoCorrente;
    
    // Widgets del form di modifica
    QVBoxLayout* m_editFormLayout;
    QGroupBox* m_editBaseGroup;
    QComboBox* m_editTipoCombo;
    QLineEdit* m_editTitoloEdit;
    QSpinBox* m_editAnnoSpin;
    QTextEdit* m_editDescrizioneEdit;
    
    // Form libro
    QGroupBox* m_editLibroGroup;
    QLineEdit* m_editAutoreEdit;
    QLineEdit* m_editEditoreEdit;
    QSpinBox* m_editPagineSpin;
    QLineEdit* m_editIsbnEdit;
    QComboBox* m_editGenereLibroCombo;
    
    // Form film
    QGroupBox* m_editFilmGroup;
    QLineEdit* m_editRegistaEdit;
    QListWidget* m_editAttoriList;
    QLineEdit* m_editNuovoAttoreEdit;
    QPushButton* m_editAggiungiAttoreBtn;
    QPushButton* m_editRimuoviAttoreBtn;
    QSpinBox* m_editDurataSpin;
    QComboBox* m_editGenereFilmCombo;
    QComboBox* m_editClassificazioneCombo;
    QLineEdit* m_editCasaProduzioneEdit;
    
    // Form articolo
    QGroupBox* m_editArticoloGroup;
    QListWidget* m_editAutoriList;
    QLineEdit* m_editNuovoAutoreEdit;
    QPushButton* m_editAggiungiAutoreBtn;
    QPushButton* m_editRimuoviAutoreBtn;
    QLineEdit* m_editRivisteEdit;
    QLineEdit* m_editVolumeEdit;
    QLineEdit* m_editNumeroEdit;
    QLineEdit* m_editPagineEdit;
    QComboBox* m_editCategoriaCombo;
    QComboBox* m_editTipoRivistaCombo;
    QDateEdit* m_editDataPubblicazioneEdit;
    QLineEdit* m_editDoiEdit;
    
    // Bottoni del pannello
    QPushButton* m_editSalvaButton;
    QPushButton* m_editAnnullaButton;
    QPushButton* m_editHelpButton;
    QLabel* m_editValidationLabel;
    bool m_editValidationEnabled;
    
    // Dimensioni e layout
    static const int CARD_WIDTH = 280;
    static const int CARD_HEIGHT = 200;
    static const int CARD_MARGIN = 10;
    static const int FILTER_WIDTH = 270;
    
    // Dimensioni dei componenti filtri
    static const int SEARCH_GROUP_HEIGHT = 100;
    static const int FILTER_GROUP_HEIGHT = 280;
    static const int STATS_GROUP_HEIGHT = 120;
    static const int ACTIONS_GROUP_HEIGHT = 180;
    
    static const int INPUT_HEIGHT = 35;
    static const int BUTTON_HEIGHT = 35;
    static const int SMALL_BUTTON_HEIGHT = 30;
    
    static const int SPINBOX_WIDTH = 70;
    static const int LABEL_WIDTH_SMALL = 25;
    static const int CLEAR_BUTTON_WIDTH = 40;
    
    // Costanti pannello edit
    static const int EDIT_PANEL_WIDTH = 600;
    static const int MAX_DESCRIZIONE_LENGTH = 500;
};

#endif // MAINWINDOW_H