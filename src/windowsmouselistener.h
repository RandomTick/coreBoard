#ifndef WINDOWSMOUSELISTENER_H
#define WINDOWSMOUSELISTENER_H

#include <QObject>

#ifdef Q_OS_WIN

class WindowsMouseListener : public QObject {
    Q_OBJECT

public:
    explicit WindowsMouseListener(QObject *parent = nullptr);
    ~WindowsMouseListener();

    void startListening();
    void stopListening();
    void setAsGlobalInstance();

signals:
    void keyPressed(int key);
    void keyReleased(int key);
};

#endif // Q_OS_WIN

#endif // WINDOWSMOUSELISTENER_H
