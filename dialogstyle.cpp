#include "dialogstyle.h"
#include "keystyle.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QColorDialog>
#include <QFontDatabase>
#include <QPainter>
#include <QStyle>

static const int kFontPreviewRowHeight = 36;
static const int kFontPreviewPointSize = 14;
static const char kFontSampleText[] = "Sample 123";

class FontPreviewDelegate : public QStyledItemDelegate {
public:
    explicit FontPreviewDelegate(const QString &previewLabel, QObject *parent = nullptr)
        : QStyledItemDelegate(parent), m_previewLabel(previewLabel) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        if (opt.widget)
            opt.widget->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

        QString family = index.data(Qt::UserRole).toString();
        QString displayName = index.data(Qt::DisplayRole).toString();
        QRect textRect = opt.rect.adjusted(6, 2, -6, -2);

        painter->setPen(opt.palette.color(QPalette::Text));

        QFont labelFont = painter->font();
        labelFont.setPointSize(9);
        painter->setFont(labelFont);
        QRect labelRect = textRect;
        labelRect.setWidth(qMin(140, textRect.width() / 2));
        painter->drawText(labelRect, Qt::AlignLeft | Qt::AlignVCenter,
                         opt.fontMetrics.horizontalAdvance(displayName) > labelRect.width()
                             ? opt.fontMetrics.elidedText(displayName, Qt::ElideRight, labelRect.width())
                             : displayName);

        QFont previewFont;
        if (!family.isEmpty()) {
            previewFont.setFamily(family);
            previewFont.setPointSize(kFontPreviewPointSize);
        } else {
            previewFont = painter->font();
            previewFont.setPointSize(kFontPreviewPointSize);
        }
        painter->setFont(previewFont);
        QRect sampleRect = textRect;
        sampleRect.setLeft(labelRect.right() + 8);
        QString previewText = m_previewLabel.isEmpty() ? QString::fromUtf8(kFontSampleText) : m_previewLabel;
        painter->drawText(sampleRect, Qt::AlignLeft | Qt::AlignVCenter, previewText);
    }

    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override {
        return QSize(200, kFontPreviewRowHeight);
    }
private:
    QString m_previewLabel;
};

DialogStyle::DialogStyle(QWidget *parent, const KeyStyle &currentStyle, const QString &previewLabel, bool isRectangle)
    : QDialog(parent)
    , m_outlineColor(currentStyle.outlineColor)
    , m_previewLabel(previewLabel)
{
    setWindowTitle(tr("Edit style"));
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QFormLayout *form = new QFormLayout();

    m_outlineColorButton = new QPushButton(tr("Outline color..."), this);
    m_outlineColorButton->setStyleSheet(QString("background-color: %1").arg(m_outlineColor.name()));
    connect(m_outlineColorButton, &QPushButton::clicked, this, &DialogStyle::chooseOutlineColor);
    form->addRow(tr("Outline color:"), m_outlineColorButton);

    m_outlineWidthSpin = new QDoubleSpinBox(this);
    m_outlineWidthSpin->setRange(0.5, 20.0);
    m_outlineWidthSpin->setSingleStep(0.5);
    m_outlineWidthSpin->setValue(currentStyle.outlineWidth);
    form->addRow(tr("Outline width (px):"), m_outlineWidthSpin);

    if (isRectangle) {
        m_cornerRadiusSpin = new QDoubleSpinBox(this);
        m_cornerRadiusSpin->setRange(0, 999);
        m_cornerRadiusSpin->setSingleStep(1);
        m_cornerRadiusSpin->setValue(currentStyle.cornerRadius >= 0 ? currentStyle.cornerRadius : 0);
        form->addRow(tr("Corner radius (px):"), m_cornerRadiusSpin);
    }

    m_fontSizeSpin = new QSpinBox(this);
    m_fontSizeSpin->setRange(6, 72);
    m_fontSizeSpin->setValue(currentStyle.fontPointSize > 0 ? currentStyle.fontPointSize : 10);
    form->addRow(tr("Font size:"), m_fontSizeSpin);

    m_fontBoldCheck = new QCheckBox(this);
    m_fontBoldCheck->setChecked(currentStyle.fontBold);
    form->addRow(tr("Bold:"), m_fontBoldCheck);

    m_fontItalicCheck = new QCheckBox(this);
    m_fontItalicCheck->setChecked(currentStyle.fontItalic);
    form->addRow(tr("Italic:"), m_fontItalicCheck);

    QListView *fontListView = new QListView(this);
    fontListView->setItemDelegate(new FontPreviewDelegate(m_previewLabel, fontListView));
    fontListView->setUniformItemSizes(true);
    fontListView->setMinimumHeight(220);
    fontListView->setMinimumWidth(280);
    fontListView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_fontCombo = new QComboBox(this);
    m_fontCombo->setMinimumWidth(280);
    m_fontCombo->setView(fontListView);
    m_fontCombo->addItem(tr("(Default)"), QString());
    for (const QString &family : QFontDatabase::families())
        m_fontCombo->addItem(family, family);
    if (currentStyle.fontFamily.isEmpty()) {
        m_fontCombo->setCurrentIndex(0);
    } else {
        int idx = m_fontCombo->findData(currentStyle.fontFamily);
        if (idx >= 0)
            m_fontCombo->setCurrentIndex(idx);
        else {
            m_fontCombo->addItem(currentStyle.fontFamily, currentStyle.fontFamily);
            m_fontCombo->setCurrentIndex(m_fontCombo->count() - 1);
        }
    }
    form->addRow(tr("Font family:"), m_fontCombo);

    mainLayout->addLayout(form);

    QHBoxLayout *buttons = new QHBoxLayout();
    QPushButton *cancelBtn = new QPushButton(tr("Cancel"), this);
    QPushButton *applyBtn = new QPushButton(tr("Apply"), this);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(applyBtn, &QPushButton::clicked, this, &QDialog::accept);
    buttons->addWidget(cancelBtn);
    buttons->addWidget(applyBtn);
    mainLayout->addLayout(buttons);
}

void DialogStyle::chooseOutlineColor() {
    QColor c = QColorDialog::getColor(m_outlineColor, this, tr("Outline color"));
    if (c.isValid()) {
        m_outlineColor = c;
        updateOutlineColorButton();
    }
}

void DialogStyle::updateOutlineColorButton() {
    m_outlineColorButton->setStyleSheet(QString("background-color: %1").arg(m_outlineColor.name()));
}

KeyStyle DialogStyle::getStyle() const {
    KeyStyle s;
    s.outlineColor = m_outlineColor;
    s.outlineWidth = m_outlineWidthSpin->value();
    s.cornerRadius = m_cornerRadiusSpin ? m_cornerRadiusSpin->value() : 0;
    s.fontPointSize = m_fontSizeSpin->value();
    s.fontBold = m_fontBoldCheck->isChecked();
    s.fontItalic = m_fontItalicCheck->isChecked();
    s.fontFamily = m_fontCombo->currentData().toString().trimmed();
    return s;
}
