#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qthread.h>
#include <QCloseEvent>

#include "qwt.h"
#include "qwt_plot_spectrocurve.h"
#include "qwt_plot_directpainter.h"
#include "qwt_color_map.h"
#include <qwt_scale_widget.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>

#define nbr(fc) QString::number(fc)
#define PI 3.1415926535898

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow{
    Q_OBJECT

    enum DevType{GlassInterface, GlassAngled, PlainArray, OPA};
    enum SourceType{SoftGauss, TFSF_YPlaneWv};

    // Units
    double meters = 1;
    double centimenters = 0.01*meters;
    double milimiters = 0.001*meters;
    double seconds = 1;
    double Hertz = 1/seconds;
    double kHertz = 1000*Hertz;
    double MHertz = 1000000*Hertz;
    double GHertz = 1000000000*Hertz;

    // Constants
    double c0 = 299792458*meters/seconds; // light speed in medium
    double e0 = 8.8541878176e-12 * 1/meters;
    double u0 = 1.2566370614e-6 * 1/meters;
    double Energ;

    // Source Param.
    int Ni, Nf; // Source start and end points
    double t0, ph, tau;
    double delD, zero, xx; // Perfect source
    double w, d;

    bool stop = false, pause = false;

    int frameStep;
    double Amp, Asrc; // E field amp
    double freq;

    int Nt; // Total time
    int Nx,Ny; // World dims
    int Nsx, Nsy; // Size of PML
    int Ns; // placement of the source
    int nt = 0;
    int plotPhase = 0;
    double Lx, dx, Ly, dy; // word res. dx=dy
    double gaussL; //
    double xyunits;
    double dt;
    double **CEx, **CEy, **CHz; // Curls
    double **Ez, **Dz, **Hx, **Hy;
    double **Uxx, **Uyy, **Epzz;
    // Update coef.
    double **sigx, **sigy;
    double **sigHx, **sigHy, **sigDx, **sigDy;
    double **mHx0, **mHx1, **mHx2, **mHx3;
    double **mHy0, **mHy1, **mHy2, **mHy3;
    double **mDz0,**mDz1, **mDz2, **mDz4;
    double **mEz1;
    double **ICEx, **ICEy, **IDz;
    double *x, *y;

    QwtPlotSpectroCurve *spec;
    QwtScaleWidget *thrmScale;
    QVector<QwtPoint3D> ElectZ;
    QwtPlotDirectPainter dirPainter;
    QwtPlotPicker *picker;
    QwtSymbol *symb;
    QwtPlotMarker marker;


    QwtLinearColorMap *gradientSpec;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pB_Start_clicked();
    void on_pB_Stop_clicked();
    void on_pB_Test_clicked();
    void on_hSldr_d_valueChanged(int value);
    void on_hSldr_zero_valueChanged(int value);
    void on_chkBx_Energ_toggled(bool checked);
    void on_spnBx_wl_editingFinished();
    void on_spnBx_step_editingFinished();
    void on_chkBx_Wave_toggled(bool checked);
    void on_chkBx_inf_toggled(bool checked);

    void on_hSldr_gaussL_valueChanged(int value);

    void on_hSldr_pltPhase_valueChanged(int value);

    void on_hSldr_width_valueChanged(int value);

    void on_chkBx_gauss_toggled(bool checked);

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private:
    void worldInit();
//    void worldCreate(bool PML,DevType device);
    void worldCreate();
    void worldDestroy();

    void Esource(double t);
    void Hsource(double t);
    double g(double t);
    double gauss(double x);
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
