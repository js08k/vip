#include "include/taskstack.h"

/* This is a linked list style fifo task list */
/* This is built with the expectation that an interrupt
 * can queue complex tasks quickly and they can be handled
 * at a non-time critical time. While at the same time stopping
 * from race conditions on multi thread implementations.
 */

struct	task {
	struct task *next;
	void *function;
	void *data;
};

struct task *TaskHead;	/* Points to the task that has been listed the longest */
struct task *TaskTail;	/* Points to the most currently listed task */

struct task *vTask( void )	{
	struct task *temp;
/* TODO: Lock task list */
	temp = TaskHead;
	/* Update the task head to look at the next task */
	TaskHead = temp.next;
/* TODO: Unlock task list */
	/* Clear the value for the next task */
	temp.next = null;
	/* Return the removed task */
	return temp;
}

void vListTask( void )	{

}
