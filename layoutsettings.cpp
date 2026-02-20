#include "layoutsettings.h"
#include <QSettings>
#include <QFileInfo>

static const char kOrg[] = "CoreBoard";
static const char kApp[] = "CoreBoard";
static const char kLastLayoutPathKey[] = "lastLayoutPath";
static const char kLastTabIndexKey[] = "lastTabIndex";
static const char kRecentGroup[] = "recentLayouts";
static const char kKeyColorKey[] = "keyColor";
static const char kHighlightColorKey[] = "highlightColor";
static const char kBackgroundColorKey[] = "backgroundColor";
static const char kTextColorKey[] = "textColor";
static const char kHighlightedTextColorKey[] = "highlightedTextColor";
static const char kLabelModeKey[] = "labelMode";

LayoutSettings::LayoutSettings()
{
    load();
}

QString LayoutSettings::lastLayoutPath() const
{
    return m_lastLayoutPath;
}

void LayoutSettings::setLastLayoutPath(const QString &path)
{
    m_lastLayoutPath = path;
}

int LayoutSettings::lastTabIndex() const
{
    return m_lastTabIndex;
}

void LayoutSettings::setLastTabIndex(int index)
{
    m_lastTabIndex = index;
}

QStringList LayoutSettings::recentLayouts(int maxCount) const
{
    if (maxCount <= 0 || maxCount > m_recentLayouts.size())
        return m_recentLayouts;
    return m_recentLayouts.mid(0, maxCount);
}

void LayoutSettings::addRecentLayout(const QString &path)
{
    if (path.isEmpty())
        return;
    m_recentLayouts.removeAll(path);
    m_recentLayouts.prepend(path);
    while (m_recentLayouts.size() > kMaxRecent)
        m_recentLayouts.removeLast();
}

void LayoutSettings::removeRecentLayout(const QString &path)
{
    if (path.isEmpty())
        return;
    m_recentLayouts.removeAll(path);
    if (m_lastLayoutPath == path)
        m_lastLayoutPath = m_recentLayouts.isEmpty() ? QString() : m_recentLayouts.first();
    save();
}

void LayoutSettings::updateLastAndRecent(const QString &path)
{
    if (path.isEmpty())
        return;
    m_lastLayoutPath = path;
    addRecentLayout(path);
    save();
}

QColor LayoutSettings::keyColor() const
{
    return m_keyColor;
}

void LayoutSettings::setKeyColor(const QColor &color)
{
    m_keyColor = color;
}

QColor LayoutSettings::highlightColor() const
{
    return m_highlightColor;
}

void LayoutSettings::setHighlightColor(const QColor &color)
{
    m_highlightColor = color;
}

QColor LayoutSettings::backgroundColor() const
{
    return m_backgroundColor;
}

void LayoutSettings::setBackgroundColor(const QColor &color)
{
    m_backgroundColor = color;
}

QColor LayoutSettings::textColor() const
{
    return m_textColor;
}

void LayoutSettings::setTextColor(const QColor &color)
{
    m_textColor = color;
}

QColor LayoutSettings::highlightedTextColor() const
{
    return m_highlightedTextColor;
}

void LayoutSettings::setHighlightedTextColor(const QColor &color)
{
    m_highlightedTextColor = color;
}

LabelMode LayoutSettings::labelMode() const
{
    return m_labelMode;
}

void LayoutSettings::setLabelMode(LabelMode mode)
{
    m_labelMode = mode;
}

void LayoutSettings::save()
{
    QSettings s(kOrg, kApp);
    s.setValue(kLastLayoutPathKey, m_lastLayoutPath);
    s.setValue(kLastTabIndexKey, m_lastTabIndex);
    s.setValue(kKeyColorKey, m_keyColor.name(QColor::HexArgb));
    s.setValue(kHighlightColorKey, m_highlightColor.name(QColor::HexArgb));
    s.setValue(kBackgroundColorKey, m_backgroundColor.name(QColor::HexArgb));
    s.setValue(kTextColorKey, m_textColor.name(QColor::HexArgb));
    s.setValue(kHighlightedTextColorKey, m_highlightedTextColor.name(QColor::HexArgb));
    s.setValue(kLabelModeKey, static_cast<int>(m_labelMode));
    s.beginWriteArray(kRecentGroup);
    for (int i = 0; i < m_recentLayouts.size(); ++i) {
        s.setArrayIndex(i);
        s.setValue("path", m_recentLayouts.at(i));
    }
    s.endArray();
}

void LayoutSettings::load()
{
    QSettings s(kOrg, kApp);
    m_lastLayoutPath = s.value(kLastLayoutPathKey).toString();
    m_lastTabIndex = s.value(kLastTabIndexKey, 1).toInt();
    if (m_lastTabIndex < 0 || m_lastTabIndex > 1)
        m_lastTabIndex = 1;
    m_keyColor = QColor(s.value(kKeyColorKey).toString());
    if (!m_keyColor.isValid())
        m_keyColor = QColor(0, 0, 255);
    m_highlightColor = QColor(s.value(kHighlightColorKey).toString());
    if (!m_highlightColor.isValid())
        m_highlightColor = Qt::red;
    m_backgroundColor = QColor(s.value(kBackgroundColorKey).toString());
    if (!m_backgroundColor.isValid())
        m_backgroundColor = QColor(53, 53, 53); // dark gray default
    m_textColor = QColor(s.value(kTextColorKey).toString());
    if (!m_textColor.isValid())
        m_textColor = Qt::white;
    m_highlightedTextColor = QColor(s.value(kHighlightedTextColorKey).toString());
    if (!m_highlightedTextColor.isValid())
        m_highlightedTextColor = Qt::black;
    int modeInt = s.value(kLabelModeKey, 0).toInt();
    if (modeInt >= 0 && modeInt <= 2)
        m_labelMode = static_cast<LabelMode>(modeInt);
    else {
        // Backward compatibility: old showLabelsByShift
        bool oldShow = s.value("showLabelsByShift", true).toBool();
        m_labelMode = oldShow ? LabelMode::FollowCapsAndShift : LabelMode::AllLowercase;
    }
    m_recentLayouts.clear();
    int n = s.beginReadArray(kRecentGroup);
    for (int i = 0; i < n; ++i) {
        s.setArrayIndex(i);
        QString p = s.value("path").toString();
        if (!p.isEmpty())
            m_recentLayouts.append(p);
    }
    s.endArray();
}
