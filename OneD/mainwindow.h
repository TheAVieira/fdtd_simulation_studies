#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qelapsedtimer.h>
#include <qdatetime.h>
#include <qthread.h>

#include <qwt.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>

namespace Ui {
class MainWindow;
}

#define PI 3.14159265359

class MainWindow : public QMainWindow {
    Q_OBJECT

    bool stop = false, pause = false;
    int Nz = 4000;
    double fc = 0.5; // Cut frequency for pulse
    double Tau = 0.5/fc, // Width of the gaussian source
    c0 = 50,
    dt = 1/(2*c0),
    dz = 0.5,
    T = 12*Tau + 5*(Nz*dz/c0),
    T_Steps = T/dt;
    double *ER, *UR, *Hx, *Ey, *z, *mEy, *mHx;
    double e1,e2,e3, h1,h2,h3; // For perfect boundary conditions

    QwtPlotCurve *EyCurve, *HxCurve;


public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionStart_triggered();
    void on_actionStop_triggered();

    void on_actionPause_triggered();

private:
    void init();
    double g(double t);
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
