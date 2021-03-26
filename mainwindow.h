#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QMenu>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>

QT_CHARTS_USE_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

private slots:
    void open();

private:
    QChart *chart;
    QAction *openAct;
    QMenu *fileMenu;

    void createActions();
    void createMenus();
    void setupChart(QLineSeries *series);
};
#endif // MAINWINDOW_H
