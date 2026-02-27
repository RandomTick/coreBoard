#include "controllerregionsdialog.h"
#include "controlleritem.h"
#include <QBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QCoreApplication>
#include <QPainter>
#include <QFont>
#include <QPolygonF>
#include <QSvgRenderer>
#include <QScrollArea>
#include <QSpinBox>
#include <QFormLayout>
#include <QGroupBox>

static const char *regionNames[] = {
    "A", "B", "X", "Y", "LB", "RB", "Back", "Start",
    "LStick", "RStick", "D-pad Up", "D-pad Down", "D-pad Left", "D-pad Right",
    "LT", "RT"
};

QVector<ControllerRegionData> ControllerRegionsDialog::builtinRegions()
{
    return {
        { 365.3, 299, 0, 0, 19 },
        { 438.047, 251.421, 0, 0, 18.77 },
        { 399.932, 211.323, 0, 0, 18.77 },
        { 438.047, 173.226, 0, 0, 18.77 },
        { 88, 106, 65, 18, -1 },
        { 427, 106, 65, 18, -1 },
        { 236, 278, 32, 28, -1 },
        { 312, 278, 32, 28, -1 },
        { 155.7, 196.78, 0, 0, 14 },
        { 386.78, 278.42, 0, 0, 21 },
        { 256, 270, 0, 0, 11 },
        { 256, 314, 0, 0, 11 },
        { 234, 292, 0, 0, 11 },
        { 278, 292, 0, 0, 11 },
        { 88, 70, 65, 28, -1 },
        { 427, 70, 65, 28, -1 }
    };
}

// Preview widget: draws controller SVG (SVG space 0..580) then overlays the 16 regions.
class ControllerRegionsPreview : public QWidget
{
public:
    static constexpr qreal SVG_SIZE = 580.032;
    QVector<ControllerRegionData> regions;
    int selectedIndex = -1;
    qreal dpadBaseWidth = 40.0;
    qreal dpadBaseHeight = 40.0;
    qreal dpadTipLength = 40.0;

    explicit ControllerRegionsPreview(QWidget *parent = nullptr) : QWidget(parent)
    {
        setMinimumSize(300, 300);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.fillRect(rect(), Qt::darkGray);
        qreal side = qMin(width(), height());
        qreal scale = side / SVG_SIZE;
        QRectF r(0, 0, side, side);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        QSvgRenderer renderer(controllerSvgBytes(QColor(80, 80, 80)));
        if (renderer.isValid())
            renderer.render(&p, r);
        p.setPen(Qt::NoPen);
        for (int i = 0; i < regions.size() && i < 16; ++i) {
            const ControllerRegionData &reg = regions.at(i);
            bool sel = (i == selectedIndex);
            p.setBrush(sel ? QColor(255, 200, 0, 180) : QColor(0, 180, 255, 100));
            if (i >= 10 && i <= 13) {
                // D-pad: rectangular base (dpadBaseWidth x dpadBaseHeight). Up/Down use that rect; Left/Right use same rect rotated 90°.
                qreal cx = reg.x, cy = reg.y, halfW = dpadBaseWidth / 2;
                QPolygonF poly;
                switch (i) {
                case 10: poly << QPointF(cx, cy + dpadTipLength) << QPointF(cx + halfW, cy) << QPointF(cx + halfW, cy - dpadBaseHeight) << QPointF(cx - halfW, cy - dpadBaseHeight) << QPointF(cx - halfW, cy); break;
                case 11: poly << QPointF(cx, cy - dpadTipLength) << QPointF(cx + halfW, cy) << QPointF(cx + halfW, cy + dpadBaseHeight) << QPointF(cx - halfW, cy + dpadBaseHeight) << QPointF(cx - halfW, cy); break;
                case 12: poly << QPointF(cx + dpadTipLength, cy) << QPointF(cx, cy + halfW) << QPointF(cx - dpadBaseHeight, cy + halfW) << QPointF(cx - dpadBaseHeight, cy - halfW) << QPointF(cx, cy - halfW); break;
                case 13: poly << QPointF(cx - dpadTipLength, cy) << QPointF(cx, cy + halfW) << QPointF(cx + dpadBaseHeight, cy + halfW) << QPointF(cx + dpadBaseHeight, cy - halfW) << QPointF(cx, cy - halfW); break;
                default: break;
                }
                if (poly.size() == 5) {
                    for (int j = 0; j < 5; ++j) poly[j] = QPointF(poly[j].x() * scale, poly[j].y() * scale);
                    p.drawPolygon(poly);
                }
                p.setPen(sel ? Qt::white : Qt::black);
                p.setFont(QFont(QString(), 8));
                qreal box = qMax(qMax(dpadBaseWidth, dpadBaseHeight), dpadTipLength) * 2;
                p.drawText(QRectF((cx - box/2) * scale, (cy - box/2) * scale, box * scale, box * scale), Qt::AlignCenter, QString::number(i));
                p.setPen(Qt::NoPen);
            } else if (reg.r >= 0) {
                p.drawEllipse(QPointF(reg.x * scale, reg.y * scale), reg.r * scale, reg.r * scale);
                p.setPen(sel ? Qt::white : Qt::black);
                p.setFont(QFont(QString(), 8));
                p.drawText(QRectF((reg.x - reg.r) * scale, (reg.y - reg.r) * scale, reg.r * 2 * scale, reg.r * 2 * scale),
                          Qt::AlignCenter, QString::number(i));
                p.setPen(Qt::NoPen);
            } else {
                p.drawRect(QRectF(reg.x * scale, reg.y * scale, reg.w * scale, reg.h * scale));
                p.setPen(sel ? Qt::white : Qt::black);
                p.setFont(QFont(QString(), 8));
                p.drawText(QRectF(reg.x * scale, reg.y * scale, reg.w * scale, reg.h * scale),
                          Qt::AlignCenter, QString::number(i));
                p.setPen(Qt::NoPen);
            }
        }
    }
};

QString ControllerRegionsDialog::regionsFilePath()
{
    return QCoreApplication::applicationDirPath() + QLatin1String("/controller_regions.json");
}

ControllerRegionsDialog::ControllerRegionsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Edit controller regions"));
    resize(800, 520);
    m_regions = builtinRegions();

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    QScrollArea *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setMinimumWidth(320);
    m_preview = new ControllerRegionsPreview(this);
    m_preview->setFixedSize(400, 400);
    scroll->setWidget(m_preview);
    mainLayout->addWidget(scroll);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    m_table = new QTableWidget(16, 7);
    m_table->setHorizontalHeaderLabels(QStringList() << tr("Index") << tr("Name") << "X" << "Y" << "W" << "H" << "R");
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionBehavior(QTableWidget::SelectRows);
    m_table->setSelectionMode(QTableWidget::SingleSelection);
    m_table->setEditTriggers(QTableWidget::DoubleClicked | QTableWidget::SelectedClicked);
    for (int i = 0; i < 16; ++i) {
        m_table->setItem(i, 0, new QTableWidgetItem(QString::number(i)));
        m_table->item(i, 0)->setFlags(m_table->item(i, 0)->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 1, new QTableWidgetItem(QLatin1String(regionNames[i])));
        m_table->item(i, 1)->setFlags(m_table->item(i, 1)->flags() & ~Qt::ItemIsEditable);
        for (int c = 2; c < 7; ++c)
            m_table->setItem(i, c, new QTableWidgetItem);
    }
    rightLayout->addWidget(m_table);

    QGroupBox *dpadGroup = new QGroupBox(tr("D-pad shape (SVG pixels)"));
    QFormLayout *dpadForm = new QFormLayout(dpadGroup);
    m_dpadBaseWidthSpin = new QSpinBox;
    m_dpadBaseWidthSpin->setRange(1, 200);
    m_dpadBaseWidthSpin->setValue(40);
    m_dpadBaseWidthSpin->setSuffix(tr(" px"));
    m_dpadBaseHeightSpin = new QSpinBox;
    m_dpadBaseHeightSpin->setRange(1, 200);
    m_dpadBaseHeightSpin->setValue(40);
    m_dpadBaseHeightSpin->setSuffix(tr(" px"));
    m_dpadTipSpin = new QSpinBox;
    m_dpadTipSpin->setRange(1, 200);
    m_dpadTipSpin->setValue(40);
    m_dpadTipSpin->setSuffix(tr(" px"));
    dpadForm->addRow(tr("Base width:"), m_dpadBaseWidthSpin);
    dpadForm->addRow(tr("Base height:"), m_dpadBaseHeightSpin);
    dpadForm->addRow(tr("Tip length:"), m_dpadTipSpin);
    rightLayout->addWidget(dpadGroup);
    connect(m_dpadBaseWidthSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ControllerRegionsDialog::updateDpadPreview);
    connect(m_dpadBaseHeightSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ControllerRegionsDialog::updateDpadPreview);
    connect(m_dpadTipSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ControllerRegionsDialog::updateDpadPreview);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    QPushButton *loadBtn = new QPushButton(tr("Load from file"));
    QPushButton *saveBtn = new QPushButton(tr("Save to file"));
    QPushButton *resetBtn = new QPushButton(tr("Reset to built-in"));
    QPushButton *closeBtn = new QPushButton(tr("Close"));
    closeBtn->setDefault(true);
    btnLayout->addWidget(loadBtn);
    btnLayout->addWidget(saveBtn);
    btnLayout->addWidget(resetBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    rightLayout->addLayout(btnLayout);
    mainLayout->addLayout(rightLayout);

    connect(m_table, &QTableWidget::itemSelectionChanged, this, &ControllerRegionsDialog::onSelectionChanged);
    connect(m_table, &QTableWidget::cellChanged, this, &ControllerRegionsDialog::onCellChanged);
    connect(loadBtn, &QPushButton::clicked, this, &ControllerRegionsDialog::loadFromFile);
    connect(saveBtn, &QPushButton::clicked, this, &ControllerRegionsDialog::saveToFile);
    connect(resetBtn, &QPushButton::clicked, this, &ControllerRegionsDialog::resetToBuiltin);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    loadRegionsFromJson(regionsFilePath());
    refreshTableFromRegions();
    m_preview->regions = m_regions;
    m_preview->dpadBaseWidth = m_dpadBaseWidth;
    m_preview->dpadBaseHeight = m_dpadBaseHeight;
    m_preview->dpadTipLength = m_dpadTipLength;
    m_preview->update();
}

void ControllerRegionsDialog::updateDpadPreview()
{
    m_dpadBaseWidth = m_dpadBaseWidthSpin->value();
    m_dpadBaseHeight = m_dpadBaseHeightSpin->value();
    m_dpadTipLength = m_dpadTipSpin->value();
    if (m_preview) {
        m_preview->dpadBaseWidth = m_dpadBaseWidth;
        m_preview->dpadBaseHeight = m_dpadBaseHeight;
        m_preview->dpadTipLength = m_dpadTipLength;
        m_preview->update();
    }
}

void ControllerRegionsDialog::onSelectionChanged()
{
    int row = m_table->currentRow();
    m_preview->selectedIndex = (row >= 0 && row < 16) ? row : -1;
    m_preview->regions = m_regions;
    m_preview->update();
}

void ControllerRegionsDialog::onCellChanged(int row, int column)
{
    if (m_ignoreCellChange || row < 0 || row >= 16 || column < 2 || column > 6) return;
    QTableWidgetItem *it = m_table->item(row, column);
    if (!it) return;
    bool ok = false;
    qreal v = it->text().toDouble(&ok);
    if (!ok) return;
    m_ignoreCellChange = true;
    ControllerRegionData &r = m_regions[row];
    switch (column) {
    case 2: r.x = v; break;
    case 3: r.y = v; break;
    case 4: r.w = v; break;
    case 5: r.h = v; break;
    case 6: r.r = v; break;
    }
    m_ignoreCellChange = false;
    m_preview->regions = m_regions;
    m_preview->update();
}

void ControllerRegionsDialog::loadFromFile()
{
    loadRegionsFromJson(regionsFilePath());
    if (m_table) refreshTableFromRegions();
    if (m_preview) { m_preview->regions = m_regions; m_preview->update(); }
}

void ControllerRegionsDialog::saveToFile()
{
    refreshRegionsFromTable();
    if (saveRegionsToJson(regionsFilePath()))
        QMessageBox::information(this, tr("Saved"), tr("Saved to %1").arg(regionsFilePath()));
    else
        QMessageBox::warning(this, tr("Error"), tr("Could not write %1").arg(regionsFilePath()));
}

void ControllerRegionsDialog::resetToBuiltin()
{
    m_regions = builtinRegions();
    m_dpadBaseWidth = 40.0;
    m_dpadBaseHeight = 40.0;
    m_dpadTipLength = 40.0;
    if (m_dpadBaseWidthSpin) m_dpadBaseWidthSpin->setValue(40);
    if (m_dpadBaseHeightSpin) m_dpadBaseHeightSpin->setValue(40);
    if (m_dpadTipSpin) m_dpadTipSpin->setValue(40);
    refreshTableFromRegions();
    if (m_preview) {
        m_preview->regions = m_regions;
        m_preview->dpadBaseWidth = m_dpadBaseWidth;
        m_preview->dpadBaseHeight = m_dpadBaseHeight;
        m_preview->dpadTipLength = m_dpadTipLength;
        m_preview->update();
    }
}

void ControllerRegionsDialog::loadRegionsFromJson(const QString &path)
{
    if (m_regions.size() < 16)
        m_regions = builtinRegions();
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return;
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
    QJsonObject root = doc.isObject() ? doc.object() : QJsonObject();
    QJsonArray arr = doc.isArray() ? doc.array() : root.value("regions").toArray();
    QVector<ControllerRegionData> builtin = builtinRegions();
    if (arr.size() >= 16 && m_regions.size() >= 16) {
        for (int i = 0; i < 16; ++i) {
            QJsonObject o = arr.at(i).toObject();
            ControllerRegionData reg;
            reg.x = o.value("x").toDouble(builtin[i].x);
            reg.y = o.value("y").toDouble(builtin[i].y);
            reg.w = o.value("w").toDouble(builtin[i].w);
            reg.h = o.value("h").toDouble(builtin[i].h);
            reg.r = o.value("r").toDouble(builtin[i].r);
            m_regions.replace(i, reg);
        }
        qreal baseSize = qBound(1.0, root.value("dpadBaseSize").toDouble(40.0), 200.0);
        m_dpadBaseWidth = qBound(1.0, root.value("dpadBaseWidth").toDouble(baseSize), 200.0);
        m_dpadBaseHeight = qBound(1.0, root.value("dpadBaseHeight").toDouble(baseSize), 200.0);
        m_dpadTipLength = qBound(1.0, root.value("dpadTipLength").toDouble(40.0), 200.0);
        if (m_dpadBaseWidthSpin) m_dpadBaseWidthSpin->setValue(static_cast<int>(m_dpadBaseWidth));
        if (m_dpadBaseHeightSpin) m_dpadBaseHeightSpin->setValue(static_cast<int>(m_dpadBaseHeight));
        if (m_dpadTipSpin) m_dpadTipSpin->setValue(static_cast<int>(m_dpadTipLength));
        if (m_preview) {
            m_preview->dpadBaseWidth = m_dpadBaseWidth;
            m_preview->dpadBaseHeight = m_dpadBaseHeight;
            m_preview->dpadTipLength = m_dpadTipLength;
        }
    }
}

bool ControllerRegionsDialog::saveRegionsToJson(const QString &path)
{
    refreshRegionsFromTable();
    if (m_dpadBaseWidthSpin) m_dpadBaseWidth = m_dpadBaseWidthSpin->value();
    if (m_dpadBaseHeightSpin) m_dpadBaseHeight = m_dpadBaseHeightSpin->value();
    if (m_dpadTipSpin) m_dpadTipLength = m_dpadTipSpin->value();
    QJsonArray arr;
    for (const ControllerRegionData &r : m_regions) {
        QJsonObject o;
        o.insert("x", r.x);
        o.insert("y", r.y);
        o.insert("w", r.w);
        o.insert("h", r.h);
        o.insert("r", r.r);
        arr.append(o);
    }
    QJsonObject root;
    root.insert("regions", arr);
    root.insert("dpadBaseWidth", static_cast<int>(m_dpadBaseWidth));
    root.insert("dpadBaseHeight", static_cast<int>(m_dpadBaseHeight));
    root.insert("dpadTipLength", static_cast<int>(m_dpadTipLength));
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) return false;
    f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

void ControllerRegionsDialog::refreshTableFromRegions()
{
    if (!m_table || m_regions.size() < 16) return;
    m_ignoreCellChange = true;
    for (int i = 0; i < 16; ++i) {
        const ControllerRegionData &r = m_regions.at(i);
        if (m_table->item(i, 2)) m_table->item(i, 2)->setText(QString::number(r.x));
        if (m_table->item(i, 3)) m_table->item(i, 3)->setText(QString::number(r.y));
        if (m_table->item(i, 4)) m_table->item(i, 4)->setText(QString::number(r.w));
        if (m_table->item(i, 5)) m_table->item(i, 5)->setText(QString::number(r.h));
        if (m_table->item(i, 6)) m_table->item(i, 6)->setText(QString::number(r.r));
    }
    m_ignoreCellChange = false;
}

void ControllerRegionsDialog::refreshRegionsFromTable()
{
    if (!m_table || m_regions.size() < 16) return;
    for (int i = 0; i < 16; ++i) {
        ControllerRegionData &r = m_regions[i];
        if (m_table->item(i, 2)) r.x = m_table->item(i, 2)->text().toDouble();
        if (m_table->item(i, 3)) r.y = m_table->item(i, 3)->text().toDouble();
        if (m_table->item(i, 4)) r.w = m_table->item(i, 4)->text().toDouble();
        if (m_table->item(i, 5)) r.h = m_table->item(i, 5)->text().toDouble();
        if (m_table->item(i, 6)) r.r = m_table->item(i, 6)->text().toDouble();
    }
}
