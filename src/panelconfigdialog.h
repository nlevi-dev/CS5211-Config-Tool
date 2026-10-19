#pragma once
#include <QDialog>
#include <cstdint>

QT_BEGIN_NAMESPACE
class QRadioButton;
class QCheckBox;
class QComboBox;
class QTableWidget;
class QGroupBox;
QT_END_NAMESPACE

// Dialog for editing the per-panel LVDS output configuration stored in the
// OCM config table entry for the currently selected panel slot.
// Covers: port count, colour depth, data format, channel mapping, P/N swap,
// power-down per channel, signal polarity inversions, and output swing.
class PanelConfigDialog : public QDialog {
    Q_OBJECT
public:
    explicit PanelConfigDialog(QWidget *parent = nullptr);

private slots:
    void onSave();
    void onCancel();
    void onMap0Changed(int index);
    void onMap1Changed(int index);

private:
    void loadFromBlob();
    void saveToBlob();

    // Basic
    QRadioButton *m_rbSinglePort;
    QRadioButton *m_rbDualPort;
    QRadioButton *m_rb6bit;
    QRadioButton *m_rb8bit;
    QRadioButton *m_rbSpwg;
    QRadioButton *m_rbOpenLdi;

    // Port mapping
    QComboBox    *m_cmbMap0;
    QComboBox    *m_cmbMap1;
    QTableWidget *m_tblPort0;
    QTableWidget *m_tblPort1;

    // LVDS swing
    QComboBox    *m_cmbCkSwing;
    QComboBox    *m_cmbDatSwing;
    QComboBox    *m_cmbVcom;

    // Advanced (hidden by default)
    QGroupBox    *m_gbAdvanced;
    QCheckBox    *m_cbInvVsync;
    QCheckBox    *m_cbInvHsync;
    QCheckBox    *m_cbInvData;
    QCheckBox    *m_cbInvClk;
    QCheckBox    *m_cbSwapPixel;
    QCheckBox    *m_cbDeOnly;

    // Register shadow
    uint8_t m_r09F, m_r1DC, m_r1DE, m_r1E1, m_r1E2;
    uint8_t m_r1E3, m_r1E4, m_r1E7, m_r1ED, m_r1F2, m_r1F3, m_r1F4;

    static const uint8_t kLclkSet[8];
    static const uint8_t kRclkSet[8];
};
