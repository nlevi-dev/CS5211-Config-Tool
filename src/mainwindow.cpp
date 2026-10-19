#include "mainwindow.h"
#include "blobstore.h"
#include "romdata.h"
#include "crc32util.h"
#include "edidEditordialog.h"
#include "panelconfigdialog.h"
#include "chipsettingsdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QWidget>
#include <QStatusBar>
#include <QCoreApplication>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("CS5211 E2PROM Configuration Tool");
    setFixedSize(560, 400);

    auto *central = new QWidget(this);
    setCentralWidget(central);

    // --- Import group ---
    auto *gbImport = new QGroupBox("Import File");
    m_edImportPath = new QLineEdit("./Template.bin");
    m_edImportPath->setReadOnly(false);
    m_btnImport = new QPushButton("Import existing file...");
    m_lblCurrentFwVer = new QLabel("Current file firmware version: --");

    auto *hImp = new QHBoxLayout;
    hImp->addWidget(m_edImportPath);
    hImp->addWidget(m_btnImport);

    auto *lblHint1 = new QLabel(
        "To edit an existing file, import it. To create a new file, skip this step (edits on default template).");
    lblHint1->setWordWrap(true);

    // Firmware sub-section
    auto *lblHint2 = new QLabel(
        "To update the MCU firmware, select a file. Otherwise skip this step.");
    lblHint2->setWordWrap(true);
    m_edFirmwarePath = new QLineEdit;
    m_btnFw = new QPushButton("Import MCU firmware...");
    m_lblNewFwVer = new QLabel;

    auto *hFw = new QHBoxLayout;
    hFw->addWidget(m_edFirmwarePath);
    hFw->addWidget(m_btnFw);

    auto *vImp = new QVBoxLayout;
    vImp->addWidget(lblHint1);
    vImp->addLayout(hImp);
    vImp->addWidget(m_lblCurrentFwVer);
    vImp->addWidget(lblHint2);
    vImp->addLayout(hFw);
    vImp->addWidget(m_lblNewFwVer);
    gbImport->setLayout(vImp);

    // --- Panel config group ---
    auto *gbPanel = new QGroupBox("Panel Configuration");
    m_cbAllSame = new QCheckBox("All 16 panels use identical configuration");
    m_cmbPanel  = new QComboBox;
    m_cmbPanel->setMinimumWidth(320);
    auto *lblGpio = new QLabel("GPIO[3:0]:");

    auto *hPanel = new QHBoxLayout;
    hPanel->addWidget(lblGpio);
    hPanel->addWidget(m_cmbPanel);
    hPanel->addStretch();

    m_btnEdid    = new QPushButton("EDID Configuration (Resolution)");
    m_btnPanelCfg = new QPushButton("Panel Parameters");
    auto *hPanelBtns = new QHBoxLayout;
    hPanelBtns->addWidget(m_btnEdid);
    hPanelBtns->addWidget(m_btnPanelCfg);

    auto *vPanel = new QVBoxLayout;
    vPanel->addWidget(m_cbAllSame);
    vPanel->addLayout(hPanel);
    vPanel->addLayout(hPanelBtns);
    gbPanel->setLayout(vPanel);

    // --- Bottom buttons ---
    m_btnChipCfg = new QPushButton("Chip Configuration (shared by all 16 panels)");
    m_btnSave    = new QPushButton("Save E2PROM file as...");
    auto *hBottom = new QHBoxLayout;
    hBottom->addWidget(m_btnChipCfg);
    hBottom->addStretch();
    hBottom->addWidget(m_btnSave);

    auto *root = new QVBoxLayout(central);
    root->addWidget(gbImport);
    root->addWidget(gbPanel);
    root->addLayout(hBottom);

    connect(m_btnImport,  &QPushButton::clicked, this, &MainWindow::onImportFile);
    connect(m_btnFw,      &QPushButton::clicked, this, &MainWindow::onImportFirmware);
    connect(m_btnEdid,    &QPushButton::clicked, this, &MainWindow::onOpenEdid);
    connect(m_btnPanelCfg,&QPushButton::clicked, this, &MainWindow::onOpenPanelConfig);
    connect(m_btnChipCfg, &QPushButton::clicked, this, &MainWindow::onOpenChipConfig);
    connect(m_btnSave,    &QPushButton::clicked, this, &MainWindow::onSaveBlob);
    connect(m_cbAllSame,  &QCheckBox::toggled,   this, &MainWindow::onAllSameToggled);

    // Load default template on startup — always look next to the executable
    QString tmpl = QCoreApplication::applicationDirPath() + "/Template-CS5211.bin";
    if (QFile::exists(tmpl)) {
        loadBlobFromFile(tmpl);
        m_edImportPath->setText(tmpl);
        refreshPanelCombo();
        m_cmbPanel->setCurrentIndex(0);
    } else {
        m_edImportPath->clear();
        setLocked(true);
    }
}

void MainWindow::setLocked(bool locked) {
    m_btnFw->setEnabled(!locked);
    m_edFirmwarePath->setEnabled(!locked);
    m_btnEdid->setEnabled(!locked);
    m_btnPanelCfg->setEnabled(!locked);
    m_btnChipCfg->setEnabled(!locked);
    m_btnSave->setEnabled(!locked);
    m_cbAllSame->setEnabled(!locked);
    m_cmbPanel->setEnabled(!locked);
}

bool MainWindow::loadBlobFromFile(const QString &path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Cannot open file: " + path);
        return false;
    }
    if (f.size() != RomLayout::kBlobSize) {
        QMessageBox::critical(this, "Error",
            QString("File size error: expected %1 bytes, got %2.")
                .arg(RomLayout::kBlobSize).arg(f.size()));
        return false;
    }
    auto &blob = BlobStore::data();
    f.read(reinterpret_cast<char*>(blob.data()), RomLayout::kBlobSize);
    uint8_t ver = blob[RomLayout::kAddrFwVersion];
    m_lblCurrentFwVer->setText(
        QString("Current file firmware version: 0x%1").arg(ver, 2, 16, QChar('0')));
    return true;
}

void MainWindow::importFirmwareFromFile(const QString &path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Cannot open firmware file: " + path);
        return;
    }
    QByteArray fw = f.readAll();
    int fwLen = fw.size();
    auto &blob = BlobStore::data();

    // Copy firmware bytes, preserving the per-panel config table region
    for (int i = 0; i < RomLayout::kCfgTableOffset; ++i)
        blob[RomLayout::kOcmBase + i] = static_cast<uint8_t>(fw[i]);
    int afterCfg = RomLayout::kCfgTableOffset + RomLayout::kCfgTableSize;
    for (int i = afterCfg; i < fwLen; ++i)
        blob[RomLayout::kOcmBase + i] = static_cast<uint8_t>(fw[i]);
    for (int i = RomLayout::kOcmBase + fwLen; i < RomLayout::kBlobSize; ++i)
        blob[i] = 0;

    int storedLen = fwLen + 2;
    blob[RomLayout::kOcmLenAddr]   = static_cast<uint8_t>((storedLen >> 8) & 0xFF);
    blob[RomLayout::kOcmLenAddr+1] = static_cast<uint8_t>(storedLen & 0xFF);

    uint8_t ver = blob[RomLayout::kAddrFwVersion];
    m_lblNewFwVer->setText(
        QString("New firmware version: 0x%1").arg(ver, 2, 16, QChar('0')));
}

QString MainWindow::panelLabel(int index) const {
    auto &blob = BlobStore::data();
    int base = index * RomLayout::kEdidBlockSize;
    QString prefix = (index < 10) ? QString("0%1").arg(index) : QString::number(index);
    if (blob[base] == 0x00 && blob[base+1] == 0xFF) {
        int ha = ((blob[base+58] >> 4) << 8) | blob[base+56];
        int va = ((blob[base+61] >> 4) << 8) | blob[base+59];
        int cfgByte1 = blob[RomLayout::kOcmBase + 16*(index+1)];
        int cfgByte2 = blob[RomLayout::kOcmBase + 16*(index+1) + 3];
        QString ports = (cfgByte1 == 1) ? "dual" : "single";
        QString depth = ((cfgByte2 & 2) == 2) ? "6" : "8";
        return QString("%1: %2x%3 %4-%5bit").arg(prefix).arg(ha).arg(va).arg(ports).arg(depth);
    }
    return prefix + ": (empty)";
}

void MainWindow::refreshPanelCombo() {
    int prev = m_cmbPanel->currentIndex();
    m_cmbPanel->blockSignals(true);
    m_cmbPanel->clear();
    for (int i = 0; i < RomLayout::kEdidPanelCount; ++i)
        m_cmbPanel->addItem(panelLabel(i));
    m_cmbPanel->blockSignals(false);
    m_cmbPanel->setCurrentIndex(qMax(0, prev));
}

void MainWindow::onAllSameToggled(bool checked) {
    m_cmbPanel->setEnabled(!checked);
}

void MainWindow::onImportFile() {
    QString path = QFileDialog::getOpenFileName(this, "Import E2PROM file",
        QString(), "Binary files (*.bin);;All files (*.*)");
    if (path.isEmpty()) return;
    if (loadBlobFromFile(path)) {
        m_edImportPath->setText(path);
        setLocked(false);
        refreshPanelCombo();
    }
}

void MainWindow::onImportFirmware() {
    QString path = QFileDialog::getOpenFileName(this, "Import MCU firmware",
        QString(), "Binary files (*.bin);;All files (*.*)");
    if (path.isEmpty()) return;
    m_edFirmwarePath->setText(path);
    importFirmwareFromFile(path);
}

void MainWindow::onOpenEdid() {
    int panel = m_cbAllSame->isChecked() ? m_cmbPanel->currentIndex()
                                         : m_cmbPanel->currentIndex();
    BlobStore::currentPanel() = panel;
    EdidEditorDialog dlg(this);
    if (m_cbAllSame->isChecked())
        dlg.setWindowTitle(dlg.windowTitle() + " — all panels use same config");
    else
        dlg.setWindowTitle(dlg.windowTitle() +
            QString(" — GPIO[3:0] = %1").arg(m_cmbPanel->currentText()));
    if (dlg.exec() == QDialog::Accepted)
        refreshPanelCombo();
}

void MainWindow::onOpenPanelConfig() {
    BlobStore::currentPanel() = m_cmbPanel->currentIndex();
    PanelConfigDialog dlg(this);
    if (m_cbAllSame->isChecked())
        dlg.setWindowTitle(dlg.windowTitle() + " — all panels use same config");
    else
        dlg.setWindowTitle(dlg.windowTitle() +
            QString(" — GPIO[3:0] = %1").arg(m_cmbPanel->currentText()));
    if (dlg.exec() == QDialog::Accepted)
        refreshPanelCombo();
}

void MainWindow::onOpenChipConfig() {
    ChipSettingsDialog dlg(this);
    dlg.exec();
}

void MainWindow::onSaveBlob() {
    QString path = QFileDialog::getSaveFileName(this, "Save E2PROM file",
        QString(), "Binary files (*.bin);;All files (*.*)",
        nullptr, QFileDialog::DontConfirmOverwrite);
    if (path.isEmpty()) return;
    if (!path.endsWith(".bin", Qt::CaseInsensitive))
        path += ".bin";
    if (QFile::exists(path)) {
        if (QMessageBox::question(this, "Overwrite?",
                QString("%1 already exists.\nDo you want to replace it?").arg(path),
                QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
            return;
    }

    auto &blob = BlobStore::data();

    // If "all same" is checked, replicate the selected panel's EDID and cfg
    if (m_cbAllSame->isChecked()) {
        int src = m_cmbPanel->currentIndex();
        for (int i = 0; i < RomLayout::kEdidPanelCount; ++i) {
            if (i == src) continue;
            for (int b = 0; b < RomLayout::kEdidBlockSize; ++b)
                blob[i * RomLayout::kEdidBlockSize + b] =
                    blob[src * RomLayout::kEdidBlockSize + b];
            int cfgBase = RomLayout::kOcmBase + RomLayout::kCfgTableOffset;
            for (int b = 0; b < RomLayout::kCfgEntrySize; ++b)
                blob[cfgBase + i * RomLayout::kCfgEntrySize + b] =
                    blob[cfgBase + src * RomLayout::kCfgEntrySize + b];
        }
    }

    // Compute and store OCM CRC
    int ocmLen = (blob[RomLayout::kOcmLenAddr] << 8) | blob[RomLayout::kOcmLenAddr+1];
    if (ocmLen > 0 && ocmLen <= RomLayout::kBlobSize - RomLayout::kOcmLenAddr) {
        uint32_t crc = Crc32Util::compute(
            blob.data() + RomLayout::kOcmLenAddr, ocmLen);
        blob[RomLayout::kOcmCrcAddr+0] = static_cast<uint8_t>((crc >> 24) & 0xFF);
        blob[RomLayout::kOcmCrcAddr+1] = static_cast<uint8_t>((crc >> 16) & 0xFF);
        blob[RomLayout::kOcmCrcAddr+2] = static_cast<uint8_t>((crc >>  8) & 0xFF);
        blob[RomLayout::kOcmCrcAddr+3] = static_cast<uint8_t>( crc        & 0xFF);
    }

    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "Error", "Cannot write file: " + path);
        return;
    }
    f.write(reinterpret_cast<const char*>(blob.data()), RomLayout::kBlobSize);
    QMessageBox::information(this, "Success", "Binary file saved successfully.");
}
