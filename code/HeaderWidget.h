/******************************************************************************
 HeaderWidget.h

	Interface for the HeaderWidget class

	Copyright (C) 1996 by John Lindal.

 ******************************************************************************/

#ifndef _H_HeaderWidget
#define _H_HeaderWidget

#include "jx-af/jx/JXColHeaderWidget.h"

class FileTreeTable;

class HeaderWidget : public JXColHeaderWidget
{
public:

	HeaderWidget(FileTreeTable* table,
					JXScrollbarSet* scrollbarSet,
					JXContainer* enclosure,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h);

	virtual ~HeaderWidget();

	static JCoordinate	GetPreferredHeight(JFontManager* fontMgr);

protected:

	void	TableDrawCell(JPainter& p, const JPoint& cell, const JRect& rect) override;

	void	HandleMouseDown(const JPoint& pt, const JXMouseButton button,
									const JSize clickCount,
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers) override;

private:

	FileTreeTable*	itsTable;	// we don't own this
};

#endif
