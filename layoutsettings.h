#ifndef LAYOUTSETTINGS_H
#define LAYOUTSETTINGS_H

#include <QString>
#include <QStringList>
#include <QColor>

class LayoutSettings
{
public:
    LayoutSettings();

    QString lastLayoutPath() const;
    void setLastLayoutPath(const QString &path);

    int lastTabIndex() const;
    void setLastTabIndex(int index);

    QStringList recentLayouts(int maxCount = 5) const;
    void addRecentLayout(const QString &path);
    void removeRecentLayout(const QString &path);

    void updateLastAndRecent(const QString &path);
    void save();

    // Visualization colors (global)
    QColor keyColor() const;
    void setKeyColor(const QColor &color);
    QColor highlightColor() const;
    void setHighlightColor(const QColor &color);
    QColor backgroundColor() const;
    void setBackgroundColor(const QColor &color);
    QColor textColor() const;
    void setTextColor(const QColor &color);
    QColor highlightedTextColor() const;
    void setHighlightedTextColor(const QColor &color);

private:
    void load();

    QString m_lastLayoutPath;
    int m_lastTabIndex = 1;
    QStringList m_recentLayouts;
    static const int kMaxRecent = 5;

    QColor m_keyColor;
    QColor m_highlightColor;
    QColor m_backgroundColor;
    QColor m_textColor;
    QColor m_highlightedTextColor;
};

#endif // LAYOUTSETTINGS_H
