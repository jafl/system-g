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

	~BeginEditingTask() override;

	void	Perform();

protected:

	void	Perform(const Time delta) override;

private:

	FileTreeTable*	itsTable;	// owns us
	FileTreeNode*	itsNode;	// not owned; nullptr if dead
};

#endif
