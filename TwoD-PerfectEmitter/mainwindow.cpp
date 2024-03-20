#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),ui(new Ui::MainWindow){
    ui->setupUi(this);
    // TODO Make this vars consistent with real physics
    // Parameter initialization
    Nt = 1000; frameStep = 1;
    Nsx = Nsy = 20;
    Nx = 200+Nsx; Ny = 400+Nsy; // Nsx/2 on each side of the grid
    Ns = Nsy;
    dx = 0.006 * meters; dy = 0.006 * meters; Lx = dx*(Nx-Nsx); Ly = dy*(Ny-Nsy);
    xyunits = 1000;
    dt = dx/(2*c0) * seconds; //1/(4*c0) * seconds;
    // Source vars
    tau = 4*dx/c0; t0=6*tau;
    Amp = 1, Asrc = 2;
    freq = c0/0.06; // c0/0.12; -> ideal wavelength

    Ni = Nsx/2+Nx/4, Nf = Nx-Nsx/2-Nx/4, w = (Nf-Ni)/2;
    ui->hSldr_d->setMaximum(Ny-Nsy/2-Ns);
    ui->spnBx_d->setMaximum((Ns-Ny/2)*dy*xyunits);
    ui->spnBx_d->setMaximum((Ny-Nsy/2-Ns)*dy*xyunits);
    ui->hSldr_d->setValue(50);
    ui->hSldr_zero->setMinimum(Nsx/2);
    ui->spnBx_zero->setMinimum((Nsx/2)*dx*xyunits);
    ui->hSldr_zero->setMaximum(Nx-Nsx/2);
    ui->spnBx_zero->setMaximum((Nx-Nsx/2)*dx*xyunits);
    ui->hSldr_zero->setValue(Nx/2);
    ui->hSldr_gaussL->setMinimum(1);
    ui->spnBx_gaussL->setMinimum(1*dx*xyunits);
    ui->hSldr_gaussL->setMaximum(Nf-Ni);
    ui->spnBx_gaussL->setMaximum((Nf-Ni)*dx*xyunits);
    ui->hSldr_gaussL->setValue(50);

    ui->hSldr_pltPhase->setMaximum(ui->spnBx_step->value()-1);
    ui->hSldr_width->setMaximum((Nx-Nsx)/2);
    ui->spnBx_width->setMaximum((Nx-Nsx)*dx*xyunits);
    ui->hSldr_width->setValue(80);

    spec = new QwtPlotSpectroCurve;
    spec->setRenderThreadCount(0);
    spec->attach(ui->qwtPlot);
    spec->setPenWidth(5);
    gradientSpec = new QwtLinearColorMap();

    gradientSpec->setColorInterval(QColor(0,0,255,255),QColor(255,0,0,255));
    gradientSpec->addColorStop(0.15,QColor(0,120,255,255)); // Light Blue
//    gradientSpec->addColorStop(.5,QColor(240,240,240,255)); // Gray
    gradientSpec->addColorStop(0.85,QColor(255,193,0,255)); // Yellow

    ui->qwtPlot->enableAxis(QwtPlot::yRight,true);
    ui->qwtPlot->enableAxis(QwtPlot::xBottom,false); ui->qwtPlot->enableAxis(QwtPlot::xTop,true);
    thrmScale = ui->qwtPlot->axisWidget(QwtPlot::yRight);
    thrmScale->setColorBarEnabled(true);// thrmScale->setMaximumWidth(20); thrmScale->setMinimumWidth(20);
    double range = 2*Amp;
    ui->qwtPlot->setAxisScale(QwtPlot::yRight,-range,range);
    thrmScale->setColorMap(QwtInterval(-range,range,QwtInterval::ExcludeMinimum),gradientSpec);

    ui->qwtPlot->setAxisScale(QwtPlot::yLeft,Ly/2*xyunits,-Ly/2*xyunits); // Inverted Y
    ui->qwtPlot->setAxisScale(QwtPlot::xTop,-Lx/2*xyunits,Lx/2*xyunits);
    ui->qwtPlot->setAxisScale(QwtPlot::xBottom,-Lx/2*xyunits,Lx/2*xyunits); // It still needs this to plot correctly..
    ui->qwtPlot->replot();

    picker = new QwtPlotPicker(ui->qwtPlot->canvas());
    picker->setTrackerPen(QPen(Qt::white));
    picker->setTrackerMode(QwtPicker::AlwaysOn);

//    symb = new QwtSymbol( QwtSymbol::XCross ); symb->setPen( Qt::white ); symb->setSize( 4 );
    marker.setLineStyle(QwtPlotMarker::Cross);
    marker.setLinePen( Qt::white, 0.0, Qt::SolidLine );
    marker.attach(ui->qwtPlot);

    spec->setColorMap(gradientSpec);
    spec->setColorRange(QwtInterval(-range, range, QwtInterval::ExcludeMinimum));

    // Other checks
    on_spnBx_step_editingFinished();
    QString lamb = QChar(0xBB, 0x03);
    ui->spnBx_wl->setPrefix(lamb+": ");
    ui->spnBx_wl->setSuffix(" mm");
    ui->spnBx_wl->setMinimum(dx*4000);

    ui->textBrowser->append("First freq: " +nbr(freq));
}

// --------------------------------------
// -------------- Buttons ---------------
// --------------------------------------

void MainWindow::on_pB_Start_clicked(){
    ui->pB_Stop->setEnabled(true);

    if(QString::compare(ui->pB_Start->text(),"Start")==0){
        ui->pB_Start->setText("Pause");
        stop = false;
        ui->textBrowser->append("Initializing");
        worldInit();
        ui->textBrowser->append("Focus y: "+nbr(dy*(Nsy+d-Ny/2)));

        ui->textBrowser->append("Running");
        for(nt=0; !stop; nt++){ // Time cycle  nt<Nt &&

            while(pause && !stop){QCoreApplication::processEvents();} // Pausing

            // CEx and CEy ----------------------------------------
            for(int nx=0; nx<Nx && !stop; nx++){
                for(int ny=0; ny<(Ny-1) && !stop; ny++){
                    CEx[nx][ny] = (Ez[nx][ny+1] - Ez[nx][ny])/dy;
                }
                CEx[nx][Ny-1]=(0 - Ez[nx][Ny-1])/dy; // Dirichlet Boundary Condition (reflective)
            }
            QCoreApplication::processEvents();
            for(int ny=0; ny<Ny && !stop; ny++){
                for(int nx=0; nx<(Nx-1) && !stop; nx++){
                    CEy[nx][ny] = -(Ez[nx+1][ny] - Ez[nx][ny])/dx;
                }
                CEy[Nx-1][ny] = -(0 - Ez[Nx-1][ny])/dx; // Dirichlet Boundary Condition (reflective)
            }
            QCoreApplication::processEvents();

            // Esource ----------------------------------------
            Esource(nt*dt);

            // ICEx and ICEy ----------------------------------------
            for(int nx=0; nx<Nx && !stop; nx++){
                for(int ny=0; ny<(Ny-1) && !stop; ny++){
                    ICEx[nx][ny] += CEx[nx][ny];
                    ICEy[nx][ny] += CEy[nx][ny];
                }
            }

            // Hx and Hy ----------------------------------------
            for(int nx=0; nx<Nx && !stop; nx++){
                for(int ny=0; ny<Ny && !stop; ny++){
                    Hx[nx][ny] = mHx1[nx][ny]*Hx[nx][ny] + mHx2[nx][ny]*CEx[nx][ny] + mHx3[nx][ny]*ICEx[nx][ny];
                    Hy[nx][ny] = mHy1[nx][ny]*Hy[nx][ny] + mHy2[nx][ny]*CEy[nx][ny] + mHy3[nx][ny]*ICEy[nx][ny];
                }
            }
            QCoreApplication::processEvents();

            // CHz ----------------------------------------
            CHz[0][0] = (Hy[0][0]-0)/dx - (Hx[0][0]-0)/dy;
            for(int nx=1; nx<Nx && !stop; nx++){
                CHz[nx][0] = (Hy[nx][0]-Hy[nx-1][0])/dx - (Hx[nx][0]-0)/dy;
            }
            for(int ny=1; ny<Ny && !stop; ny++){
                CHz[0][ny] = (Hy[0][ny]-0)/dx - (Hx[0][ny]-Hx[0][ny-1])/dy;
                for(int nx=1; nx<Nx && !stop; nx++){
                    CHz[nx][ny] = (Hy[nx][ny]-Hy[nx-1][ny])/dx - (Hx[nx][ny]-Hx[nx][ny-1])/dy;
                }
            }
            QCoreApplication::processEvents();

            // Hsource ----------------------------------------
            Hsource(nt*dt);

            // IDz ----------------------------------------
            for(int nx=0; nx<Nx && !stop; nx++){
                for(int ny=0; ny<Ny; ny++){
                IDz[nx][ny] += Dz[nx][ny];
                }
            }

            // Dz ----------------------------------------
            for(int nx=0; nx<Nx && !stop; nx++){
                for(int ny=0; ny<Ny; ny++){
                    Dz[nx][ny] = mDz1[nx][ny]*Dz[nx][ny] + mDz2[nx][ny]*CHz[nx][ny] + mDz4[nx][ny]*IDz[nx][ny];
                }
            }
            QCoreApplication::processEvents();

            /// Source ---------------------------------------- Working Dz[Nx/2][Ny*1/5] += 10*Amp*exp(-pow(((nt*dt-t0)/tau),2));

            // Ez ----------------------------------------
            for(int nx=0; nx<Nx && !stop; nx++)
                for(int ny=0; ny<Ny && !stop; ny++)
                    Ez[nx][ny] = mEz1[nx][ny]*Dz[nx][ny];

            ElectZ.resize(0);
            for(int ny=Nsy/2; ny<Ny-Nsy/2 && !stop; ny++){ // Excluding PML
                for(int nx=Nsx/2; nx<Nx-Nsx/2 && !stop; nx++){
                    Energ = Epzz[nx][ny]*Ez[nx][ny]*Ez[nx][ny] + Uxx[nx][ny]*Hx[nx][ny]*Hx[nx][ny] + Uyy[nx][ny]*Hy[nx][ny]*Hy[nx][ny];
                    if(ui->chkBx_Energ->isChecked()==1)
                        ElectZ.append(QwtPoint3D(x[nx]*xyunits, y[ny]*xyunits, Energ/5 )); // Ez[nx][ny] Energ/5
                    else
                        ElectZ.append(QwtPoint3D(x[nx]*xyunits, y[ny]*xyunits, Ez[nx][ny] )); // Ez[nx][ny] Energ/5
                }
            }
            QCoreApplication::processEvents();

            // Visulize ----------------------------------------
            if(!stop && nt%frameStep == plotPhase){
                ui->lineEdit->setText("Frame "+nbr(nt));
                spec->setSamples(ElectZ);
                marker.setValue((zero-Nx/2)*dx*xyunits, (Nsy+d-Ny/2)*dy*xyunits);
                marker.attach(ui->qwtPlot);
                ui->qwtPlot->replot();
            }

            QCoreApplication::processEvents();
        } //for(nt<Nt) - Time cycle

        nt=0;
        worldDestroy();
        ui->textBrowser->append("End");
        ui->pB_Stop->setEnabled(false);

    }else if(QString::compare(ui->pB_Start->text(),"Pause")==0){
        ui->pB_Start->setText("Continue");
        pause = true;
    }else if(QString::compare(ui->pB_Start->text(),"Continue")==0){
        ui->pB_Start->setText("Pause");
        pause = false;
    }

    if(stop == true || nt == Nt){
        pause = false;
        ui->textBrowser->append("Stoped");
        ui->pB_Start->setText("Start");
        ui->pB_Stop->setEnabled(false);
        stop = false;
    }
}

void MainWindow::on_pB_Stop_clicked(){
    stop = true;
}

void MainWindow::on_pB_Test_clicked(){

    ui->textBrowser->setText("Start Test >>>>>>>>>>>");
    ElectZ.resize(0);

    for(int ny=0; ny<Ny; ny++){
        for(int nx=0; nx<Nx; nx++){
            ElectZ.append(QwtPoint3D(x[nx], y[ny], Amp*(sigDx[nx][ny]+sigDy[nx][ny]) ));
        }
        QCoreApplication::processEvents();
    }
    spec->setSamples(ElectZ);
    dirPainter.drawSeries(spec, 0, Nx*Ny-1);
    ui->qwtPlot->replot();
    ui->textBrowser->append("Done Test");
}

//---- END Buttons ----
//---- Source functions ----

void MainWindow::Esource(double t){
    for(int nx=Ni; nx<Nf; nx++){
        xx = (double)nx - zero;
        delD = sqrt(d*d+xx*xx) - d;
        CEx[nx][Ns] -= (1/dy)* g(t) * gauss(xx)
                ; // phase lag is k*del
    }
}

void MainWindow::Hsource(double t){
    for(int nx=Ni; nx<Nf; nx++){
        xx = (double)nx - zero;
        delD = sqrt(d*d+xx*xx) - d;
        CHz[nx][Ns+1] += (1/dy) * g(t + dy/(2*c0) + dt/2 ) * gauss(xx)
                ; // sqrt(Ezz[nx][Ns-1]/Uxx[nx][Ns-1]) que = 1
    }
}

double MainWindow::g(double t){
	ph=delD*dy/c0;
    if(ui->chkBx_Wave->isChecked())
        return Asrc*Amp*sin(2*PI*freq*(t + ph));
    else
        return Asrc*Amp*exp(-pow(((t-t0+ph)/tau),2)) //+ 1.2*Asrc*Amp*exp(-pow(((t+t0+2*tau)/tau),2)) + 1.5*Asrc*Amp*exp(-pow(((t+t0+4*tau)/tau),2))
            ;
}

double MainWindow::gauss(double x){
    if(ui->chkBx_gauss->isChecked())
        return exp(-pow((x/gaussL),2));
    else
        return 1;
}

//---- Init Vars ----
void MainWindow::worldInit(){
    // Curls
    CEx = new double*[Nx];
    CEy = new double*[Nx];
    CHz = new double*[Nx];

    Ez = new double*[Nx];
    Dz = new double*[Nx];
    Hx = new double*[Nx];
    Hy = new double*[Nx];
    Uxx = new double*[Nx];
    Uyy = new double*[Nx];
    Epzz = new double*[Nx];

    sigx = new double*[2*Nx];
    sigy = new double*[2*Nx];
    sigHx = new double*[Nx];
    sigHy = new double*[Nx];
    sigDx = new double*[Nx];
    sigDy = new double*[Nx];

    mHx0 = new double*[Nx];
    mHx1 = new double*[Nx];
    mHx2 = new double*[Nx];
    mHx3 = new double*[Nx];
    mHy0 = new double*[Nx];
    mHy1 = new double*[Nx];
    mHy2 = new double*[Nx];
    mHy3 = new double*[Nx];
    mDz0 = new double*[Nx];
    mDz1 = new double*[Nx];
    mDz2 = new double*[Nx];
    mDz4 = new double*[Nx];
    mEz1 = new double*[Nx];
    ICEx = new double*[Nx];
    ICEy = new double*[Nx];
    IDz = new double*[Nx];

    x = new double[Nx];
    y = new double[Ny];

    for(int nx=0; nx<2*Nx; nx++){ // 2x Grid method
        sigx[nx] = new double[2*Ny];
        sigy[nx] = new double[2*Ny];
    }

    for(int nx=0; nx<Nx; nx++){
        CEx[nx] = new double[Ny];
        CEy[nx] = new double[Ny];
        CHz[nx] = new double[Ny];
        Ez[nx] = new double[Ny];
        Dz[nx] = new double[Ny];
        Hx[nx] = new double[Ny];
        Hy[nx] = new double[Ny];
        Uxx[nx] = new double[Ny];
        Uyy[nx] = new double[Ny];
        Epzz[nx] = new double[Ny];

        sigHx[nx] = new double[Ny];
        sigHy[nx] = new double[Ny];
        sigDx[nx] = new double[Ny];
        sigDy[nx] = new double[Ny];

        mHx0[nx] = new double[Ny];
        mHx1[nx] = new double[Ny];
        mHx2[nx] = new double[Ny];
        mHx3[nx] = new double[Ny];
        mHy0[nx] = new double[Ny];
        mHy1[nx] = new double[Ny];
        mHy2[nx] = new double[Ny];
        mHy3[nx] = new double[Ny];
        mDz0[nx] = new double[Ny];
        mDz1[nx] = new double[Ny];
        mDz2[nx] = new double[Ny];
        mDz4[nx] = new double[Ny];
        mEz1[nx] = new double[Ny];
        ICEx[nx] = new double[Ny];
        ICEy[nx] = new double[Ny];
        IDz[nx] = new double[Ny];

    }

    for(int nx=0; nx<Nx; nx++){
        x[nx] = dx*nx - dx*Nx/2; // X coordinates
    }
    for(int ny=0; ny<Ny; ny++){
        y[ny] = dy*ny - dy*Ny/2; // Y coordinates
    }

    worldCreate();
}

void MainWindow::worldCreate(){
    //--- world properties ---
    double m = 25; // DEBUG

    // Create sigx & sigy ------ 2x Grid method
    for(int nx=0; nx<2*Nx; nx++){
        for(int ny=0; ny<2*Ny; ny++){
            sigx[nx][ny]=0; sigy[nx][ny]=0;
        }
    }
    // sigx
    for(int ny=0; ny<2*Ny; ny++){
        for(int nx=0; nx<Nsx; nx++){
            sigx[Nsx-nx-1][ny] = m*e0/(2*dt) * pow( ((double)nx/(2*(double)Nsx)), 3);
            sigx[2*Nx-Nsx+nx][ny] = m*e0/(2*dt) * pow( ((double)nx/(2*(double)Nsx)), 3);
        }
    }

    //sigy
    for(int nx=0; nx<2*Nx; nx++){
        for(int ny=0; ny<Nsy; ny++){
            sigy[nx][Nsy-ny-1] = m*e0/(2*dt) * pow( ((double)ny/(2*(double)Nsy)), 3);
            sigy[nx][2*Ny-Nsy+ny] = m*e0/(2*dt) * pow( ((double)ny/(2*(double)Nsy)), 3);
        }
    }

    // Prepare Uxx, Uyy and Ezz
    for(int nx=0; nx<Nx; nx++){
        for(int ny=0; ny<Ny; ny++){
            Uxx[nx][ny] = 1;
            Uyy[nx][ny] = 1;
            Epzz[nx][ny] = 1;
        }
    }


    // Create others -------------------------------------------------------
    for(int nx=0; nx<Nx; nx++){
        for(int ny=0; ny<Ny; ny++){

            sigHx[nx][ny] = sigx[2*nx][2*ny+1];
            sigHy[nx][ny] = sigy[2*nx][2*ny+1]; // 2x Grid method
            mHx0[nx][ny] = (1/dt)+sigHy[nx][ny]/(2*e0);
            mHx1[nx][ny] = ( (1/dt) - sigHy[nx][ny]/(2*e0) )/mHx0[nx][ny];
            mHx2[nx][ny] = - c0/(Uxx[nx][ny]*mHx0[nx][ny]);
            mHx3[nx][ny] = - (c0*dt/e0)*sigHx[nx][ny]/(Uxx[nx][ny]*mHx0[nx][ny]);

            sigHx[nx][ny] = sigx[2*nx+1][2*ny];
            sigHy[nx][ny] = sigy[2*nx+1][2*ny]; // 2x Grid method
            mHy0[nx][ny] = (1/dt) + sigHx[nx][ny]/(2*e0);
            mHy1[nx][ny] = ((1/dt) - sigHx[nx][ny]/(2*e0))/mHy0[nx][ny];
            mHy2[nx][ny] = -c0/(Uyy[nx][ny]*mHy0[nx][ny]);
            mHy3[nx][ny] = - (c0*dt/e0)*sigHy[nx][ny]/(Uyy[nx][ny]*mHy0[nx][ny]);

            sigDx[nx][ny] = sigx[2*nx][2*ny];
            sigDy[nx][ny] = sigy[2*nx][2*ny]; // 2x Grid method
            mDz0[nx][ny] = (1/dt) + (sigDx[nx][ny]+sigDy[nx][ny])/(2*e0) + sigDx[nx][ny]*sigDy[nx][ny]*dt/(4*e0*e0);
            mDz1[nx][ny] = ( (1/dt) - (sigDx[nx][ny]+sigDy[nx][ny])/(2*e0) - sigDx[nx][ny]*sigDy[nx][ny]*dt/(4*e0*e0) )/mDz0[nx][ny];
            mDz2[nx][ny] = c0/mDz0[nx][ny];
            mDz4[nx][ny] = -(dt/(e0*e0))*sigDx[nx][ny]*sigDy[nx][ny]/mDz0[nx][ny];

            mEz1[nx][ny] = 1/Epzz[nx][ny];

            CEx[nx][ny]=0;
            CEy[nx][ny]=0;
            CHz[nx][ny]=0;
            Ez[nx][ny]=0;
            Dz[nx][ny]=0;
            Hx[nx][ny]=0;
            Hy[nx][ny]=0;
            ICEx[nx][ny]=0;
            ICEy[nx][ny]=0;
            IDz[nx][ny]=0;
        }
        QCoreApplication::processEvents();
    }

}

void MainWindow::worldDestroy(){
    for(int nx=0; nx<2*Nx; nx++){
        delete sigx[nx]; delete sigy[nx];
    }

    for(int nx=0; nx<Nx; nx++){
        delete CEx[nx];
        delete CEy[nx];
        delete CHz[nx];
        delete Ez[nx];
        delete Dz[nx];
        delete Hx[nx];
        delete Hy[nx];
        delete Uxx[nx];
        delete Uyy[nx];
        delete Epzz[nx];

        delete sigHx[nx];
        delete sigHy[nx];
        delete sigDx[nx];
        delete sigDy[nx];

        delete mHx0[nx];
        delete mHx1[nx];
        delete mHx2[nx];
        delete mHx3[nx];
        delete mHy0[nx];
        delete mHy1[nx];
        delete mHy2[nx];
        delete mHy3[nx];
        delete mDz0[nx];
        delete mDz1[nx];
        delete mDz2[nx];
        delete mDz4[nx];
        delete mEz1[nx];
        delete ICEx[nx];
        delete ICEy[nx];
        delete IDz[nx];
    }

    delete CEx;
    delete CEy;
    delete CHz;
    delete Ez;
    delete Dz;
    delete Hx;
    delete Hy;
    delete Uxx;
    delete Uyy;
    delete Epzz;

    delete sigHx;
    delete sigHy;
    delete sigDx;
    delete sigDy;

    delete mHx0;
    delete mHx1;
    delete mHx2;
    delete mHx3;
    delete mHy0;
    delete mHy1;
    delete mHy2;
    delete mHy3;
    delete mDz0;
    delete mDz1;
    delete mDz2;
    delete mDz4;
    delete mEz1;
    delete ICEx;
    delete ICEy;
    delete IDz;

    delete x;
    delete y;
}

void MainWindow::closeEvent(QCloseEvent *event){
    if(QString::compare(ui->pB_Start->text(),"Pause")==0 || QString::compare(ui->pB_Start->text(),"Continue")==0){
        stop = true;
        worldDestroy();
    }
    event->accept();
}

MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::on_hSldr_d_valueChanged(int value){
    if(ui->chkBx_inf->isChecked())
        d=100000;
    else
        d = value;
    if(nt==0){
        marker.setValue((zero-Nx/2)*dx*xyunits,(Nsy+d-Ny/2)*dy*xyunits);
        marker.attach(ui->qwtPlot);
        ui->qwtPlot->replot();
    }
    ui->spnBx_d->setValue((Nsy+d-Ny/2)*dy*xyunits);
}


void MainWindow::on_hSldr_zero_valueChanged(int value){
    zero = value;
    if(nt==0){
        marker.setValue((zero-Nx/2)*dx*xyunits,(Nsy+d-Ny/2)*dy*xyunits);
        marker.attach(ui->qwtPlot);
        ui->qwtPlot->replot();
    }
    ui->spnBx_zero->setValue( (zero-Nx/2)*dx*xyunits );
}

void MainWindow::on_chkBx_Energ_toggled(bool checked){
    if(QString::compare(ui->chkBx_Energ->text(),"Energy")==0){
        ui->chkBx_Energ->setText("Electric Field");
    }else if(QString::compare(ui->chkBx_Energ->text(),"Electric Field")==0){
        ui->chkBx_Energ->setText("Energy");
    }
}

void MainWindow::on_spnBx_step_editingFinished(){
     frameStep = ui->spnBx_step->value();
     ui->hSldr_pltPhase->setMaximum(ui->spnBx_step->value()-1);
}

void MainWindow::on_spnBx_wl_editingFinished(){
    freq = c0/(ui->spnBx_wl->value()/1000);
}

void MainWindow::on_chkBx_Wave_toggled(bool checked){
    if(QString::compare(ui->chkBx_Wave->text(),"Wave")==0){
        ui->chkBx_Wave->setText("Pulse");
    }else if(QString::compare(ui->chkBx_Wave->text(),"Pulse")==0){
        ui->chkBx_Wave->setText("Wave");
    }
}

void MainWindow::on_chkBx_gauss_toggled(bool checked){
    if(checked)
        ui->frame_gauss->setEnabled(true);
    else
        ui->frame_gauss->setEnabled(false);
}

void MainWindow::on_chkBx_inf_toggled(bool checked){
    on_hSldr_d_valueChanged(50);
}

void MainWindow::on_hSldr_gaussL_valueChanged(int value){
    gaussL = value;
    ui->spnBx_gaussL->setValue(gaussL*dx*xyunits);
}

void MainWindow::on_hSldr_pltPhase_valueChanged(int value){
    ui->spnBx_pltPhase->setValue(value);
    plotPhase = value;
}

void MainWindow::on_hSldr_width_valueChanged(int value){
    w = value;
    Ni = Nx/2-w;
    Nf = Nx/2+w;
    ui->spnBx_width->setValue(2*w*dx*xyunits);
    ui->hSldr_gaussL->setMaximum(Nf-Ni);
    ui->spnBx_gaussL->setMaximum((Nf-Ni)*dx*xyunits);
}





