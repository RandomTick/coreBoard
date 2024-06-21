#ifndef DIALOGTEXTCHANGE_H
#define DIALOGTEXTCHANGE_H

#include <QDialog>
#include <QLineEdit>

class DialogTextChange : public QDialog {
    Q_OBJECT

public:
    explicit DialogTextChange(QWidget *parent = nullptr, QString currentText = "");
    QString getText() const;

private slots:
    void insertSymbol(const QString &symbol);

private:
    QLineEdit *lineEdit;
};

#endif // DIALOGTEXTCHANGE_H
