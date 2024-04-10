#ifndef SYMBOLINPUTDIALOG_H
#define SYMBOLINPUTDIALOG_H

#include <QDialog>
#include <QLineEdit>

class SymbolInputDialog : public QDialog {
    Q_OBJECT

public:
    explicit SymbolInputDialog(QWidget *parent = nullptr, QString currentText = "");
    QString getText() const;

private slots:
    void insertSymbol(const QString &symbol);

private:
    QLineEdit *lineEdit;
};

#endif // SYMBOLINPUTDIALOG_H
