#pragma once
#include <QDialog>

QT_BEGIN_NAMESPACE
class QRadioButton;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QLabel;
QT_END_NAMESPACE

// Dialog for editing chip-global settings that apply to all 16 panel slots:
// eDP link speed, lane count, PWM mode, power sequencing timings, SSC, and
// LVDS slew rate.
class ChipSettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit ChipSettingsDialog(QWidget *parent = nullptr);

private slots:
    void onSave();
    void onCancel();
    void onSscModeChanged(int index);
    void onClkSourceChanged();
    void onSscDivChanged(const QString &text);
    void onSscStepChanged(int index);
    void onPwmMode1Toggled(bool checked);
    void onKeyAdjustToggled(bool checked);

private:
    void loadFromBlob();
    void saveToBlob();
    void recalcSscFreq();

    // eDP
    QRadioButton *m_rb27G;
    QRadioButton *m_rb162G;
    QRadioButton *m_rb2Lane;
    QRadioButton *m_rb1Lane;

    // Clock source
    QRadioButton *m_rbCrystal;
    QRadioButton *m_rbInternal;

    // PWM
    QRadioButton *m_rbPwmMode1;
    QRadioButton *m_rbPwmMode2;
    QCheckBox    *m_cbInvertPwm;
    QCheckBox    *m_cbPowerDownKey;
    QCheckBox    *m_cbKeyAdjust;
    QComboBox    *m_cmbDefaultDuty;
    QComboBox    *m_cmbMinDuty;
    QComboBox    *m_cmbMaxDuty;

    // Power sequencing
    QLineEdit    *m_edT2;
    QLineEdit    *m_edT3;
    QLineEdit    *m_edT4;
    QLineEdit    *m_edT5;
    QLineEdit    *m_edT7;

    // SSC
    QComboBox    *m_cmbSscMode;
    QLineEdit    *m_edSscDiv;
    QComboBox    *m_cmbSscStep;
    QLabel       *m_lblSscFreq;

    // LVDS slew
    QComboBox    *m_cmbSlewCtrl;
};
