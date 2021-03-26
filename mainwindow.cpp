#include "mainwindow.h"
#include <QFileDialog>
#include <QMenuBar>
#include <QString>
#include <QMessageBox>
#include <QtCharts/QChartView>
#include <QtCharts/QValueAxis>
#include <QLineSeries>
#include <QValueAxis>

#include <regex>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <memory>

std::ifstream& validateHeader(std::ifstream &input)
{
    std::vector<std::string> seive {"^# pendulum instruments ab, timeview32 v(\\d)\\.(\\d+)$",
                                    "^# frequency (\\w+)$",
                                    "^# (\\w+)\\s+(\\w+)\\s+(\\d+)\\s+(\\d+):(\\d+):(\\d+)\\s+(\\d+)$",
                                    "^# measuring time: (\\d+) (\\w+)\\s+single: (\\w+)$",
                                    "^# input a: (\\w+), (\\w+), (\\w+), (\\w+), (\\w+)\\s+filter: (\\w+)$",
                                    "^# input b: (\\w+), (\\w+), (\\w+), (\\w+), (\\w+)\\s+common: (\\w+)$",
                                    "^# ext.arm: (\\w+)\\s+ref.osc: (\\w+)$",
                                    "^# hold off: (\\w+)\\s+statistics: (\\w+)"};

    std::string line;
    for (size_t i = 0; i < seive.size() && std::getline(input, line); ++i)
    {
        line.erase(line.size() - 1); // we should delete "\r" before match
        std::regex re(seive[i], std::regex::icase);
        std::smatch result;
        if (!std::regex_match(line, result, re))
        {
            throw std::invalid_argument(line);
            return input;
        }
    }

    if (input.eof())
    {
        throw std::length_error("EOF was reached while reading the header");
        return input;
    }

    return input;
}

void parseData(std::ifstream &ifs, QLineSeries *series)
{
    for (std::string line; std::getline(ifs, line); )
    {
        if (line == "\r") continue;

        std::istringstream iss{line};
        double x, y;
        iss >> x >> y;

        if (iss.fail() && !iss.eof()) throw std::invalid_argument("invalid_argument: " + line);
        if (iss.peek() != '\r') throw std::length_error("length_error: " + line);

        series->append(x, y);
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    chart(new QChart())
{
    createActions();
    createMenus();
}

void MainWindow::open()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open the file", "./", "Pendulum (*.ssd *.rsd)",
                                                    nullptr, QFileDialog::DontUseNativeDialog);
    if (filename.isEmpty()) return;

    //QString is u16 by default, convert it
    std::string utf8_filename = filename.toUtf8().constData();
    std::ifstream file {utf8_filename};

    if (file.bad())
    {
        QMessageBox::warning(this, "Warning", "Cannot open the file");
        return;
    }

    try {
        validateHeader(file);
    } catch (std::logic_error er) {
        QMessageBox::critical(this, "Error", "The header is corrupted");
        std::cerr << er.what() << std::endl;
        return;
    }

    QLineSeries *series = new QLineSeries(this);
    try {
        parseData(file, series);
    } catch (std::logic_error er) {
        QMessageBox::critical(this, "Error", "The Data is mailformed");
        std::cerr << er.what() << std::endl;
        delete series;
        return;
    }

    setupChart(series);
}

void MainWindow::createActions()
{
    openAct = new QAction("Open", this);
    connect(openAct, &QAction::triggered, this, &MainWindow::open);
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu("File");
    fileMenu->addAction(openAct);
}

void MainWindow::setupChart(QLineSeries *series)
{
    chart->removeAllSeries();
    chart->addSeries(series);
    QValueAxis *axisX = new QValueAxis(this);
    QValueAxis *axisY = new QValueAxis(this);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    chart->legend()->hide();

    QChartView *chartView = new QChartView(chart);
    setCentralWidget(chartView);
}
