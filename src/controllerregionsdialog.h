#ifndef CONTROLLERREGIONSDIALOG_H
#define CONTROLLERREGIONSDIALOG_H

#include <QDialog>
#include <QVector>

struct ControllerRegionData {
    qreal x = 0, y = 0, w = 0, h = 0, r = 0;
};

class QTableWidget;
class QSpinBox;
class ControllerRegionsPreview;

class ControllerRegionsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ControllerRegionsDialog(QWidget *parent = nullptr);
    static QString regionsFilePath();

private slots:
    void onSelectionChanged();
    void onCellChanged(int row, int column);
    void loadFromFile();
    void saveToFile();
    void resetToBuiltin();

private:
    void loadRegionsFromJson(const QString &path);
    bool saveRegionsToJson(const QString &path);
    void refreshTableFromRegions();
    void refreshRegionsFromTable();
    void updateDpadPreview();
    static QVector<ControllerRegionData> builtinRegions();

    QVector<ControllerRegionData> m_regions;
    ControllerRegionsPreview *m_preview = nullptr;
    QTableWidget *m_table = nullptr;
    QSpinBox *m_dpadBaseWidthSpin = nullptr;
    QSpinBox *m_dpadBaseHeightSpin = nullptr;
    QSpinBox *m_dpadTipSpin = nullptr;
    qreal m_dpadBaseWidth = 40.0;
    qreal m_dpadBaseHeight = 40.0;
    qreal m_dpadTipLength = 40.0;
    bool m_ignoreCellChange = false;
};

#endif // CONTROLLERREGIONSDIALOG_H
