#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_fetchConv_btn_clicked();

private:
    Ui::MainWindow *ui;

    void print(const QString &text);
    void convertSpriteFormatToClassFormat();
};

#endif // MAINWINDOW_H
