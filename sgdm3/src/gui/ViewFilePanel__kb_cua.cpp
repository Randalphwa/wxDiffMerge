// ViewFilePanel__kb_cua.cpp
// keyboard/keybinding-related stuff for CUA (used on Win32 & Linux).
//////////////////////////////////////////////////////////////////
/*
  <table>
    <tr>
      <th>key</th>
	  <th>open</th>
	  <th>control</th>
	  <th>shift</th>
	  <th>shift-control</th>
	</tr>
	<tr>
	  <th>HOME</th>
	  <td>move caret to BOL</td>
	  <td>move caret to BOF</td>
	  <td>select to BOL</td>
	  <td>select to BOF</td>
	</tr>
	<tr>
	  <th>END</th>
	  <td>move caret to EOL</td>
	  <td>move caret to EOF</td>
	  <td>select to EOL</td>
	  <td>select to EOF</td>
	</tr>
	<tr>
	  <th>PAGE-UP</th>
	  <td>move caret up 1 page</td>
	  <td>scroll 1 page (do not move caret)</td>
	  <td>select up 1 page</td>
	  <td>select up 1 page</td>
	</tr>
	<tr>
	  <th>PAGE-DOWN</th>
	  <td>move caret down 1 page</td>
	  <td>scroll 1 page (do not move caret)</td>
	  <td>select down 1 page</td>
	  <td>select down 1 page</td>
	</tr>
	<tr>
	  <th>ESCAPE</th>
	  <td colspan=4>close window</td>
	</tr>
	<tr>
	  <th>TAB</th>
	  <td>insert a TAB character (editing panel only)</td>
	  <td>cycle keyboard focus</td>
	  <td>insert a TAB character (editing panel only)</td>
	  <td>cycle keyboard focus backwards</td>
	</tr>
  </table>

  <table>
    <tr>
      <th>key</th>
	  <th>open</th>
	  <th>control</th>
	  <th>shift</th>
	  <th>shift-control</th>
	</tr>
	<tr>
	  <th>BACKSPACE</th>
	  <td>first clear selection, then delete previous character</td>
	  <td>first clear selection, then delete previous word</td>
	  <td>first clear selection, then delete previous character</td>
	  <td>first clear selection, then delete previous word</td>
	</tr>
  </table>
  <table>
	<tr>
	  <th>DELETE</th>
	  <td>first clear selection, then delete next character</td>
	  <td>first clear selection, then delete next word</td>
	  <td>if selection, cut to clipboard; else delete next character</td>
	  <td>first clear selection, then delete next word</td>
	</tr>
  </table>
  <table>
	<tr>
	  <th>ENTER</th>
	  <td></td>
	  <td colspan=3>TODO decide what to do with these.  For now, pretend to be simple ENTER.</td>
	</tr>
  </table>
  <table>
	<tr>
	  <th>INSERT</th>
	  <td>ignore</td>
	  <td>copy to clipboard</td>
	  <td>paste from clipboard</td>
	  <td>ignore</td>
	</tr>
  </table>
  <table>
	<tr>
	  <th>WINDOWS_MENU_KEY</th>
	  <td>raise context menu</td>
	  <td>raise context menu</td>
	  <td>raise context menu</td>
	  <td>raise context menu</td>
	</tr>
  </table>
  <table>
	<tr>
	  <th>key</th>
	  <td></td>
	  <td></td>
	  <td></td>
	  <td></td>
	</tr>
  </table>

  <p>Because of the way wxWidgets constructs and controls the accelerator table
     (automatically build/maintained by the MenuBar), we cannot add accelerators
     that do not have menu items in the main menu.  So, we catch a few keys here.
  </p>
  <table>
    <tr>
      <th>key</th>
      <th>action</th>
    </tr>
    <tr>
      <td>Ctrl+Shift+Z</td><td>alias for REDO</td>
	  <td>F5</td><td>Reload</td>
    </tr>
  </table>

*/

//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fs.h>
#include <fl.h>
#include <de.h>
#include <fd.h>
#include <gui.h>

//////////////////////////////////////////////////////////////////

#if defined(__WXMSW__) || defined(__WXGTK__)

//////////////////////////////////////////////////////////////////

void ViewFilePanel::onCharEvent(wxKeyEvent & e)
{
#if 0 && defined(DEBUG)
	wxChar c = e.GetUnicodeKey();
	wxLogTrace(wxTRACE_Messages, _T("ViewFilePanel:CharEvent: [%p][char %x][keycode %x][cmd %d][meta %d][alt %d][control %d][shift %d][modifiers %x]"),
			   this,
			   (int)c,
			   e.GetKeyCode(),
			   e.CmdDown(),
			   e.MetaDown(),
			   e.AltDown(),
			   e.ControlDown(),
			   e.ShiftDown(),
			   e.GetModifiers());
#endif

	// on MSW *and apparently now on Linux under 2.9.5*
	// when the user press ALT-F -- that is, hold down ALT and press F
	// (as opposed to pressing ALT, releasing ALT, and pressing F) the user should
	// get the File menu raised.  but we get a KEYDOWN for the ALT, a KEYDOWN for
	// the F and then a CHAR event for "F (with Alt)".  wxWidgets doesn't give this
	// to the menu until *AFTER* we have done something with it.  so we need to
	// interceed here and Skip() it and *NOT* try to insert the character.

	if (e.AltDown())
	{
		e.Skip();
		return;
	}

	if (!_dispatchCharEvent(e))
		e.Skip();
}

bool ViewFilePanel::_dispatchCharEvent(wxKeyEvent & e)
{
	// non-printables (ascii control characters 0x00 thru 0x1f)
	// should be handled using the key-event.  some of these are
	// common (like ^m & ^j) and some are per-platform (cua vs mac...)
	// (like ^a on the mac).
	//
	// Update 2013/07/30 under 2.9.5 (at least on Linux) we get the
	// char event *before* the accelerator table is inspected. So we
	// need to return false so that our caller will call skip() so
	// that wxWidgets will process the accelerator for the various
	// control keys.  W8669.

	if (e.GetUnicodeKey() < 0x20)
		return false;

	// this problem got better in 2.6.3 but still has problems.
	// 
	// in particular, in 2.6.3, numeric keys on the keypad on MSW
	// cause duplicate CHAR messages -- wxWidgets sends a bogus
	// char-event - 'a' for '0', 'b' for '1', ... and then MSW
	// sends the real char event.
	// 
	// WX-ODDITY: for some reason, we get CHAR events in addition
	// WX-ODDITY: KEY-DOWN events for all of the various labeled
	// WX-ODDITY: keys on the keyboard -- like F1, INSERT, etc.
	// WX-ODDITY: what's worse is that they are different on all
	// WX-ODDITY: three platforms.  so, we filter these out.
	// WX-ODDITY: we don't want to do the filtering in the key-down
	// WX-ODDITY: event because that can mess up menu accelerators
	// WX-ODDITY: and the Win32 system keys (like F10).

	if (e.GetKeyCode() >= WXK_START)	// silently eat (bogus) "chars"
		return true;

	if (!isEditPanel())
		return _kb_alert();

	// insert the given character at the caret.  i'm not sure
	// that we need the cua-vs-mac indirection for CHAR events
	// because all chars should do a self-insert.

	return _kb_insert_text( e.GetUnicodeKey() );
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::onKeyDownEvent(wxKeyEvent & e)
{
#if 0 && defined(DEBUG)
	wxLogTrace(wxTRACE_Messages, _T("ViewFilePanel:KeyDown: [%p][keycode %x][cmd %d][meta %d][alt %d][control %d][shift %d][modifiers %x]"),
			   this,
			   e.GetKeyCode(),
			   e.CmdDown(),
			   e.MetaDown(),
			   e.AltDown(),
			   e.ControlDown(),
			   e.ShiftDown(),
			   e.GetModifiers());
#endif

	if (isBound() && _dispatchKeyDownEvent(e))
		return;

	e.Skip();
}

bool ViewFilePanel::_dispatchKeyDownEvent(wxKeyEvent & e)
{
	switch (e.GetKeyCode())
	{
	case WXK_PAGEDOWN:	case WXK_NUMPAD_PAGEDOWN:	return _vk_pagedown(e);
	case WXK_PAGEUP:	case WXK_NUMPAD_PAGEUP:		return _vk_pageup(e);
	
	case WXK_HOME:		case WXK_NUMPAD_HOME:		return _vk_home(e);
	case WXK_END:		case WXK_NUMPAD_END:		return _vk_end(e);

	case WXK_LEFT:		case WXK_NUMPAD_LEFT:		return _vk_left(e);
	case WXK_RIGHT:		case WXK_NUMPAD_RIGHT:		return _vk_right(e);

	case WXK_UP:		case WXK_NUMPAD_UP:			return _vk_up(e);
	case WXK_DOWN:		case WXK_NUMPAD_DOWN:		return _vk_down(e);

	case WXK_TAB:		return _vk_tab(e);
	case WXK_ESCAPE:	return _vk_escape(e);

	case WXK_BACK:									return _vk_backspace(e);
	case WXK_DELETE:	case WXK_NUMPAD_DELETE:		return _vk_delete(e);

	case WXK_RETURN:	case WXK_NUMPAD_ENTER:		return _vk_enter(e);

	case WXK_INSERT:	case WXK_NUMPAD_INSERT:		return _vk_insert(e);

	case WXK_WINDOWS_MENU:							return _vk_context_menu(e);

	default:			return _vk__default(e);
	}
}

//////////////////////////////////////////////////////////////////

#define DCL_KB(fn)		bool ViewFilePanel::fn(wxKeyEvent & e)
#define DCL_KBu(fn)		bool ViewFilePanel::fn(wxKeyEvent & /*e*/)

//////////////////////////////////////////////////////////////////
// keybindings functions to handle an individual VK.
// [] return TRUE if we completely handled the key and key processing should stop;
// [] return FALSE if caller should call e.skip() to allow processing to continue.
//////////////////////////////////////////////////////////////////

DCL_KB(_vk_pagedown)
{
	if (e.ControlDown() && !e.ShiftDown())	// plain-control just scrolls
	{
		m_pViewFile->keyboardScroll_pagedown(m_kSync,wxVERTICAL);
		return true;
	}

	int col = _getGoal();
	int rowCurrent = m_vfcCaret.getRow();
	int page = m_pViewFile->getScrollThumbSize(m_kSync,wxVERTICAL);
	int row  = rowCurrent + page - 1;

	if (!_kb_getNextRow(row, &row))
	{
		int rowValid = rowCurrent;
		row = rowCurrent;

		while (_kb_getNextRow(row, &row))
			rowValid = row;

		if (rowCurrent == rowValid)		// caret already is on last valid line.
			return true;

		row = rowValid;
	}
	
	if (!_kb_getThisCol(row,col, &col))
		return true;			// void row -- should not happen

	_doUpdateCaret(e.ShiftDown(),row,col);
	_kb_ensureCaretVisible();

	return true;
}

DCL_KB(_vk_pageup)
{
	if (e.ControlDown() && !e.ShiftDown())	// plain-control just scrolls
	{
		m_pViewFile->keyboardScroll_pageup(m_kSync,wxVERTICAL);
		return true;
	}

	// we want to go up a page-worth (while respecting the goal column).
	// if that row is a void and doesn't exist, we keep going until we
	// hit a valid row.  if we run off the beginning, we need to find
	// the first valid line (which should be prior to the caret).

	int col = _getGoal();
	int rowCurrent = m_vfcCaret.getRow();
	int page = m_pViewFile->getScrollThumbSize(m_kSync,wxVERTICAL);
	int row  = rowCurrent - page + 1;

	if (!_kb_getPrevRow(row, &row))
	{
		int rowValid = rowCurrent;
		row = rowCurrent;

		while (_kb_getPrevRow(row, &row))
			rowValid = row;

		if (rowCurrent == rowValid)		// caret already is on first valid line.
			return true;

		row = rowValid;
	}
	
	if (!_kb_getThisCol(row,col, &col))
		return true;			// void row -- should not happen

	_doUpdateCaret(e.ShiftDown(),row,col);
	_kb_ensureCaretVisible();

	return true;
}

DCL_KB(_vk_home)
{
	int col = 0;
	int row = m_vfcCaret.getRow();

	if (e.ControlDown())		// goto to beginning-of-document; (else goto beginning-of-line)
		row = 0;

	_doUpdateCaret(e.ShiftDown(),row,col);
	_setGoal(col);
	_kb_ensureCaretVisible();

	return true;
}

DCL_KB(_vk_end)
{
	int col = 0;
	int row = m_vfcCaret.getRow();

	if (e.ControlDown())		// goto to end-of-document
	{
		col = 0;				// column 0 of the EOF row (past the end of the content)
		row = _kb_getEOFRow();
	}
	else						// goto to end-of-line
	{
		(void)_kb_getEOLCol(row,&col);
	}

	_doUpdateCaret(e.ShiftDown(),row,col);
	_setGoal(col);
	_kb_ensureCaretVisible();

	return true;
}

DCL_KB(_vk_left)
{
	// LEFT-ARROW

	// alt -- apply default patch from right (copy from right
	//              "left-ward") (like winmerge)  auto-advance
	//              if enabled in options dialog.
	// alt+control -- apply and advance (like winmerge) (without
	//              regard to options dialog setting.)
	// shift is ignored when alt is down

	if (e.AltDown())
	{
		// the focus panel (us) could be any of the 3 panels.
		// we want this to behave as if it came from the parent or frame
		// and apply stuff in PANEL_T2 to the edit panel.  therefore,
		// this key is only defined on a merge window.

		if (m_pViewFile->getNrTopPanels() != 3)
			return true;	// TODO should we beep or something?

		bool bAdvance = (   e.ControlDown()
						 || (gpGlobalProps->getBool(GlobalProps::GPL_MISC_AUTO_ADVANCE_AFTER_APPLY)));

		m_pViewFile->applyDefaultAction(PANEL_T2,bAdvance);

		return true;
	}

	// open -- move caret backward 1 char
	// control - move caret backward 1 word
	// shift -- select backward 1 char
	// shift+control -- select backward 1 word

	// when Ctrl pressed, we do things by word.
	// when Ctrl not pressed, we do things by character.

	// when Shift pressed, we start/extend the selection.
	// otherwise, we just move the caret to the new destination.

	ViewFileCoord vfc;
	if (!_kb_getPrev(e.ControlDown(),&m_vfcCaret,&vfc,NULL))
		return true;

	_doUpdateCaret(e.ShiftDown(),vfc);
	_setGoal(vfc.getCol());
	_kb_ensureCaretVisible();

	return true;
}

DCL_KB(_vk_right)
{
	// RIGHT-ARROW

	// alt -- apply default patch from left (copy from left
	//              "right-ward") (like winmerge)  auto-advance
	//              if enabled in options dialog.
	// alt+control -- apply and advance (like winmerge) (without
	//              regard to options dialog setting.)
	// shift is ignored when alt is down

	if (e.AltDown())
	{
		bool bAdvance = (   e.ControlDown()
						 || (gpGlobalProps->getBool(GlobalProps::GPL_MISC_AUTO_ADVANCE_AFTER_APPLY)));

		m_pViewFile->applyDefaultAction(PANEL_T0,bAdvance);

		return true;
	}

	// open -- move caret foreward 1 char
	// control - move caret foreward 1 word
	// shift -- select foreward 1 char
	// shift+control -- select foreward 1 word

	// when Ctrl pressed, we do things by word.
	// when Ctrl not pressed, we do things by character.

	// when Shift pressed, we start/extend the selection.
	// otherwise, we just move the caret to the new destination.

	ViewFileCoord vfc;
	if (!_kb_getNext(e.ControlDown(),&m_vfcCaret,&vfc,NULL))
		return true;

	_doUpdateCaret(e.ShiftDown(),vfc);
	_setGoal(vfc.getCol());
	_kb_ensureCaretVisible();

	return true;
}

DCL_KB(_vk_up)
{
	// UP-ARROW

	// alt -- move to previous change (like our Shift-F7 accelerator and winmerge keybinding)
	// control is ignored when alt is down
	// shift is ignored when alt is down

	if (e.AltDown())
	{
		m_pViewFile->keyboardScroll_delta(false,false);
		return true;
	}

	// open -- move caret up 1 line
	// control -- scroll 1 line (do not move caret)
	// shift -- select up 1 line
	// shift+control -- select up 1 line
	//

	if (e.ControlDown() && !e.ShiftDown())	// plain-control just scrolls
	{
		m_pViewFile->keyboardScroll_lineup(m_kSync,wxVERTICAL);
		return true;
	}

	// otherwise, move caret/extend selection to previous line
	// using goal column.  since goal column may not exist (or
	// may be in the middle of a tab) in the previous line, we
	// do not update the goal column -- just the caret.  if the
	// previous line is a void, we keep going until we find a
	// valid line.

	int col = _getGoal();
	int row = m_vfcCaret.getRow();

	if (!_kb_getPrevRow(row, &row))
		return true;			// no previous row (at row 0 or only void)
	if (!_kb_getThisCol(row,col, &col))
		return true;			// void row -- should not happen

	_doUpdateCaret(e.ShiftDown(),row,col);
	_kb_ensureCaretVisible();

	return true;
}

DCL_KB(_vk_down)
{
	// DOWN-ARROW

	// alt -- move to next change (like our F7 accelerator and winmerge keybinding)
	// control is ignored when alt is down
	// shift is ignored when alt is down

	if (e.AltDown())
	{
		m_pViewFile->keyboardScroll_delta(true,false);
		return true;
	}

	// open -- move caret down 1 line
	// control -- scroll 1 line (do not move caret)
	// shift -- select down 1 line
	// shift+control -- select down 1 line

	if (e.ControlDown() && !e.ShiftDown())	// plain-control just scrolls
	{
		m_pViewFile->keyboardScroll_linedown(m_kSync,wxVERTICAL);
		return true;
	}

	int col = _getGoal();
	int row = m_vfcCaret.getRow();

	if (!_kb_getNextRow(row, &row))
		return true;			// no next row (at row 0 or only void)
	if (!_kb_getThisCol(row,col, &col))
		return true;			// void row -- should not happen

	_doUpdateCaret(e.ShiftDown(),row,col);
	_kb_ensureCaretVisible();

	return true;
}

DCL_KBu(_vk_escape)
{
	m_pViewFile->getFrame()->postEscapeCloseCommand();

	return true;
}

DCL_KB(_vk_tab)
{
	// the normal wxWidgets handling causes this to do a dialog-like
	// focus thing -- so that focus cycles between panels just like
	// the fields on a dialog.
	//
	// this is ok for for read-only panels, but when in the 
	// editable panel tab should insert a tab.
	//
	// to avoid confusing everybody, let's let:
	// [] CTRL-TAB do the focus cycling,
	// [] SHIFT-CTRL-TAB should do the backward-focus-cycle,
	// [] plain-TAB do character insert (when editable), and
	// [] SHIFT-plain-TAB should likewise just do an insert.
	//
	// note: we cannot use the default wxWidgets mechanism to do
	// note: the focus cycling -- because it doesn't like the CTRL.
	// note: CTRL-TAB only works for Notebook Panels.

	if (e.ControlDown())
	{
		m_pViewFile->cycleFocus( !e.ShiftDown() );
		return true;
	}

	if (!isEditPanel())
		return _kb_alert();

	return _kb_insert_text( _T("\t") );
}

//////////////////////////////////////////////////////////////////

DCL_KB(_vk_backspace)
{
	if (!isEditPanel())
		return _kb_alert();

	fim_offset docPosOriginal,docPosTarget;

	if (haveSelection())	// delete the current selection
	{
		if (!_mapCoordToDocPosition2(&m_vfcSelection0, &docPosTarget, true))
			return true;	// should not happen
		if (!_mapCoordToDocPosition2(&m_vfcSelection1, &docPosOriginal, true))
			return true;	// should not happen
	}
	else					// delete backward-{char,word}
	{
		if (!_mapCoordToDocPosition(&m_vfcCaret,&docPosOriginal))
			return true;

		ViewFileCoord vfcTarget;
		if (!_kb_getPrev(e.ControlDown(),&m_vfcCaret,&vfcTarget,&docPosTarget))
			return true;
	}
	
	_deleteText(docPosTarget,docPosOriginal);

	_kb_ensureCaretVisible(true);

	return true;
}

DCL_KB(_vk_delete)
{
	if (!isEditPanel())
		return _kb_alert();

	fim_offset docPosOriginal,docPosTarget;

	if (haveSelection())	// delete the current selection
	{
		// support the (obsolete/deprecated/ancient) circa dos/win3.1
		// Shift+Delete == cut
		// this doesn't really go with the other bindings we have
		// for DELETE (like what is C+S+Delete supposed to do??),
		// but people have asked for it.  item:12439
		if (e.ShiftDown() && !e.ControlDown() && !e.AltDown())
		{
			cutToClipboard();
			return true;
		}

		if (!_mapCoordToDocPosition2(&m_vfcSelection0, &docPosOriginal, true))
			return true;	// should not happen
		if (!_mapCoordToDocPosition2(&m_vfcSelection1, &docPosTarget, true))
			return true;	// should not happen
	}
	else					// delete forward-{char,word}
	{
		if (!_mapCoordToDocPosition(&m_vfcCaret,&docPosOriginal))
			return true;

		ViewFileCoord vfcTarget;
		if (!_kb_getNext(e.ControlDown(),&m_vfcCaret,&vfcTarget,&docPosTarget))
			return true;
	}
	
	_deleteText(docPosOriginal,docPosTarget);

	_kb_ensureCaretVisible(true);

	return true;
}

//////////////////////////////////////////////////////////////////

DCL_KBu(_vk_enter)
{
	if (!isEditPanel())
		return _kb_alert();

	// insert an EOL at the caret.

	fim_ptable * pPTable = m_pViewFile->getPTable(m_kSync,m_kPanel);

	wxString str;

	switch (pPTable->getEolMode())
	{
	default:				// quiets compiler
	case FIM_MODE_UNSET:
		wxASSERT_MSG( (0), _T("Coding Error") );
		pPTable->setEolMode(FIM_MODE_NATIVE_DISK);
		str = FIM_MODE_NATIVE_DISK_STR;
		break;
		
	case FIM_MODE_LF:
		str = _T("\n");
		break;

	case FIM_MODE_CRLF:
		str = _T("\r\n");
		break;

	case FIM_MODE_CR:
		str = _T("\r");
		break;
	}

	return _kb_insert_text(str);
}

//////////////////////////////////////////////////////////////////

DCL_KB(_vk_insert)
{
	// support the (obsolete/deprecated/ancient) circa dos/win3.1
	// Ctrl+Insert == copy
	// Shift+Insert == paste (only allowed on edit panel)
	// item:12439

	if (e.ControlDown() && !e.ShiftDown() && !e.AltDown())
	{
		copyToClipboard();
		return true;
	}

	if (e.ShiftDown() && !e.ControlDown() && !e.AltDown())
	{
		if (!isEditPanel())
			return _kb_alert();

		pasteFromClipboard();
		return true;
	}
	
	// silently eat the INSERT key -- if left unbound, we get 3
	// different CHAR events on the 3 different platforms -- none
	// of which we want.

	return true;
}

//////////////////////////////////////////////////////////////////

DCL_KB(_vk_context_menu)
{
	// the windows-context-menu key (between right-alt and right-ctrl).
	// raise the context menu just like the right mouse does.

	if (haveSelection())
	{
		// if we have a selection, the caret must be within it (or rather
		// at one end or the other (either way, it counts)), so we want
		// to see a context menu based upon having a selection.

		_do_context_menu__selection();
		return true;
	}

	if (!m_vfcCaret.isSet())
	{
		// if we don't have a caret (??), we can't tell what the
		// context is --OR-- where the menu should appear.  just bail.

		return true;
	}

	// get the caret's row and see what's there.  this can only be content
	// since the caret cannot refer to a MARK or VOID.

	long rowCaret = m_vfcCaret.getRow();

	// e.ControlDown():
	// normally, we extend around the clicked row to get the complete
	// context of the patch (usually only significant when in a complex
	// conflict).  when the control key is down, we allow them to restrict
	// to just this sync-node (this should let them pick-n-choose within
	// the conflict, for example).
	//
	// e.ShiftDown():
	// if there is a default action go ahead and try to apply it without
	// raising the context menu.

	_raise_content_context_menu(rowCaret,e.ControlDown(),e.ShiftDown());

	return true;
}

//////////////////////////////////////////////////////////////////

DCL_KB(_vk__default)
{
	switch (e.GetKeyCode())
	{
	case 'Z':
		if (e.ControlDown() && e.ShiftDown())
		{
			redo();
			return true;
		}
		break;

	case WXK_F5:
		{
			m_pViewFile->queueEvent(VIEWFILE_QUEUE_EVENT_FORCE_RELOAD);
			return true;
		}

	default:
		break;
	}
	
	return false;	// allow key event to be handled by the system.
}

//////////////////////////////////////////////////////////////////
#endif//Windows & Linux
