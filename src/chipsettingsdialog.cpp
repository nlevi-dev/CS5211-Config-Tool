#include "chipsettingsdialog.h"
#include "romdata.h"
#include "blobstore.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QIntValidator>
#include <cmath>

static const QStringList kDutyItems = {
    "0% (low)", "10%", "20%", "30%", "40%", "50%",
    "60%", "70%", "80%", "90%", "100% (high)"
};
static const QStringList kSscModeItems = {
    "0: No SSC", "1: +/- 0.25%", "2: +/- 0.5%",
    "3: +/- 0.75%", "4: +/- 1.0%", "5: +/- 1.5%", "6: +/- 2.5%"
};
static const QStringList kSscStepItems = {
    "0=20","1=24","2=28","3=32","4=40","5=48","6=56","7=64",
    "8=80","9=96","A=112","B=128","C=160","D=192","E=224","F=255"
};
static const QStringList kSlewItems = {
    "00 -- Fastest", "01 -- Fast", "10 -- Slow", "11 -- Slowest"
};

ChipSettingsDialog::ChipSettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Chip Configuration (shared by all 16 panels)");
    setModal(true);

    // --- eDP speed ---
    auto *gbSpeed = new QGroupBox("eDP Link Speed");
    m_rb27G  = new QRadioButton("2.7 Gbps");
    m_rb162G = new QRadioButton("1.62 Gbps");
    m_rb27G->setChecked(true);
    auto *hSpeed = new QHBoxLayout;
    hSpeed->addWidget(m_rb27G);
    hSpeed->addWidget(m_rb162G);
    gbSpeed->setLayout(hSpeed);

    // --- eDP lanes ---
    auto *gbLane = new QGroupBox("eDP Lane Count");
    m_rb2Lane = new QRadioButton("2 lanes");
    m_rb1Lane = new QRadioButton("1 lane");
    m_rb2Lane->setChecked(true);
    auto *hLane = new QHBoxLayout;
    hLane->addWidget(m_rb2Lane);
    hLane->addWidget(m_rb1Lane);
    gbLane->setLayout(hLane);

    // --- Clock source ---
    auto *gbClk = new QGroupBox("Chip Clock Source");
    m_rbCrystal  = new QRadioButton("External crystal (SSC allowed)");
    m_rbInternal = new QRadioButton("Internal 27 MHz");
    m_rbCrystal->setChecked(true);
    auto *hClk = new QHBoxLayout;
    hClk->addWidget(m_rbCrystal);
    hClk->addWidget(m_rbInternal);
    gbClk->setLayout(hClk);

    // --- PWM ---
    auto *gbPwm = new QGroupBox("PWM Configuration");
    m_rbPwmMode1   = new QRadioButton("Mode 1: chip generates 500 Hz clock output");
    m_rbPwmMode2   = new QRadioButton("Mode 2: PWM input → PWM output (pass-through)");
    m_rbPwmMode1->setChecked(true);
    m_cbInvertPwm   = new QCheckBox("Invert key function");
    m_cbPowerDownKey = new QCheckBox("Enable power-down key");
    m_cbKeyAdjust   = new QCheckBox("Enable key brightness adjustment, range:");
    m_cmbDefaultDuty = new QComboBox; m_cmbDefaultDuty->addItems(kDutyItems);
    m_cmbMinDuty     = new QComboBox; m_cmbMinDuty->addItems(kDutyItems);
    m_cmbMaxDuty     = new QComboBox; m_cmbMaxDuty->addItems(kDutyItems);
    m_cmbDefaultDuty->setCurrentIndex(10);
    m_cmbMaxDuty->setCurrentIndex(10);

    auto *pwmRow1 = new QHBoxLayout;
    pwmRow1->addWidget(m_rbPwmMode1);
    pwmRow1->addWidget(m_rbPwmMode2);

    auto *pwmRow2 = new QHBoxLayout;
    pwmRow2->addWidget(new QLabel("Default duty:"));
    pwmRow2->addWidget(m_cmbDefaultDuty);
    pwmRow2->addWidget(m_cbKeyAdjust);
    pwmRow2->addWidget(m_cmbMinDuty);
    pwmRow2->addWidget(new QLabel("to"));
    pwmRow2->addWidget(m_cmbMaxDuty);

    auto *pwmRow3 = new QHBoxLayout;
    pwmRow3->addWidget(m_cbPowerDownKey);
    pwmRow3->addWidget(m_cbInvertPwm);
    pwmRow3->addStretch();

    auto *vPwm = new QVBoxLayout;
    vPwm->addLayout(pwmRow1);
    vPwm->addLayout(pwmRow2);
    vPwm->addLayout(pwmRow3);
    gbPwm->setLayout(vPwm);

    // --- Power sequencing ---
    auto *gbSeq = new QGroupBox("Power Sequencing Timings (set to multiples of 2 ms)");
    auto *seqGrid = new QHBoxLayout;
    auto makeSeqField = [&](const QString &label, QLineEdit *&ed, const QString &range) {
        auto *col = new QVBoxLayout;
        col->addWidget(new QLabel(label));
        ed = new QLineEdit;
        ed->setValidator(new QIntValidator(0, 32766, ed));
        ed->setFixedWidth(60);
        ed->setAlignment(Qt::AlignRight);
        col->addWidget(ed);
        col->addWidget(new QLabel(range));
        seqGrid->addLayout(col);
    };
    makeSeqField("T2 (ms)", m_edT2, "0–510");
    makeSeqField("T3 (ms)", m_edT3, "0–32766");
    makeSeqField("T4 (ms)", m_edT4, "0–510");
    makeSeqField("T5 (ms)", m_edT5, "0–32766");
    makeSeqField("T7 (ms)", m_edT7, "0–32766");
    gbSeq->setLayout(seqGrid);

    // --- SSC ---
    auto *gbSsc = new QGroupBox("Spread Spectrum Clock (SSC)");
    m_cmbSscMode = new QComboBox; m_cmbSscMode->addItems(kSscModeItems);
    m_edSscDiv   = new QLineEdit("0");
    m_edSscDiv->setValidator(new QIntValidator(0, 255, m_edSscDiv));
    m_edSscDiv->setFixedWidth(50);
    m_cmbSscStep = new QComboBox; m_cmbSscStep->addItems(kSscStepItems);
    m_lblSscFreq = new QLabel("-- kHz");

    auto *hSsc = new QHBoxLayout;
    hSsc->addWidget(new QLabel("SSC mode:"));
    hSsc->addWidget(m_cmbSscMode);
    hSsc->addWidget(new QLabel("Divider (0–255):"));
    hSsc->addWidget(m_edSscDiv);
    hSsc->addWidget(new QLabel("SSC step:"));
    hSsc->addWidget(m_cmbSscStep);
    hSsc->addWidget(new QLabel("Mod. freq:"));
    hSsc->addWidget(m_lblSscFreq);
    gbSsc->setLayout(hSsc);

    // --- LVDS slew ---
    auto *gbSlew = new QGroupBox("LVDS Slew Rate Control (EMI)");
    m_cmbSlewCtrl = new QComboBox; m_cmbSlewCtrl->addItems(kSlewItems);
    auto *hSlew = new QHBoxLayout;
    hSlew->addWidget(new QLabel("Slew rate:"));
    hSlew->addWidget(m_cmbSlewCtrl);
    hSlew->addStretch();
    gbSlew->setLayout(hSlew);

    // --- Buttons ---
    auto *btnSave   = new QPushButton("Save and close");
    auto *btnCancel = new QPushButton("Discard and close");
    auto *hBtn = new QHBoxLayout;
    hBtn->addStretch();
    hBtn->addWidget(btnSave);
    hBtn->addWidget(btnCancel);

    // --- Top-level layout ---
    auto *topRow = new QHBoxLayout;
    topRow->addWidget(gbSpeed);
    topRow->addWidget(gbLane);
    topRow->addWidget(gbClk);

    auto *root = new QVBoxLayout(this);
    root->addLayout(topRow);
    root->addWidget(gbSsc);
    root->addWidget(gbSlew);
    root->addWidget(gbPwm);
    root->addWidget(gbSeq);
    root->addLayout(hBtn);

    connect(btnSave,   &QPushButton::clicked, this, &ChipSettingsDialog::onSave);
    connect(btnCancel, &QPushButton::clicked, this, &ChipSettingsDialog::onCancel);
    connect(m_cmbSscMode,  QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ChipSettingsDialog::onSscModeChanged);
    connect(m_rbCrystal,   &QRadioButton::toggled, this, &ChipSettingsDialog::onClkSourceChanged);
    connect(m_edSscDiv,    &QLineEdit::textChanged, this, &ChipSettingsDialog::onSscDivChanged);
    connect(m_cmbSscStep,  QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ChipSettingsDialog::onSscStepChanged);
    connect(m_rbPwmMode1,  &QRadioButton::toggled, this, &ChipSettingsDialog::onPwmMode1Toggled);
    connect(m_cbKeyAdjust, &QCheckBox::toggled,    this, &ChipSettingsDialog::onKeyAdjustToggled);

    loadFromBlob();
}

void ChipSettingsDialog::recalcSscFreq() {
    bool ok1, ok2;
    int div  = m_edSscDiv->text().toInt(&ok1);
    int stepIdx = m_cmbSscStep->currentIndex();
    static const int kStepValues[] = {20,24,28,32,40,48,56,64,80,96,112,128,160,192,224,255};
    if (!ok1 || stepIdx < 0) return;
    int step = kStepValues[stepIdx];
    float freq = 27000.0f / ((div + 1.0f) * 2.0f * step);
    m_lblSscFreq->setText(QString::number(freq, 'f', 2) + " kHz");
}

void ChipSettingsDialog::onSscModeChanged(int index) {
    bool enabled = (index != 0);
    m_edSscDiv->setEnabled(enabled);
    m_cmbSscStep->setEnabled(enabled);
}

void ChipSettingsDialog::onClkSourceChanged() {
    if (!m_rbCrystal->isChecked()) {
        m_cmbSscMode->setCurrentIndex(0);
        m_cmbSscMode->setEnabled(false);
    } else {
        m_cmbSscMode->setEnabled(true);
    }
}

void ChipSettingsDialog::onSscDivChanged(const QString &) { recalcSscFreq(); }
void ChipSettingsDialog::onSscStepChanged(int)            { recalcSscFreq(); }

void ChipSettingsDialog::onPwmMode1Toggled(bool checked) {
    m_cmbDefaultDuty->setEnabled(checked);
    m_cmbMinDuty->setEnabled(checked && m_cbKeyAdjust->isChecked());
    m_cmbMaxDuty->setEnabled(checked && m_cbKeyAdjust->isChecked());
    m_cbKeyAdjust->setEnabled(checked);
}

void ChipSettingsDialog::onKeyAdjustToggled(bool checked) {
    m_cmbMinDuty->setEnabled(checked);
    m_cmbMaxDuty->setEnabled(checked);
}

void ChipSettingsDialog::loadFromBlob() {
    auto &blob = BlobStore::data();

    uint8_t speed = blob[RomLayout::kAddrLinkSpeed];
    m_rb27G->setChecked(speed == 10);
    m_rb162G->setChecked(speed == 6);

    uint8_t lanes = blob[RomLayout::kAddrLaneCnt];
    m_rb2Lane->setChecked(lanes == 2);
    m_rb1Lane->setChecked(lanes == 1);

    uint8_t pwm = blob[RomLayout::kAddrPwmMode];
    m_cbPowerDownKey->setChecked(pwm & 0x10);
    m_cbInvertPwm->setChecked(pwm & 0x20);
    int pwmMode = pwm & 0x03;
    m_rbPwmMode1->setChecked(pwmMode != 2);
    m_rbPwmMode2->setChecked(pwmMode == 2);
    m_cbKeyAdjust->setChecked(pwmMode == 1);

    m_cmbDefaultDuty->setCurrentIndex(qBound(0, (int)blob[RomLayout::kAddrPwmDefault], 10));
    m_cmbMinDuty->setCurrentIndex(qBound(0, (int)blob[RomLayout::kAddrPwmMin], 10));
    m_cmbMaxDuty->setCurrentIndex(qBound(0, (int)blob[RomLayout::kAddrPwmMax], 10));

    m_edT2->setText(QString::number(2 * blob[RomLayout::kAddrT2]));
    m_edT4->setText(QString::number(2 * blob[RomLayout::kAddrT4]));
    int t3 = (blob[RomLayout::kAddrT3H] << 8) | blob[RomLayout::kAddrT3L];
    int t5 = (blob[RomLayout::kAddrT5H] << 8) | blob[RomLayout::kAddrT5L];
    int t7 = (blob[RomLayout::kAddrT7H] << 8) | blob[RomLayout::kAddrT7L];
    m_edT3->setText(QString::number(2 * t3));
    m_edT5->setText(QString::number(2 * t5));
    m_edT7->setText(QString::number(2 * t7));

    m_rbCrystal->setChecked((blob[RomLayout::kAddrClkSel] & 1) == 0);
    m_rbInternal->setChecked((blob[RomLayout::kAddrClkSel] & 1) != 0);

    uint8_t sscMode = blob[RomLayout::kAddrSscMode];
    if (sscMode == 0xAA) sscMode = 0;
    m_cmbSscMode->setCurrentIndex(sscMode & 0x07);
    m_cmbSscStep->setCurrentIndex(blob[RomLayout::kAddrSscStep] & 0x0F);
    m_edSscDiv->setText(QString::number(blob[RomLayout::kAddrSscDiv]));

    uint8_t slew = blob[RomLayout::kAddrSlewCtrl];
    if (slew == 0xAA) slew = 0;
    m_cmbSlewCtrl->setCurrentIndex(slew & 0x03);

    recalcSscFreq();
}

void ChipSettingsDialog::saveToBlob() {
    auto &blob = BlobStore::data();

    blob[RomLayout::kAddrLinkSpeed] = m_rb27G->isChecked() ? 10 : 6;
    blob[RomLayout::kAddrLaneCnt]   = m_rb2Lane->isChecked() ? 2 : 1;

    int pwm = 0;
    if (m_cbKeyAdjust->isChecked()) pwm = 1;
    if (m_rbPwmMode2->isChecked())  pwm = 2;
    if (m_cbInvertPwm->isChecked())   pwm |= 0x20;
    if (m_cbPowerDownKey->isChecked()) pwm |= 0x10;
    blob[RomLayout::kAddrPwmMode]    = static_cast<uint8_t>(pwm);
    blob[RomLayout::kAddrPwmDefault] = static_cast<uint8_t>(m_cmbDefaultDuty->currentIndex());
    blob[RomLayout::kAddrPwmMin]     = static_cast<uint8_t>(m_cmbMinDuty->currentIndex());
    blob[RomLayout::kAddrPwmMax]     = static_cast<uint8_t>(m_cmbMaxDuty->currentIndex());

    blob[RomLayout::kAddrT2] = static_cast<uint8_t>(m_edT2->text().toInt() >> 1);
    blob[RomLayout::kAddrT4] = static_cast<uint8_t>(m_edT4->text().toInt() >> 1);
    int t3 = m_edT3->text().toInt() >> 1;
    int t5 = m_edT5->text().toInt() >> 1;
    int t7 = m_edT7->text().toInt() >> 1;
    blob[RomLayout::kAddrT3H] = static_cast<uint8_t>((t3 >> 8) & 0xFF);
    blob[RomLayout::kAddrT3L] = static_cast<uint8_t>(t3 & 0xFF);
    blob[RomLayout::kAddrT5H] = static_cast<uint8_t>((t5 >> 8) & 0xFF);
    blob[RomLayout::kAddrT5L] = static_cast<uint8_t>(t5 & 0xFF);
    blob[RomLayout::kAddrT7H] = static_cast<uint8_t>((t7 >> 8) & 0xFF);
    blob[RomLayout::kAddrT7L] = static_cast<uint8_t>(t7 & 0xFF);

    blob[RomLayout::kAddrClkSel]  = static_cast<uint8_t>(m_rbInternal->isChecked() ? 1 : 0);
    blob[RomLayout::kAddrSscStep] = static_cast<uint8_t>(m_cmbSscStep->currentIndex());
    blob[RomLayout::kAddrSscDiv]  = static_cast<uint8_t>(m_edSscDiv->text().toInt());
    blob[RomLayout::kAddrSscMode] = static_cast<uint8_t>(m_cmbSscMode->currentIndex());
    blob[RomLayout::kAddrSlewCtrl]= static_cast<uint8_t>(m_cmbSlewCtrl->currentIndex());
}

void ChipSettingsDialog::onSave() {
    if (m_cmbMinDuty->currentIndex() > m_cmbMaxDuty->currentIndex()) {
        QMessageBox::critical(this, "Error",
            "PWM range error: minimum duty cycle is greater than maximum.");
        return;
    }
    saveToBlob();
    accept();
}

void ChipSettingsDialog::onCancel() {
    if (QMessageBox::question(this, "Confirm", "Discard changes and close?",
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        reject();
}
