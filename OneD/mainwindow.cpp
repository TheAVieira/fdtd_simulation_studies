#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);

    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->setPen( Qt::darkGray, 0.0, Qt::DotLine );
    grid->enableX( true );
    grid->enableXMin( true );
    grid->enableY( true );
    grid->enableYMin( false );
    grid->attach(ui->qwtPlot);

    EyCurve = new QwtPlotCurve("Ey");
    EyCurve->setPen( Qt::red, 1 );
    EyCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
    EyCurve->attach(ui->qwtPlot);
    EyCurve->setRenderThreadCount(0);

    HxCurve = new QwtPlotCurve("Hx");
    HxCurve->setPen( Qt::blue, 1 );
    HxCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
    HxCurve->attach(ui->qwtPlot);
    HxCurve->setRenderThreadCount(0);

    ui->actionPause->setEnabled(false);
    ui->qwtPlot->setAxisScale(QwtPlot::yLeft,-1.2,1.2);
}

void MainWindow::on_actionStart_triggered(){
    init();

    QElapsedTimer myTimer; // Real time
    myTimer.start();
    double EySrc, HxSrc;
    double delt  = dz/(2*c0) + dt/2; // Phase shift needed to match E and H
    int nsrc = 50;
    double Amp = -sqrt(ER[nsrc]/UR[nsrc]);

    stop = false, pause = false;
    ui->actionStart->setEnabled(false);
    ui->actionPause->setEnabled(true);
    ui->textBrowser->append("Starting\nRun time: " + QString::number(T-dt)+"s");

    for(int n = 0; n<T_Steps && !stop; n++){ // time loop
        while(pause && !stop)
            QCoreApplication::processEvents();

        for(int nz = 0; nz<(Nz-1); nz++){ // Update H from E
            Hx[nz] += mHx[nz]*(Ey[nz+1] - Ey[nz])/dz;
        }
        Hx[Nz-1] += mHx[Nz-1]*(e3 - Ey[Nz-1])/dz; // Absorbing boundary Condition
        h3=h2;
        h2=h1;
        h1=Hx[1]; // Boundary vals for ABC

        Ey[0] += mEy[0]*(Hx[0] - h3)/dz; // Absorbing boundary Condition
        for(int nz = 1; nz<Nz; nz++){ // Update E from H
            Ey[nz] += mEy[nz]*(Hx[nz] - Hx[nz-1])/dz;
        }
        e3=e2;
        e2=e1;
        e1=Ey[Nz-1]; // Boundary vals for ABC

        // Source 1 - Matched source. E and H are both specified to produce a source pulse
        EySrc = g(n*dt-6*Tau)/2;
        HxSrc = Amp * g((n*dt-6*Tau + delt))/2;
        Hx[nsrc-1] -= (mHx[nsrc-1]/dz)*EySrc;
        Ey[nsrc] -= (mEy[nsrc]/dz)*HxSrc; // This term ensures the wave propagates right.

        // Source 2 - Only E is specificed and consequently H (in blue) also sees a pulse propagating in opposite direction.
        if(n<2000)
            Ey[Nz/2] = g(n*dt-6*Tau); // Ey Source - Alternative to the H and E sources already written

        EyCurve->setRawSamples(z,Ey,Nz);
        HxCurve->setRawSamples(z,Hx,Nz);
        ui->qwtPlot->setTitle("Time: "+QString::number(n*dt));
        ui->qwtPlot->replot();
        QCoreApplication::processEvents();
    }// --- for(n<T_Steps) time loop

    ui->actionStart->setEnabled(true);
    ui->textBrowser->append("Finnished in: " + QDateTime::fromTime_t(myTimer.elapsed()/1000).toString("hh:mm:ss.zzz"));
}

void MainWindow::on_actionPause_triggered(){
    if(!pause)
        ui->actionPause->setText("Continue");
    else
        ui->actionPause->setText("Pause");
    pause = !pause;
}

void MainWindow::on_actionStop_triggered(){
    stop = true;
    ui->actionStart->setEnabled(true);
    ui->actionPause->setEnabled(false);
    ui->actionPause->setText("Pause");
    ui->textBrowser->append("Stopping");
}

void MainWindow::init(){ // ER, UR, Hx, Ey, z, mEy, mHx
    ER = new double[Nz];
    UR = new double[Nz];
    Hx = new double[Nz];
    Ey = new double[Nz];
    z = new double[Nz];
    mEy = new double[Nz];
    mHx = new double[Nz];
    e1 = e2 = e3 = 0;
    h1 = h2 = h3 = 0;
    for(int i=0; i<Nz; i++){
        ER[i] = 1;
        UR[i] = 1;
        Hx[i] = 0;
        Ey[i] = 0;
        z[i] = i*dz;
        mEy[i] = c0*dt/ER[i];
        mHx[i] = c0*dt/UR[i];
    }
    ui->textBrowser->append("mHx[1]: "+QString::number(mHx[1]));
}

double MainWindow::g(double t){
    int n = t/dt;
    return exp(- pow( (n*dt/Tau) ,2) );
}

MainWindow::~MainWindow(){
    delete ui;
}

