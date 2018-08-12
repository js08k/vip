#ifndef LINUXMAIN_H
#define LINUXMAIN_H

#include <QCoreApplication>
#include <QObject>
#include <QTimer>
#include <QDebug>

class ucSimulator
        : public QObject
{
    Q_OBJECT
public:
    ucSimulator();
    virtual ~ucSimulator();

public slots:
    void infoTimer();

private slots:
    // uc backend
    void scanHardware();

    // uC Tasks
    void ipKernel();
    void memscan();

    // Handler for interupts
    void ipKernelInterrupt();

signals:
    // Interupts
    void interrupt();

private:
    QTimer m_timer;

    QTimer m_infoTimer;

    int m_interruptAverage;
};

#endif // MAIN_H
