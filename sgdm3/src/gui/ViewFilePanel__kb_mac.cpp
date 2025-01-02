// ViewFilePanel__kb_mac.cpp
// keyboard/keybinding-related stuff for MAC.
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
// NOTE 2013/06/10 as of wxWidgets 2.9.4, e.ControlDown() reports
// NOTE            the state of the Command/Apple/Clover key.
// NOTE            and we've switched to using it whenever possible.
// NOTE            and where I say control+ and alt+control+ in
// NOTE            comments below, it now meands Command/Apple/Clover
// NOTE            and not the raw Control key.  (sigh)
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// see apple's docs for some of the keyboard standards
// http://developer.apple.com/documentation/UserExperience/Conceptual/OSXHIGuidelines/XHIGUserInput/chapter_11_section_3.html
// http://developer.apple.com/documentation/UserExperience/Conceptual/OSXHIGuidelines/index.html#//apple_ref/doc/uid/20000957
//////////////////////////////////////////////////////////////////

#if defined(__WXMAC__)

//////////////////////////////////////////////////////////////////

void ViewFilePanel::onCharEvent(wxKeyEvent & e)
{
#if 0 && defined(DEBUG)
	wxChar c = e.GetUnicodeKey();
	wxLogTrace(wxTRACE_Messages, _T("VFP:CharEvent: [char %04x][keycode %02x][cmd %d][meta %d][alt %d][control %d %d][shift %d][modifiers %x][raw_code 0x%x][raw_flags 0x%x]"),
			   (int)c,
			   e.GetKeyCode(),
			   e.CmdDown(),
			   e.MetaDown(),
			   e.AltDown(),
			   e.ControlDown(), e.RawControlDown(),
			   e.ShiftDown(),
			   e.GetModifiers(),
			   e.GetRawKeyCode(), e.GetRawKeyFlags());
#endif

	if (_dispatchCharEvent(e))
		return;

#if 0 && defined(DEBUG)
	wxLogTrace(wxTRACE_Messages, _T("ViewFilePanel:CharEvent: skipping."));
#endif
	e.Skip();
}

bool ViewFilePanel::_dispatchCharEvent(wxKeyEvent & e)
{
	wxChar chUnicode = e.GetUnicodeKey();

	if (chUnicode == 0x0000)
		return true;

	if (e.RawControlDown())
	{
		// TODO 2013/07/17 Currently with 2.9.4 on MAC, when (the
		// TODO            real) "Ctrl" key is pressed and then any
		// TODO            other key is pressed, it sends us an
		// TODO            onCharEvent() for all A..Z regardless of
		// TODO            which keyboard layout is installed.
		// TODO
		// TODO            *AND* for Ctrl-A, we get:
		// TODO                GetUnicodeKey()='A' and
		// TODO                GetKeyCode()=0x01
		// TODO            which seems like a bug -- the unicode key
		// TODO            looks wrong.
		// TODO
		// TODO            We do not get (that I detected) onCharEvents
		// TODO            for non A..Z.  Ctrl+<symbols> should have
		// TODO            been handled/rejected in onKeyDown().

		int ch = e.GetKeyCode();
		if ((ch >= WXK_CONTROL_A) && (ch <= WXK_CONTROL_Z))
		{
			// It's tempting to insert our own half-baked
			// set of emacs-like key bindings here, but I
			// just noticed that we get Ctrl+A when the key
			// next to the CapsLock is pressed *regardless*
			// of which keyboard layout is installed.
			// TextEdit only expands Ctrl+A on a US keyboard;
			// that key is not bound when using a Russian keyboard.
			// 
			// So I'm going to pass for now.
			return _kb_alert();
		}
	}

	if (e.ControlDown())
	{
		// On 2.9.4 MAC, Command+<key> (which are normally accelerators)
		// are eaten by OS X and/or wxWidgets when they are bound to an
		// *ENABLED* (non-grayed) menu item.  We still get CharEvents for
		// unbound keys *AND* for grayed items.
		//
		// Eat them all.
		//
		// TODO 2013/07/23 we could catch these keys in _vk_default()
		// TODO            and prevent the KeyDownEvent from generating
		// TODO            this CharEvent.  Not sure which is best at
		// TODO            this point....
		return _kb_alert();
	}

	if (!isEditPanel())
		return _kb_alert();

	// insert the given character at the caret.  i'm not sure
	// that we need the cua-vs-mac indirection for CHAR events
	// because all chars should do a self-insert.

	return _kb_insert_text( chUnicode );
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::onKeyDownEvent(wxKeyEvent & e)
{
#if 0 && defined(DEBUG)
	wxChar c = e.GetUnicodeKey();
	wxLogTrace(wxTRACE_Messages,
			   _T("VFP:KeyDown: [char %04x][keycode %02x][cmd %d][meta %d][alt %d][control %d %d][shift %d][modifiers %x][raw_code 0x%x][raw_flags 0x%x]"),
			   (int)c,
			   e.GetKeyCode(),
			   e.CmdDown(),
			   e.MetaDown(),
			   e.AltDown(),
			   e.ControlDown(), e.RawControlDown(),
			   e.ShiftDown(),
			   e.GetModifiers(),
			   e.GetRawKeyCode(), e.GetRawKeyFlags());
#endif

	if (isBound() && _dispatchKeyDownEvent(e))
		return;

#if 0 && defined(DEBUG)
	wxLogTrace(wxTRACE_Messages,
			   _T("VFP:KeyDown: skip"));
#endif
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
	// PAGE-DOWN
	//
	// open -- scroll without changing caret
	// ctrl -- move caret one page (respecting goal column)
	// alt/option -- move caret one page (respecting goal column)
	//
	// shift+{ctrl,alt} -- extend selection correspondingly
	// shift+open -- extend selection one page

	if (e.GetModifiers() == wxMOD_NONE)	// open
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
	// PAGE-UP
	//
	// open -- scroll without changing caret
	// ctrl -- move caret one page (respecting goal column)
	// alt/option -- move caret one page (respecting goal column)
	//
	// shift+{ctrl,alt} -- extend selection correspondingly
	// shift+open -- extend selection one page

	if (e.GetModifiers() == wxMOD_NONE)	// open
	{
		m_pViewFile->keyboardScroll_pageup(m_kSync,wxVERTICAL);
		return true;
	}

	int col = _getGoal();
	int rowCurrent = m_vfcCaret.getRow();
	int page = m_pViewFile->getScrollThumbSize(m_kSync,wxVERTICAL);
	int row  = rowCurrent - page + 1;

	if (row < 0)
		row = 0;

	if (!_kb_getNextRow(row, &row))
	{
		int rowValid = rowCurrent;
		row = rowCurrent;

		while (_kb_getPrevRow(row, &row))
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

DCL_KB(_vk_home)
{
	// HOME
	//
	// open -- scroll to BOF without affecting caret (matches TextEdit)
	// shift -- extend selection to BOF
	//
	// ctrl -- does nothing
	// alt/option -- does nothing
	// apple/cmd -- does nothing

	if (e.HasModifiers())
		return true;

	if (e.ShiftDown() && m_vfcCaret.isSet())
	{
		if (haveSelection())
			m_vfcAnchor.set(m_vfcSelection1);

		_doUpdateCaret(true,0,0);
		_setGoal(0);
	}

	// warp scroll to BOF

	m_pViewFile->keyboardScroll_top(m_kSync,wxVERTICAL);
	m_pViewFile->keyboardScroll_top(m_kSync,wxHORIZONTAL);
	return true;
}

DCL_KB(_vk_end)
{
	// END
	// 
	// open -- scroll to EOF without affecting caret (matches TextEdit)
	// shift -- extend selection to EOF
	// 
	// ctrl -- does nothing
	// alt/option -- does nothing
	// apple/cmd -- does nothing

	if (e.HasModifiers())
		return true;

	if (e.ShiftDown() && m_vfcCaret.isSet())
	{
		if (haveSelection())
			m_vfcAnchor.set(m_vfcSelection0);

		_doUpdateCaret(true,_kb_getEOFRow(),0);
		_setGoal(0);
	}
	
	// warp scroll to EOF

	m_pViewFile->keyboardScroll_bottom(m_kSync,wxVERTICAL);	// warp to last row (actually just past last actual row)
	m_pViewFile->keyboardScroll_top(m_kSync,wxHORIZONTAL);	// warp to column 0
	return true;
}

DCL_KB(_vk_left)
{
	// LEFT-ARROW
	//
	// alt(option)+control -- apply default patch from right (copy from right panel
	//                          "left-ward") (like winmerge)  auto-advance if enabled
	//                          in options dialog.
	// all other modifiers ignored when alt+control down.

	if (e.AltDown() && e.ControlDown())
	{
		// the focus panel (us) could be any of the 3 panels.
		// we want this to behave as if it came from the parent or frame
		// and apply stuff in PANEL_T2 to the edit panel.  therefore,
		// this key is only defined on a merge window.

		if (m_pViewFile->getNrTopPanels() != 3)
			return true;	// TODO should we beep or something?

		bool bAdvance = gpGlobalProps->getBool(GlobalProps::GPL_MISC_AUTO_ADVANCE_AFTER_APPLY);

		m_pViewFile->applyDefaultAction(PANEL_T2,bAdvance);

		return true;
	}

	// open -- move caret backward 1 char
	// apple/cmd -- move caret to beginning of line
	// alt/option -- move caret to beginning of word or to beginning of previous word
	//
	// shift+{apple,alt} -- extend selection to destination
	// shift -- extend selection backward 1 char

	if (!m_vfcCaret.isSet())
		return true;

	ViewFileCoord vfc;

	if (e.ControlDown())
		vfc.set(m_vfcCaret.getRow(),0);
	else if (!_kb_getPrev(e.AltDown(),&m_vfcCaret,&vfc,NULL))
		return true;

	_doUpdateCaret(e.ShiftDown(),vfc);
	_setGoal(vfc.getCol());
	_kb_ensureCaretVisible();

	return true;
}

DCL_KB(_vk_right)
{
	// RIGHT-ARROW
	// 
	// alt(option)+control -- apply default patch from left (copy from left panel
	//                          "right-ward") (like winmerge)  auto-advance if
	//                          enabled in options dialog.
	// all other modifiers ignored when alt+control down.

	if (e.AltDown() && e.ControlDown())
	{
		bool bAdvance = gpGlobalProps->getBool(GlobalProps::GPL_MISC_AUTO_ADVANCE_AFTER_APPLY);

		m_pViewFile->applyDefaultAction(PANEL_T0,bAdvance);

		return true;
	}

	// open -- move caret forward 1 char
	// apple/cmd -- move caret to end of line
	// alt/option -- move caret to end of word or to end of previous word
	//
	// shift+{ctrl,apple,alt} -- extend selection to destination
	// shift -- extend selection forward 1 char

	if (!m_vfcCaret.isSet())
		return true;

	ViewFileCoord vfc;

	if (e.ControlDown())
	{
		int colEOL;
		_kb_getEOLCol(m_vfcCaret.getRow(),&colEOL);
		vfc.set(m_vfcCaret.getRow(),colEOL);
	}
	else if (!_kb_getNext(e.AltDown(),&m_vfcCaret,&vfc,NULL))
		return true;

	_doUpdateCaret(e.ShiftDown(),vfc);
	_setGoal(vfc.getCol());
	_kb_ensureCaretVisible();

	return true;
}

DCL_KB(_vk_up)
{
	// UP-ARROW
	//
	// alt(option)+control -- move to previous change 
	// other modifiers ignored when alt+control down

	if (e.AltDown() && e.ControlDown())
	{
		m_pViewFile->keyboardScroll_delta(false,false);
		return true;
	}

	// open -- move caret up one line (respecting goal column)
	// apple/cmd -- move caret to BOF
	// alt/option -- move caret up one line (should be move to beginning of paragraph but we don't have that)
	//
	// shift+{open,apple,alt} -- extend selection similarly

	if (!m_vfcCaret.isSet())
		return true;

	ViewFileCoord vfc;

	if (e.ControlDown())
	{
		_doUpdateCaret(e.ShiftDown(),0,0);
		_kb_ensureCaretVisible();
		return true;
	}
	else	// open or alt or shift+open or shift+alt
	{
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
}

DCL_KB(_vk_down)
{
	// DOWN-ARROW
	//
	// alt(option)+control -- move to next change 
	// other modifiers ignored when alt+control down

	if (e.AltDown() && e.ControlDown())
	{
		m_pViewFile->keyboardScroll_delta(true,false);
		return true;
	}

	// open -- move caret down one line (respecting goal column)
	// apple/cmd -- move caret to EOF
	// alt/option -- move caret down one line (should be by paragraph)
	//
	// shift+{open,apple,alt} -- extend selection similarly

	if (!m_vfcCaret.isSet())
		return true;

	ViewFileCoord vfc;

	if (e.ControlDown())
	{
		_doUpdateCaret(e.ShiftDown(),_kb_getEOFRow(),0);
		_kb_ensureCaretVisible();
		return true;
	}
	else	// open or alt or shift+open or shift+alt
	{
		// otherwise, move caret/extend selection to previous line
		// using goal column.  since goal column may not exist (or
		// may be in the middle of a tab) in the next line, we
		// do not update the goal column -- just the caret.  if the
		// next line is a void, we keep going until we find a
		// valid line.

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
}

DCL_KB(_vk_escape)
{
	m_pViewFile->getFrame()->postEscapeCloseCommand();

	return true;
}

DCL_KB(_vk_tab)
{
	// Update 2013/06/09 with 2.9.4, keyboard modifiers were changed.
	// The Command/Apple/Clover key is now accessed via e.ControlDown().
	// The *real* Control key is assessed via the e.RawControlDown().
	// The e.CmdDown() pseudo method is a macro for e.ControlDown().
	//
	// The OS handles Command-Tab to switch apps.
	// The panel notebook (I think) catches Real-Control-Tab to
	// switch focus between panels (like a dialog).
	// TODO assert !RawControlDown() and !ControlDown().
	//
	// So I'm going to catch Alt-Tab to do my own panel cycling
	// (as opposed to beeping or inserting a TAB into the buffer).

	if (e.AltDown())
	{
		m_pViewFile->cycleFocus( !e.ShiftDown() );
		return true;
	}

	if (!isEditPanel())
		return _kb_alert();

	return _kb_insert_text( _T("\t") );
}

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

DCL_KB(_vk_enter)
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

DCL_KB(_vk_insert)
{
	// silently eat the INSERT key -- if left unbound, we get 3
	// different CHAR events on the 3 different platforms -- none
	// of which we want.

	//wxLogTrace(wxTRACE_Messages,_T("kb_mac:Insert key"));
	return true;
}

DCL_KB(_vk_context_menu)
{
	// we never get these -- not even when we have a MSFT
	// keyboard plugged into the MAC

	//wxLogTrace(wxTRACE_Messages,_T("kb_mac:ContextMenu key"));
	return true;
}

DCL_KB(_vk__default)
{
	//wxLogTrace(wxTRACE_Messages,_T("kb_mac:vk_default: %x"),e.GetKeyCode());
	return false;	// allow key event to be handled by the system.
}

//////////////////////////////////////////////////////////////////

#endif
