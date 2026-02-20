#ifndef DIALOGTEXTCHANGE_H
#define DIALOGTEXTCHANGE_H

#include <QDialog>
#include <QLineEdit>

class DialogTextChange : public QDialog {
    Q_OBJECT

public:
    explicit DialogTextChange(QWidget *parent = nullptr, QString currentText = "", QString currentShiftText = "");
    QString getText() const;
    QString getShiftText() const;

private slots:
    void insertSymbol(const QString &symbol);

private:
    QLineEdit *lineEdit;
    QLineEdit *shiftLineEdit;
};

#endif // DIALOGTEXTCHANGE_H
