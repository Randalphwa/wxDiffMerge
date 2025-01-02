// util.cpp
// various miscellaneous utilities
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>

#include <GEN_BuildNumber.h>

#include <key.h>
#include "sghash__sha1.h"
#include <wx/time.h>

//////////////////////////////////////////////////////////////////

#define T_DLG_QUERY__S__VAULT		0x01

//////////////////////////////////////////////////////////////////


wxString util_my_branch(void)
{
	// Getting this from the VER_BUILD_LABEL (which is
	// a slight repurposing of that field).

	wxString strBranch(VER_BUILD_LABEL);
	if (strBranch.StartsWith(wxT("@"))			// if optional BUILDLABEL was NOT set by script running the compiler
		|| strBranch.StartsWith(wxT("_BLANK_"))	// or if it contains a placeholder,
		|| strBranch.Length() == 0)
		strBranch = wxT("stable");

	return strBranch;
}

wxString util_my_package(void)
{
	return wxString::Format(wxT("%s"), VER_FLAVOR_ARCH);
}

wxString util_my_version(void)
{
	return wxString::Format(wxT("%02d.%02d.%02d.%05d"),
							VER_MAJOR_VERSION,
							VER_MINOR_VERSION,
							VER_MINOR_SUBVERSION,
							VER_BUILD_NUMBER);
}

wxString util_my_dpf_stats(void)
{
	// return an unbounded vector of how many times the user has
	// encountered PAID features.

	return gpGlobalProps->getString(GlobalProps::GPS_DPF_STATS);
}


//////////////////////////////////////////////////////////////////

/**
 * build query string to fetch the requested page and append
 * the various credential/tracking fields.  These fields are
 * universal and not dependent upon which page we are loading.
 */
wxString util_create_query_url(const wxString & strUrlPath)
{
	// P=<package> is an alias for the build we are running.
	wxString strPackage = util_my_package();

	// B=<branch> is for something like "stable" or "unstable".
	wxString strBranch = util_my_branch();


	// V=<version> is a fixed-width formatting of the our
	// build version to make it easier for us and the server
	// to compare.
	wxString strVersion = util_my_version();

	//////////////////////////////////////////////////////////////////
	// Build URL with always-present query fields

	wxString strResult = strUrlPath;
	if (strResult.Find(wxT('?')) == wxNOT_FOUND)
		strResult += wxT('?');
	else
		strResult += wxT('&');

	strResult += wxString::Format( wxT("B=%s&P=%s&V=%s"),
								  strBranch.wc_str(),
								  strPackage.wc_str(),
								  strVersion.wc_str()
								  );

	//////////////////////////////////////////////////////////////////

	return strResult;
}


/**
 * TODO 2013/05/31 This routine has grown beyond just the NAG stuff.
 * TODO            I'm using it for building the diffmerge server url
 * TODO            in additional contexts now.
 * TODO
 * TODO            At issue is whether all server urls should have
 * TODO            the query fields tacked on or not.  For example,
 * TODO            yes for upgrade and license check, probably for
 * TODO            visit SG links, probably not for webhelp.
 */
wxString util_make_url(TYPE_URL t, bool bSendQueryFields)
{
	wxString strUrl;

	if (wxGetEnv( wxT("DIFFMERGE_SERVER_ROOT"), &strUrl))
	{
		// right-trim whitespace and append trailing slash to
		// whatever we get from the user.
		strUrl.Trim();
		int len = strUrl.length();
		if ((len > 0) && (strUrl[len-1] != wxT('/')))
			strUrl += wxT('/');
	}
	else
	{
		// TODO 2013/05/15 Use an official address.
		strUrl = wxT("http://download.sourcegear.com/DiffMerge/content/");
	}

	switch (t)
	{
	case T__URL__LICENSE:
		strUrl += wxT("license_info.zip");
		break;

	case T__URL__UPDATE:
		strUrl += util_my_branch();
		strUrl += wxT(".zip");
		break;

	case T__URL__VISIT:
		strUrl += wxT("visit/index.html");
		break;

	case T__URL__WEBHELP:
		strUrl += wxT("webhelp/index.html");
		break;

	default:
		wxASSERT_MSG( (0), wxT("Coding Error") );
		break;
	}

	if (bSendQueryFields)
		return util_create_query_url(strUrl);
	else
		return strUrl;
}


/* 
 * util_create_temp_dir() will create a sub-directory
 * within the system's TEMP directory using the previx
 * or "tmpd" if no prefix is provided.
 */
wxString util_create_temp_dir(const wxString& prefix)
{
	bool bCreated = false;

	// note, if no prefix was specified, use "tmpd"
	wxString strTempRootWithPrefix;

	wxString strT = wxFileName::GetTempDir();
	if (strT.EndsWith(wxFileName::GetPathSeparator()) == false )
		strT += wxFileName::GetPathSeparator();

	if (prefix.IsEmpty() == false) { strTempRootWithPrefix = strT + prefix; }
	else { strTempRootWithPrefix = strT + wxT("tmpd"); }

	// try up to 25x to create the subdirectory
	wxString strTempSubDir;
	int i = 0;
	do
	{
		// directory using ticks and a prime (randomly picked at the time of this method)
		strTempSubDir = wxString::Format(wxT("%s_%u"), strTempRootWithPrefix, static_cast<int>((wxGetUTCTimeMillis().GetValue() % 40459)));
		bCreated = wxFileName::Mkdir(strTempSubDir);

		// try again in a bit
		if (bCreated == false) { wxMilliSleep(71); }
	} while ((bCreated == false) && (i++ < 25));

	if (bCreated == false) { strTempSubDir.Empty(); }

	return strTempSubDir;
}


/*
 * util_copy_stream will copy from pFSIn to pFSOut for bytesToCopy or if bytesToCopy is -1, until EOF
 */
wxLongLong_t util_copy_stream(wxInputStream &fsIn, wxOutputStream &fsOut, wxLongLong_t bytesToCopy /*=-1*/)
{
	wxLongLong_t bytesCopied = 0L;

	// need a buffer (16K)
	const int nBufSize = 16 * 1024;
	wxByte *pBuf = new wxByte[nBufSize];
	if (pBuf != NULL)
	{
		// the real numeric types used to do the work
		wxLongLong_t bytesLeft = bytesToCopy;

		if (bytesToCopy > 0L)
		{
			do
			{	// determine how many bytes to read/write this iteration
				size_t const bytesToReadWrite = ((bytesToCopy == -1) || (bytesLeft >= nBufSize)) ? nBufSize : (size_t)(bytesToCopy - bytesCopied);

				// while bytes have been read, write out the stream
				// and update byte count
				fsIn.Read(pBuf, bytesToReadWrite);
				size_t const bytesRead = fsIn.LastRead();

				fsOut.Write(pBuf, bytesRead);
				size_t const bytesWritten = fsOut.LastWrite();

				bytesCopied += bytesWritten;
				if (bytesWritten == bytesRead)
				{
					if (bytesToCopy != -1)
					{	// adjust the counters
						bytesLeft -= bytesWritten;
					}

					// if EOF, set the flags to break the loop
					if ((bytesLeft > 0) && (fsIn.Eof() == true)) { bytesToCopy = bytesLeft = 0L; }
				}
				else
				{	// make it negative to break the loop
					bytesCopied *= -1;
				}
			} while ((bytesCopied >= 0L) && ((bytesLeft > 0L) || (bytesToCopy == -1)));
		}

		// a little cleanup
		delete[] pBuf;
	}

	return bytesCopied;
}


/*
 * __swapBits() is required by official CRC-32 standard.
 * is used during the initialization of the CRC table
*/
wxUint32 __swapBits(wxUint32 n, wxUint16 b)
{
	const wxUint32 nOne = 0x01;
	wxUint32 nRet = 0x00;

	// swap bit positions from bit 0 to bit b
	// bit 0 for bit b; bit 1 for bit b-1, etc.
	for (wxUint16 i = 1; i < (b + 1); i++)
	{
		if ((n & 1) != 0)
			nRet |= nOne << (b - i);
		n >>= nOne;
	}

	return nRet;
}

/*
 * initialize the CRC table
*/
void __initCRCTable(wxUint32 arTable[256])
{
	if (arTable != NULL)
	{
		const wxUint32 ulPolynomial = 0x04c11db7;

		// all 256 ASCII character codes.
		for (int i = 0; i < 256; i++)
		{
			arTable[i] = __swapBits(i, 8) << 24;

			for (int j = 0; j < 8; j++)
				arTable[i] = (arTable[i] << 1) ^ (((arTable[i] & (0x1 << 31)) != 0) ? ulPolynomial : 0);

			arTable[i] = __swapBits(arTable[i], 32);
		}
	}
}


/* 
 * Calculate the CRC32 for the buffer up to len characters
 */
wxUint32 util_calc_crc32(const wxByte* pBuf, size_t len)
{
	if ((pBuf == NULL) || (len <= 0)) return 0;

	// initialize the CRC table
	wxUint32 arCRCTable[256];
	__initCRCTable(arCRCTable);

	wxByte b = 0;
	wxUint32 nCRC32 = 0xFFFFFFFF;

	for (int i = 0; i < len; i++)
	{
		b = (wxByte)(nCRC32 ^ pBuf[i]);
		nCRC32 = (nCRC32 >> 8) ^ arCRCTable[b];
	}

	// exclusive or with the beginning value
	return (nCRC32 ^ 0xFFFFFFFF);
}
