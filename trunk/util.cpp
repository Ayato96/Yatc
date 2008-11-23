//////////////////////////////////////////////////////////////////////
// Yet Another Tibia Client
//////////////////////////////////////////////////////////////////////
// Utility functions
//////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//////////////////////////////////////////////////////////////////////

#include <sstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
	#include <sys/stat.h> // mkdir
	#include <errno.h>
#endif
#include "config.h"
#include "util.h"

#ifdef WIN32
#include <windows.h>
#endif


#define WINCE_INSTALLDIR "/Storage Card/YATC/"

void str_replace(std::string &s, const std::string& what, const std::string& with) {
	std::string::size_type p, l;
	for (l = 0; (p = s.find(what, l)) != std::string::npos; l = p + with.size()) {
		s.replace(p, what.size(), with);
	}
}


void NativeGUIError(const char* text, const char *title) {
	#ifdef __APPLE__
	printf("NativeGUIError('%s','%s');\n", text, title);
	#endif
	#ifdef WIN32
		#ifndef WINCE
			MessageBox(HWND_DESKTOP, text, title, MB_ICONSTOP);
		#else
			int a, b;

			BSTR unicodetext = SysAllocStringLen(NULL, a=strlen(text));
			BSTR unicodetitle = SysAllocStringLen(NULL, b=strlen(title));
			MultiByteToWideChar(CP_ACP, 0, text, a, unicodetext, a);
			MultiByteToWideChar(CP_ACP, 0, title, b, unicodetitle, b);



			MessageBox(HWND_DESKTOP, unicodetext, unicodetitle, MB_ICONSTOP);
			SysFreeString(unicodetext);
			SysFreeString(unicodetitle);

		#endif
	#else
		std::string texts=text, titles=title;
		str_replace(texts, "\\", "\\\\");
		str_replace(texts, "\"", "\\\"");

		std::stringstream cmd;
		cmd << "xmessage -title \"" << title << "\" -center \"" << texts << "\"";

		system(cmd.str().c_str());
	#endif
}


bool fileexists(const char* filename){
    FILE *f;
    if ((f = yatc_fopen(filename, "r"))) {
        fclose(f);
        return true;
    } else {
        return false;
	}
}

static bool __internal_fileexists(const char* filename){
    FILE *f;
    if ((f = fopen(filename, "r"))) {
        fclose(f);
        return true;
    } else
        return false;
}

static std::vector<std::string > searchpaths;


std::string yatc_findfile(const char* filename){
    for (std::vector<std::string>::iterator it = searchpaths.begin(); it != searchpaths.end(); it++) {
		std::string fn = (*it + "/" + filename).c_str();
		#ifndef WIN32
		if (fn[0] == '~')
			fn = std::string(getenv("HOME")) + "/" + fn.substr(1);
		#endif

		if (__internal_fileexists(fn.c_str())) {
			FILE *f=fopen(fn.c_str(), "r");

			if (f) {
			    fclose(f);
				return fn;
			}
			// if the above is false, we probably don't have enough permissions for requested mode
		}
	}
	FILE *f=fopen(filename, "r");
	if(f){
	    fclose(f);
	    return filename;
	}
	return "";
}

FILE *yatc_fopen(const char* filename, const char* mode) {
#ifdef WINCE
	/* ugly as hell, but WINCE's fopen does not use current working dir
	 * in its path ... it's also probably faster than using the
	 * searchpath.
	 * something also tells me SDL_LoadBMP() won't like this, but
	 * we'll be avoiding it anyway */

	char fopen_fn[255];
	sprintf(fopen_fn, "%s%s", WINCE_INSTALLDIR, filename);


	return fopen(fopen_fn, mode);
#else
	if (__internal_fileexists(filename)) {
		FILE *f = fopen(filename, mode);
		if (f)
			return f;
	}
	for (std::vector<std::string>::iterator it = searchpaths.begin(); it != searchpaths.end(); it++) {
		std::string fn = (*it + "/" + filename).c_str();
		#ifndef WIN32
		if (fn[0] == '~')
			fn = std::string(getenv("HOME")) + "/" + fn.substr(1);
		#endif

		if (__internal_fileexists(fn.c_str())) {
			FILE *f=fopen(fn.c_str(), mode);

			if (f) {
				return f;
			}
			// if the above is false, we probably don't have enough permissions for requested mode
		}
	}
	// if not found anywhere in the path, let's see what we can do with it
	#ifndef WIN32 // if these aren't windows, it's probably a unioxid; if not, we'll port later
	if (mode[0] == 'w' || mode[0] == 'a') {// if we're trying to access for writing
		std::string outfn = (std::string(getenv("HOME")) + "/.yatc/" + filename);
		FILE *f = fopen(outfn.c_str(), mode);
		if (f)
			return f;
		printf("Trying to make cfg dir in home, fopen(...,w) was failing: %s\n", strerror(errno));
		int i;
		if (i=mkdir((std::string(getenv("HOME")) + "/.yatc/").c_str(), 0700))
			printf("Failed to make cfg dir: %s\n", strerror(errno));
		else {
			f = fopen(outfn.c_str(), mode);
			if (f)
				return f;
		}
		printf("Still failed to write in home: %s\n", strerror(errno));
	}
	#endif

	// if we resume here, either we're under windows, or we're not attempting to open for writing


	return fopen(filename, mode);
#endif
}

#ifndef DESTDIRS
	#ifndef __APPLE__
		#define DESTDIRS ""
	#else
		#define DESTDIRS "Resources/"
	#endif
#endif

void yatc_fopen_init(char *cmdline) {
#if (HAVE_GETENV==1)
	const char *searchpath = getenv("YATC_PATH");
#else
	printf("Getenv() not supported; leaving searchpath empty. (Perhaps HAVE_GETENV isn't defined)\n");
	const char *searchpath = NULL;
#endif
	char temp[250];
	const char *lp;
	if (!searchpath)
	searchpath =
	#ifndef WIN32
		"~/.yatc/:"
		"/usr/games/share/yatc-data/:"
		"/usr/games/share/tibia/:"
	#endif
		DESTDIRS;

	lp = searchpath;
	for (const char* p=searchpath; ; p++) {
		if (*p==':' || *p==0) {
			char *a = new char[p-lp + 1];
			memcpy(a, lp, p-lp);
			a[p-lp] = 0;
			if (strlen(a)) {
				printf("Adding %s to search path\n", a);
				searchpaths.insert(searchpaths.end(),std::string(a));
			}
			delete []a;
			lp = p+1;
			if (!*p)
				break;
		}
	}
	if (cmdline) {
		for (int i = strlen(cmdline)-1; i >= 0; i--) {
			if (cmdline[i] == '/') {
				char *tmp = new char[i+2];
				memcpy(tmp, cmdline, i+1);
				tmp[i+1]=0;
				searchpaths.insert(searchpaths.end(), std::string(tmp));
				printf("Adding extra path %s\n", tmp);
#ifdef __APPLE__
				std::string tmp2 = std::string(tmp) + "../Resources/";
				printf("Also adding Mac bundle resource path %s\n", tmp2.c_str());
				searchpaths.insert(searchpaths.end(), tmp2);
#endif
				delete[] tmp;
				break;
			}
		}
	} else {
        printf("cmdline: null, can't be used for yatc_fopen_init()\n");
	}
}


#if defined(WINCE) && defined(_MSC_VER)
time_t time( time_t *inTT )
{
SYSTEMTIME sysTimeStruct;
FILETIME fTime;
ULARGE_INTEGER int64time;
time_t locTT = 0;

if ( inTT == NULL ) {
inTT = &locTT;
}

GetSystemTime( &sysTimeStruct );
if ( SystemTimeToFileTime( &sysTimeStruct, &fTime ) ) {
memcpy( &int64time, &fTime, sizeof( FILETIME ) );
/* Subtract the value for 1970-01-01 00:00 (UTC) */
int64time.QuadPart -= 0x19db1ded53e8000;
/* Convert to seconds. */
int64time.QuadPart /= 10000000;
*inTT = (time_t)int64time.QuadPart;
}
return *inTT;
}
#endif
