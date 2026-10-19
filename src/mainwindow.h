#pragma once
#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QComboBox;
class QCheckBox;
class QLineEdit;
class QLabel;
class QPushButton;
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onImportFile();
    void onImportFirmware();
    void onOpenEdid();
    void onOpenPanelConfig();
    void onOpenChipConfig();
    void onSaveBlob();
    void onAllSameToggled(bool checked);

private:
    bool loadBlobFromFile(const QString &path);
    void importFirmwareFromFile(const QString &path);
    void refreshPanelCombo();
    QString panelLabel(int index) const;
    void setLocked(bool locked);

    QComboBox   *m_cmbPanel;
    QCheckBox   *m_cbAllSame;
    QLineEdit   *m_edImportPath;
    QLineEdit   *m_edFirmwarePath;
    QLabel      *m_lblCurrentFwVer;
    QLabel      *m_lblNewFwVer;
    QPushButton *m_btnEdid;
    QPushButton *m_btnPanelCfg;
    QPushButton *m_btnImport;
    QPushButton *m_btnFw;
    QPushButton *m_btnChipCfg;
    QPushButton *m_btnSave;
};
