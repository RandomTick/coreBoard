#ifndef DIALOGTEXTCHANGE_H
#define DIALOGTEXTCHANGE_H

#include <QDialog>
#include <QPlainTextEdit>
#include <QKeyEvent>

class DialogTextChange : public QDialog {
    Q_OBJECT

public:
    explicit DialogTextChange(QWidget *parent = nullptr, QString currentText = "", QString currentShiftText = "");
    QString getText() const;
    QString getShiftText() const;

private slots:
    void insertSymbol(const QString &symbol);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    QPlainTextEdit *lineEdit;
    QPlainTextEdit *shiftLineEdit;
};

#endif // DIALOGTEXTCHANGE_H
