//! Local project includes
#include "main.h"

#include <unistd.h>

//! Qt Includes
#include <QCoreApplication>
#include <QTime>

//! vIPstack includes
#include "vmang.h"
#include "vstring.h"
#include "vipv4.h"
#include "vheap.h"
#include "telnet.h"
#include "httpd.h"
#include "vbase.h"

const char *rootPage =
        "<html>"
        "   <body>"
        "       <p>Root Page</p>"
        "       <a href=\"/hello\">"
        "           <p>Hello Link</p>"
        "       </a>"
        "   </body>"
        "</html>";

const char *HelloWorldPage =
        "<html>"
        "   <head>"
        "       <title>Hello World!</title>"
        "   </head>"
        "   <body>"
        "       <p>Hello World!</p>"
        "       <a href=\"/\">"
        "           <p>Root Link</p>"
        "       </a>"
        "   </body>"
        "</html>";

// Declared within vportable.cpp
extern QByteArray networkBuffer;

// Declared within vportable.cpp
extern int network_fd;

int main( int argc, char *argv[] )
{
    QCoreApplication app( argc, argv );

    ucSimulator sim;
    Q_UNUSED( sim )

    // Add the webpage
    addContent( "/", rootPage, vstrlen( rootPage ) );
    addContent( "/hello", HelloWorldPage, vstrlen( HelloWorldPage ) );

    return app.exec();
}

ucSimulator::ucSimulator()
    : m_interruptAverage( 0 )
{
    networkBuffer.clear();

    vIPInitializeManager();

    // Create the ip address, gateway and subnet
    vifup( vstrtoip( "10.0.1.130" ) );
    vifnm( 0, vstrtoip( "255.255.255.0" ) );
    vifgw( 0, vstrtoip( "10.0.1.1" ) );

    // Initialize the telnet
    vInitializeTelnet();

    vInitializeHttpd();

    // Scan hardware, read raw socket
    connect( &m_timer, SIGNAL( timeout() ), SLOT( scanHardware() ) );

    // Connect uC regular tasks here
    connect( &m_timer, SIGNAL( timeout() ), SLOT( ipKernel() ) );
    connect( &m_timer, SIGNAL( timeout() ), SLOT( memscan() ) );

    // Connect uC interrupts here
    connect( this, SIGNAL( interrupt() ), SLOT( ipKernelInterrupt() ) );

    // Start the timer, this starts the uC simulator
    m_timer.start();
}

ucSimulator::~ucSimulator()
{

}

void ucSimulator::scanHardware()
{
    int readSize = 0;

    QByteArray readInBuffer( 1500, 0x00 );

    // Read in up to 64 bytes from the network_fd
    readSize = read( network_fd, readInBuffer.data(), readInBuffer.size() );

    if( readSize > 0 )
    {
        // Append the read data to the network buffer
        networkBuffer.append( readInBuffer, readSize );

        emit interrupt();
    }
}

void ucSimulator::ipKernel()
{
    vMangKernel();
}

void ucSimulator::memscan()
{
    vMemCheck();
}

void ucSimulator::ipKernelInterrupt()
{
    QTime timer;
    timer.start();

    // Interrupt content
    {
        vGetMACpacket();
    }

    m_interruptAverage += timer.elapsed();
    m_interruptAverage /= 2;
}

void ucSimulator::infoTimer()
{
    qDebug() << "Interrupt is averaging" << m_interruptAverage << "mS.";
}


