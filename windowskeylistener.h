#ifndef WINDOWSKEYLISTENER_H
#define WINDOWSKEYLISTENER_H

#include <QObject>

class WindowsKeyListener : public QObject {
    Q_OBJECT

public:
    explicit WindowsKeyListener(QObject *parent = nullptr);
    ~WindowsKeyListener();

    void startListening();
    void stopListening();

signals:
    void keyPressed(int key);
    void keyReleased(int key);
};

#endif // WINDOWSKEYLISTENER_H
