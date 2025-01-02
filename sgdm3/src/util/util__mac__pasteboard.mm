#if defined(__WXMAC__)
//////////////////////////////////////////////////////////////////
// util__mac__pasteboard.mm
// mac-specific code to handle NSFindPboard -- the system
// Pasteboard for Find (as opposed to the NSGenericPboard
// for Copy/Paste).  The goal of this is to make Command+E
// work betwen apps.

//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>

#include <AppKit/AppKit.h>
#include <wx/osx/core/cfstring.h>

//////////////////////////////////////////////////////////////////

@interface util__mac__search_text : NSObject
// - (NSString *)getSearchTextFromPasteboard;
// - (void)putFindStringOnPasteboard:(NSString *)searchText;
@end

@implementation util__mac__search_text

+ (NSString *)getSearchTextFromPasteboard
{
	NSString* searchText;

	NSPasteboard* findPboard = [NSPasteboard pasteboardWithName:NSFindPboard];
	if ([[findPboard types] indexOfObject:NSStringPboardType] != NSNotFound)
		searchText = [findPboard stringForType:NSStringPboardType];
	else
		searchText = @"";

	return searchText;
}

+ (void)putFindStringOnPasteboard:(NSString *)searchText
{
	NSPasteboard* pasteboard = [NSPasteboard pasteboardWithName:NSFindPboard];

	[pasteboard declareTypes:[NSArray arrayWithObject:NSStringPboardType] owner:nil];
	[pasteboard setString:searchText forType:NSStringPboardType];
}
@end

//////////////////////////////////////////////////////////////////

wxString util__mac__get_system_search_text(void)
{
	return wxCFStringRef::AsString( [util__mac__search_text getSearchTextFromPasteboard] );
}

void util__mac__set_system_search_text(const wxString & str)
{
	wxCFStringRef cf( str );

	[util__mac__search_text putFindStringOnPasteboard:cf.AsNSString()];
}

//////////////////////////////////////////////////////////////////

#endif // __WXMAC__
