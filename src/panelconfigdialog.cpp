#include "panelconfigdialog.h"
#include "blobstore.h"
#include "romdata.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QComboBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>

const uint8_t PanelConfigDialog::kLclkSet[8] = {3,1,4,0,4,0,3,1};
const uint8_t PanelConfigDialog::kRclkSet[8] = {3,0,4,1,4,1,3,0};

static const QStringList kMap0Items = {
    "0: Data0,Data1,Data2,Clock0,Data3",
    "1: Data3,Clock0,Data2,Data1,Data0",
    "2: Data0,Data1,Data2,Data3,Clock0",
    "3: Clock0,Data3,Data2,Data1,Data0",
    "4: Data4,Data5,Data6,Data7,Clock1",
    "5: Clock1,Data7,Data6,Data5,Data4",
    "6: Data4,Data5,Data6,Clock1,Data7",
    "7: Data7,Clock1,Data6,Data5,Data4"
};
static const QStringList kMap1Items = {
    "0: Data4,Data5,Data6,Clock1,Data7",
    "1: Clock1,Data7,Data6,Data5,Data4",
    "2: Data4,Data5,Data6,Data7,Clock1",
    "3: Data7,Clock1,Data6,Data5,Data4",
    "4: Data0,Data1,Data2,Data3,Clock0",
    "5: Data3,Clock0,Data2,Data1,Data0",
    "6: Data0,Data1,Data2,Clock0,Data3",
    "7: Clock0,Data3,Data2,Data1,Data0"
};
static const QStringList kSwingItems = {
    "0: 0.465V","1: 0.465V","2: 0.465V","3: 0.387V",
    "4: 0.308V","5: 0.230V","6: 0.153V","7: 0.077V"
};
static const QStringList kVcomItems = {
    "0: 1.452V","1: 1.452V","2: 1.397V","3: 1.340V",
    "4: 1.285V","5: 1.229V","6: 1.173V","7: 1.117V"
};

static QTableWidget *makePortTable() {
    auto *t = new QTableWidget(5, 4);
    t->setHorizontalHeaderLabels({"Channel","Signal","Power Down","P/N Swap"});
    t->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    t->verticalHeader()->setDefaultSectionSize(22);
    t->setFixedHeight(140);
    return t;
}

static void setPortRows(QTableWidget *t,
    const QStringList &chNames, const QStringList &sigNames) {
    for (int r = 0; r < 5; ++r) {
        auto *ch = new QTableWidgetItem(chNames[r]);
        ch->setFlags(ch->flags() & ~Qt::ItemIsEditable);
        t->setItem(r, 0, ch);
        auto *sig = new QTableWidgetItem(sigNames[r]);
        sig->setFlags(sig->flags() & ~Qt::ItemIsEditable);
        t->setItem(r, 1, sig);
        for (int c = 2; c <= 3; ++c) {
            auto *cb = new QTableWidgetItem;
            cb->setCheckState(Qt::Unchecked);
            t->setItem(r, c, cb);
        }
    }
}

PanelConfigDialog::PanelConfigDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Panel (LCD) Configuration");
    setModal(true);

    // --- Basic ---
    auto *gbBasic = new QGroupBox("Basic Parameters");
    auto *gbPorts = new QGroupBox("LVDS Port Count");
    m_rbSinglePort = new QRadioButton("Single port"); m_rbSinglePort->setChecked(true);
    m_rbDualPort   = new QRadioButton("Dual port");
    auto *hPorts = new QHBoxLayout;
    hPorts->addWidget(m_rbSinglePort); hPorts->addWidget(m_rbDualPort);
    gbPorts->setLayout(hPorts);

    auto *gbDepth = new QGroupBox("Panel Colour Depth");
    m_rb6bit = new QRadioButton("6-bit (18-bit, 262K colours)");
    m_rb8bit = new QRadioButton("8-bit (24-bit, 16.7M colours)"); m_rb8bit->setChecked(true);
    auto *hDepth = new QHBoxLayout;
    hDepth->addWidget(m_rb6bit); hDepth->addWidget(m_rb8bit);
    gbDepth->setLayout(hDepth);

    auto *gbFmt = new QGroupBox("LVDS Data Format");
    m_rbSpwg    = new QRadioButton("SPWG (VESA) [common]"); m_rbSpwg->setChecked(true);
    m_rbOpenLdi = new QRadioButton("OPENLDI (JEIDA)");
    auto *hFmt = new QHBoxLayout;
    hFmt->addWidget(m_rbSpwg); hFmt->addWidget(m_rbOpenLdi);
    gbFmt->setLayout(hFmt);

    auto *hBasic = new QHBoxLayout;
    hBasic->addWidget(gbPorts); hBasic->addWidget(gbDepth); hBasic->addWidget(gbFmt);
    gbBasic->setLayout(hBasic);

    // --- Port mapping ---
    auto *gbMapping = new QGroupBox("LVDS Output Port Configuration");
    m_cmbMap0 = new QComboBox; m_cmbMap0->addItems(kMap0Items);
    m_cmbMap1 = new QComboBox; m_cmbMap1->addItems(kMap1Items);
    m_tblPort0 = makePortTable();
    m_tblPort1 = makePortTable();
    setPortRows(m_tblPort0,
        {"Channel 0","Channel 1","Channel 2","Clock","Channel 3"},
        {"Data0","Data1","Data2","Clk0","Data3"});
    setPortRows(m_tblPort1,
        {"Channel 0","Channel 1","Channel 2","Clock","Channel 3"},
        {"Data4","Data5","Data6","Clk1","Data7"});

    auto *mapRow = new QHBoxLayout;
    auto *col0 = new QVBoxLayout;
    col0->addWidget(new QLabel("Port 0 (Port A) output signal:"));
    col0->addWidget(m_cmbMap0);
    col0->addWidget(m_tblPort0);
    auto *col1 = new QVBoxLayout;
    col1->addWidget(new QLabel("Port 1 (Port B) output signal:"));
    col1->addWidget(m_cmbMap1);
    col1->addWidget(m_tblPort1);
    mapRow->addLayout(col0);
    mapRow->addLayout(col1);
    gbMapping->setLayout(mapRow);

    // --- Swing ---
    auto *gbSwing = new QGroupBox("LVDS Output Amplitude");
    m_cmbCkSwing  = new QComboBox; m_cmbCkSwing->addItems(kSwingItems);
    m_cmbDatSwing = new QComboBox; m_cmbDatSwing->addItems(kSwingItems);
    m_cmbVcom     = new QComboBox; m_cmbVcom->addItems(kVcomItems);
    auto *hSwing = new QHBoxLayout;
    hSwing->addWidget(new QLabel("Clock swing:")); hSwing->addWidget(m_cmbCkSwing);
    hSwing->addWidget(new QLabel("Data swing:"));  hSwing->addWidget(m_cmbDatSwing);
    hSwing->addWidget(new QLabel("DC voltage:"));  hSwing->addWidget(m_cmbVcom);
    gbSwing->setLayout(hSwing);

    // --- Advanced ---
    m_gbAdvanced = new QGroupBox("Advanced Parameters");
    m_cbInvVsync  = new QCheckBox("Invert VSYNC polarity");
    m_cbInvHsync  = new QCheckBox("Invert HSYNC polarity");
    m_cbInvData   = new QCheckBox("Invert data polarity");
    m_cbInvClk    = new QCheckBox("Invert clock polarity");
    m_cbSwapPixel = new QCheckBox("Swap odd/even pixels");
    m_cbDeOnly    = new QCheckBox("Force VSYNC/HSYNC = 0 (DE-only mode)");
    auto *hAdv = new QHBoxLayout;
    hAdv->addWidget(m_cbInvVsync); hAdv->addWidget(m_cbInvHsync);
    hAdv->addWidget(m_cbInvData);  hAdv->addWidget(m_cbInvClk);
    hAdv->addWidget(m_cbSwapPixel); hAdv->addWidget(m_cbDeOnly);
    m_gbAdvanced->setLayout(hAdv);

    // --- Buttons ---
    auto *btnSave    = new QPushButton("Save and close");
    auto *btnCancel  = new QPushButton("Discard and close");
    auto *hBtn = new QHBoxLayout;
    hBtn->addStretch();
    hBtn->addWidget(btnSave);
    hBtn->addWidget(btnCancel);

    auto *root = new QVBoxLayout(this);
    root->addWidget(gbBasic);
    root->addWidget(gbMapping);
    root->addWidget(gbSwing);
    root->addWidget(m_gbAdvanced);
    root->addLayout(hBtn);

    connect(btnSave,   &QPushButton::clicked, this, &PanelConfigDialog::onSave);
    connect(btnCancel, &QPushButton::clicked, this, &PanelConfigDialog::onCancel);
    connect(m_cmbMap0, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PanelConfigDialog::onMap0Changed);
    connect(m_cmbMap1, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PanelConfigDialog::onMap1Changed);

    m_cmbCkSwing->setCurrentIndex(4);
    m_cmbDatSwing->setCurrentIndex(4);
    m_cmbVcom->setCurrentIndex(4);

    loadFromBlob();
}

static bool bit(uint8_t v, int b) { return (v >> b) & 1; }

void PanelConfigDialog::loadFromBlob() {
    auto &blob = BlobStore::data();
    int panel = BlobStore::currentPanel();
    int offset = RomLayout::kOcmBase + RomLayout::kCfgTableOffset
               + panel * RomLayout::kCfgEntrySize;

    m_r09F = blob[offset+0];
    m_r1DC = blob[offset+1];
    m_r1DE = blob[offset+2];
    m_r1E1 = blob[offset+3];
    m_r1E2 = blob[offset+4];
    m_r1E3 = blob[offset+5];
    m_r1E4 = blob[offset+6];
    m_r1E7 = blob[offset+7];
    m_r1ED = blob[offset+8];
    m_r1F2 = blob[offset+9];
    m_r1F3 = blob[offset+10];
    m_r1F4 = blob[offset+11];

    // Port 0 power-down
    m_tblPort0->item(0,2)->setCheckState(bit(m_r1DC,3)?Qt::Checked:Qt::Unchecked);
    m_tblPort0->item(1,2)->setCheckState(bit(m_r1DC,2)?Qt::Checked:Qt::Unchecked);
    m_tblPort0->item(2,2)->setCheckState(bit(m_r1DC,1)?Qt::Checked:Qt::Unchecked);
    m_tblPort0->item(3,2)->setCheckState(bit(m_r1DC,0)?Qt::Checked:Qt::Unchecked);
    m_tblPort0->item(4,2)->setCheckState(bit(m_r1DE,7)?Qt::Checked:Qt::Unchecked);
    // Port 1 power-down
    m_tblPort1->item(0,2)->setCheckState(bit(m_r1DE,5)?Qt::Checked:Qt::Unchecked);
    m_tblPort1->item(1,2)->setCheckState(bit(m_r1DE,4)?Qt::Checked:Qt::Unchecked);
    m_tblPort1->item(2,2)->setCheckState(bit(m_r1DE,3)?Qt::Checked:Qt::Unchecked);
    m_tblPort1->item(3,2)->setCheckState(bit(m_r1DE,2)?Qt::Checked:Qt::Unchecked);
    m_tblPort1->item(4,2)->setCheckState(bit(m_r1DE,1)?Qt::Checked:Qt::Unchecked);

    m_cbInvVsync->setChecked(bit(m_r1E1,7));
    m_cbInvHsync->setChecked(bit(m_r1E1,6));
    m_cbInvData->setChecked(bit(m_r1E1,5));
    m_cbInvClk->setChecked(bit(m_r1E1,4));
    m_rbOpenLdi->setChecked(bit(m_r1E1,2));
    m_rbSpwg->setChecked(!bit(m_r1E1,2));
    m_rb6bit->setChecked(bit(m_r1E1,1));
    m_rb8bit->setChecked(!bit(m_r1E1,1));

    m_cbDeOnly->setChecked(bit(m_r1E2,7));
    m_cmbMap0->setCurrentIndex((m_r1E2 & 0x70) >> 4);
    m_cmbMap1->setCurrentIndex(m_r1E2 & 0x07);

    // Port 0 P/N swap
    m_tblPort0->item(0,3)->setCheckState(bit(m_r1E3,4)?Qt::Checked:Qt::Unchecked);
    m_tblPort0->item(1,3)->setCheckState(bit(m_r1E3,3)?Qt::Checked:Qt::Unchecked);
    m_tblPort0->item(2,3)->setCheckState(bit(m_r1E3,2)?Qt::Checked:Qt::Unchecked);
    m_tblPort0->item(3,3)->setCheckState(bit(m_r1E3,1)?Qt::Checked:Qt::Unchecked);
    m_tblPort0->item(4,3)->setCheckState(bit(m_r1E3,0)?Qt::Checked:Qt::Unchecked);
    // Port 1 P/N swap
    m_tblPort1->item(0,3)->setCheckState(bit(m_r1E4,4)?Qt::Checked:Qt::Unchecked);
    m_tblPort1->item(1,3)->setCheckState(bit(m_r1E4,3)?Qt::Checked:Qt::Unchecked);
    m_tblPort1->item(2,3)->setCheckState(bit(m_r1E4,2)?Qt::Checked:Qt::Unchecked);
    m_tblPort1->item(3,3)->setCheckState(bit(m_r1E4,1)?Qt::Checked:Qt::Unchecked);
    m_tblPort1->item(4,3)->setCheckState(bit(m_r1E4,0)?Qt::Checked:Qt::Unchecked);

    m_cbSwapPixel->setChecked(bit(m_r1E7,1));
    m_cmbCkSwing->setCurrentIndex((m_r1F3 & 0x70) >> 4);
    m_cmbDatSwing->setCurrentIndex(m_r1F3 & 0x07);
    m_cmbVcom->setCurrentIndex(m_r1F4 & 0x07);
    m_rbDualPort->setChecked(bit(m_r09F,0));
    m_rbSinglePort->setChecked(!bit(m_r09F,0));
}

static bool chk(QTableWidget *t, int r, int c) {
    return t->item(r,c)->checkState() == Qt::Checked;
}
static int b2i(bool v, int shift) { return (v ? 1 : 0) << shift; }

void PanelConfigDialog::saveToBlob() {
    m_r1DC = static_cast<uint8_t>(
        b2i(chk(m_tblPort0,0,2),3)|b2i(chk(m_tblPort0,1,2),2)|
        b2i(chk(m_tblPort0,2,2),1)|b2i(chk(m_tblPort0,3,2),0));
    m_r1DE = static_cast<uint8_t>(
        b2i(chk(m_tblPort0,4,2),7)|b2i(chk(m_tblPort1,0,2),5)|
        b2i(chk(m_tblPort1,1,2),4)|b2i(chk(m_tblPort1,2,2),3)|
        b2i(chk(m_tblPort1,3,2),2)|b2i(chk(m_tblPort1,4,2),1));
    m_r1E1 = static_cast<uint8_t>(
        b2i(m_cbInvVsync->isChecked(),7)|b2i(m_cbInvHsync->isChecked(),6)|
        b2i(m_cbInvData->isChecked(),5)|b2i(m_cbInvClk->isChecked(),4)|
        b2i(m_rbOpenLdi->isChecked(),2)|b2i(m_rb6bit->isChecked(),1)|1);
    m_r1E2 = static_cast<uint8_t>(
        b2i(m_cbDeOnly->isChecked(),7)|
        (m_cmbMap0->currentIndex()<<4)|b2i(true,3)|m_cmbMap1->currentIndex());
    m_r1E3 = static_cast<uint8_t>(
        b2i(chk(m_tblPort0,0,3),4)|b2i(chk(m_tblPort0,1,3),3)|
        b2i(chk(m_tblPort0,2,3),2)|b2i(chk(m_tblPort0,3,3),1)|
        b2i(chk(m_tblPort0,4,3),0));
    m_r1E4 = static_cast<uint8_t>(
        b2i(chk(m_tblPort1,0,3),4)|b2i(chk(m_tblPort1,1,3),3)|
        b2i(chk(m_tblPort1,2,3),2)|b2i(chk(m_tblPort1,3,3),1)|
        b2i(chk(m_tblPort1,4,3),0));
    m_r1E7 = static_cast<uint8_t>(b2i(m_cbSwapPixel->isChecked(),1)|8);
    m_r1ED = kLclkSet[m_cmbMap0->currentIndex()];
    m_r1F2 = kRclkSet[m_cmbMap1->currentIndex()];
    m_r1F3 = static_cast<uint8_t>((m_cmbCkSwing->currentIndex()<<4)|m_cmbDatSwing->currentIndex());
    m_r1F4 = static_cast<uint8_t>(m_cmbVcom->currentIndex());
    m_r09F = static_cast<uint8_t>(m_rbDualPort->isChecked() ? 1 : 0);

    auto &blob = BlobStore::data();
    int panel = BlobStore::currentPanel();
    int offset = RomLayout::kOcmBase + RomLayout::kCfgTableOffset
               + panel * RomLayout::kCfgEntrySize;
    blob[offset+0]  = m_r09F;
    blob[offset+1]  = m_r1DC;
    blob[offset+2]  = m_r1DE;
    blob[offset+3]  = m_r1E1;
    blob[offset+4]  = m_r1E2;
    blob[offset+5]  = m_r1E3;
    blob[offset+6]  = m_r1E4;
    blob[offset+7]  = m_r1E7;
    blob[offset+8]  = m_r1ED;
    blob[offset+9]  = m_r1F2;
    blob[offset+10] = m_r1F3;
    blob[offset+11] = m_r1F4;
}

void PanelConfigDialog::onMap0Changed(int index) {
    QString text = m_cmbMap0->currentText().mid(3);
    QStringList sigs = text.split(',');
    if (sigs.size() == 5)
        for (int r = 0; r < 5; ++r)
            m_tblPort0->item(r,1)->setText(sigs[r].trimmed());
}

void PanelConfigDialog::onMap1Changed(int index) {
    QString text = m_cmbMap1->currentText().mid(3);
    QStringList sigs = text.split(',');
    if (sigs.size() == 5)
        for (int r = 0; r < 5; ++r)
            m_tblPort1->item(r,1)->setText(sigs[r].trimmed());
}

void PanelConfigDialog::onSave() {
    saveToBlob();
    accept();
}

void PanelConfigDialog::onCancel() {
    if (QMessageBox::question(this, "Confirm", "Discard changes and close?",
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        reject();
}
