#include <QMouseEvent>
#include <QGraphicsItem>
#include "layouteditor.h"
#include "mainwindow.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QFileDialog>
#include <QJsonArray>
#include <QMenu>
#include <QFileInfo>
#include <QMessageBox>
#include <QShortcut>
#include <QApplication>
#include <QClipboard>
#include <cmath>
#include "layoutsettings.h"
#include "keystyle.h"

// Minimum width so toolbar buttons (Open, New, Save, etc.) stay visible
static const int kMinimumEditorWidth = 720;

namespace {
// Preset polygon templates in 0-100 unit space (centered, fits in 100x100)
QPolygonF starTemplate() {
    QPolygonF p;
    const int n = 5;
    const qreal cx = 50, cy = 50, outer = 48, inner = 20;
    for (int i = 0; i < 2 * n; ++i) {
        qreal r = (i % 2 == 0) ? outer : inner;
        qreal angle = M_PI / 2 + 2 * M_PI * i / (2 * n);
        p << QPointF(cx + r * std::cos(angle), cy - r * std::sin(angle));
    }
    return p;
}
QPolygonF diamondTemplate() {
    return QPolygonF() << QPointF(50, 2) << QPointF(98, 50) << QPointF(50, 98) << QPointF(2, 50);
}
QPolygonF hexagonTemplate() {
    QPolygonF p;
    const qreal cx = 50, cy = 50, r = 48;
    for (int i = 0; i < 6; ++i) {
        qreal angle = M_PI / 6 + 2 * M_PI * i / 6;
        p << QPointF(cx + r * std::cos(angle), cy - r * std::sin(angle));
    }
    return p;
}
QPolygonF triangleTemplate() {
    QPolygonF p;
    const qreal cx = 50, cy = 50, r = 48;
    for (int i = 0; i < 3; ++i) {
        qreal angle = M_PI / 2 + 2 * M_PI * i / 3;
        p << QPointF(cx + r * std::cos(angle), cy - r * std::sin(angle));
    }
    return p;
}
}

LayoutEditor::LayoutEditor(QWidget *parent) : QWidget(parent)
{

    view = new LayoutEditorGraphicsView(this);
    scene = new QGraphicsScene(view);
    view->setSceneAndStore(scene);
    view->setMouseTracking(true);

    //connect translators
    MainWindow* myParentMainWindow = qobject_cast<MainWindow*>(this->parentWidget());
    connect(myParentMainWindow, &MainWindow::languageChanged, this, &LayoutEditor::updateLanguage);



    // Initialize the scene's size
    QRectF newRect = QRectF(QPointF(0, 0), QSizeF(view->viewport()->size()));
    scene->setSceneRect(newRect);

    // Initialize buttons
    QString buttonStyleSheet = "QPushButton {"
                               "background-color: transparent;"
                               "}"
                               "QPushButton:hover {"
                               "background-color: rgba(150, 51, 150, 0.4);"
                               "}"
                               "QPushButton:checked {"
                               "background-color: rgba(150, 51, 150, 0.7);"
                               "}";

    openButton = new QPushButton(tr("Open Layout"), this);
    openButton->setStyleSheet(buttonStyleSheet);

    newButton = new QPushButton(tr("New Layout"), this);
    newButton->setStyleSheet(buttonStyleSheet);

    undoButton = new QPushButton("", this);
    undoButton->setIcon(QIcon(":/icons/undo.png"));
    undoButton->setIconSize(QSize(18,18));
    undoButton->setFixedSize(24,24);
    undoButton->setStyleSheet(buttonStyleSheet);
    undoButton->setToolTip(tr("Undo (Ctrl+Z)"));
    undoButton->setEnabled(false);

    redoButton = new QPushButton("", this);
    redoButton->setIcon(QIcon(":/icons/redo.png"));
    redoButton->setIconSize(QSize(18,18));
    redoButton->setFixedSize(24,24);
    redoButton->setStyleSheet(buttonStyleSheet);
    redoButton->setToolTip(tr("Redo (Ctrl+Y)"));
    redoButton->setEnabled(false);

    QShortcut *undoShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z), this);
    connect(undoShortcut, &QShortcut::activated, view, &LayoutEditorGraphicsView::undoLastAction);
    QShortcut *redoShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y), this);
    connect(redoShortcut, &QShortcut::activated, view, &LayoutEditorGraphicsView::redoLastAction);

    addButton = new QPushButton(tr("Add Shape"), this);
    addButton->setStyleSheet(buttonStyleSheet);

    copyButton = new QPushButton(this);
    copyButton->setCheckable(true);
    copyButton->setIcon(QIcon(":/icons/content_copy.svg"));
    copyButton->setIconSize(QSize(18, 18));
    copyButton->setFixedSize(24, 24);
    copyButton->setStyleSheet(buttonStyleSheet);
    copyButton->setToolTip(tr("Copy key (click, then click a key to copy)"));

    pasteButton = new QPushButton(this);
    pasteButton->setCheckable(false);
    pasteButton->setIcon(QIcon(":/icons/content_paste.svg"));
    pasteButton->setIconSize(QSize(18, 18));
    pasteButton->setFixedSize(24, 24);
    pasteButton->setStyleSheet(buttonStyleSheet);
    pasteButton->setToolTip(tr("Paste key"));
    pasteButton->setEnabled(true);

    pickStyleButton = new QPushButton(this);
    pickStyleButton->setCheckable(true);
    pickStyleButton->setIcon(QIcon(":/icons/pick_style.svg"));
    pickStyleButton->setIconSize(QSize(18, 18));
    pickStyleButton->setFixedSize(24, 24);
    pickStyleButton->setStyleSheet(buttonStyleSheet);
    pickStyleButton->setToolTip(tr("Pick style from a key"));

    applyStyleButton = new QPushButton(this);
    applyStyleButton->setCheckable(true);
    applyStyleButton->setIcon(QIcon(":/icons/apply_style.svg"));
    applyStyleButton->setIconSize(QSize(18, 18));
    applyStyleButton->setFixedSize(24, 24);
    applyStyleButton->setStyleSheet(buttonStyleSheet);
    applyStyleButton->setToolTip(tr("Apply picked style to key(s)"));
    applyStyleButton->setEnabled(false);

    m_addShapeMenu = new QMenu(this);
    m_actRect = m_addShapeMenu->addAction(tr("Rectangle"));
    m_actCircle = m_addShapeMenu->addAction(tr("Circle"));
    m_addShapeMenu->addSeparator();
    m_customShapeMenu = m_addShapeMenu->addMenu(tr("Custom shape"));
    m_actStar = m_customShapeMenu->addAction(tr("Star"));
    m_actDiamond = m_customShapeMenu->addAction(tr("Diamond"));
    m_actHexagon = m_customShapeMenu->addAction(tr("Hexagon"));
    m_actTriangle = m_customShapeMenu->addAction(tr("Triangle"));
    addButton->setMenu(m_addShapeMenu);

    saveButton = new QPushButton(tr("Save"), this);
    saveButton->setStyleSheet(buttonStyleSheet);

    saveAsButton = new QPushButton(tr("Save As"), this);
    saveAsButton->setStyleSheet(buttonStyleSheet);

    // Connect button signals to slots
    connect(openButton, &QPushButton::clicked, this, &LayoutEditor::loadLayoutButton);
    connect(newButton, &QPushButton::clicked, this, &LayoutEditor::newLayout);
    connect(undoButton, &QPushButton::clicked, view, &LayoutEditorGraphicsView::undoLastAction);
    connect(redoButton, &QPushButton::clicked, view, &LayoutEditorGraphicsView::redoLastAction);
    connect(m_actRect, &QAction::triggered, this, &LayoutEditor::addShape);
    connect(m_actCircle, &QAction::triggered, this, [this]() {
        ResizableEllipseItem *ellipse = addEllipse("", 100, 100, 100, 100);
        view->addRectAction(ellipse);
    });
    connect(m_actStar, &QAction::triggered, this, [this]() {
        ResizablePolygonItem *poly = addPolygon(starTemplate(), "", 100, 100, 100, 100);
        view->addRectAction(poly);
    });
    connect(m_actDiamond, &QAction::triggered, this, [this]() {
        ResizablePolygonItem *poly = addPolygon(diamondTemplate(), "", 100, 100, 100, 100);
        view->addRectAction(poly);
    });
    connect(m_actHexagon, &QAction::triggered, this, [this]() {
        ResizablePolygonItem *poly = addPolygon(hexagonTemplate(), "", 100, 100, 100, 100);
        view->addRectAction(poly);
    });
    connect(m_actTriangle, &QAction::triggered, this, [this]() {
        ResizablePolygonItem *poly = addPolygon(triangleTemplate(), "", 100, 100, 100, 100);
        view->addRectAction(poly);
    });
    connect(saveButton, &QPushButton::clicked, this, &LayoutEditor::saveLayout);
    connect(saveAsButton, &QPushButton::clicked, this, &LayoutEditor::saveLayoutAs);
    connect(pickStyleButton, &QPushButton::toggled, this, &LayoutEditor::onPickStyleToggled);
    connect(applyStyleButton, &QPushButton::toggled, this, &LayoutEditor::onApplyStyleClicked);
    connect(view, &LayoutEditorGraphicsView::stylePicked, this, [this]() {
        pickStyleButton->blockSignals(true);
        pickStyleButton->setChecked(false);
        pickStyleButton->blockSignals(false);
        applyStyleButton->setEnabled(view->hasPickedStyle());
    });
    connect(view, &LayoutEditorGraphicsView::applyStyleModeExited, this, [this]() {
        pickStyleButton->blockSignals(true);
        pickStyleButton->setChecked(false);
        pickStyleButton->blockSignals(false);
        applyStyleButton->blockSignals(true);
        applyStyleButton->setChecked(false);
        applyStyleButton->blockSignals(false);
        copyButton->blockSignals(true);
        copyButton->setChecked(false);
        copyButton->blockSignals(false);
    });
    connect(view, &LayoutEditorGraphicsView::keyCopied, this, [this]() {
        copyButton->blockSignals(true);
        copyButton->setChecked(false);
        copyButton->blockSignals(false);
    });
    connect(copyButton, &QPushButton::toggled, this, &LayoutEditor::onCopyToggled);
    connect(pasteButton, &QPushButton::clicked, this, &LayoutEditor::pasteKey);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(openButton);
    buttonLayout->addWidget(newButton);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(saveAsButton);
    buttonLayout->addWidget(undoButton);
    buttonLayout->addWidget(redoButton);
    buttonLayout->addWidget(copyButton);
    buttonLayout->addWidget(pasteButton);
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(pickStyleButton);
    buttonLayout->addWidget(applyStyleButton);
    buttonLayout->addStretch(1);


    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addLayout(buttonLayout);
    layout->addWidget(view);

    setLayout(layout);

    //addRectangle("T",150,150,50,50);
    //addRectangle("S",44,77,200,300);
}

void LayoutEditor::updateButtons(bool undoCommandsExist, bool redoCommandsExist){
    undoButton->setEnabled(undoCommandsExist);
    redoButton->setEnabled(redoCommandsExist);
}

void LayoutEditor::setLayoutSettings(LayoutSettings *settings)
{
    m_layoutSettings = settings;
}

void LayoutEditor::markDirty()
{
    m_dirty = true;
}



void LayoutEditor::newLayout(){
    view->clearAlignmentHelpers();
    scene->clear();
    view->clearUndoRedo();
    _currentLayoutPath.clear();
    m_dirty = false;
    updateButtons(false, false);
}

void LayoutEditor::loadLayoutButton(){
    QMenu menu(this);
    QAction *openAction = menu.addAction(tr("Open..."));
    openAction->setData(QString());
    QString initialDir;
    if (m_layoutSettings && !m_layoutSettings->lastLayoutPath().isEmpty())
        initialDir = QFileInfo(m_layoutSettings->lastLayoutPath()).absolutePath();
    QStringList recent = m_layoutSettings ? m_layoutSettings->recentLayouts(5) : QStringList();
    for (const QString &path : recent) {
        QAction *a = menu.addAction(QFileInfo(path).fileName());
        a->setData(path);
    }
    if (recent.isEmpty())
        menu.addAction(tr("No recent files"))->setEnabled(false);
    QAction *chosen = menu.exec(openButton->mapToGlobal(QPoint(0, openButton->height())));
    if (!chosen)
        return;
    QString path = chosen->data().toString();
    if (path.isEmpty()) {
        QString filePath = QFileDialog::getOpenFileName(this, tr("Select Layout File to Edit"), initialDir, tr("Layout Files (*.json);;All Files (*)"));
        if (!filePath.isEmpty())
            loadLayout(filePath);
        return;
    }
    loadLayout(path);
}


void LayoutEditor::loadLayout(const QString &fileName){
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("Layout not found"),
            tr("The layout file could not be opened. It may have been moved or deleted.\n\n\"%1\"\n\nIt has been removed from the recent list.").arg(fileName));
        if (m_layoutSettings)
            m_layoutSettings->removeRecentLayout(fileName);
        return;
    }

    _currentLayoutPath = fileName;
    m_dirty = false;
    view->clearAlignmentHelpers();
    scene->clear();

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject rootObject = doc.object();
    QJsonArray elements = rootObject.value("Elements").toArray();

    // Compute bounding box from all elements' positions (not just size) so offset from 0,0 is included
    qreal overallMinX = 1e9, overallMinY = 1e9, overallMaxX = -1e9, overallMaxY = -1e9;
    bool hasBounds = false;
    for (const QJsonValue &element : elements) {
        QJsonObject keyData = element.toObject();
        if (keyData.value("__type").toString() != "KeyboardKey")
            continue;
        QJsonArray boundaries = keyData.value("Boundaries").toArray();
        if (boundaries.size() < 4)
            continue;
        for (const QJsonValue &pv : boundaries) {
            QJsonObject po = pv.toObject();
            qreal px = po["X"].toDouble();
            qreal py = po["Y"].toDouble();
            overallMinX = qMin(overallMinX, px);
            overallMinY = qMin(overallMinY, py);
            overallMaxX = qMax(overallMaxX, px);
            overallMaxY = qMax(overallMaxY, py);
            hasBounds = true;
        }
    }

    for (const QJsonValue &element : elements) {
        if (element.toObject().value("__type").toString() == "KeyboardKey") {
            createKey(element.toObject());
        }
    }

    // Minimum size must fit the rightmost and bottommost points of all elements
    const int marginRight = 56;
    const int marginBottom = 80;
    int minW = marginRight;
    int minH = marginBottom;
    if (hasBounds) {
        minW = static_cast<int>(overallMaxX) + marginRight;
        minH = static_cast<int>(overallMaxY) + marginBottom;
    } else {
        int fallbackW = rootObject.value("Width").toInt();
        int fallbackH = rootObject.value("Height").toInt();
        if (fallbackW > 0 || fallbackH > 0) {
            minW = fallbackW + marginRight;
            minH = fallbackH + marginBottom;
        }
    }
    minW = qMax(minW, kMinimumEditorWidth);
    setMinimumSize(minW, minH);

    if (m_layoutSettings && fileName != QLatin1String(":/default.json"))
        m_layoutSettings->updateLastAndRecent(fileName);
    emit layoutLoaded(fileName);
}

QGraphicsItem* LayoutEditor::createKey(const QJsonObject &keyData){
    QJsonArray boundaries = keyData.value("Boundaries").toArray();
    if (boundaries.size() < 4) {
        qWarning("Invalid boundaries data (need at least 4 points).");
        return nullptr;
    }
    QString label = keyData.value("Text").toString();
    QString shiftText = keyData.value("ShiftText").toString();
    if (shiftText.isEmpty())
        shiftText = label;
    KeyStyle keyStyle = KeyStyle::fromJson(keyData);

    QJsonArray kc = keyData.value("KeyCodes").toArray();
    std::list<int> keyCodes = {};
    for (int i = 0; i < kc.count(); i++) {
        keyCodes.push_back(kc.at(i).toInt());
    }

    if (boundaries.size() == 4) {
        qreal minX = 1e9, minY = 1e9, maxX = -1e9, maxY = -1e9;
        QPolygonF fourPoints;
        for (const QJsonValue &pv : boundaries) {
            QJsonObject po = pv.toObject();
            qreal px = po["X"].toDouble();
            qreal py = po["Y"].toDouble();
            fourPoints << QPointF(px, py);
            minX = qMin(minX, px);
            minY = qMin(minY, py);
            maxX = qMax(maxX, px);
            maxY = qMax(maxY, py);
        }
        // Only treat as rectangle if the 4 points form an axis-aligned rectangle (not a trapezoid etc.)
        bool isAxisAlignedRect = true;
        for (const QPointF &p : fourPoints) {
            if ((p.x() != minX && p.x() != maxX) || (p.y() != minY && p.y() != maxY)) {
                isAxisAlignedRect = false;
                break;
            }
        }
        if (isAxisAlignedRect && (maxX - minX) > 0 && (maxY - minY) > 0) {
            qreal x = minX;
            qreal y = minY;
            qreal width = maxX - minX;
            qreal height = maxY - minY;
            ResizableRectItem *rect = addRectangle(label, width, height, x, y, keyCodes, keyStyle);
            rect->setShiftText(shiftText);
            return rect;
        } else {
            qreal w = maxX - minX;
            qreal h = maxY - minY;
            QPolygonF templatePolygon;
            for (const QPointF &p : fourPoints) {
                templatePolygon << QPointF(p.x() - minX, p.y() - minY);
            }
            ResizablePolygonItem *polyItem = addPolygon(templatePolygon, label, minX, minY, w, h, keyCodes, keyStyle);
            polyItem->setShiftText(shiftText);
            return polyItem;
        }
    } else {
        qreal minX = 1e9, minY = 1e9, maxX = -1e9, maxY = -1e9;
        for (const QJsonValue &pv : boundaries) {
            QJsonObject po = pv.toObject();
            qreal px = po["X"].toDouble();
            qreal py = po["Y"].toDouble();
            minX = qMin(minX, px);
            minY = qMin(minY, py);
            maxX = qMax(maxX, px);
            maxY = qMax(maxY, py);
        }
        qreal w = maxX - minX;
        qreal h = maxY - minY;
        // High point count => saved ellipse (we write 64 points for circles); load as ellipse so it stays smooth
        const int ellipsePointThreshold = 32;
        if (boundaries.size() >= ellipsePointThreshold) {
            ResizableEllipseItem *ellipse = addEllipse(label, w, h, minX, minY, keyCodes, keyStyle);
            ellipse->setShiftText(shiftText);
            return ellipse;
        } else {
            QPolygonF poly;
            for (const QJsonValue &pv : boundaries) {
                QJsonObject po = pv.toObject();
                qreal px = po["X"].toDouble();
                qreal py = po["Y"].toDouble();
                poly << QPointF(px, py);
            }
            QPolygonF templatePolygon;
            for (const QPointF &p : poly) {
                templatePolygon << QPointF(p.x() - minX, p.y() - minY);
            }
            ResizablePolygonItem *polyItem = addPolygon(templatePolygon, label, minX, minY, w, h, keyCodes, keyStyle);
            polyItem->setShiftText(shiftText);
            return polyItem;
        }
    }
    return nullptr;
}

void LayoutEditor::addShape(){
    ResizableRectItem *rect = addRectangle("",100,100,100,100);
    view->addRectAction(rect);
}

void LayoutEditor::onPickStyleToggled(bool checked){
    if (checked) {
        view->setApplyStyleMode(false);
        applyStyleButton->setChecked(false);
        view->setPickStyleMode(true);
    } else {
        view->setPickStyleMode(false);
    }
}

void LayoutEditor::onApplyStyleClicked(bool checked){
    if (!checked) {
        view->setApplyStyleMode(false);
        return;
    }
    if (!view->hasPickedStyle())
        return;
    view->setPickStyleMode(false);
    view->setApplyStyleMode(true);
}

ResizableEllipseItem * LayoutEditor::addEllipse(const QString &text, qreal h, qreal w, qreal x, qreal y, const std::list<int> keyCodes, const KeyStyle &keyStyle) {
    ResizableEllipseItem *ellipse = new ResizableEllipseItem(QRectF(0, 0, h, w), text, keyCodes);
    ellipse->setKeyStyle(keyStyle);
    scene->addItem(ellipse);
    ellipse->setPos(x, y);
    return ellipse;
}

ResizablePolygonItem * LayoutEditor::addPolygon(const QPolygonF &templatePolygon, const QString &text, qreal x, qreal y, qreal w, qreal h, const std::list<int> keyCodes, const KeyStyle &keyStyle) {
    ResizablePolygonItem *poly = new ResizablePolygonItem(templatePolygon, text, keyCodes);
    poly->setKeyStyle(keyStyle);
    scene->addItem(poly);
    poly->setRect(x, y, w, h);
    return poly;
}

ResizableRectItem * LayoutEditor::addRectangle(const QString &text, qreal h, qreal w, qreal x, qreal y, const std::list<int> keyCodes, const KeyStyle &keyStyle) {
    ResizableRectItem *rect = new ResizableRectItem(QRectF(0, 0, h, w), text, keyCodes);
    rect->setKeyStyle(keyStyle);
    scene->addItem(rect);
    rect->setPos(x,y);
    return rect;
}

void LayoutEditor::addItemToScene(QGraphicsItem *item){
    scene->addItem(item);
}

void LayoutEditor::updateMinimumSizeFromScene()
{
    const int marginRight = 56;
    const int marginBottom = 80;
    qreal maxX = -1e9;
    qreal maxY = -1e9;
    bool hasItems = false;

    for (QGraphicsItem *item : scene->items()) {
        if (item->parentItem() != nullptr)
            continue;
        ResizableRectItem *rectItem = dynamic_cast<ResizableRectItem *>(item);
        ResizableEllipseItem *ellipseItem = dynamic_cast<ResizableEllipseItem *>(item);
        ResizablePolygonItem *polygonItem = dynamic_cast<ResizablePolygonItem *>(item);
        if (!rectItem && !ellipseItem && !polygonItem)
            continue;
        QRectF sceneBr = item->sceneBoundingRect();
        maxX = qMax(maxX, sceneBr.right());
        maxY = qMax(maxY, sceneBr.bottom());
        hasItems = true;
    }

    int minW = kMinimumEditorWidth;
    int minH = marginBottom;
    if (hasItems) {
        minW = qMax(static_cast<int>(maxX) + marginRight, kMinimumEditorWidth);
        minH = static_cast<int>(maxY) + marginBottom;
    }
    setMinimumSize(minW, minH);
}

void LayoutEditor::saveLayout() {
    if (_currentLayoutPath.isEmpty() || _currentLayoutPath == QLatin1String(":/default.json")) {
        saveLayoutAs();
        return;
    }
    if (writeLayoutToFile(_currentLayoutPath)) {
        m_dirty = false;
        updateMinimumSizeFromScene();
        if (m_layoutSettings)
            m_layoutSettings->updateLastAndRecent(_currentLayoutPath);
        emit layoutLoaded(_currentLayoutPath);
    } else {
        QMessageBox::warning(this, tr("Save failed"),
            tr("Could not save the layout to:\n\n%1\n\nPossible causes:\n• The file or folder is read-only\n• Another program has the file open\n• You don't have permission to write to this location\n\nTry \"Save As\" to save to a different location.").arg(_currentLayoutPath));
    }
}

void LayoutEditor::saveLayoutAs() {
    QString initialDir;
    if (!_currentLayoutPath.isEmpty() && _currentLayoutPath != QLatin1String(":/default.json"))
        initialDir = QFileInfo(_currentLayoutPath).absolutePath();
    if (initialDir.isEmpty() && m_layoutSettings && !m_layoutSettings->lastLayoutPath().isEmpty())
        initialDir = QFileInfo(m_layoutSettings->lastLayoutPath()).absolutePath();
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save Layout As"), initialDir, tr("Layout Files (*.json);;All Files (*)"));
    if (!filePath.isEmpty()) {
        if (writeLayoutToFile(filePath)) {
            _currentLayoutPath = filePath;
            m_dirty = false;
            updateMinimumSizeFromScene();
            if (m_layoutSettings)
                m_layoutSettings->updateLastAndRecent(_currentLayoutPath);
            emit layoutLoaded(_currentLayoutPath);
        } else {
            QMessageBox::warning(this, tr("Save failed"),
                tr("Could not save the layout to:\n\n%1\n\nPossible causes:\n• The file or folder is read-only\n• Another program has the file open\n• You don't have permission to write to this location").arg(filePath));
        }
    }
}

bool LayoutEditor::writeLayoutToFile(const QString &fileName) {
    QJsonArray elementsArray;
    qreal minX = 0, minY = 0, maxX = 0, maxY = 0;
    bool first = true;
    int id = 0;
    const int ellipsePoints = 64;

    for (QGraphicsItem *item : scene->items()) {
        if (item->parentItem() != nullptr)
            continue;

        QJsonArray boundaries;
        QString text;
        QString shiftText;
        qreal centerX = 0, centerY = 0, itemMinX = 0, itemMinY = 0, itemMaxX = 0, itemMaxY = 0;
        QJsonArray keyCodesArray;

        ResizableRectItem *rectItem = dynamic_cast<ResizableRectItem *>(item);
        ResizableEllipseItem *ellipseItem = dynamic_cast<ResizableEllipseItem *>(item);
        ResizablePolygonItem *polygonItem = dynamic_cast<ResizablePolygonItem *>(item);

        if (rectItem) {
            QRectF r = rectItem->rect();
            qreal w = r.width();
            qreal h = r.height();
            QPointF corners[4] = { QPointF(0, 0), QPointF(w, 0), QPointF(w, h), QPointF(0, h) };
            for (const QPointF &p : corners) {
                QPointF sc = rectItem->mapToScene(p);
                boundaries.append(QJsonObject{{"X", static_cast<int>(sc.x())}, {"Y", static_cast<int>(sc.y())}});
            }
            text = rectItem->getText();
            shiftText = rectItem->getShiftText();
            QPointF centerSc = rectItem->mapToScene(QPointF(w / 2, h / 2));
            centerX = centerSc.x();
            centerY = centerSc.y();
            for (int kc : rectItem->getKeycodes()) keyCodesArray.append(kc);
            QRectF sceneBr = rectItem->sceneBoundingRect();
            itemMinX = sceneBr.left(); itemMinY = sceneBr.top(); itemMaxX = sceneBr.right(); itemMaxY = sceneBr.bottom();
        } else if (ellipseItem) {
            QRectF r = ellipseItem->rect();
            qreal w = r.width();
            qreal h = r.height();
            qreal rx = w / 2;
            qreal ry = h / 2;
            qreal cx = w / 2;
            qreal cy = h / 2;
            for (int i = 0; i < ellipsePoints; ++i) {
                qreal angle = 2.0 * M_PI * i / ellipsePoints;
                QPointF local(cx + rx * std::cos(angle), cy + ry * std::sin(angle));
                QPointF sc = ellipseItem->mapToScene(local);
                boundaries.append(QJsonObject{{"X", static_cast<int>(sc.x())}, {"Y", static_cast<int>(sc.y())}});
            }
            text = ellipseItem->getText();
            shiftText = ellipseItem->getShiftText();
            QPointF centerSc = ellipseItem->mapToScene(QPointF(cx, cy));
            centerX = centerSc.x();
            centerY = centerSc.y();
            for (int kc : ellipseItem->getKeycodes()) keyCodesArray.append(kc);
            QRectF sceneBr = ellipseItem->sceneBoundingRect();
            itemMinX = sceneBr.left(); itemMinY = sceneBr.top(); itemMaxX = sceneBr.right(); itemMaxY = sceneBr.bottom();
        } else if (polygonItem) {
            QPolygonF poly = polygonItem->polygon();
            for (const QPointF &p : poly) {
                QPointF sc = polygonItem->mapToScene(p);
                boundaries.append(QJsonObject{{"X", static_cast<int>(sc.x())}, {"Y", static_cast<int>(sc.y())}});
            }
            text = polygonItem->getText();
            shiftText = polygonItem->getShiftText();
            QPointF centerSc = polygonItem->mapToScene(polygonItem->boundingRect().center());
            centerX = centerSc.x();
            centerY = centerSc.y();
            for (int kc : polygonItem->getKeycodes()) keyCodesArray.append(kc);
            QRectF sceneBr = polygonItem->sceneBoundingRect();
            itemMinX = sceneBr.left(); itemMinY = sceneBr.top();
            itemMaxX = sceneBr.right(); itemMaxY = sceneBr.bottom();
        } else {
            continue;
        }

        KeyStyle itemStyle;
        if (rectItem)
            itemStyle = rectItem->keyStyle();
        else if (ellipseItem)
            itemStyle = ellipseItem->keyStyle();
        else if (polygonItem)
            itemStyle = polygonItem->keyStyle();

        QJsonObject keyObj;
        keyObj.insert("__type", "KeyboardKey");
        keyObj.insert("Id", id++);
        keyObj.insert("Boundaries", boundaries);
        keyObj.insert("KeyCodes", keyCodesArray);
        keyObj.insert("Text", text);
        keyObj.insert("TextPosition", QJsonObject{{"X", static_cast<int>(centerX)}, {"Y", static_cast<int>(centerY)}});
        keyObj.insert("ChangeOnCaps", false);
        keyObj.insert("ShiftText", shiftText.isEmpty() ? text : shiftText);
        QJsonObject styleObj = itemStyle.toJson();
        for (auto it = styleObj.begin(); it != styleObj.end(); ++it)
            keyObj.insert(it.key(), it.value());
        elementsArray.append(keyObj);

        if (first) {
            minX = itemMinX; minY = itemMinY; maxX = itemMaxX; maxY = itemMaxY;
            first = false;
        } else {
            minX = qMin(minX, itemMinX);
            minY = qMin(minY, itemMinY);
            maxX = qMax(maxX, itemMaxX);
            maxY = qMax(maxY, itemMaxY);
        }
    }

    int width = static_cast<int>(first ? 0 : maxX - minX);
    int height = static_cast<int>(first ? 0 : maxY - minY);

    QJsonObject rootObject;
    rootObject.insert("Elements", elementsArray);
    rootObject.insert("Width", width);
    rootObject.insert("Height", height);

    QByteArray jsonData = QJsonDocument(rootObject).toJson(QJsonDocument::Indented);

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("Failed to open file for writing: %s", qPrintable(fileName));
        return false;
    }
    file.write(jsonData);
    file.close();
    return true;
}

static QJsonObject keyItemToJson(QGraphicsItem *item) {
    const int ellipsePoints = 64;
    ResizableRectItem *rectItem = dynamic_cast<ResizableRectItem *>(item);
    ResizableEllipseItem *ellipseItem = dynamic_cast<ResizableEllipseItem *>(item);
    ResizablePolygonItem *polygonItem = dynamic_cast<ResizablePolygonItem *>(item);
    if (!rectItem && !ellipseItem && !polygonItem) return QJsonObject();

    QJsonArray boundaries;
    QString text, shiftText;
    qreal centerX = 0, centerY = 0;
    QJsonArray keyCodesArray;

    if (rectItem) {
        QRectF r = rectItem->rect();
        qreal w = r.width(), h = r.height();
        QPointF corners[4] = { QPointF(0, 0), QPointF(w, 0), QPointF(w, h), QPointF(0, h) };
        for (const QPointF &p : corners) {
            QPointF sc = rectItem->mapToScene(p);
            boundaries.append(QJsonObject{{"X", static_cast<int>(sc.x())}, {"Y", static_cast<int>(sc.y())}});
        }
        text = rectItem->getText(); shiftText = rectItem->getShiftText();
        QPointF centerSc = rectItem->mapToScene(QPointF(w / 2, h / 2));
        centerX = centerSc.x(); centerY = centerSc.y();
        for (int kc : rectItem->getKeycodes()) keyCodesArray.append(kc);
    } else if (ellipseItem) {
        QRectF r = ellipseItem->rect();
        qreal w = r.width(), h = r.height();
        qreal rx = w / 2, ry = h / 2, cx = w / 2, cy = h / 2;
        for (int i = 0; i < ellipsePoints; ++i) {
            qreal angle = 2.0 * M_PI * i / ellipsePoints;
            QPointF local(cx + rx * std::cos(angle), cy + ry * std::sin(angle));
            QPointF sc = ellipseItem->mapToScene(local);
            boundaries.append(QJsonObject{{"X", static_cast<int>(sc.x())}, {"Y", static_cast<int>(sc.y())}});
        }
        text = ellipseItem->getText(); shiftText = ellipseItem->getShiftText();
        QPointF centerSc = ellipseItem->mapToScene(QPointF(cx, cy));
        centerX = centerSc.x(); centerY = centerSc.y();
        for (int kc : ellipseItem->getKeycodes()) keyCodesArray.append(kc);
    } else {
        QPolygonF poly = polygonItem->polygon();
        for (const QPointF &p : poly) {
            QPointF sc = polygonItem->mapToScene(p);
            boundaries.append(QJsonObject{{"X", static_cast<int>(sc.x())}, {"Y", static_cast<int>(sc.y())}});
        }
        text = polygonItem->getText(); shiftText = polygonItem->getShiftText();
        QPointF centerSc = polygonItem->mapToScene(polygonItem->boundingRect().center());
        centerX = centerSc.x(); centerY = centerSc.y();
        for (int kc : polygonItem->getKeycodes()) keyCodesArray.append(kc);
    }

    KeyStyle itemStyle = rectItem ? rectItem->keyStyle() : (ellipseItem ? ellipseItem->keyStyle() : polygonItem->keyStyle());
    QJsonObject keyObj;
    keyObj.insert("__type", "KeyboardKey");
    keyObj.insert("Id", 0);
    keyObj.insert("Boundaries", boundaries);
    keyObj.insert("KeyCodes", keyCodesArray);
    keyObj.insert("Text", text);
    keyObj.insert("TextPosition", QJsonObject{{"X", static_cast<int>(centerX)}, {"Y", static_cast<int>(centerY)}});
    keyObj.insert("ChangeOnCaps", false);
    keyObj.insert("ShiftText", shiftText.isEmpty() ? text : shiftText);
    QJsonObject styleObj = itemStyle.toJson();
    for (auto it = styleObj.begin(); it != styleObj.end(); ++it)
        keyObj.insert(it.key(), it.value());
    return keyObj;
}

static void offsetKeyJson(QJsonObject &keyObj, qreal dx, qreal dy) {
    QJsonArray boundaries = keyObj["Boundaries"].toArray();
    QJsonArray newBoundaries;
    for (const QJsonValue &v : boundaries) {
        QJsonObject p = v.toObject();
        newBoundaries.append(QJsonObject{
            {"X", static_cast<int>(p["X"].toDouble() + dx)},
            {"Y", static_cast<int>(p["Y"].toDouble() + dy)}
        });
    }
    keyObj["Boundaries"] = newBoundaries;
    QJsonObject tp = keyObj["TextPosition"].toObject();
    keyObj["TextPosition"] = QJsonObject{
        {"X", static_cast<int>(tp["X"].toDouble() + dx)},
        {"Y", static_cast<int>(tp["Y"].toDouble() + dy)}
    };
}

void LayoutEditor::onCopyToggled(bool checked) {
    if (checked) {
        view->setPickStyleMode(false);
        view->setApplyStyleMode(false);
        pickStyleButton->setChecked(false);
        applyStyleButton->setChecked(false);
        view->setCopyMode(true);
    } else {
        view->setCopyMode(false);
    }
}

void LayoutEditor::copyKeyFromItem(QGraphicsItem *item) {
    if (!item) return;
    QJsonObject keyObj = keyItemToJson(item);
    if (keyObj.isEmpty()) return;
    QJsonArray arr; arr.append(keyObj);
    QJsonObject root;
    root.insert("Elements", arr);
    root.insert("Width", 100);
    root.insert("Height", 100);
    QApplication::clipboard()->setText(QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Compact)));
}

void LayoutEditor::pasteKey() {
    QString txt = QApplication::clipboard()->text();
    if (txt.isEmpty()) return;
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(txt.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) return;
    QJsonObject root = doc.object();
    QJsonArray elements = root["Elements"].toArray();
    if (elements.isEmpty()) return;
    QJsonObject keyObj = elements.first().toObject();
    if (keyObj["__type"].toString() != "KeyboardKey") return;

    QJsonArray boundaries = keyObj["Boundaries"].toArray();
    if (boundaries.isEmpty()) return;
    qreal minX = 1e9, minY = 1e9;
    for (const QJsonValue &v : boundaries) {
        QJsonObject p = v.toObject();
        minX = qMin(minX, p["X"].toDouble());
        minY = qMin(minY, p["Y"].toDouble());
    }
    qreal dx = 100 - minX;
    qreal dy = 100 - minY;
    offsetKeyJson(keyObj, dx, dy);

    QGraphicsItem *added = createKey(keyObj);
    if (added) {
        view->addRectAction(added);
        scene->clearSelection();
        added->setSelected(true);
    }
}

void LayoutEditor::updateLanguage() {
    addButton->setText(tr("Add Shape"));
    openButton->setText(tr("Open Layout"));
    newButton->setText(tr("New Layout"));
    saveButton->setText(tr("Save"));
    saveAsButton->setText(tr("Save As"));
    copyButton->setToolTip(tr("Copy key"));
    pasteButton->setToolTip(tr("Paste key"));
    pickStyleButton->setToolTip(tr("Pick style from a key"));
    applyStyleButton->setToolTip(tr("Apply picked style to key(s)"));
    if (m_actRect) m_actRect->setText(tr("Rectangle"));
    if (m_actCircle) m_actCircle->setText(tr("Circle"));
    if (m_customShapeMenu) m_customShapeMenu->setTitle(tr("Custom shape"));
    if (m_actStar) m_actStar->setText(tr("Star"));
    if (m_actDiamond) m_actDiamond->setText(tr("Diamond"));
    if (m_actHexagon) m_actHexagon->setText(tr("Hexagon"));
    if (m_actTriangle) m_actTriangle->setText(tr("Triangle"));
}
