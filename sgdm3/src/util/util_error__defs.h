// util_error__defs.h
// define error codes.  this file is included multiple times.
//////////////////////////////////////////////////////////////////

UE(OK,						_T("No error."))
UE(CANCELED,				_T("Canceled."))
UE(UNSPECIFIED_ERROR,		_T("Unspecified error."))

UE(CANNOT_ALLOC,			_T("Cannot allocate enough memory for operation."))

UE(INVALID_PATHNAME,		_T("Invalid pathname."))
UE(CANNOT_OPEN_FILE,		_T("Cannot open file."))
UE(CANNOT_STAT_FILE,		_T("Cannot stat file."))
UE(CANNOT_READ_FILE,		_T("Cannot read file."))
UE(CANNOT_WRITE_FILE,		_T("Cannot write file."))
UE(CANNOT_CHMOD_FILE,		_T("Cannot change permissions on file."))
UE(CANNOT_MKDIR,			_T("Cannot create directory."))

UE(CANNOT_OPEN_FOLDER,		_T("Cannot open folder."))
UE(NOT_A_FOLDER,			_T("Pathname does not refer to a folder."))
UE(CANNOT_COMPARE_ITEMS,	_T("Cannot compare these items."))

UE(CANNOT_IMPORT_CONV,		_T("Cannot import file using given character encoding."))
UE(CANNOT_EXPORT_CONV,		_T("Cannot export file using given character encoding."))

UE(LINE_NR_RANGE,			_T("Line number out of range."))
UE(LINE_ALREADY_HAS_MARK,	_T("Line already has alignment mark."))
UE(MARKS_CANNOT_OVERLAP,	_T("Alignment marks cannot overlap with other alignment marks."))

UE(NO_UNICODE_BOM,			_T("No UNICODE Byte-Order Mark"))

UE(CONV_ODD_BUFFER_LEN,		_T("Length of file data not a multiple of the character size for this encoding."))
UE(CONV_BUFFER_HAS_NUL,		_T("File data contains NUL character.  Is this a binary file or in a different character encoding?"))

UE(UNSUPPORTED,				_T("Unsupported case."))
UE(CANNOT_SPAWN_TOOL,		_T("Cannot spawn external tool."))
UE(LIMIT_EXCEEDED,			_T("Limit exceeded."))

UE(CANNOT_AUTOMATICALLY_DETERMINE_CHARENC,	_T("Cannot automatically determine character encoding."))

UE(OLE, _T("OLE Error"))

UE(CANNOT_READ_SYMLINK,		_T("Cannot read symlink."))
UE(CANNOT_CREATE_SYMLINK,	_T("Cannot create symlink."))

UE(CANNOT_CREATE_RUN_THREAD,_T("Cannot create or run background thread."))
