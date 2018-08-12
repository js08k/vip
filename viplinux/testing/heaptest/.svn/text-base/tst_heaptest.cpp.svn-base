#include <QString>
#include <QtTest>
#include <QDateTime>
#include <QDebug>
#include "vheap.h"

class HeapTest : public QObject
{
    Q_OBJECT
    
public:
    HeapTest();
    
private Q_SLOTS:
    void initTestCase();
    void alloc();
    void alloc2();
    void resize();
    void resize2();
    void fileDescriptor();
    void buffer();

private:
    void memcheck( void *memory, int size );
};

void HeapTest::memcheck( void *memory, int size )
{
    memset( memory, 0xFF, size );
    QVERIFY2( !vMemCheck(), "Memory check failed during memcheck 0xFF" );

    memset( memory, 0xAA, size );
    QVERIFY2( !vMemCheck(), "Memory check failed during memcheck 0xAA" );

    memset( memory, 0x55, size );
    QVERIFY2( !vMemCheck(), "Memory check failed during memcheck 0x55" );

    memset( memory, 0x00, size );
    QVERIFY2( !vMemCheck(), "Memory check failed during memcheck 0x00" );
}

void HeapTest::initTestCase()
{
    int seed = QDateTime::currentDateTimeUtc().toTime_t();

    qDebug() << "Seeding with" << seed;

    qsrand( seed );

    // Initialize the heap
    initializeucheap();
}

HeapTest::HeapTest()
{

}

/*!
 * \brief HeapTest::alloc Test a simple alloc & free
 */
void HeapTest::alloc()
{
    // Choose a testValue between 10 & 1000
    int testValue = qrand() % 990 + 10;

    char *testPointer = (char *)ucalloc( testValue );

    QVERIFY2( testPointer, "Memory allocation failed" );
    QVERIFY2( !vMemCheck(), "Memory check after allocation failed" );

    memcheck( testPointer, testValue );

    ucfree( testPointer );

    QVERIFY2( !vMemCheck(), "Memory check failed after free" );
}

void HeapTest::alloc2()
{
    // Choose a testValue between 10 & 1000
    int testValue = qrand() % 990 + 10;

    char *testPointer1 = (char *)ucalloc( testValue );

    QVERIFY2( testPointer1, "First memory allocation failed" );
    QVERIFY2( !vMemCheck(), "Memory check after first allocation failed" );

    memcheck( testPointer1, testValue );

    char *testPointer2 = (char *)ucalloc( testValue );

    QVERIFY2( testPointer2, "Second memory allocation failed" );
    QVERIFY2( !vMemCheck(), "Memory check after second allocation failed" );

    memcheck( testPointer2, testValue );

    ucfree( testPointer1 );

    QVERIFY2( !vMemCheck(), "Memory check after first free failed" );

    ucfree( testPointer2 );

    QVERIFY2( !vMemCheck(), "Memory check after second free failed" );
}

void HeapTest::resize()
{
    // Choose a testValue between 10 & 1000
    int testValue1 = ( qrand() % 990 + 10 ) / 2;
    int testValue2 = testValue1 * 2;

    // Allocate memory
    char *testPointer = (char *)ucalloc( testValue1 );
    QVERIFY2( testPointer, "Memory allocation failed" );
    QVERIFY2( !vMemCheck(), "Memory check after allocation failed" );

    // Perform a write check to the memory
    memcheck( testPointer, testValue1 );

    testPointer = (char *)ucrealloc( testPointer, testValue2 );
    QVERIFY2( testPointer, "Memory allocation failed" );
    QVERIFY2( !vMemCheck(), "Memory check after reallocation failed" );

    // Perform a write check to the memory
    memcheck( testPointer, testValue2 );

    // Free the memory
    ucfree( testPointer );
    QVERIFY2( !vMemCheck(), "Memory check after free failed" );
}

// Allocates a block of memory of size A. A control block is allocated
// using the OS allocation and random data is written to this control block.
// The random data is copied to the testBlock. A second test block is created
// (the heap tries to allocate in order, this will help in forcing the heap to
// perform a move when reallocating). The first block is then reallocated to
// size A*2 (requiring a move). The memory contained in the first block's new
// location is compared to the control block.
void HeapTest::resize2()
{
    // Choose a testValue between 10 & 1000
    int testValue1 = ( qrand() % 990 + 10 ) / 2;
    int testValue2 = testValue1 * 2;

    // Allocate memory
    char *testPointer1 = (char *)ucalloc( testValue1 );
    QVERIFY2( testPointer1, "Memory allocation failed" );
    QVERIFY2( !vMemCheck(), "Memory check failed" );

    // Perform a write check to the memory
    memcheck( testPointer1, testValue1 );

    // Allocate memory for the second testPointer
    char *control = (char *)ucalloc( testValue1 );
    QVERIFY2( control, "Memory allocation failed" );
    QVERIFY2( !vMemCheck(), "Memory check failed" );

    // Perform a write check to the testPointer2
    memcheck( control, testValue1 );

    // Create a random control character set
    for( int i = 0; i < testValue1; ++i )
    {
        control[i] = 0xFF & qrand();
    }

    // Set testPointer1 to contain the same data as the control set.
    vmemcpy( testPointer1, control, testValue1 );
    QVERIFY2( memcmp( testPointer1, control, testValue1 ) == 0, "Copy failed" );
    QVERIFY2( !vMemCheck(), "Memory check failed" );

    // Perform a reallocation, this should require a move
    testPointer1 = (char *)ucrealloc( testPointer1, testValue2 );
    QVERIFY2( testPointer1, "Reallocation failed" );
    QVERIFY2( !vMemCheck(), "Memory check failed" );

    // compare moved memory to the control set
    QVERIFY2( memcmp( testPointer1, control, testValue1 ) == 0, "Realloc failed during copy" );

    // Perform a write check to the memory
    memcheck( testPointer1, testValue2 );

    // Free the memory of the first pointer
    ucfree( testPointer1 );
    QVERIFY2( !vMemCheck(), "Memory check failed" );

    // Free the memory of the second pointer
    ucfree( control );
    QVERIFY2( !vMemCheck(), "Memory check failed");
}

void HeapTest::fileDescriptor()
{
    int testSize = ( qrand() % 90 + 10 );

    int *fileDescriptors = (int *)ucalloc( sizeof( int ) * testSize );
    QVERIFY2( !vMemCheck(), "Memory check failed" );

    for( int i = 0; i < testSize; ++i )
    {
        fileDescriptors[i] = vfdGet();
        QVERIFY2( fileDescriptors[i], "vfdGet() failed" );
        QVERIFY2( !vMemCheck(), "Memory check failed" );
    }

    for( int i = 0; i < testSize; ++i )
    {
        QVERIFY2( vfdDel( fileDescriptors[i] ), "vfdDel() failed" );
        QVERIFY2( !vMemCheck(), "Memory check failed");
    }

    ucfree( fileDescriptors );
    QVERIFY2( !vMemCheck(), "Memory check failed" );
}

void HeapTest::buffer()
{
    int testSize2 = ( qrand() % 45 + 10 );
    int testSize1 = testSize2 * 2;

    int bufferfd = vOpenBuf( testSize1 );
    QVERIFY( !vMemCheck() );

    /*
     * Test that the buffer was correctly initialized
     */
    {
        // Verify the buffer was allocated
        QVERIFY( bufferfd != 0 );

        // Verify the full buffer size was allocated
        QVERIFY( vBufGetFree( bufferfd ) == testSize1 );

        // Verify no data exists within the buffer
        QVERIFY( vBufGetPending( bufferfd ) == 0 );
    }

    char *control = (char *)ucalloc( testSize2 );
    QVERIFY( !vMemCheck() );

    char *result = (char *)ucalloc( testSize2 );
    QVERIFY( !vMemCheck() );

    /*
     * Create the control data set for testing writing and reading
     * to and from the buffers.
     */
    for( int i = 0; i < testSize2; ++i )
    {
        control[i] = 0xff & qrand();
        QVERIFY( !vMemCheck() );
    }

    QVERIFY( vWriteBuf( bufferfd, control, testSize2 ) == testSize2 );
    QVERIFY( !vMemCheck() );

    /*
     * Verify the size of data written to the buffer matches the size
     * remaining in the buffer as well as size pending in the buffer.
     */
    {
        QVERIFY( vBufGetPending( bufferfd ) == testSize2 );

        QVERIFY( vBufGetFree( bufferfd ) ==  ( testSize1 - testSize2 ) );
    }

    QVERIFY( vPeekBuf( bufferfd, result, testSize2 ) == testSize2 );
    QVERIFY( !vMemCheck() );

    /*
     *  Verify the operation of the vPeekBuf.
     *  Verify the correct data was read and that the data was not removed.
     */
    {
        QVERIFY( memcmp( result, control, testSize2 ) == 0 );

        QVERIFY( vBufGetPending( bufferfd ) == testSize2 );

        QVERIFY( vBufGetFree( bufferfd ) ==  ( testSize1 - testSize2 ) );
    }

    // Clear out data in result
    memcheck( result, testSize2 );

    QVERIFY( vReadBuf( bufferfd, result, testSize2 ) == testSize2 );
    QVERIFY( !vMemCheck() );

    /*
     *  Verify the operation of the vReadBuff.
     *  Verify the correct data was read and that the data was removed.
     */
    {
        QVERIFY( memcmp( result, control, testSize2 ) == 0 );

        QVERIFY( vBufGetPending( bufferfd ) == 0 );

        QVERIFY( vBufGetFree( bufferfd ) == testSize1 );
    }

    QVERIFY( vWriteBuf( bufferfd, control, testSize2 ) == testSize2 );
    QVERIFY( !vMemCheck() );

    QVERIFY( vWriteBuf( bufferfd, control, testSize2 ) == testSize2 );
    QVERIFY( !vMemCheck() );

    /*
     * Verify the write as pending data
     * Verify the write filled the buffer
     */
    {
        QVERIFY( vBufGetPending( bufferfd ) == testSize1 );
        QVERIFY( vBufGetFree( bufferfd ) == 0 );
    }

    QVERIFY( vReadBuf( bufferfd, 0, testSize1 ) == testSize1 );
    QVERIFY( !vMemCheck() );

    /*
     * Verify that removing the data worked.
     */
    {
        QVERIFY( vBufGetPending( bufferfd ) == 0 );
        QVERIFY( vBufGetFree( bufferfd ) == testSize1 );
    }

    vCloseBuf( bufferfd );
    QVERIFY( !vMemCheck() );

    ucfree( control );
    QVERIFY( !vMemCheck() );

    ucfree( result );
    QVERIFY( !vMemCheck() );
}

QTEST_APPLESS_MAIN(HeapTest)

#include "tst_heaptest.moc"
