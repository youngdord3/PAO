#ifndef MEDIADIALOG_H
#define MEDIADIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QTabWidget>
#include <QListWidget>
#include <QScrollArea>
#include <memory>

class Media;

/**
 * @brief Dialog per la creazione e modifica di media
 * 
 * Implementa form dinamici che si adattano al tipo di media
 * selezionato, utilizzando il polimorfismo per gestire
 * i diversi tipi di input
 */
class MediaDialog : public QDialog
{
    Q_OBJECT

public:
    // Costruttore per nuovo media
    explicit MediaDialog(QWidget *parent = nullptr);
    
    // Costruttore per modifica media esistente
    explicit MediaDialog(Media* media, QWidget *parent = nullptr, bool readOnly = false);
    
    ~MediaDialog() = default;
    
    // Ottieni il media creato/modificato
    std::unique_ptr<Media> getMedia() const;

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void onTipoChanged();
    void onAccettaClicked();
    void onAnnullaClicked();
    void onAggiungiAutoreClicked();
    void onRimuoviAutoreClicked();
    void onAggiungiAttoreClicked();
    void onRimuoviAttoreClicked();
    void onValidazioneChanged();

private:
    void setupUI();
    void setupBaseForm();
    void setupTipoSpecificForm();
    void setupButtons();
    
    // Setup form specifici
    void setupLibroForm();
    void setupFilmForm();
    void setupArticoloForm();
    
    // Gestione dati
    void loadMediaData();
    void saveMediaData();
    bool validateInput();
    QStringList getValidationErrors();
    
    // Utility
    void clearSpecificForm();
    void updateFormVisibility();
    void populateComboBoxes();
    void enableForm(bool enabled);
    
    // Factory methods per creazione media
    std::unique_ptr<Media> createLibro() const;
    std::unique_ptr<Media> createFilm() const;
    std::unique_ptr<Media> createArticolo() const;
    
    // Membri privati
    Media* m_mediaOriginale;
    bool m_readOnly;
    bool m_isEditing;
    QString m_tipoCorrente;
    
    // Layout principali
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    QScrollArea* m_scrollArea;
    QWidget* m_formWidget;
    QFormLayout* m_formLayout;
    
    // Form base (comune a tutti i media)
    QGroupBox* m_baseGroup;
    QComboBox* m_tipoCombo;
    QLineEdit* m_titoloEdit;
    QSpinBox* m_annoSpin;
    QTextEdit* m_descrizioneEdit;
    
    // Form libro
    QGroupBox* m_libroGroup;
    QLineEdit* m_autoreEdit;
    QLineEdit* m_editoreEdit;
    QSpinBox* m_pagineSpin;
    QLineEdit* m_isbnEdit;
    QComboBox* m_genereLibroCombo;
    
    // Form film
    QGroupBox* m_filmGroup;
    QLineEdit* m_registaEdit;
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
    QLineEdit* m_rivistaEdit;
    QLineEdit* m_volumeEdit;
    QLineEdit* m_numeroEdit;
    QLineEdit* m_pagineEdit;
    QComboBox* m_categoriaCombo;
    QComboBox* m_tipoRivistaCombo;
    QDateEdit* m_dataPubblicazioneEdit;
    QLineEdit* m_doiEdit;
    
    // Bottoni
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    QPushButton* m_helpButton;
    
    // Validation
    QLabel* m_validationLabel;
    bool m_validationEnabled;
    
    // Costanti
    static const int DIALOG_WIDTH = 600;
    static const int DIALOG_HEIGHT = 500;
    static const int MAX_DESCRIZIONE_LENGTH = 500;
};

#endif // MEDIADIALOG_H