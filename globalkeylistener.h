#ifndef GLOBALKEYLISTENER_H
#define GLOBALKEYLISTENER_H

#include <QObject>

class GlobalKeyListener : public QObject {
    Q_OBJECT

public:
    explicit GlobalKeyListener(QObject *parent = nullptr);
    ~GlobalKeyListener();

    void startListening();
    void stopListening();

signals:
    void keyPressed(int key);
    void keyReleased(int key);

private:
         // Windows-specific members for the hook
         // On Linux, this will be different
};

#endif // GLOBALKEYLISTENER_H
