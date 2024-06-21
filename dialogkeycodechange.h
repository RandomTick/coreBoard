#ifndef DIALOGKEYCODECHANGE_H
#define DIALOGKEYCODECHANGE_H

#include <QDialog>
#include <QLineEdit>
#include <QKeyEvent>
#include "windowskeylistener.h"

class DialogKeycodeChange : public QDialog {
    Q_OBJECT

public:
    explicit DialogKeycodeChange(QWidget *parent = nullptr, std::list<int> currentKeyCodes = {});
    ~DialogKeycodeChange();
    std::list<int> getKeyCodes() const;

private slots:
    void insertKeycode(const int keyCode);
    void setKeyCodes(const std::list<int> newKeyCodes);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:

    WindowsKeyListener* keyListener;
    std::list<int> keyCodes;

    void updateDisplay(QLineEdit *display);
};

#endif // DIALOGKEYCODECHANGE_H
