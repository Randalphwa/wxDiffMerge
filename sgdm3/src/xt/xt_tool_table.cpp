// xt_tool_table.cpp
// code to manipulate the table of configured external tools.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <poi.h>
#include <xt.h>

//////////////////////////////////////////////////////////////////

xt_tool_table::xt_tool_table(void)
{
}

xt_tool_table::~xt_tool_table(void)
{
	for (TVector_ToolsIterator it = m_vec.begin(); (it != m_vec.end()); it++)
	{
		xt_tool * pxt = (*it);
		delete pxt;
	}
	m_vec.clear();
}

//////////////////////////////////////////////////////////////////

xt_tool_table::xt_tool_table(const xt_tool_table & xtt)		// copy constructor
{
	for (TVector_ToolsConstIterator it = xtt.m_vec.begin(); (it != xtt.m_vec.end()); it++)
	{
		const xt_tool * pxt = (*it);
		xt_tool * pxtNew = new xt_tool(*pxt);

		m_vec.push_back(pxtNew);
	}
}

//////////////////////////////////////////////////////////////////

void xt_tool_table::OnInit(bool bBuiltinOnly)
{
	if (bBuiltinOnly)
	{
		_load_builtin_tools();
	}
	else
	{
		// load config info for external tools from the registry/config.
		// if we get an error or a bogus value, fall back to the builtin set.

		wxString strSaved = gpGlobalProps->getString(GlobalProps::GPS_EXTERNAL_TOOLS_SERIALIZED);
		if ((strSaved.Length() == 0) || (!_doImport(strSaved)))
			_load_builtin_tools();
	}

#ifdef DEBUG
//	dump(10);
#endif
}

//////////////////////////////////////////////////////////////////

void xt_tool_table::addTool(xt_tool * pxt)
{
	wxASSERT_MSG( (pxt), _T("Coding Error!") );

	m_vec.push_back(pxt);
}

void xt_tool_table::replaceTool(int index, xt_tool * pxtNew, bool bDelete)
{
	xt_tool * pxtOld = m_vec[index];

	m_vec[index] = pxtNew;

	if (bDelete)
		delete pxtOld;
}

void xt_tool_table::deleteTool(int index)
{
	// what i want to do is a: vec.erase[k] that removes the kth
	// cell from the vector (and making it shorter).  but erase()
	// only takes iterators, so we do it the hard way.

	int k = 0;
	
	for (TVector_ToolsIterator it = m_vec.begin(); (it != m_vec.end()); it++, k++)
		if (k == index)
		{
			xt_tool * pxt = (*it);
			delete pxt;
			m_vec.erase(it);
			return;
		}
}

void xt_tool_table::moveToolUpOne(int index)
{
	// move the item at v[index] to v[index-1] shifting the one at
	// v[index-1] to v[index]

	wxASSERT_MSG( (index < (int)m_vec.size()), _T("Coding Error!") );

	if (index < 1)		// bogus call,
		return;			// nothing to do.

	TVector_ToolsIterator itCurrent = m_vec.begin() + index;
	if (itCurrent == m_vec.end())			// should not happen
		return;

	xt_tool * pxt = (*itCurrent);
	m_vec.erase(itCurrent);

	TVector_ToolsIterator itNew = m_vec.begin() + (index - 1);

	m_vec.insert(itNew,pxt);
}

void xt_tool_table::moveToolDownOne(int index)
{
	// move the item at v[index] to v[index+1] shifting the one at
	// v[index+1] to v[index]

	wxASSERT_MSG( (index+1 < (int)m_vec.size()), _T("Coding Error!") );

	moveToolUpOne(index+1);
}

int xt_tool_table::getIndex(const xt_tool * pxt) const
{
	// lookup the given tool and return it's index in the vector.
	// return -1 when not found.

	int kLimit = getCountTools();
	for (int k=0; k<kLimit; k++)
		if (pxt == getNthTool(k))
			return k;

	return -1;
}

//////////////////////////////////////////////////////////////////

int xt_tool_table::allocateArrayOfNames(wxString ** array) const
{
	int kLimit = getCountTools();
	if (kLimit == 0)
	{
		*array = NULL;
		return 0;
	}

	*array = new wxString[kLimit];

	for (int k=0; k<kLimit; k++)
		(*array)[k] = getNthTool(k)->getName();

	return kLimit;
}

void xt_tool_table::freeArrayOfNames(wxString * array) const
{
	delete [] array;
}

//////////////////////////////////////////////////////////////////
// Exporting -- Serializing Tool into a string so that they can be
// saved in the config file/registry between sessions.
//////////////////////////////////////////////////////////////////
// Using a memory-output-stream and a data-output-stream, we create
// a binary BLOB from the xt_tool_table (details later in this file).
// For each variable that we wish to save in the blob, we create a
// RECORD.  Our overall BLOB will contain a variable number of these
// variable sized RECORDS.  Each RECORD has the following structure:
// [] key -- what the record is
// [] type -- what data type follows -- that is, the type of the value
// [] value -- a variant -- encoded by wxDataOutputStream

typedef enum K { TABLE_VERSION,			// do not reorder or insert in the middle of this enum
				 TABLE_EOF,
				 TOOL_BEGIN,
				 TOOL_NAME,
				 TOOL_SUFFIXES,
				 TOOL_ENABLED2,
				 TOOL_GUI2EXE,
				 TOOL_GUI2ARGS,
				 TOOL_ENABLED3,
				 TOOL_GUI3EXE,
				 TOOL_GUI3ARGS,
				 TOOL_END,
				 __LAST__FIELD__,
} T_RecordKey;

typedef enum T { T_BYTE		= 0x42,		// 'B'
				 T_LONG		= 0x4C,		// 'L'
				 T_STRING	= 0x53,		// 'S'
} T_RecordType;

struct _my_xt_record
{
	unsigned char		m_key;			// see T_RecordKey
	unsigned char		m_type;			// see T_RecordType

	unsigned char		m_u8Value;		// treat all values as a union
	unsigned long		m_ulValue;
	wxString			m_strValue;
};

static void _exportByte(  wxDataOutputStream & dos, unsigned char u8Key, unsigned char u8Value)		{dos.Write8(u8Key); dos.Write8(T_BYTE);   dos.Write8(u8Value);       }
static void _exportLong(  wxDataOutputStream & dos, unsigned char u8Key, unsigned long ulValue)		{dos.Write8(u8Key); dos.Write8(T_LONG);   dos.Write32(ulValue);      }
static void _exportString(wxDataOutputStream & dos, unsigned char u8Key, const wxString & strValue)	{dos.Write8(u8Key); dos.Write8(T_STRING); dos.WriteString(strValue); }
//static void _exportString(wxDataOutputStream & dos, unsigned char u8Key, const wxString * strValue)	{dos.Write8(u8Key); dos.Write8(T_STRING); dos.WriteString(*strValue);}

static bool _import_record(wxDataInputStream & dis, struct _my_xt_record * pRec)
{
	pRec->m_key = dis.Read8();
	pRec->m_type = dis.Read8();

	switch (pRec->m_type)
	{
	case T_BYTE:	pRec->m_u8Value  = dis.Read8();      return true;
	case T_LONG:	pRec->m_ulValue  = dis.Read32();     return true;
	case T_STRING:	pRec->m_strValue = dis.ReadString(); return true;
	default:		return false;
	}
}

//////////////////////////////////////////////////////////////////

#define XT_STREAM_FORMAT_VERSION			1

//////////////////////////////////////////////////////////////////
// Encoding -- Converting data BLOB into a string so that we can
// write it to the config files and/or the registry.
//////////////////////////////////////////////////////////////////

static wxString _encodeBlob(wxMemoryOutputStream & mos)
{
	// copy the data-output-stream into a raw buffer -- a BLOB.

	off_t lenDos = mos.TellO();
	unsigned char * buf = (unsigned char *)calloc(lenDos,sizeof(char));
	mos.CopyTo((char *)buf,lenDos);

#if 0
#ifdef DEBUG
	wxString strDebug;
	for (int k=0; k<lenDos; k++)
		if ((buf[k] >= 0x20) && (buf[k] < 0x7f))
			strDebug += buf[k];
		else
			strDebug += wxString::Format(_T("\\x%02hhx"),buf[k]);
	wxLogTrace(TRACE_XT_DUMP, _T("EncodingBlob:\n%s\n"), strDebug.wc_str());
#endif
#endif

	// take the BLOB and encode it in BASE16 in a string variable.
	// [yes, i can hear you groaning now.]  this solves a couple
	// of potential problems:
	// 
	// [] global props (and the wxConfig stuff) only take strings
	//    and since we are in a unicode build, wxConfig won't be
	//    able to safely digest the blob (it'll try to convert it
	//    to UTF8 before writing) **AND** we'll have lots of
	//    zeroes within our buffer, so various string functions
	//    won't work.
	//    
	// [] base64 might be more kosher, but i have to dig up the
	//    routines somewhere **AND** base64 uses <>= (and maybe
	//    other chars) that XML finds useful, so we might have
	//    another quoting/escaping problem if the wxConfig layer
	//    on a platform uses XML under the hood.  [they might
	//    take care of it, but i don't want to rely on it.]
	//
	// so, we base16 it and be done with it.

	static const wxChar * szHex = _T("0123456789abcdef");

	wxString strEncoded;
	for (int kEnc=0; kEnc<lenDos; kEnc++)
	{
		wxChar ch = szHex[((buf[kEnc] >> 4) & 0x0f)];
		wxChar cl = szHex[((buf[kEnc]     ) & 0x0f)];

		strEncoded += ch;
		strEncoded += cl;
	}
	free(buf);

//	wxLogTrace(TRACE_XT_DUMP, _T("EncodedBlob: \n%s\n"), strEncoded.wc_str());

	return strEncoded;
}

static bool _decodeBlob(const wxString & strEncoded, unsigned char ** ppbuf, size_t * pLenBuf)
{
//	wxLogTrace(TRACE_XT_DUMP, _T("DecodeBlob: \n%s\n"), strEncoded.wc_str());

	// decode the base16 string back into a blob.

	size_t lenMis = strEncoded.Length() / 2;
	if (lenMis == 0)
		return false;
	
	const wxChar * szEncoded = strEncoded.wc_str();
	
	unsigned char * buf = (unsigned char *)calloc(lenMis,sizeof(char));

	for (size_t k=0; k<lenMis; k++)
	{
		wxChar ch = *szEncoded++;
		wxChar cl = *szEncoded++;

		if (      (ch >= _T('0')) && (ch <= _T('9')) )	buf[k] = (unsigned char)(( ch - _T('0')      ) << 4);
		else if ( (ch >= _T('a')) && (ch <= _T('f')) )	buf[k] = (unsigned char)(( ch - _T('a') + 10 ) << 4);
		else if ( (ch >= _T('A')) && (ch <= _T('F')) )	buf[k] = (unsigned char)(( ch - _T('A') + 10 ) << 4);
		else goto Failed;

		if (      (cl >= _T('0')) && (cl <= _T('9')) )	buf[k] |= (unsigned char)( cl - _T('0')      );
		else if ( (cl >= _T('a')) && (cl <= _T('f')) )	buf[k] |= (unsigned char)( cl - _T('a') + 10 );
		else if ( (cl >= _T('A')) && (cl <= _T('F')) )	buf[k] |= (unsigned char)( cl - _T('A') + 10 );
		else goto Failed;
	}
	
#if 0
#ifdef DEBUG
	{
		wxString strDebug;
		for (size_t k=0; k<lenMis; k++)
			if ((buf[k] >= 0x20) && (buf[k] < 0x7f))
				strDebug += buf[k];
			else
				strDebug += wxString::Format(_T("\\x%02hhx"),buf[k]);
		wxLogTrace(TRACE_XT_DUMP, _T("DecodedBlob:\n%s\n"), strDebug.wc_str());
	}
#endif
#endif

	*ppbuf = buf;
	*pLenBuf = lenMis;
	return true;

Failed:
	free(buf);
	return false;
}
	
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////

void xt_tool_table::doExport(void) const
{
	// serialize external tool table into a string and save to registry.
	// see big note in rs_ruleset_table::doExport() for rationale for
	// using a stream (rather than XML and/or individual config fields)
	// for dumping everything into a single string.

	wxMemoryOutputStream mos;
	wxDataOutputStream dos(mos);

	_exportByte(dos,TABLE_VERSION, XT_STREAM_FORMAT_VERSION);

	int k=0;
	for (TVector_ToolsConstIterator it = m_vec.begin(); (it != m_vec.end()); it++)
	{
		const xt_tool * pXT = (*it);
		wxASSERT_MSG( (pXT), _T("Coding Error!") );

		_exportTool(dos,pXT,k++);
	}

	_exportByte(dos,TABLE_EOF, XT_STREAM_FORMAT_VERSION);

	wxString strEncoded = _encodeBlob(mos);

	gpGlobalProps->setString(GlobalProps::GPS_EXTERNAL_TOOLS_SERIALIZED,strEncoded);
}

void xt_tool_table::_exportTool(wxDataOutputStream & dos, const xt_tool * pXT, int index) const
{
	// emit a tool -- start with header and then dump all fields within it.

	_exportLong(dos, TOOL_BEGIN, index);

	_exportString(dos, TOOL_NAME, pXT->getName());
	_exportString(dos, TOOL_SUFFIXES, pXT->getSuffixes());

	_exportByte(dos, TOOL_ENABLED2, pXT->getEnabled2());
	_exportString(dos, TOOL_GUI2EXE, pXT->getGui2Exe());
	_exportString(dos, TOOL_GUI2ARGS, pXT->getGui2Args());

	_exportByte(dos, TOOL_ENABLED3, pXT->getEnabled3());
	_exportString(dos, TOOL_GUI3EXE, pXT->getGui3Exe());
	_exportString(dos, TOOL_GUI3ARGS, pXT->getGui3Args());

	_exportLong(dos, TOOL_END, index);
}

//////////////////////////////////////////////////////////////////

bool xt_tool_table::_doImport(const wxString & strEncoded)
{
	// import external tool table from serialized string.
	// convert the given string (containing base16-encoded data stream)
	// into our external tool table.

	unsigned char * buf = NULL;
	size_t lenBuf = 0;

	if (!_decodeBlob(strEncoded,&buf,&lenBuf))
		return false;
	
	// put blob into memory-input-stream and let data-input-stream
	// start reading fields -- this takes care of UTF8->UNICODE and
	// byte-swap issues and etc.

	wxMemoryInputStream mis(buf,lenBuf);	// we must free buf
	wxDataInputStream dis(mis);

	// read the records and process -- we dispatch on key,type as
	// we try to adapt to what we find --- as opposed to assuming
	// that we find exactly what we wrote last time --- i'd hate
	// to go berzerk here because someone edited the value in registry...

	xt_tool * pXT = NULL;

	struct _my_xt_record rec;
	while (_import_record(dis,&rec))
	{
		switch (rec.m_key)
		{
		case TABLE_VERSION:
			if ((rec.m_type != T_BYTE) || (rec.m_u8Value > XT_STREAM_FORMAT_VERSION))	goto Failed;	// if written by newer version, puke
			break;

		case TABLE_EOF:
			goto SawEOF;

		case TOOL_BEGIN:
			pXT = new xt_tool();
			break;

		case TOOL_END:
			if (rec.m_type != T_LONG)	goto Failed;
			// ignore rec.m_ulValue
			m_vec.push_back(pXT);
			pXT = NULL;
			break;

		case TOOL_NAME:
			if ((rec.m_type != T_STRING) || (!pXT))	goto Failed;
			pXT->setName(rec.m_strValue);
			break;

		case TOOL_SUFFIXES:
			if ((rec.m_type != T_STRING) || (!pXT))	goto Failed;
			pXT->setSuffixes(rec.m_strValue);
			break;

		case TOOL_ENABLED2:
			if ((rec.m_type != T_BYTE) || (!pXT))	goto Failed;
			pXT->setEnabled2( (rec.m_u8Value != 0) );
			break;

		case TOOL_GUI2EXE:
			if ((rec.m_type != T_STRING) || (!pXT))	goto Failed;
			pXT->setGui2Exe(rec.m_strValue);
			break;

		case TOOL_GUI2ARGS:
			if ((rec.m_type != T_STRING) || (!pXT))	goto Failed;
			pXT->setGui2Args(rec.m_strValue);
			break;

		case TOOL_ENABLED3:
			if ((rec.m_type != T_BYTE) || (!pXT))	goto Failed;
			pXT->setEnabled3( (rec.m_u8Value != 0) );
			break;

		case TOOL_GUI3EXE:
			if ((rec.m_type != T_STRING) || (!pXT))	goto Failed;
			pXT->setGui3Exe(rec.m_strValue);
			break;

		case TOOL_GUI3ARGS:
			if ((rec.m_type != T_STRING) || (!pXT))	goto Failed;
			pXT->setGui3Args(rec.m_strValue);
			break;

		default:
			goto Failed;
		}
	}
	// if _import_record() failed, we had bogus BLOB or we ran off
	// the end (and thus didn't the EOF marker), so let's puke.
	goto Failed;

SawEOF:
	delete pXT;
	free(buf);

	return true;

Failed:
	delete pXT;
	free(buf);

	// we could not load the tools from the string given.
	// destroy any everything that we created.

	for (TVector_ToolsIterator it = m_vec.begin(); (it != m_vec.end()); it++)
	{
		xt_tool * pXTItem = (*it);
		delete pXTItem;
	}
	m_vec.clear();
	
	return false;
}

//////////////////////////////////////////////////////////////////

wxString xt_tool_table::dumpSupportInfo(void) const
{
	// build a string containing a human readable dump of the
	// complete xt_tool table.  this is used to populate the
	// support dialog.

	wxString str;

	str += wxString::Format(_T("External Tools Table: [Version %d]\n"), XT_STREAM_FORMAT_VERSION);

	for (TVector_ToolsConstIterator it = m_vec.begin(); (it != m_vec.end()); it++)
	{
		const xt_tool * pxt = (*it);
		str += pxt->dumpSupportInfo();
	}

	str += _T("\n");

	return str;
}

//////////////////////////////////////////////////////////////////

static bool _my_pathname_type_is_file(const poi_item * pPoi)
{
	// return false if error or directory
	// return true if file

	if (pPoi->isFile()
#if defined(__WXMAC__) || defined(__WXGTK__)
		|| (pPoi->getFullPath().Cmp(_T("/dev/null")) == 0)	// help GIT use us.  see item:13303.
#endif
		)
		return true;

	return false;
}

const xt_tool * xt_tool_table::_do_automatic_best_guess(int nrPoi, poi_item * pPoiTable[]) const
{
	// try to automatically guess the best tool for the given set of files.
	// 
	// this can either be easy (when opening a file-diff window on "a.c"
	// vs. "b.c") or more involved (when tmp or version number suffixes
	// are used).
	//
	// WARNING: both Vault and SOS use a lot of TEMP names.

	// we require 2 or 3 or 4 pathnames.

	if (nrPoi < 2)
		return NULL;

	// require all input pathnames to be to files.  (we don't require result-path
	// to already exist.)
	// we don't support directories.
	int nrInput = MyMin(3,nrPoi);
	for (int k=0; k<nrInput; k++)
		if (!_my_pathname_type_is_file(pPoiTable[k]))
			return NULL;

	// we have 2 automatic matching strategies:
	// [] match all -- all N file suffixes must match
	// [] match any -- any file can trigger a match (useful when "a.c" vs "a.c~")
	// either way, we take the first match we come to.

	bool bMatchAllFiles = gpGlobalProps->getBool(GlobalProps::GPL_EXTERNAL_TOOLS_REQUIRE_COMPLETE_MATCH);

	for (TVector_ToolsConstIterator it = m_vec.begin(); (it != m_vec.end()); it++)
	{
		const xt_tool * pxt = (*it);
		if (!pxt)
			continue;
		if (nrPoi == 2)				// a 2-way merge
		{
			if ((!pxt->getEnabled2())					// if 2-ways are turned off in this tool
				|| (pxt->getGui2Exe().Length() == 0))	// or if no exe is set, ignore this one.
				continue;
		}
		else						// nrPoi > 2 -- a merge (with or without result path)
		{
			if ((!pxt->getEnabled3())					// if 3-ways are turned off in this tool
				|| (pxt->getGui3Exe().Length() == 0))	// of if no exe is set, ignore this one.
				continue;
		}
			
		int cVotes = 0;
		for (int k=0; k<nrPoi; k++)
		{
			if (pxt->testPathnameSuffix(pPoiTable[k]))
				cVotes++;
		}
		bool bMatch = ((bMatchAllFiles) ? (cVotes == nrPoi) : (cVotes > 0));
		if (bMatch)
		{
			wxLogTrace(TRACE_XT_DUMP, _T("XT:_do_automatic_best_guess: matched [%s]"), pxt->getName().wc_str());
			return pxt;
		}
	}

	// could not find a match

	return NULL;
}

#if 0 // not needed
const xt_tool * xt_tool_table::findExternalTool(int nrPoi, poi_item * pPoiTable[]) const
{
	// see if the given set of files should be handled by
	// an external tool.  if so, return a handle to it.
	// if no match, return null.

	// if the entire external tool mechanism is turned off, give up.

	if (!gpGlobalProps->getBool(GlobalProps::GPL_EXTERNAL_TOOLS_ENABLE_TOOLS))
		return NULL;

	return _do_automatic_best_guess(nrPoi,pPoiTable);
}
#endif
											 
const xt_tool * xt_tool_table::findExternalTool(int nrParams,
												const wxString & s0,
												const wxString & s1,
												const wxString & s2) const
{
	// see if the set of files given should be handled by an external tool.

	if (!gpGlobalProps->getBool(GlobalProps::GPL_EXTERNAL_TOOLS_ENABLE_TOOLS))
		return NULL;

	poi_item * pPoiTable[3];
	pPoiTable[0] =                   gpPoiItemTable->addItem(s0);
	pPoiTable[1] =                   gpPoiItemTable->addItem(s1);
	pPoiTable[2] = ((nrParams > 2) ? gpPoiItemTable->addItem(s2) : NULL);

	return _do_automatic_best_guess(nrParams,pPoiTable);
}

const xt_tool * xt_tool_table::findExternalTool(const wxString & s0,
												const wxString & s1,
												const wxString & s2,
												const wxString & sResultPathname) const
{
	// see if the set of files given should be handled by an external tool.
	// we check all 3 input pathnames and the result pathname.

	if (!gpGlobalProps->getBool(GlobalProps::GPL_EXTERNAL_TOOLS_ENABLE_TOOLS))
		return NULL;

	poi_item * pPoiTable[4];
	pPoiTable[0] = gpPoiItemTable->addItem(s0);
	pPoiTable[1] = gpPoiItemTable->addItem(s1);
	pPoiTable[2] = gpPoiItemTable->addItem(s2);
	pPoiTable[3] = gpPoiItemTable->addItem(sResultPathname);

	return _do_automatic_best_guess(4,pPoiTable);
}

//////////////////////////////////////////////////////////////////

void xt_tool_table::_load_builtin_tools(void)
{
#ifdef DEBUG		
#if 0
	xt_tool * pxtTest = new xt_tool();
	pxtTest->setName(L"Test External Tool");
	pxtTest->setSuffixes(L"test tst");
	pxtTest->setEnabled2(true);
	pxtTest->setEnabled3(true);
	pxtTest->setGui2Exe(L"hello.exe");
	pxtTest->setGui2Args(L"/ro1 /ro2 /title1:\"%LEFT_LABEL%\" /title2:\"%RIGHT_LABEL%\" \"%LEFT_PATH%\" \"%RIGHT_PATH%\"");
	pxtTest->setGui3Exe(L"world.exe");
	pxtTest->setGui3Args(L"/ro1 /ro3 /merge /title1:\"%WORKING_LABEL%\" /title2:\"%DEST_LABEL%\" /title3:\"%OTHER_LABEL%\" /result:\"%DEST_PATH%\" \"%WORKING_PATH%\" \"%BASELINE_PATH%\" \"%OTHER_PATH%\"");
	addTool(pxtTest);
#endif

#if 0
	xt_tool * pxtTest2 = new xt_tool();
	pxtTest2->setName(L"Test External Tool");
	pxtTest2->setSuffixes(L"test tst");
	pxtTest2->setEnabled2(true);
	pxtTest2->setEnabled3(true);
	pxtTest2->setGui2Exe(L"C:\\Program Files\\Windows NT\\Accessories\\wordpad.exe");
	pxtTest2->setGui2Args(L"\"%LEFT_PATH%\" \"%RIGHT_PATH%\"");
	pxtTest2->setGui3Exe(L"c:/emacs-22.1/bin/runemacs.exe");
	pxtTest2->setGui3Args(L"\"%WORKING_PATH%\" \"%BASELINE_PATH%\" \"%OTHER_PATH%\"");
	addTool(pxtTest2);
#endif
#endif
}

//////////////////////////////////////////////////////////////////

#ifdef DEBUG
void xt_tool_table::dump(int indent) const
{
	wxLogTrace(TRACE_XT_DUMP, _T("%*cXT_TOOL_TABLE: [count %d]"),
			   indent, ' ', getCountTools());
	for (TVector_ToolsConstIterator it = m_vec.begin(); (it != m_vec.end()); it++)
	{
		const xt_tool * pxt = (*it);
		pxt->dump(indent+5);
	}
}
#endif
