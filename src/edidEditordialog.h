#pragma once
#include <QDialog>
#include <array>
#include <cstdint>
#include "romdata.h"

QT_BEGIN_NAMESPACE
class QComboBox;
class QLineEdit;
class QRadioButton;
class QLabel;
class QGroupBox;
QT_END_NAMESPACE

// Dialog for editing the 128-byte EDID block for one panel slot.
// Covers timing parameters (two descriptor blocks), manufacturer info,
// screen dimensions, gamma, sync polarity, and colour depth.
class EdidEditorDialog : public QDialog {
    Q_OBJECT
public:
    explicit EdidEditorDialog(QWidget *parent = nullptr);

private slots:
    void onSave();
    void onCancel();
    void onResolutionPreset(int index);
    void onSyncTypeChanged(int index);
    void recalcPixelClock();
    void recalcPixelClock2();
    void recalcRefreshRate();
    void recalcRefreshRate2();

private:
    using EdidBlock = std::array<uint8_t, RomLayout::kEdidBlockSize>;

    void loadFromEdid();
    void saveToEdid();
    void applyPreset(int hActive, int vActive);
    uint8_t calcChecksum(const EdidBlock &b);
    void initDefaultEdid(EdidBlock &b);

    EdidBlock m_edid;
    bool m_loaded = false;

    QComboBox  *m_cmbPreset;

    // Timing block 1
    QLineEdit  *m_edPixClk;
    QLineEdit  *m_edHactive;
    QLineEdit  *m_edHblank;
    QLineEdit  *m_edHfrontP;
    QLineEdit  *m_edHsyncW;
    QLineEdit  *m_edVactive;
    QLineEdit  *m_edVblank;
    QLineEdit  *m_edVfrontP;
    QLineEdit  *m_edVsyncW;
    QLineEdit  *m_edRefresh;

    // Timing block 2 (read-only display)
    QLineEdit  *m_edPixClk2;
    QLineEdit  *m_edHactive2;
    QLineEdit  *m_edVactive2;
    QLineEdit  *m_edRefresh2;
    QGroupBox  *m_gbBlock2;

    // Physical dimensions
    QLineEdit  *m_edImgH;
    QLineEdit  *m_edImgV;
    QLineEdit  *m_edScreenH;
    QLineEdit  *m_edScreenV;

    // Advanced
    QComboBox  *m_cmbSyncType;
    QComboBox  *m_cmbSyncSerr;
    QComboBox  *m_cmbVpolarity;
    QComboBox  *m_cmbHpolarity;
    QLabel     *m_lblVpol;
    QLabel     *m_lblHpol;
    QRadioButton *m_rb6bit;
    QRadioButton *m_rb8bit;
    QLineEdit  *m_edGamma;

    // Manufacturer info
    QLineEdit  *m_edManufName;
    QLineEdit  *m_edProdCode;
    QLineEdit  *m_edSerial;
    QLineEdit  *m_edWeek;
    QLineEdit  *m_edYear;
    QLineEdit  *m_edMonitorName;
};
