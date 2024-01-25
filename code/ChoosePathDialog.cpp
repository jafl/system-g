/******************************************************************************
 ChoosePathDialog.cpp

	Clients must call BuildWindow() after constructing the object.
	Derived classes must override BuildWindow() and call SetObjects()
	at the end of their implementation.

	BASE CLASS = JXChoosePathDialog

	Copyright (C) 1999 by Glenn W. Bach.

 ******************************************************************************/

#include "ChoosePathDialog.h"
#include "PrefsMgr.h"
#include "globals.h"
#include <jx-af/jx/JXWindow.h>
#include <jx-af/jx/JXStaticText.h>
#include <jx-af/jx/JXTextButton.h>
#include <jx-af/jx/JXTextCheckbox.h>
#include <jx-af/jx/JXPathInput.h>
#include <jx-af/jx/JXPathHistoryMenu.h>
#include <jx-af/jx/JXNewDirButton.h>
#include <jx-af/jx/JXScrollbarSet.h>
#include <jx-af/jx/JXCurrentPathMenu.h>
#include <jx-af/jcore/jDirUtil.h>
#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Create (static)

 ******************************************************************************/

ChoosePathDialog*
ChoosePathDialog::Create
	(
	const SelectPathType	type,
	const JString&			startPath,
	const JString&			fileFilter,
	const JString&			message
	)
{
	auto* dlog = jnew ChoosePathDialog(type, fileFilter);
	dlog->BuildWindow(startPath, message);
	return dlog;
}

/******************************************************************************
 Constructor

 ******************************************************************************/

ChoosePathDialog::ChoosePathDialog
	(
	const SelectPathType	type,
	const JString&			fileFilter
	)
	:
	JXChoosePathDialog(type, fileFilter)
{
}

/******************************************************************************
 Destructor

 ******************************************************************************/

ChoosePathDialog::~ChoosePathDialog()
{
}

/******************************************************************************
 BuildWindow (private)

 ******************************************************************************/

void
ChoosePathDialog::BuildWindow
	(
	const JString& startPath,
	const JString& message
	)
{
// begin JXLayout

	auto* window = jnew JXWindow(this, 310,400, JString::empty);

	auto* pathLabel =
		jnew JXStaticText(JGetString("pathLabel::ChoosePathDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedBottom, 20,20, 40,20);
	pathLabel->SetToLabel(false);

	auto* pathHistory =
		jnew JXPathHistoryMenu(1, window,
					JXWidget::kFixedRight, JXWidget::kFixedBottom, 260,20, 30,20);
	assert( pathHistory != nullptr );

	auto* filterLabel =
		jnew JXStaticText(JGetString("filterLabel::ChoosePathDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedBottom, 20,50, 40,20);
	filterLabel->SetToLabel(false);

	auto* filterHistory =
		jnew JXStringHistoryMenu(1, window,
					JXWidget::kFixedRight, JXWidget::kFixedBottom, 260,50, 30,20);
	assert( filterHistory != nullptr );

	auto* showHiddenCB =
		jnew JXTextCheckbox(JGetString("showHiddenCB::ChoosePathDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedBottom, 60,80, 130,20);
	showHiddenCB->SetShortcuts(JGetString("showHiddenCB::shortcuts::ChoosePathDialog::JXLayout"));

	auto* currPathMenu =
		jnew JXCurrentPathMenu(JGetRootDirectory(), window,
					JXWidget::kFixedRight, JXWidget::kFixedBottom, 20,110, 180,20);
	assert( currPathMenu != nullptr );

	auto* scrollbarSet =
		jnew JXScrollbarSet(window,
					JXWidget::kHElastic, JXWidget::kFixedBottom, 20,140, 180,190);
	assert( scrollbarSet != nullptr );

	auto* upButton =
		jnew JXTextButton(JGetString("upButton::ChoosePathDialog::JXLayout"), window,
					JXWidget::kFixedRight, JXWidget::kFixedBottom, 220,140, 30,20);
	upButton->SetShortcuts(JGetString("upButton::shortcuts::ChoosePathDialog::JXLayout"));

	auto* homeButton =
		jnew JXTextButton(JGetString("homeButton::ChoosePathDialog::JXLayout"), window,
					JXWidget::kFixedRight, JXWidget::kFixedBottom, 250,140, 40,20);
	homeButton->SetShortcuts(JGetString("homeButton::shortcuts::ChoosePathDialog::JXLayout"));

	auto* desktopButton =
		jnew JXTextButton(JGetString("desktopButton::ChoosePathDialog::JXLayout"), window,
					JXWidget::kFixedRight, JXWidget::kFixedBottom, 220,160, 70,20);
	assert( desktopButton != nullptr );

	auto* newDirButton =
		jnew JXNewDirButton(window,
					JXWidget::kFixedRight, JXWidget::kFixedBottom, 220,190, 70,20);
	assert( newDirButton != nullptr );

	itsSelectButton =
		jnew JXTextButton(JGetString("itsSelectButton::ChoosePathDialog::JXLayout"), window,
					JXWidget::kFixedRight, JXWidget::kFixedBottom, 220,250, 70,20);
	itsSelectButton->SetShortcuts(JGetString("itsSelectButton::shortcuts::ChoosePathDialog::JXLayout"));

	itsOpenButton =
		jnew JXTextButton(JGetString("itsOpenButton::ChoosePathDialog::JXLayout"), window,
					JXWidget::kFixedRight, JXWidget::kFixedBottom, 219,279, 72,22);
	itsOpenButton->SetShortcuts(JGetString("itsOpenButton::shortcuts::ChoosePathDialog::JXLayout"));

	auto* cancelButton =
		jnew JXTextButton(JGetString("cancelButton::ChoosePathDialog::JXLayout"), window,
					JXWidget::kFixedRight, JXWidget::kFixedBottom, 220,310, 70,20);
	cancelButton->SetShortcuts(JGetString("cancelButton::shortcuts::ChoosePathDialog::JXLayout"));

	auto* explanText =
		jnew JXStaticText(JGetString("explanText::ChoosePathDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedBottom, 20,340, 270,20);
	explanText->SetToLabel(false);

	itsNewWindowCB =
		jnew JXTextCheckbox(JGetString("itsNewWindowCB::ChoosePathDialog::JXLayout"), window,
					JXWidget::kFixedLeft, JXWidget::kFixedBottom, 20,370, 150,20);
	itsNewWindowCB->SetShortcuts(JGetString("itsNewWindowCB::shortcuts::ChoosePathDialog::JXLayout"));

	auto* pathInput =
		jnew JXPathInput(window,
					JXWidget::kHElastic, JXWidget::kFixedBottom, 60,20, 200,20);
	pathInput->SetIsRequired(true);
	pathInput->ShouldAllowInvalidPath(false);
	pathInput->ShouldRequireWritable(false);

	auto* filterInput =
		jnew JXInputField(window,
					JXWidget::kHElastic, JXWidget::kFixedBottom, 60,50, 200,20);
	assert( filterInput != nullptr );

// end JXLayout

	itsNewWindowCB->SetState(GetPrefsMgr()->WillOpenNewWindows());

	SetObjects(scrollbarSet, pathLabel, pathInput, pathHistory,
			   filterLabel, filterInput, filterHistory,
			   itsOpenButton, itsSelectButton, cancelButton,
			   upButton, homeButton, desktopButton,
			   newDirButton, showHiddenCB, currPathMenu, startPath, message);
}

/******************************************************************************
 OpenInNewWindow

 ******************************************************************************/

bool
ChoosePathDialog::OpenInNewWindow()
	const
{
	return itsNewWindowCB->IsChecked();
}
