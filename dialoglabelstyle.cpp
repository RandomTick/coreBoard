#include "dialoglabelstyle.h"
#include "keystyle.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QColorDialog>
#include <QFontDatabase>
#include <QListView>
#include <QStyledItemDelegate>
#include <QStyle>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QAbstractItemView>

class LabelFontPreviewDelegate : public QStyledItemDelegate {
public:
    explicit LabelFontPreviewDelegate(const QString &previewLabel, QObject *parent = nullptr)
        : QStyledItemDelegate(parent), m_previewLabel(previewLabel.isEmpty() ? QStringLiteral("Sample 123") : previewLabel) {}
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        if (opt.widget)
            opt.widget->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);
        QString family = index.data(Qt::UserRole).toString();
        QString displayName = index.data(Qt::DisplayRole).toString();
        QRect textRect = opt.rect.adjusted(6, 2, -6, -2);
        painter->setPen(opt.palette.color(QPalette::Text));
        QFont f = painter->font();
        f.setPointSize(9);
        painter->setFont(f);
        QRect labelRect = textRect;
        labelRect.setWidth(qMin(140, textRect.width() / 2));
        painter->drawText(labelRect, Qt::AlignLeft | Qt::AlignVCenter,
                         opt.fontMetrics.horizontalAdvance(displayName) > labelRect.width()
                             ? opt.fontMetrics.elidedText(displayName, Qt::ElideRight, labelRect.width())
                             : displayName);
        QFont previewFont;
        if (!family.isEmpty()) previewFont.setFamily(family);
        previewFont.setPointSize(14);
        painter->setFont(previewFont);
        QRect sampleRect = textRect;
        sampleRect.setLeft(labelRect.right() + 8);
        painter->drawText(sampleRect, Qt::AlignLeft | Qt::AlignVCenter, m_previewLabel);
    }
    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override {
        return QSize(200, 36);
    }
private:
    QString m_previewLabel;
};

DialogLabelStyle::DialogLabelStyle(QWidget *parent, const KeyStyle &currentStyle, const QString &previewLabel)
    : QDialog(parent)
    , m_previewLabel(previewLabel)
{
    setWindowTitle(tr("Edit label style"));
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *form = new QFormLayout();

    m_textColor = currentStyle.keyTextColor.isValid() ? currentStyle.keyTextColor : QColor(Qt::white);
    m_textColorButton = new QPushButton(this);
    updateTextColorButton();
    connect(m_textColorButton, &QPushButton::clicked, this, [this]() {
        QColor c = QColorDialog::getColor(m_textColor.isValid() ? m_textColor : Qt::white, this, tr("Text color"));
        if (c.isValid()) { m_textColor = c; updateTextColorButton(); }
    });
    form->addRow(tr("Text color:"), m_textColorButton);

    m_fontSizeSpin = new QSpinBox(this);
    m_fontSizeSpin->setRange(6, 72);
    m_fontSizeSpin->setValue(currentStyle.fontPointSize > 0 ? currentStyle.fontPointSize : 10);
    form->addRow(tr("Font size:"), m_fontSizeSpin);

    QListView *fontListView = new QListView(this);
    fontListView->setItemDelegate(new LabelFontPreviewDelegate(m_previewLabel, fontListView));
    fontListView->setUniformItemSizes(true);
    fontListView->setMinimumHeight(180);
    fontListView->setMinimumWidth(260);
    fontListView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_fontCombo = new QComboBox(this);
    m_fontCombo->setMinimumWidth(260);
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

void DialogLabelStyle::updateTextColorButton() {
    if (m_textColor.isValid()) {
        QColor fg = (m_textColor.value() > 128) ? Qt::black : Qt::white;
        m_textColorButton->setStyleSheet(QString("background-color: %1; color: %2; border: 1px solid #888; min-width: 80px;")
            .arg(m_textColor.name()).arg(fg.name()));
        m_textColorButton->setText(m_textColor.name());
    } else {
        m_textColorButton->setStyleSheet("background-color: #ddd; color: #666; border: 1px solid #888; min-width: 80px;");
        m_textColorButton->setText(tr("Default"));
    }
}

KeyStyle DialogLabelStyle::getStyle() const {
    KeyStyle s;
    s.fontPointSize = m_fontSizeSpin->value();
    s.fontFamily = m_fontCombo->currentData().toString().trimmed();
    s.keyTextColor = m_textColor;
    return s;
}
