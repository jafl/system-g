/******************************************************************************
 BeginEditingTask.h

	Copyright (C) 2000 by John Lindal.

 ******************************************************************************/

#ifndef _H_BeginEditingTask
#define _H_BeginEditingTask

#include <jx-af/jx/JXIdleTask.h>
#include <jx-af/jcore/JBroadcaster.h>

class JPoint;
class FileTreeTable;
class FileTreeNode;

class BeginEditingTask : public JXIdleTask, virtual public JBroadcaster
{
public:

	BeginEditingTask(FileTreeTable* table, const JPoint& cell);

	virtual ~BeginEditingTask();

	void			Perform();
	virtual void	Perform(const Time delta, Time* maxSleepTime);

private:

	FileTreeTable*	itsTable;	// owns us
	FileTreeNode*	itsNode;	// not owned; nullptr if dead
};

#endif
