#include "edidEditordialog.h"
#include "blobstore.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QLineEdit>
#include <QRadioButton>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QRegularExpressionValidator>

static const QStringList kPresets = {
    "Set Manually","1920x1200","1920x1080","1680x1050","1600x1200",
    "1600x900","1440x1050","1440x900","1366x768","1280x1024",
    "1280x960","1280x800","1280x768","1280x720","1024x768",
    "1024x600","800x600"
};

static QLineEdit *numEdit(int maxLen, QWidget *parent = nullptr) {
    auto *e = new QLineEdit(parent);
    e->setMaxLength(maxLen);
    e->setFixedWidth(70);
    e->setAlignment(Qt::AlignRight);
    return e;
}

EdidEditorDialog::EdidEditorDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("EDID Configuration (Resolution)");
    setModal(true);

    // preset combo
    m_cmbPreset = new QComboBox;
    m_cmbPreset->addItems(kPresets);

    // --- Timing block 1 ---
    auto *gbT1 = new QGroupBox("Primary Timing Parameters");
    m_edPixClk  = numEdit(6); m_edPixClk->setEnabled(false);
    m_edHactive = numEdit(4);
    m_edHblank  = numEdit(4);
    m_edHfrontP = numEdit(3);
    m_edHsyncW  = numEdit(3);
    m_edVactive = numEdit(4);
    m_edVblank  = numEdit(4);
    m_edVfrontP = numEdit(3);
    m_edVsyncW  = numEdit(3);
    m_edRefresh = numEdit(5);

    auto *fl1 = new QFormLayout;
    fl1->addRow("Pixel Clock (MHz):", m_edPixClk);
    fl1->addRow("Refresh Rate (Hz):", m_edRefresh);
    fl1->addRow("Hactive:", m_edHactive);
    fl1->addRow("Hblank:",  m_edHblank);
    fl1->addRow("HFrontPorch:", m_edHfrontP);
    fl1->addRow("HSyncWidth:", m_edHsyncW);
    fl1->addRow("Vactive:", m_edVactive);
    fl1->addRow("Vblank:",  m_edVblank);
    fl1->addRow("VFrontPorch (0-63):", m_edVfrontP);
    fl1->addRow("VSyncWidth (0-63):", m_edVsyncW);
    gbT1->setLayout(fl1);

    // --- Timing block 2 ---
    m_gbBlock2  = new QGroupBox("Secondary Timing Parameters");
    m_edPixClk2  = numEdit(6); m_edPixClk2->setEnabled(false);
    m_edHactive2 = numEdit(4); m_edHactive2->setEnabled(false);
    m_edVactive2 = numEdit(4); m_edVactive2->setEnabled(false);
    m_edRefresh2 = numEdit(5);
    auto *fl2 = new QFormLayout;
    fl2->addRow("Pixel Clock (MHz):", m_edPixClk2);
    fl2->addRow("Refresh Rate (Hz):", m_edRefresh2);
    fl2->addRow("Hactive:", m_edHactive2);
    fl2->addRow("Vactive:", m_edVactive2);
    m_gbBlock2->setLayout(fl2);


    // --- Physical size ---
    auto *gbSize = new QGroupBox("Physical Dimensions");
    m_edImgH    = numEdit(3);
    m_edImgV    = numEdit(3);
    m_edScreenH = numEdit(2);
    m_edScreenV = numEdit(2);
    auto *flSz = new QFormLayout;
    flSz->addRow("Image width (mm):",  m_edImgH);
    flSz->addRow("Image height (mm):", m_edImgV);
    flSz->addRow("Screen width (cm):",  m_edScreenH);
    flSz->addRow("Screen height (cm):", m_edScreenV);
    gbSize->setLayout(flSz);

    // --- Advanced ---
    auto *gbAdv = new QGroupBox("Advanced Parameters");
    m_rb6bit = new QRadioButton("6-bit");
    m_rb8bit = new QRadioButton("8-bit");
    m_rb8bit->setChecked(true);
    m_edGamma = numEdit(4);
    m_edGamma->setValidator(new QDoubleValidator(1.0, 3.54, 2, m_edGamma));

    m_cmbSyncType = new QComboBox;
    m_cmbSyncType->addItems({"Digital Composite Sync","Digital Separate Sync"});
    m_cmbSyncSerr = new QComboBox;
    m_cmbSyncSerr->addItems({"Without Serrations","With Serrations (H-sync during V-sync)"});
    m_cmbVpolarity = new QComboBox;
    m_cmbVpolarity->addItems({"Negative","Positive"});
    m_cmbHpolarity = new QComboBox;
    m_cmbHpolarity->addItems({"Negative","Positive"});
    m_lblVpol = new QLabel("Vsync polarity:");
    m_lblHpol = new QLabel("Hsync polarity:");

    auto *flAdv = new QFormLayout;
    auto *hBit = new QHBoxLayout;
    hBit->addWidget(m_rb6bit); hBit->addWidget(m_rb8bit);
    flAdv->addRow("Colour depth:", hBit);
    flAdv->addRow("Gamma (1.00–3.54):", m_edGamma);
    flAdv->addRow("Sync type:", m_cmbSyncType);
    flAdv->addRow("Serrations:", m_cmbSyncSerr);
    flAdv->addRow(m_lblVpol, m_cmbVpolarity);
    flAdv->addRow(m_lblHpol, m_cmbHpolarity);
    gbAdv->setLayout(flAdv);

    // --- Manufacturer info ---
    auto *gbMfr = new QGroupBox("Manufacturer Information");
    m_edManufName   = numEdit(3);
    m_edProdCode    = numEdit(4);
    m_edSerial      = numEdit(8);
    m_edWeek        = numEdit(2);
    m_edYear        = numEdit(4);
    m_edMonitorName = numEdit(13); m_edMonitorName->setFixedWidth(130);
    auto *flMfr = new QFormLayout;
    flMfr->addRow("Manufacturer (3 chars):", m_edManufName);
    flMfr->addRow("Product code (hex 4):", m_edProdCode);
    flMfr->addRow("Serial number (hex 8):", m_edSerial);
    flMfr->addRow("Week of manufacture:", m_edWeek);
    flMfr->addRow("Year of manufacture:", m_edYear);
    flMfr->addRow("Monitor name (13):", m_edMonitorName);
    gbMfr->setLayout(flMfr);

    // --- Buttons ---
    auto *btnSave    = new QPushButton("Save and close");
    auto *btnCancel  = new QPushButton("Discard and close");
    auto *hBtn = new QHBoxLayout;
    hBtn->addStretch();
    hBtn->addWidget(btnSave);
    hBtn->addWidget(btnCancel);

    // --- Layout ---
    auto *presetRow = new QHBoxLayout;
    presetRow->addWidget(new QLabel("Resolution preset:"));
    presetRow->addWidget(m_cmbPreset);
    presetRow->addStretch();

    auto *leftCol = new QVBoxLayout;
    leftCol->addWidget(gbT1);
    leftCol->addWidget(m_gbBlock2);

    auto *rightCol = new QVBoxLayout;
    rightCol->addWidget(gbSize);
    rightCol->addWidget(gbAdv);
    rightCol->addWidget(gbMfr);

    auto *cols = new QHBoxLayout;
    cols->addLayout(leftCol);
    cols->addLayout(rightCol);

    auto *root = new QVBoxLayout(this);
    root->addLayout(presetRow);
    root->addLayout(cols);
    root->addLayout(hBtn);

    connect(btnSave,    &QPushButton::clicked, this, &EdidEditorDialog::onSave);
    connect(btnCancel,  &QPushButton::clicked, this, &EdidEditorDialog::onCancel);
    connect(m_cmbPreset, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EdidEditorDialog::onResolutionPreset);
    connect(m_cmbSyncType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EdidEditorDialog::onSyncTypeChanged);
    connect(m_edRefresh,  &QLineEdit::returnPressed, this, &EdidEditorDialog::recalcPixelClock);
    connect(m_edRefresh2, &QLineEdit::returnPressed, this, &EdidEditorDialog::recalcPixelClock2);

    loadFromEdid();
}

void EdidEditorDialog::initDefaultEdid(EdidBlock &b) {
    b.fill(0);
    b[0]=0; b[1]=0xFF; b[2]=0xFF; b[3]=0xFF; b[4]=0xFF; b[5]=0xFF; b[6]=0xFF; b[7]=0;
    b[8]=0x4A; b[9]=0x87; b[10]=0x11; b[11]=0x52;
    b[16]=16; b[17]=29; b[18]=1; b[19]=4; b[20]=0xA5;
    b[21]=53; b[22]=30; b[23]=120;
    // standard timing blocks filled with 0x01
    for (int i=38;i<=53;++i) b[i]=1;
    // default timing descriptor 1: 1920x1080@60
    b[54]=0x1A; b[55]=0x36; b[56]=0x80; b[57]=0xA0; b[58]=0x70;
    b[59]=0x38; b[60]=0x1F; b[61]=0x40; b[62]=0x30; b[63]=0x20;
    b[64]=0x35; b[65]=0x00; b[66]=0x07; b[67]=0x44; b[68]=0x21;
    b[69]=0; b[70]=0; b[71]=0x1A;
    // monitor name descriptor at offset 108
    b[111]=0xFE; b[112]=0;
    for (int i=113;i<=125;++i) b[i]=' ';
    b[127] = calcChecksum(b);
}

uint8_t EdidEditorDialog::calcChecksum(const EdidBlock &b) {
    int sum = 0;
    for (int i = 0; i < RomLayout::kEdidBlockSize - 1; ++i) sum += b[i];
    return static_cast<uint8_t>((256 - (sum % 256)) % 256);
}

void EdidEditorDialog::loadFromEdid() {
    auto &blob = BlobStore::data();
    int panelIdx = BlobStore::currentPanel();
    int base = panelIdx * RomLayout::kEdidBlockSize;
    for (int i = 0; i < RomLayout::kEdidBlockSize; ++i)
        m_edid[i] = blob[base + i];

    if (m_edid[0] != 0x00 || m_edid[1] != 0xFF) {
        initDefaultEdid(m_edid);
    }

    // Manufacturer name (3 chars packed in bytes 8-9)
    char c0 = ((m_edid[8] >> 2) & 0x1F) | 0x40;
    char c1 = (((m_edid[8] << 3) | (m_edid[9] >> 5)) & 0x1F) | 0x40;
    char c2 = (m_edid[9] & 0x1F) | 0x40;
    m_edManufName->setText(QString("%1%2%3").arg(c0).arg(c1).arg(c2));

    m_edProdCode->setText(QString("%1%2")
        .arg(m_edid[11], 2, 16, QChar('0'))
        .arg(m_edid[10], 2, 16, QChar('0')).toUpper());
    m_edSerial->setText(QString("%1%2%3%4")
        .arg(m_edid[15],2,16,QChar('0'))
        .arg(m_edid[14],2,16,QChar('0'))
        .arg(m_edid[13],2,16,QChar('0'))
        .arg(m_edid[12],2,16,QChar('0')).toUpper());

    m_edWeek->setText(QString::number(m_edid[16] < 55 ? m_edid[16] : 0));
    m_edYear->setText(QString::number(m_edid[17] > 15 ? 1990 + m_edid[17] : 1990));

    m_rb6bit->setChecked((m_edid[20] & 0x70) == 0x10);
    m_rb8bit->setChecked((m_edid[20] & 0x70) != 0x10);

    m_edScreenH->setText(QString::number(m_edid[21]));
    m_edScreenV->setText(QString::number(m_edid[22]));

    float gamma = (m_edid[23] + 100) / 100.0f;
    m_edGamma->setText(QString::number(gamma, 'f', 2));

    // Timing descriptor 1 (bytes 54-71)
    float pclk = ((m_edid[55] << 8) | m_edid[54]) / 100.0f;
    m_edPixClk->setText(QString::number(pclk, 'f', 2));

    int ha = ((m_edid[58] >> 4) << 8) | m_edid[56];
    int hb = ((m_edid[58] & 0xF) << 8) | m_edid[57];
    int va = ((m_edid[61] >> 4) << 8) | m_edid[59];
    int vb = ((m_edid[61] & 0xF) << 8) | m_edid[60];
    m_edHactive->setText(QString::number(ha));
    m_edHblank->setText(QString::number(hb));
    m_edVactive->setText(QString::number(va));
    m_edVblank->setText(QString::number(vb));

    int hfp = (((m_edid[65] & 0xC0) >> 6) << 8) | m_edid[62];
    int hsw = (((m_edid[65] & 0x30) >> 4) << 8) | m_edid[63];
    int vfp = (((m_edid[65] & 0x0C) >> 2) << 4) | (m_edid[64] >> 4);
    int vsw = ((m_edid[65] & 0x03) << 4) | (m_edid[64] & 0x0F);
    m_edHfrontP->setText(QString::number(hfp));
    m_edHsyncW->setText(QString::number(hsw));
    m_edVfrontP->setText(QString::number(vfp));
    m_edVsyncW->setText(QString::number(vsw));

    int imgH = ((m_edid[68] >> 4) << 8) | m_edid[66];
    int imgV = ((m_edid[68] & 0xF) << 8) | m_edid[67];
    m_edImgH->setText(QString::number(imgH));
    m_edImgV->setText(QString::number(imgV));

    // Sync flags (byte 71)
    bool sepSync = (m_edid[71] & 0x08) != 0;
    m_cmbSyncType->setCurrentIndex(sepSync ? 1 : 0);
    if (sepSync) {
        m_cmbVpolarity->setCurrentIndex((m_edid[71] & 0x04) ? 1 : 0);
        m_cmbHpolarity->setCurrentIndex((m_edid[71] & 0x02) ? 1 : 0);
    } else {
        m_cmbSyncSerr->setCurrentIndex((m_edid[71] & 0x04) ? 1 : 0);
    }
    onSyncTypeChanged(m_cmbSyncType->currentIndex());

    // Timing descriptor 2 (bytes 72-89)
    if (m_edid[72] != 0 || m_edid[73] != 0) {
        float pclk2 = ((m_edid[73] << 8) | m_edid[72]) / 100.0f;
        m_edPixClk2->setText(QString::number(pclk2, 'f', 2));
        m_edHactive2->setText(QString::number(((m_edid[76] >> 4) << 8) | m_edid[74]));
        m_edVactive2->setText(QString::number(((m_edid[79] >> 4) << 8) | m_edid[77]));
        recalcRefreshRate2();
    }

    // Monitor name (bytes 113-125)
    QString name;
    for (int i = 113; i <= 125; ++i) name += QChar(m_edid[i]);
    m_edMonitorName->setText(name.trimmed());

    recalcRefreshRate();
    m_loaded = true;
}

void EdidEditorDialog::recalcRefreshRate() {
    bool ok1, ok2, ok3, ok4;
    float pclk = m_edPixClk->text().toFloat(&ok1) * 1e6f;
    int htotal = m_edHactive->text().toInt(&ok2) + m_edHblank->text().toInt(&ok3);
    int vtotal = m_edVactive->text().toInt(&ok4) + m_edVblank->text().toInt();
    if (ok1 && ok2 && ok3 && ok4 && htotal > 0 && vtotal > 0)
        m_edRefresh->setText(QString::number(pclk / (htotal * vtotal), 'f', 2));
}

void EdidEditorDialog::recalcRefreshRate2() {
    bool ok;
    float pclk2 = m_edPixClk2->text().toFloat(&ok) * 1e6f;
    int htotal = m_edHactive->text().toInt() + m_edHblank->text().toInt();
    int vtotal = m_edVactive->text().toInt() + m_edVblank->text().toInt();
    if (ok && htotal > 0 && vtotal > 0)
        m_edRefresh2->setText(QString::number(pclk2 / (htotal * vtotal), 'f', 2));
}

void EdidEditorDialog::recalcPixelClock() {
    int htotal = m_edHactive->text().toInt() + m_edHblank->text().toInt();
    int vtotal = m_edVactive->text().toInt() + m_edVblank->text().toInt();
    float hz = m_edRefresh->text().toFloat();
    if (htotal > 0 && vtotal > 0 && hz > 0)
        m_edPixClk->setText(QString::number(htotal * vtotal * hz / 1e6f, 'f', 2));
}

void EdidEditorDialog::recalcPixelClock2() {
    int htotal = m_edHactive->text().toInt() + m_edHblank->text().toInt();
    int vtotal = m_edVactive->text().toInt() + m_edVblank->text().toInt();
    float hz = m_edRefresh2->text().toFloat();
    if (htotal > 0 && vtotal > 0 && hz > 0)
        m_edPixClk2->setText(QString::number(htotal * vtotal * hz / 1e6f, 'f', 2));
}

void EdidEditorDialog::applyPreset(int hActive, int vActive) {
    m_edHactive->setText(QString::number(hActive));
    m_edHactive2->setText(QString::number(hActive));
    m_edHblank->setText("160");
    m_edHfrontP->setText("80");
    m_edHsyncW->setText("32");

    // vblank: original uses integer division before adding 1, matching C# int arithmetic
    int vblank = (276 * vActive / 9724) + 1;
    m_edVactive->setText(QString::number(vActive));
    m_edVactive2->setText(QString::number(vActive));
    m_edVblank->setText(QString::number(vblank));
    m_edVfrontP->setText("3");

    // Vsync width depends on aspect ratio
    int vsw = 10;
    if (hActive == vActive * 4 / 3)       vsw = 4;
    else if (hActive == vActive * 16 / 9)  vsw = 5;
    else if (hActive == vActive * 16 / 10) vsw = 6;
    else if (hActive == vActive * 5 / 4)   vsw = 7;
    m_edVsyncW->setText(QString::number(vsw));

    m_edRefresh->setText("60");
    recalcPixelClock();
    m_edRefresh2->setText("50");
    recalcPixelClock2();
    m_rb8bit->setChecked(true);
}

void EdidEditorDialog::onResolutionPreset(int index) {
    if (!m_loaded) {
        initDefaultEdid(m_edid);
        loadFromEdid();
    }
    if (index == 0) return;
    QString text = m_cmbPreset->currentText();
    m_edMonitorName->setText(text);
    QStringList parts = text.split('x');
    if (parts.size() == 2)
        applyPreset(parts[0].toInt(), parts[1].toInt());
}

void EdidEditorDialog::onSyncTypeChanged(int index) {
    bool sep = (index == 1);
    m_cmbSyncSerr->setVisible(!sep);
    m_cmbVpolarity->setVisible(sep);
    m_cmbHpolarity->setVisible(sep);
    m_lblVpol->setVisible(sep);
    m_lblHpol->setVisible(sep);
}

void EdidEditorDialog::saveToEdid() {
    m_edid[0]=0; m_edid[1]=0xFF; m_edid[2]=0xFF; m_edid[3]=0xFF;
    m_edid[4]=0xFF; m_edid[5]=0xFF; m_edid[6]=0xFF; m_edid[7]=0;

    // Manufacturer name
    QString mfr = m_edManufName->text().toUpper().leftJustified(3,'_');
    int n0 = mfr[0].toLatin1() & 0x1F;
    int n1 = mfr[1].toLatin1() & 0x1F;
    int n2 = mfr[2].toLatin1() & 0x1F;
    m_edid[8] = static_cast<uint8_t>((n0 << 2) | (n1 >> 3));
    m_edid[9] = static_cast<uint8_t>((n1 << 5) | n2);

    // Product code
    QString pc = m_edProdCode->text().rightJustified(4,'0');
    m_edid[10] = static_cast<uint8_t>(pc.mid(2,2).toInt(nullptr,16));
    m_edid[11] = static_cast<uint8_t>(pc.mid(0,2).toInt(nullptr,16));

    // Serial (fixed to 0x0401 as original)
    m_edid[12] = 1; m_edid[13] = 4;

    // Week / year
    m_edid[16] = static_cast<uint8_t>(m_edWeek->text().toInt());
    m_edid[17] = static_cast<uint8_t>(m_edYear->text().toInt() - 1990);

    m_edid[20] = 0x85;
    if (m_rb8bit->isChecked()) m_edid[20] = (m_edid[20] & 0x8F) | 0x20;
    else                        m_edid[20] = (m_edid[20] & 0x8F) | 0x10;

    m_edid[21] = static_cast<uint8_t>(m_edScreenH->text().toInt());
    m_edid[22] = static_cast<uint8_t>(m_edScreenV->text().toInt());

    // Standard timing blocks
    for (int i=24;i<=53;++i) {
        static const uint8_t kStd[] = {
            2,128,66,172,81,48,180,37,16,80,83,0,0,0,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1
        };
        m_edid[i] = kStd[i-24];
    }

    // Helper lambda to write a timing descriptor
    auto writeTiming = [&](int base, float pclk, int ha, int hb,
                           int va, int vb, int hfp, int hsw,
                           int vfp, int vsw, int imgH, int imgV, uint8_t flags) {
        int pc100 = static_cast<int>(pclk * 100.0f);
        m_edid[base+1] = static_cast<uint8_t>(pc100 >> 8);
        m_edid[base+0] = static_cast<uint8_t>(pc100 & 0xFF);
        m_edid[base+2] = static_cast<uint8_t>(ha & 0xFF);
        m_edid[base+3] = static_cast<uint8_t>(hb & 0xFF);
        m_edid[base+4] = static_cast<uint8_t>(((ha & 0xF00) >> 4) | ((hb & 0xF00) >> 8));
        m_edid[base+5] = static_cast<uint8_t>(va & 0xFF);
        m_edid[base+6] = static_cast<uint8_t>(vb & 0xFF);
        m_edid[base+7] = static_cast<uint8_t>(((va & 0xF00) >> 4) | ((vb & 0xF00) >> 8));
        m_edid[base+8]  = static_cast<uint8_t>(hfp & 0xFF);
        int ov = 0;
        ov |= (hfp >> 8) << 6;
        m_edid[base+9]  = static_cast<uint8_t>(hsw & 0xFF);
        ov |= (hsw >> 8) << 4;
        ov |= (vfp >> 4) << 2;
        m_edid[base+10] = static_cast<uint8_t>(((vfp & 0xF) << 4) | (vsw & 0xF));
        ov |= (vsw >> 4);
        m_edid[base+11] = static_cast<uint8_t>(ov);
        m_edid[base+12] = static_cast<uint8_t>(imgH & 0xFF);
        m_edid[base+13] = static_cast<uint8_t>(imgV & 0xFF);
        m_edid[base+14] = static_cast<uint8_t>(((imgH & 0xF00) >> 4) | ((imgV & 0xF00) >> 8));
        m_edid[base+15] = 0; m_edid[base+16] = 0;
        m_edid[base+17] = flags;
    };

    float pclk  = m_edPixClk->text().toFloat();
    float pclk2 = m_edPixClk2->text().toFloat();
    int ha = m_edHactive->text().toInt(), hb = m_edHblank->text().toInt();
    int va = m_edVactive->text().toInt(), vb = m_edVblank->text().toInt();
    int hfp = m_edHfrontP->text().toInt(), hsw = m_edHsyncW->text().toInt();
    int vfp = m_edVfrontP->text().toInt(), vsw = m_edVsyncW->text().toInt();
    int imgH = m_edImgH->text().toInt(), imgV = m_edImgV->text().toInt();

    uint8_t syncFlags = m_edid[71] & 0xF1;
    if (m_cmbSyncType->currentIndex() == 1) {
        syncFlags |= 0x08;
        if (m_cmbVpolarity->currentIndex() == 1) syncFlags |= 0x04;
        if (m_cmbHpolarity->currentIndex() == 1) syncFlags |= 0x02;
    } else {
        if (m_cmbSyncSerr->currentIndex() == 1) syncFlags |= 0x04;
    }

    writeTiming(54, pclk,  ha, hb, va, vb, hfp, hsw, vfp, vsw, imgH, imgV, syncFlags);
    writeTiming(72, pclk2, ha, hb, va, vb, hfp, hsw, vfp, vsw, imgH, imgV, syncFlags);

    // Descriptor 3: blank
    for (int i=90;i<=107;++i) m_edid[i]=0;
    m_edid[93]=0xFE; m_edid[94]=0; m_edid[95]=0x0A;
    for (int i=96;i<=107;++i) m_edid[i]=0x20;

    // Descriptor 4: monitor name (tag 0xFE, unspecified ASCII text, matching original)
    m_edid[108]=0; m_edid[109]=0; m_edid[110]=0; m_edid[111]=0xFE; m_edid[112]=0;
    QString name = m_edMonitorName->text().leftJustified(13,' ');
    for (int i=0;i<13;++i) m_edid[113+i] = static_cast<uint8_t>(name[i].toLatin1());

    m_edid[126] = 0;
    m_edid[127] = calcChecksum(m_edid);

    // Write back to blob
    auto &blob = BlobStore::data();
    int panelIdx = BlobStore::currentPanel();
    int base = panelIdx * RomLayout::kEdidBlockSize;
    for (int i = 0; i < RomLayout::kEdidBlockSize; ++i)
        blob[base + i] = m_edid[i];
}

void EdidEditorDialog::onSave() {
    float hz = m_edRefresh->text().toFloat();
    if (hz < 59.0f || hz > 61.0f) {
        if (QMessageBox::question(this, "Confirm",
                QString("Refresh rate is %1 Hz. Save anyway?").arg(hz),
                QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
            return;
    }
    recalcPixelClock();
    if (m_edRefresh2->text().trimmed().isEmpty()) m_edRefresh2->setText("50");
    recalcPixelClock2();
    saveToEdid();
    accept();
}

void EdidEditorDialog::onCancel() {
    if (QMessageBox::question(this, "Confirm", "Discard changes and close?",
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        reject();
}
