/*******************************************************************
 * File: vheap.c
 * Written by: Joshua Strickland
 *
 * Version: 1.0		September 2010
 *			1.1		November  2010
 *			2.0		June	  2011
 *			2.1		October	  2011
 *
 * Dependencies: vheap.h
 * 				 vtypes.h
 *
 * Note:
 * 	1.0: This source file needs to be verified of all situations
 * That it will work properly. Designed to work for uc0S, a real
 * time operating system.
 *	1.1: Redesigned to work with Virtual IP as a buffer for
 * incoming and outgoing IP packets. Files renamed from ucHeap to
 * vIPheap.c/vIPheap.h. The idea of ucOS has been dropped.
 *  2.0: Changed buffer handle to be a bfd (buffer file descriptor)
 *  this stops modification to the buffer outside the API. Buffer
 *  check-summing was removed since the purpose of bfds replaced the
 *  purpose of the checksums. A buffer peek function was created
 *  allowing the next character to be read without being removed. A
 *  mass peek function was created functioning just as a read does but
 *  without removing the data from the buffer. Structure definitions
 *  were moved to the source file to follow a more proprietary format.
 *  Function added to enable the ability to uncover a heap being
 *  corrupted. This can be run during an idle task and is fairly
 *  accurate as long as ucfree/ucalloc/ucrealloc are not in the middle
 *  of being run in another task/process on a single core processor.
 *  A variable that tracks the current amount of memory being used
 *  has been added, along with a function that can return this value.
 *  2.1: Locks added to the heap. The real API was changed to add
 *  a "use locks" flag. However a wrapper was created that resembled
 *  the old API but will always set the "use locks" flag. This allows
 *  for internal calls to functions to execute without blocking but
 *  external calls will be blocked if the heap is blocked. And internal
 *  calls to functions to not be blocked. This stops multiple calls
 *  from occurring on a multi-threading operating system. A watermark
 *  was added to show the maximum amount of memory that had been used
 *  at any given time.
 *  2.1.1: Slight code changes, nothing that affects API. Comment
 *  review and corrections. << these changes not finished >>
 *******************************************************************/
#include "vheap.h"
#include "vportable.h"

/*!
    Heap Tracking structure
 */
struct heapTrack
{
    uint8_t offset:7;
    uint8_t inUse:1;
    uint8_t reserved[BLOCKSIZE-1];
};

struct bufTrack
{
    void *pBase; 	/* Pointer to the base of the buffer */
    void *pHead; 	/* Pointer to the head (next free location) */
    void *pTail; 	/* Pointer to the tail (next location to read) */
    uint16_t bSize;	/* Size of the buffer */
    uint16_t bfd;	/* The buffer's file descriptor */
    uint16_t pipe;	/* */
};

/*******************************
    GLOBAL Variables:
********************************/
static struct heapTrack heap[HEAPSIZE]; // Should only be accessed from the API 
static struct bufTrack *bufStack;
static int heapusage;
static int watermark;

static struct 
{
    int *stack;
    int size;
} fdStack = { null, 0 }; // File descriptor stack

// True API - function prototypes 
void *_alloc_( uint16_t size, bool locks );
void *_realloc_( void *base, uint16_t size, bool locks );
void *_free_( void *base, bool locks );

/* Fake API - function definitions, wrappers to the true API */
void *ucalloc( uint16_t size )
{
#ifndef __glib_malloc__
    return _alloc_( size, true );
#else
    return malloc( size );
#endif
}

void *ucrealloc( void *base, uint16_t size )	{
#ifndef __glib_malloc__
    return _realloc_( base, size, true );
#else
    return realloc( base, size );
#endif
}

void *ucfree( void *base )	{
#ifndef __glib_malloc__
    return _free_( base, true );
#else
    free( base );
    return null;
#endif
}

/*******************************************
    FUNCTION Definitions:
********************************************/

/*!
    Returns the current number of bytes that are used by the algorithm
*/
int vUsage( void )	{
    return heapusage;
}

/*!
    Returns the peak number of bytes that have been used by the algorithm
*/
int vWatermark( void )	{
    return watermark;
}

/*!
    Initializes the heap api. This must be called before using any other portion of the api.
*/
void initializeucheap( void )
{
    int i;

    /* Initialize all blocks to be control blocks */
    for( i = 0; i < HEAPSIZE; ++i )
    {
        heap[i].inUse = false;
        heap[i].offset = 1;
    }

    heap[ HEAPSIZE - 1 ].offset = 0;

    /* Initialize the heap usage to be zero */
    heapusage = 0;
    /* Initialize the water mark to zero */
    watermark = 0;

    /* Initialize the file descriptor stack */
    vInitializefd();

    /* Initialize buffer API, heap must be initialized first */
    vInitializeBuffer();

    /* Initialize the heaplock */
//    mutex_init( &heaplock );
}

#ifndef __glib_malloc__
/*******************************************************
    void *vAlloc( uint16_t size )
        Allocate memory from the heap returning a pointer
    to be used. If the heap is found to be full, a memory
    check is performed to ensure no corruption has taken
    place. If no corruption has been found null is returned
    indicating that no pointer was given and that the heap
    is full. If an error is found in the heap, thel
    allocation program will loop indefinitely. This will
    cause a handler to execute in the future.
 *******************************************************/
void *_alloc_( uint16_t size, bool locks )
{
    (void)locks;
    int cnt, start, nSpace, aSpace;

    /* Size is zero, no need to allocate space */
    if( size == 0 )
    {
        return 0;
    }

//    if( locks )
//    {
//        mutex_lock( &heaplock );
//    }

    // Increment size by one to acomadate control block.
    ++size;

    // Convert the size into blocks. Always assuming one extra block is needed
    // since ( BLOCKSIZE - 1 )/BLOCKSIZE percent times this assumption is true.
    nSpace = ( size / BLOCKSIZE ) + 1;

    // End of available region (measured in blocks)
    aSpace = ( HEAPSIZE - nSpace + 1 );

    // Set the start point to zero
    start = 0;

    while( start < aSpace )
    {
        // For each block denoted 'start', search for the size requirement
        // specified by 'nSpace'. This is a check that there is enough space
        // left within the heap.
        for( cnt=0; cnt < nSpace; ++cnt )
        {
            // If a block is found to be in use, reposition 'start'
            // and restart the search.
            if( heap[ start + cnt ].inUse )
            {
                // Reset the start value to point to
                // the next possible block to be used
                start = start + cnt + heap[ start + cnt ].offset;

                // Leave the contiguous free space checking
                break;
            }

            // All space is free and contiguous
            if( cnt == ( nSpace - 1 ) )
            {
                // When cnt has reached the last value of the search and
                // no errors were found, this value of 'start' can be
                // used.
                heap[start].inUse = 1;
                heap[start].offset = nSpace;

                // Calculate the resulting memory being used
                heapusage += ( BLOCKSIZE * nSpace ) - 1;

                if( heapusage > watermark )
                {
                    watermark = heapusage;
                }

//                if( locks )
//                {
//                    mutex_unlock( &heaplock );
//                }

                // return the pointer that was found
                return (void *)( (pointer)&( heap[start] ) + 1 );
            }
        }
    }
    // The search did not find a usable block, performing a memory check

//    if( locks )
//    {
//        mutex_unlock( &heaplock );
//    }

    // memory was found to be corrupted
    if( vMemCheck() )
    {
        // No handlers have been implemented, lock the processor
        while( 1 );
    }
    else
    {
        // Heap is legitimately full
        // return null, indicating no pointer was assigned
        return null;
    }
}
#endif

#ifndef __glib_malloc__
/*****************************************************************
    void *vRealloc( void *base, uint16_t size )
        When expanding memory an attempt to allocate contiguous
    memory to the already allocated memory will be made, if it is
    not possible an allocation is made for the entire new memory and
    data in the old memory is copied to the new while the old memory
    is deallocated.
    When shrinking memory the memory is not moved but memory that
    is no longer needed is deallocated.
    If the operation does not shrink or expand the memory the
    pointer passed as base is returned.
 *****************************************************************/
void *_realloc_( void *base, uint16_t size, bool locks )
{
    struct heapTrack *pheapTrack;	/* Overlay to track the state of memory 				*/
    int nSpace;						/* nSpace: how many blocks are needed	 				*/
    int cnt;						/* Generic counter 										*/
    void *ptr;						/* Temporary pointer for expanding allocation			*/

    /* If base is null, divert call to _alloc_() */
    if( !base || size == 0 )
    {
        return _alloc_( size, locks );
    }

//    if( locks )
//    {
//        mutex_lock( &heaplock );
//    }

    // Increment the size by one.
    ++size;

    /* Set base to point at a valid start of block */
    pheapTrack = (struct heapTrack *)( (pointer)base - 1 );

    /* Calculate the amount of blocks needed for the new allocation */
    nSpace = size / BLOCKSIZE;

    nSpace++;

    /* More than is allocated is required */
    if( nSpace > pheapTrack->offset )	{
        /* Attempt to expand memory without using vAlloc, will only succeed if	*/
        /* memory following already allocated memory is contiguous and free		*/
        /* Set this search to start at the end of known allocated memory		*/
        cnt = pheapTrack->offset;
        /*  */
        while( cnt < nSpace )
        {
            // Check if the next contigous block of memory is in use.
            if( pheapTrack[cnt].inUse /*&& !pheapTrack[cnt].offset*/ )
            {
                // Memory was in use, have to move.
                break;
            }
            /* Memory was able to be expanded, expand	*/
            /* and return the original pointer passed	*/
            else if( ( cnt + 1 ) == nSpace ) {
                /* Calculate the resulting memory being used */
                heapusage += ( ( nSpace - pheapTrack->offset ) * BLOCKSIZE );
                if( heapusage > watermark )
                    watermark = heapusage;
                /* Set the new offset to the control block */
                pheapTrack->offset = nSpace;
                /* Unlock the heap if locks are being used */
//                if( locks )
//                {
//                    mutex_unlock( &heaplock );
//                }
                /* Return the original pointer passed */
                return base;
            }
            /* not finished searching, continuing */
            else
                cnt++;
        }

        /* Allocate memory for the new size */
        ptr = _alloc_( size, false );
        /* If a new allocation of memory was successful */
        if( ptr )	{
            /* Copy the old data to the new memory location */
            vmemcpy( ptr, base, ( ( pheapTrack->offset * BLOCKSIZE ) - 1 ) );
            /* Free the old memory */
            _free_( base, false );
        }
//        if( locks )
//        {
//            mutex_unlock( &heaplock );
//        }
        /* Return a pointer to the new memory, null	*/
        /* will be returned if vAlloc has failed	*/
        return ptr;
    }
    /* Less memory than is allocated is required */
    else if( nSpace < pheapTrack->offset )	{
        /* free the no longer needed space */
        for( cnt=nSpace; cnt < pheapTrack->offset; cnt++ )	{
            pheapTrack[cnt].inUse = 0;
            pheapTrack[cnt].offset = 1;
            heapusage -= BLOCKSIZE;
        }
        /* Set the new value for the offset, must be performed last */
        pheapTrack->offset = nSpace;
//        if( locks )
//        {
//            mutex_unlock( &heaplock );
//        }
        /* return the pointer */
        return base;
    }
    /* Requesting the same amount of memory that is allocated */
    else	{
//        if( locks )
//        {
//            mutex_unlock( &heaplock );
//        }
        /* return result */
        return base;
    }
}
#endif

#ifndef __glib_malloc__
/***********************************************************
    void *vFree( void *base )
        Attempts to free the memory allocated at the pointer
    base. An error can occur if base does not exist within the
    heap.
************************************************************/
void *_free_( void *base, bool locks )	{
    (void)locks;
    struct heapTrack *pheapTrack;
    int cnt;

    /* Stop the free command from trying to free memory that is known to not be in the stack */
    /*	if( !( ( base > ( (void *)heap - 1 ) ) & ( base < (void *)&( heap[HEAPSIZE] ) ) ) )	{
        return null;
    }
*/
//    if( locks )
//    {
//        mutex_lock( &heaplock );
//    }

    /* Set the pointer to the control block */
    pheapTrack = (struct heapTrack *)( (pointer)base - 1 );

    if( !pheapTrack->inUse )
    {
        while(1);
    }

    /* Free blocks used (all except for the control block) */
    for( cnt=1; cnt < pheapTrack->offset; cnt++ )	{
        /* Set inUse flag to be a free block */
        pheapTrack[cnt].inUse = false;
        /* Set offset pointer to point to the next block */
        pheapTrack[cnt].offset = 1;
    }
    /* Calculate heap usage prior to releasing the control block */
    heapusage -= ( ( pheapTrack->offset * BLOCKSIZE ) - 1 );
    /* Release the control block */
    /* Set the offset pointer to the next possible block */
    pheapTrack->offset = 1;
    /* Clear the inUse flag of the control block */
    pheapTrack->inUse = 0;
//    if( locks )
//    {
//        mutex_unlock( &heaplock );
//    }
    /* Return null, indication of a successful free operation */
    return null;
}
#endif

/*********************************************************************
    bool vMemCheck( void )
        Performs a light check of the heap. Cycling through the
    heap calculating the amount of memory that is expected to be
    allocated based on the control structures. The calculated value is
    compared to the value tracked from within vAlloc, vRealloc & vFree.
    If the numbers match, memory is assumed to be non-corrupted (it is
    possible for corruption to still be present) and false is returned.
    If the numbers do not match, memory corruption can be guaranteed
    and true is returned.
*********************************************************************/
bool vMemCheck( void )	{
//    mutex_lock( &heaplock );

    uint16_t cnt;
    uint16_t trackused;
    cnt = 0;
    trackused = 0;

    /* cycle through the entire heap */
    while( cnt < HEAPSIZE && heap[cnt].offset )
    {
        /* the block is in use, calculate the amount of memory used by this block */
        if( heap[cnt].inUse )
            trackused += ( ( BLOCKSIZE * heap[cnt].offset ) - 1 );
        /* Move the generic counter to the next available block */
        cnt += heap[cnt].offset;
    }
//    mutex_unlock( &heaplock );
    /* no errors were found in the memory */
    if( trackused == heapusage )
    {
        return false;
    }
    /* error(s) were found in the memory */
    else
    {
        while(1);
        return true;
    }
}

bool vInitializefd( void )	{
    return true;
}

/*******************************************
    int vfdGet( void )
        Generate a new file descriptor. Add
    it to the heap and return the value.
 *******************************************/
int vfdGet( void )	{
    int n,fd;
    void *temp;

    fd = 1;
    n = 0;

    if( !fdStack.stack )
    {
        // If the stack has not previously been allocated, allocate for it
        temp = ucalloc( sizeof( int ) );
    }
    else
    {
        // Resize the stack to accomidate the new fd being added
        temp = ucrealloc( (void *)fdStack.stack, sizeof( int ) * ( fdStack.size + 1 ) );
    }

    // Check the results of the allocation/reallocation
    if( !temp )
    {
        return 0;
    }

    /* Save the new value for the stack pointer */
    fdStack.stack = (int *)temp;

    /* find the lowest usable fd */
    while( n < fdStack.size )
    {
        if( fdStack.stack[n++] == fd )
        {
            /* Increment the fd that is intended for use */
            fd++;
            /* Reset the value of the search */
            n = 0;
            /* if fd overflows */
            if( fd < 1 )
                return 0;
        }
    }
    /* Save the fd that is being used */
    fdStack.stack[fdStack.size++] = fd;
    /* Return the file descriptor */
    return fd;
}

/*
    bool vfdDel( int fd )
 */
bool vfdDel( int fd )	{
    int n = 0;

    if( !fdStack.stack )
    {
        return false;
    }

    /* find the location of the fd */
    while( n < fdStack.size )	{
        if( fdStack.stack[n] == fd )	{
            while( ( n + 1 ) < fdStack.size )	{
                fdStack.stack[n] = fdStack.stack[n+1];
                n++;
            }

            --fdStack.size;

            if( fdStack.size )
                /* Resize the fd stack */
                fdStack.stack = (int *)ucrealloc( fdStack.stack, ( sizeof( int ) * ( fdStack.size ) ) );
            else	{
                ucfree( fdStack.stack );
                fdStack.stack = null;
            }

            return true;
        }
        else
            n++;
    }

    return false;
}


/*
    given a buffer or a pipe file descriptor, this function
returns the address of the fifo buffer. Null is returned if
the buffer does not exist.
 */
struct bufTrack *fdtobuffer( uint16_t fd )
{
    uint16_t n = 0;

    /* do not search for a match on a null file descriptor */
    if( !fd )
        return null;

    /* locate the buffer in the buffer stack */
    while( bufStack[n].bfd > 0 )	{
        /* the buffer was found if the fd matches either fd */
        if( ( bufStack[n].bfd == fd ) || ( bufStack[n].pipe == fd ) )
            return (struct bufTrack *)&( bufStack[n] );
        /* keep searching... */
        else
            n++;
    }

    return null;
}

void vInitializeBuffer( void )
{
    bufStack = (struct bufTrack *)ucalloc( sizeof( struct bufTrack ) );
    /* allocation of the buffer stack was successful */
    if( bufStack )
    {
        /* Buffer fd must be greater than zero to be valid */
        bufStack[0].bfd = 0;
    }
}

uint16_t vOpenBuf( uint16_t size )
{
    uint16_t buf;
    void *tmpptr;

    ++size;

    buf = 0;

    while(1)	{
        /* the last entry in the buffer stack was reached */
        if( bufStack[buf].bfd == 0 )
        {
            /* attempt to increase the size of the buffer stack by one structure */
            tmpptr = ucrealloc( (void *)bufStack, sizeof( struct bufTrack ) * ( buf + 2 ) );

            /* reallocation was successful */
            if( tmpptr )
            {
                /* set the new position of the buffer stack */
                bufStack = (struct bufTrack *)tmpptr;
                /* set the new entry to be the new last entry in the stack */
                bufStack[buf+1].bfd = 0;
                /* Allocate the requested memory for the buffer */
                bufStack[buf].pBase = ucalloc( size );

                /* Allocation of memory for the buffer was successful */
                if( bufStack[buf].pBase )	{
                    /* assign a fd to the new buffer */
                    bufStack[buf].bfd = vfdGet();

                    if( !bufStack[buf].bfd )
                    {
                        // Release the memory taken by the
                        ucfree( bufStack[buf].pBase );

                        /* Return buffer stack to the its previous size, this will end up using */
                        /* vShrink, no worry about losing the bufStack pointer to an error */
                        bufStack = (struct bufTrack *)ucrealloc( (void *)bufStack, sizeof( struct bufTrack ) * ( buf + 1 ) );

                        /* Return that an error has occurred */
                        return null;
                    }

                    /* set the size of the buffer in the buffer stack */
                    bufStack[buf].bSize = size;
                    /* set the initial pointer to the head value */
                    bufStack[buf].pHead = bufStack[buf].pBase;
                    /* set the initial pointer to the tail value */
                    bufStack[buf].pTail = (void *)( (pointer)bufStack[buf].pBase + (pointer)bufStack[buf].bSize - 1 );
                    /* */
                    bufStack[buf].pipe = 0;
                    /* return the new buffer file descriptor */
                    return bufStack[buf].bfd;
                }
                /* Allocation of memory for the buffer was not successful */
                else	{
                    /* Return buffer stack to the its previous size, this will end up using */
                    /* vShrink, no worry about losing the bufStack pointer to an error */
                    bufStack = (struct bufTrack *)ucrealloc( (void *)bufStack, sizeof( struct bufTrack ) * ( buf + 1 ) );
                    /* Return that an error has occurred */
                    return null;
                }
            }
            /* reallocation of the buffer stack was not successful */
            else
                /* return unsuccessful */
                return null;
        }
        /* increment the buffer stack counter */
        buf++;
    }
}
/************************************************************
bool vPeekBuf( uint16_t bfd, void *buffer, uint16_t size )
    This function performs Identically to vReadBuf with the
difference being when data is read from the buffer, the data
is not removed from the buffer.

When used as a pipe, both the input and output ends of the
pipe will be able to use this function.
where output pipe is defined in such a way:

uint16_t pfd[2];

ucpipe( pfd );

pfd[0] == bufStack[n].bfd == input pipe
pfd[1] == bufStack[n].pipe == output pipe
*************************************************************/
uint16_t vPeekBuf( uint16_t bfd, void *buffer, uint16_t size )
{
    struct bufTrack *fifo;
    uint16_t count;
    void *temp;

    /* locate the buffer in the buffer stack */
    fifo = fdtobuffer( bfd );

    /* if the buffer was not found */
    if( !fifo )
        return null;

    /* Copy the pointer to the tail so as not to modify the tail */
    temp = fifo->pTail;

    for( count=0; count < size; count++ )	{
        /* no more readable data, exit the for loop */
        if( ( ( (pointer)temp + 1 ) == ( (pointer)fifo->pBase + (pointer)fifo->bSize ) ) && ( (pointer)fifo->pHead == (pointer)fifo->pBase ) )
            break;
        else if( ( (pointer)temp + 1 ) == (pointer)fifo->pHead )
            break;
        temp = (void *)( (pointer)temp + 1 );
        /* Set the location of the last filled spot */
        /* Roll the buffer if it reaches the end */
        if( (pointer)temp == ( (pointer)fifo->pBase + (pointer)fifo->bSize ) )
            temp = fifo->pBase;
        /* Copy data into the buffer */
        ( (uint8_t *)buffer )[count] = *( (uint8_t *)temp );
    }

    /* return the amount of data that was read */
    return count;
}

/*
 * If the buffer is a pipe, only the output fd may read
 * use this function. passing the input will return zero
 * and no data will be written to buffer[].
 */
uint16_t vReadBuf( uint16_t bfd, void *buffer, uint16_t size )
{
    struct bufTrack *fifo;
    uint16_t count;

    /* locate the buffer in the buffer stack */
    fifo = fdtobuffer( bfd );

    /* if this is a pipe */
    if( fifo->pipe > 0 )	{
        /* Do not allow input side of	*/
        /* the pipe to read the data	*/
        if( fifo->bfd == bfd )
            return null;
    }

    for( count=0; count < size; count++ )
    {
        /* no more readable data, exit the for loop */
        if( ( ( (pointer)fifo->pTail + 1 ) == ( (pointer)fifo->pBase + fifo->bSize ) ) && ( (pointer)fifo->pHead == (pointer)fifo->pBase ) )
            break;
        else if( ( (pointer)fifo->pTail + 1 ) == (pointer)fifo->pHead )
            break;

        /* Set the location of the last filled spot */
        fifo->pTail = (void *)( (pointer)fifo->pTail + 1 );
        /* Roll the buffer if it reaches the end */
        if( (pointer)fifo->pTail == ( (pointer)fifo->pBase + (pointer)fifo->bSize ) )
            fifo->pTail = fifo->pBase;
        if( buffer )
        {
            /* Copy data into the buffer */
            ( (uint8_t *)buffer )[count] = *( (uint8_t *)fifo->pTail );
        }
    }

    /* return the amount of data that was read */
    return count;
}
/*
 * If the buffer is a pipe, only the input fd may read
 * use this function. passing the output fd will return zero
 * and no data will be written into the buffer.
 */
uint16_t vWriteBuf( uint16_t bfd, const void *buffer, uint16_t size )
{
    struct bufTrack *fifo;
    uint16_t count;

    /* locate the buffer in the buffer stack */
    fifo = fdtobuffer( bfd );

    /* if this is a pipe */
    if( fifo->pipe > 0 )	{
        /* Do not allow output side of		*/
        /* the pipe to write to the buffer	*/
        if( fifo->pipe == bfd )
            return null;
    }

    for( count=0; count < size; count++ )	{
        /* Next location has not been read yet, break so not to write over pending data */
        if( fifo->pHead == fifo->pTail )
            break;
        /* Copy data into the buffer */
        *( (uint8_t *)fifo->pHead ) = ( (uint8_t *)buffer )[count];
        /* Set the location of the next open spot */
        fifo->pHead = (void *)( (pointer)fifo->pHead + 1 );
        /* Roll the buffer if it reaches the end */
        if( (pointer)fifo->pHead == ( (pointer)fifo->pBase + (pointer)fifo->bSize ) )	{
            fifo->pHead = fifo->pBase;
        }
    }
    /* return the amount of data that was written to the buffer */
    return count;
}
/*
When used as a pipe, both the input and output ends of the
pipe will be able to use this function.
*/
uint16_t vBufGetFree( uint16_t bfd )
{
    struct bufTrack *fifo;

    /* locate the buffer in the buffer stack */
    fifo = fdtobuffer( bfd );

    if( fifo->pHead == fifo->pTail )
    {
        return 0;
    }
    else if( fifo->pHead > fifo->pTail )
    {
        return ( fifo->bSize - (uint16_t)( (pointer)fifo->pHead - (pointer)fifo->pTail ) );
    }
    else
    {
        return (uint16_t)( (pointer)fifo->pTail - (pointer)fifo->pHead );
    }
}

uint16_t vBufGetSize( uint16_t bfd )
{
    struct bufTrack *fifo;

    fifo = fdtobuffer( bfd );

    return fifo->bSize;
}

/*
When used as a pipe, both the input and output ends of the
pipe will be able to use this function.
*/
uint16_t vBufGetPending( uint16_t bfd )
{
    struct bufTrack *fifo;

    /* locate the buffer in the buffer stack */
    fifo = fdtobuffer( bfd );

    if( fifo->pTail == fifo->pHead )
    {
        return fifo->bSize - 1;
    }
    else if( fifo->pTail > fifo->pHead )
    {
        return ( fifo->bSize - (uint16_t)( (pointer)fifo->pTail - (pointer)fifo->pHead + 1 ) );
    }
    else
    {
        return ( (pointer)fifo->pHead - (pointer)fifo->pTail - 1 );
    }
}

void vCloseBuf( uint16_t bfd )
{
    uint16_t buf;

    buf = 0;

    /* locate the buffer in the buffer stack */
    while(1)	{
        /* the end of the stack was reached */
        if( bufStack[buf].bfd == 0 )
            break;
        /* the buffer was found */
        else if( bufStack[buf].bfd == bfd )
            break;
        /* keep searching... */
        else
            buf++;
    }
    /* if the buffer was found in the stack */
    if( bufStack[buf].bfd == bfd )
    {
        vfdDel( bufStack[buf].bfd );
        /* deallocating memory associated with this buffer */
        ucfree( bufStack[buf].pBase );
        while( bufStack[buf].bfd > 0 )
        {
            /* Next entry of the stack is not empty, copy it to the current entry */
            if( bufStack[buf+1].bfd > 0 )
            {
                bufStack[buf].pBase = bufStack[buf+1].pBase;
                bufStack[buf].pHead = bufStack[buf+1].pHead;
                bufStack[buf].pTail = bufStack[buf+1].pTail;
                bufStack[buf].bSize = bufStack[buf+1].bSize;
                bufStack[buf].bfd 	= bufStack[buf+1].bfd;
                buf++;
            }
            /* This is now last valid entry in the stack */
            else
            {
                /* set all entries to be null */
                bufStack[buf].pBase = null;
                bufStack[buf].pHead = null;
                bufStack[buf].pTail = null;
                bufStack[buf].bSize = 0;
                bufStack[buf].bfd 	= 0;
            }
        }

        bufStack = (struct bufTrack *)ucrealloc( (void *)bufStack, sizeof( struct bufTrack ) * ( buf + 1 ) );
    }
}

/*******************************************************************************
 * void vipMemcpy( void* destination, const void *source, unsigned short size )
 * 		Copies a number of bytes from the source to the destination, the
 * 	number of bytes are specified by the number "size".
 *******************************************************************************/
void vmemcpy( void *destination, const void *source, int size )
{
    int i = 0;

    if( source )
        for( ; i < size; ++i )
            ( (char *)destination )[i] = ( (const char *)source )[i];
    else
        for( ; i < size; ++i )
            ( (char *)destination )[i] = 0;
}
