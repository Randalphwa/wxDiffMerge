// MessageOutputString.cpp
// a trivial wxMessageOutput handler that collects msgs in a string.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <MessageOutputString.h>

//////////////////////////////////////////////////////////////////

void MessageOutputString::Printf(const wxChar * format, ...)
{
	wxString out;

    va_list args;
    va_start(args, format);

    out.PrintfV(format, args);
    va_end(args);

    out.Replace(wxT("\t"), wxT("        "));

	Output(out);
}

void MessageOutputString::Output(const wxString & str)
{
	msg += str;
}
