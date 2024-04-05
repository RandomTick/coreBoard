#include <QMouseEvent>
#include <QGraphicsItem>
#include "layouteditor.h"
#include "mainwindow.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QFileDialog>
#include <QJsonArray>
#include "resizablerectitem.h"

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
                               "}";

    openButton = new QPushButton(tr("Open Layout"), this);
    openButton->setStyleSheet(buttonStyleSheet);

    undoButton = new QPushButton("", this);
    undoButton->setIcon(QIcon(":/icons/undo.png"));
    undoButton->setIconSize(QSize(18,18));
    undoButton->setFixedSize(24,24);
    undoButton->setStyleSheet(buttonStyleSheet);
    undoButton->setEnabled(false);

    redoButton = new QPushButton("", this);
    redoButton->setIcon(QIcon(":/icons/redo.png"));
    redoButton->setIconSize(QSize(18,18));
    redoButton->setFixedSize(24,24);
    redoButton->setStyleSheet(buttonStyleSheet);
    redoButton->setEnabled(false);

    addButton = new QPushButton(tr("Add Shape"), this);
    addButton->setStyleSheet(buttonStyleSheet);

    // Connect button signals to slots
    connect(openButton, &QPushButton::clicked, this, &LayoutEditor::loadLayoutButton);
    connect(undoButton, &QPushButton::clicked, view, &LayoutEditorGraphicsView::undoLastAction);
    connect(redoButton, &QPushButton::clicked, view, &LayoutEditorGraphicsView::redoLastAction);


    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(openButton);
    buttonLayout->addWidget(undoButton);
    buttonLayout->addWidget(redoButton);
    buttonLayout->addWidget(addButton);
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



void LayoutEditor::loadLayoutButton(){
    QString filePath = QFileDialog::getOpenFileName(this, tr("Select Layout File to Edit"), "", tr("Layout Files (*.json);;All Files (*)"));

    // Check if a file was selected (filePath is not empty)
    if (!filePath.isEmpty()) {
        // Call loadLayout with the selected file path
        loadLayout(filePath);
    }
}


void LayoutEditor::loadLayout(const QString &fileName){

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Failed to open layout file.");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject rootObject = doc.object();
    QJsonArray elements = rootObject.value("Elements").toArray();

    for (const QJsonValue &element : elements) {
        if (element.toObject().value("__type").toString() == "KeyboardKey") {
            createKey(element.toObject());
        }
    }

    int maxWidth = rootObject.value("Width").toInt();
    int maxHeight = rootObject.value("Height").toInt();

    setMinimumSize(maxWidth + 20, maxHeight + 60);// +20/+60 to ensure a margin for layout editor
}

void LayoutEditor::createKey(const QJsonObject &keyData){
    QJsonArray boundaries = keyData.value("Boundaries").toArray();
    if (boundaries.size() < 4) {//TODO: Handle others than rectangles here!
        qWarning("Invalid boundaries data.");
        return;
    }
    QString label = keyData.value("Text").toString();
    // Assuming rectangular shape and the points are given in order
    int x = boundaries[0].toObject()["X"].toInt();
    int y = boundaries[0].toObject()["Y"].toInt();
    int width = boundaries[1].toObject()["X"].toInt() - x;
    int height = boundaries[3].toObject()["Y"].toInt() - y;


    addRectangle(label,width,height,x,y);

}


void LayoutEditor::addRectangle(const QString &text, qreal h, qreal w, qreal x, qreal y) {
    ResizableRectItem *rect = new ResizableRectItem(QRectF(0, 0, h, w), text);

    scene->addItem(rect);
    rect->setPos(x,y);
}

void LayoutEditor::updateLanguage() {
    // Update the text of your widgets here
    addButton->setText(tr("Add Shape"));
    openButton->setText(tr("Open Layout"));
    // ...
}
