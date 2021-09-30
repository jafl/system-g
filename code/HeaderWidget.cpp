/******************************************************************************
 HeaderWidget.cpp

	Maintains a row to match the columns of a JTable.

	BASE CLASS = JXTable

	Copyright (C) 1996 by John Lindal.

 ******************************************************************************/

#include "HeaderWidget.h"
#include "FileTreeTable.h"
#include <jx-af/jx/JXDisplay.h>
#include <jx-af/jx/JXWindow.h>
#include <jx-af/jcore/JFontManager.h>
#include <jx-af/jx/JXColorManager.h>
#include <jx-af/jx/jXPainterUtil.h>
#include <jx-af/jx/jXGlobals.h>
#include <jx-af/jcore/jAssert.h>

const JCoordinate kBorderWidth = 3;

/******************************************************************************
 Constructor

 ******************************************************************************/

HeaderWidget::HeaderWidget
	(
	FileTreeTable*	table,
	JXScrollbarSet*		scrollbarSet,
	JXContainer*		enclosure,
	const HSizingOption	hSizing,
	const VSizingOption	vSizing,
	const JCoordinate	x,
	const JCoordinate	y,
	const JCoordinate	w,
	const JCoordinate	h
	)
	:
	JXColHeaderWidget(table, scrollbarSet, enclosure, hSizing,vSizing, x,y, w,h),
	itsTable(table)
{
	JColorManager* mgr = GetDisplay()->GetColorManager();

	SetRowBorderInfo(0, mgr->GetBlackColor());
	SetColBorderInfo(0, mgr->GetBlackColor());

	WantInput(false,false);	// we don't want focus
	SetBackColor(mgr->GetWhiteColor());
}

/******************************************************************************
 Destructor

 ******************************************************************************/

HeaderWidget::~HeaderWidget()
{
}

/******************************************************************************
 GetPreferredHeight (static)

 ******************************************************************************/

JCoordinate
HeaderWidget::GetPreferredHeight
	(
	JFontManager* fontMgr
	)
{
	return JFontManager::GetDefaultFont().GetLineHeight(fontMgr) + 2*kBorderWidth;
}

/******************************************************************************
 TableDrawCell (virtual protected)

 ******************************************************************************/

void
HeaderWidget::TableDrawCell
	(
	JPainter&		p,
	const JPoint&	cell,
	const JRect&	rect
	)
{
	JFont font = JFontManager::GetDefaultFont();
	if (itsTable->IsCurrentCol(cell.x))
	{
		font.SetUnderlineCount(1);
	}
	p.SetFont(font);

	const JString str				= itsTable->GetColTitle(cell.x);
	const JCoordinate bufferWidth	= itsTable->GetBufferWidth(cell.x);

	JRect sRect = rect;
	if (bufferWidth < 0)
	{
		sRect.right += bufferWidth;
		p.String(sRect, str, JPainter::kHAlignRight, JPainter::kVAlignCenter);
	}
	else
	{
		sRect.left += bufferWidth;
		p.String(sRect, str, JPainter::kHAlignLeft, JPainter::kVAlignCenter);
	}
}

/******************************************************************************
 HandleMouseDown (virtual protected)

 ******************************************************************************/

void
HeaderWidget::HandleMouseDown
	(
	const JPoint&			pt,
	const JXMouseButton	button,
	const JSize			clickCount,
	const JXButtonStates&	buttonStates,
	const JXKeyModifiers&	modifiers
	)
{
	JPoint cell;
	if (GetCell(pt, &cell))
	{
		itsTable->SetCurrentColIndex(cell.x);
		TableRefresh();
	}
}
