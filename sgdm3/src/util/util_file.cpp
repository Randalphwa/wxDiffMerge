// util_file.cpp
// various wxFile-related helper functions
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>

//////////////////////////////////////////////////////////////////

#if !defined(O_BINARY)
#	define O_BINARY		(0)
#endif

//////////////////////////////////////////////////////////////////
// this duplicates the functionality of wxFile::Open() but does not
// send stuff to wxLogSysError() when we have problems.
// 
// WXBUG: there is a bug in wxString::PrintfV() that causes us to hang
// WXBUG: when szFileName contains non ascii characters on the MAC and
// WXBUG: is passed to wxLogSysError().

int util_file_open(const wxChar *szFileName, wxFile::OpenMode mode, int accessMode)
{
    int flags = O_BINARY;

    switch ( mode )
    {
        case wxFile::read:
            flags |= O_RDONLY;
            break;

        case wxFile::write_append:
            if ( wxFileExists(szFileName) )
            {
                flags |= O_WRONLY | O_APPEND;
                break;
            }
            //else: fall through as write_append is the same as write if the
            //      file doesn't exist

        case wxFile::write:
            flags |= O_WRONLY | O_CREAT | O_TRUNC;
            break;

        case wxFile::write_excl:
            flags |= O_WRONLY | O_CREAT | O_EXCL;
            break;

        case wxFile::read_write:
            flags |= O_RDWR;
            break;
    }

#ifdef __WINDOWS__
    // only read/write bits for "all" are supported by this function under
    // Windows, and VC++ 8 returns EINVAL if any other bits are used in
    // accessMode, so clear them as they have at best no effect anyhow
    accessMode &= wxS_IRUSR | wxS_IWUSR;
#endif // __WINDOWS__

	// wxOpen() takes care of wc2mb stuff if necessary.

    int fd = wxOpen( szFileName, flags, accessMode);

	return fd;
}

int util_file_open(util_error & err, const wxChar *szFileName, wxFile::OpenMode mode, int accessMode)
{
	int fd = util_file_open(szFileName,mode,accessMode);

	if (fd == -1)
	{
		wxString strMsg(_T("Cannot open file '"));		// don't use wxString::PrintfV() because of hang/crash on mac
		strMsg += szFileName;							// when pathname contains non ascii chars.
		strMsg += _T("'.");

		err.set(util_error::UE_CANNOT_OPEN_FILE,strMsg);
		return -1;
	}
	else
	{
		err.clear();
		return fd;
	}
}

//////////////////////////////////////////////////////////////////

#if defined(__WXMSW__)
/**
 * Is this file a Windows SHELL LINK (.lnk) SHORTCUT?
 *
 * The binary format is documented here:
 * http://msdn.microsoft.com/en-us/library/dd871305.aspx
 * 
 * The first part of the header contains:
 *
 *     HeaderSize (4 bytes): The size, in bytes, of this structure.
 *     This value MUST be 0x0000004C.
 *
 *     LinkCLSID (16 bytes): A class identifier (CLSID). This value
 *     MUST be 00021401-0000-0000-C000-000000000046.
 *
 * See also class util_shell_lnk.
 * 
 */
util_error util_file__is_shell_lnk(const wxString & pathname, bool * pbIsLnk)
{
	*pbIsLnk = false;

	util_error err;

	size_t len = pathname.Length();
	if (len < 4)
		return err;

	const wxChar * psz = pathname.wc_str();
	const wxChar * p4 = &psz[len-4];
	if (_wcsicmp(p4, L".lnk") != 0)
		return err;

	{
		util_logToString uLog(&err.refExtraInfo());
		int fd = util_file_open(err, pathname, wxFile::read);
		if (fd == -1)
			return err;

		wxFile file;
		file.Attach(fd);

		byte buf[20];			// get first 20 bytes, little-endian
		if (file.Read(buf,20) < 20)
		{
			err.set(util_error::UE_CANNOT_READ_FILE);
			return err;
		}
		
		static byte def[] = { 0x4c, 0x00, 0x00, 0x00,
							  0x01, 0x14, 0x02, 0x00,
							  0x00, 0x00, 0x00, 0x00,
							  0xc0, 0x00, 0x00, 0x00,
							  0x00, 0x00, 0x00, 0x46 
		};

		if (memcmp(buf, def, 20) != 0)
			return err;
	}

	wxLogTrace(TRACE_LNK, _T("Found shell link: %s"), pathname.wc_str());

	*pbIsLnk = true;
	return err;
}
#endif//__WXMSW__
