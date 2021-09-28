/******************************************************************************
 SyGBeginEditingTask.h

	Copyright (C) 2000 by John Lindal.

 ******************************************************************************/

#ifndef _H_SyGBeginEditingTask
#define _H_SyGBeginEditingTask

#include <jx-af/jx/JXIdleTask.h>
#include <jx-af/jcore/JBroadcaster.h>

class JPoint;
class SyGFileTreeTable;
class SyGFileTreeNode;

class SyGBeginEditingTask : public JXIdleTask, virtual public JBroadcaster
{
public:

	SyGBeginEditingTask(SyGFileTreeTable* table, const JPoint& cell);

	virtual ~SyGBeginEditingTask();

	void			Perform();
	virtual void	Perform(const Time delta, Time* maxSleepTime);

private:

	SyGFileTreeTable*	itsTable;	// owns us
	SyGFileTreeNode*	itsNode;	// not owned; nullptr if dead
};

#endif
