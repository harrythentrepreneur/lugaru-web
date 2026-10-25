/*
Copyright (C) 2003, 2010 - Wolfire Games
Copyright (C) 2010-2017 - Lugaru contributors (see AUTHORS file)

This file is part of Lugaru.

Lugaru is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Lugaru is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Lugaru.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Folders.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wvariadic-macro-arguments-omitted"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#if PLATFORM_UNIX
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#if _WIN32
#include <shlobj.h> // to get paths related functions
#include <windows.h>
#endif

const std::string Folders::dataDir = DATA_DIR;

std::string Folders::getScreenshotDir()
{
    std::string screenshotDir = getUserDataPath() + "/Screenshots";
    makeDirectory(screenshotDir);
    return screenshotDir;
}

std::string Folders::getUserDataPath()
{
    std::string userDataPath;
#ifdef _WIN32
    char path[MAX_PATH];
    // %APPDATA% (%USERPROFILE%\Application Data)
    if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_APPDATA, nullptr, 0, path))) {
        userDataPath = std::string(path) + "/Lugaru/";
    } else {
        return dataDir;
    }
#elif (defined(__APPLE__) && defined(__MACH__))
    const char* homePath = getHomeDirectory();
    if (homePath == NULL) {
        userDataPath = ".";
    } else {
        userDataPath = std::string(homePath) + "/Library/Application Support/Lugaru";
    }
#else // Linux
    userDataPath = getGenericDirectory("XDG_DATA_HOME", ".local/share");
#endif
    makeDirectory(userDataPath);
    return userDataPath;
}

std::string Folders::getConfigFilePath()
{
    std::string configFolder;
#if defined(_WIN32) || (defined(__APPLE__) && defined(__MACH__))
    configFolder = getUserDataPath();
#else // Linux
    configFolder = getGenericDirectory("XDG_CONFIG_HOME", ".config");
    makeDirectory(configFolder);
#endif
    return configFolder + "/config.txt";
}

#if PLATFORM_LINUX
/* Generic code for XDG ENVVAR test and fallback */
std::string Folders::getGenericDirectory(const char* ENVVAR, const std::string& fallback)
{
    const char* path = getenv(ENVVAR);
    std::string ret;
    if ((path != NULL) && (strlen(path) != 0)) {
        ret = std::string(path) + "/lugaru";
    } else {
        const char* homedir = getHomeDirectory();
        if ((homedir != NULL) && (strlen(homedir) != 0)) {
            ret = std::string(homedir) + '/' + fallback + "/lugaru";
        } else {
            ret = ".";
        }
    }
    return ret;
}
#endif

#if PLATFORM_UNIX
const char* Folders::getHomeDirectory()
{
    const char* homedir = getenv("HOME");
    if (homedir != NULL) {
        return homedir;
    }
    struct passwd* pw = getpwuid(getuid());
    if (pw != NULL) {
        return pw->pw_dir;
    }
    return NULL;
}
#endif

bool Folders::makeDirectory(const std::string& path)
{
#ifdef _WIN32
    int status = CreateDirectory(path.c_str(), NULL);
    return ((status != 0) || (GetLastError() == ERROR_ALREADY_EXISTS));
#else
    errno = 0;
    int status = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    return ((status == 0) || (errno == EEXIST));
#endif
}

void Folders::initPersistentStorage()
{
#ifdef __EMSCRIPTEN__
    /* Mount Emscripten's IDBFS over the two directories the Linux codepath
     * uses (HOME is /home/web_user in Emscripten):
     *   - ~/.config/lugaru      -> config.txt
     *   - ~/.local/share/lugaru -> users, Screenshots, Maps, ...
     * Then synchronously (via Asyncify) pull the saved data out of IndexedDB
     * before LoadSettings() runs. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdollar-in-identifier-extension"
    EM_ASM({
        function mkdirTree(path) {
            var parts = path.split('/').filter(function (p) { return p.length > 0; });
            var current = "";
            for (var i = 0; i < parts.length; i++) {
                current += '/' + parts[i];
                try {
                    FS.mkdir(current);
                } catch (e) {
                    /* already exists */
                }
            }
        }

        /* Remove a directory tree, but never a regular file.  Older builds
         * created dataDir + '/users' as a directory; if such a directory was
         * persisted (or survives a failed IDBFS mount), it makes the account
         * loader read a directory as a binary file and can hang boot. */
        function removeDirectoryTree(path) {
            var st;
            try {
                st = FS.stat(path);
            } catch (e) {
                return false;
            }
            if (!FS.isDir(st.mode)) {
                return false;
            }
            var entries = [];
            try {
                entries = FS.readdir(path);
            } catch (e) {}
            for (var i = 0; i < entries.length; i++) {
                var name = entries[i];
                if (name === '.' || name === '..') continue;
                var child = path + '/' + name;
                var childStat;
                try {
                    childStat = FS.stat(child);
                } catch (e) {
                    continue;
                }
                if (FS.isDir(childStat.mode)) {
                    removeDirectoryTree(child);
                } else {
                    try { FS.unlink(child); } catch (e) {}
                }
            }
            try {
                FS.rmdir(path);
                return true;
            } catch (e) {
                return false;
            }
        }

        var configDir = '/home/web_user/.config/lugaru';
        var dataDir = '/home/web_user/.local/share/lugaru';
        /* Clear any pre-existing native artifacts left behind by prior runs
         * (or IDBFS leftovers from an earlier mount) before re-mounting, so
         * the browser-backed IndexedDB filesystem is the only source. */
        if (!Module['__lugaruIdbMounted'] && typeof IDBFS !== 'undefined') {
            try { FS.unlink(configDir + '/config.txt'); } catch (e) {}
            try { FS.unlink(dataDir + '/users'); } catch (e) {}
            try { FS.unmount(configDir); } catch (e) {}
            try { FS.unmount(dataDir); } catch (e) {}
        }
        mkdirTree(configDir);
        mkdirTree(dataDir);
        /* Do NOT create dataDir + '/users': that path is the save file
         * returned by Folders::getUserSavePath(), not a directory. Creating
         * it makes fopen() hit EISDIR (or parse directory data as a huge
         * account count). Let the first save create the file instead. */
        mkdirTree(dataDir + '/Maps');

        if (!Module['__lugaruIdbMounted'] && typeof IDBFS !== 'undefined') {
            try {
                FS.mount(IDBFS, {}, configDir);
                FS.mount(IDBFS, {}, dataDir);
                Module['__lugaruIdbMounted'] = true;
                console.log('[Lugaru] IDBFS mounted for settings and profiles.');
            } catch (e) {
                console.error('[Lugaru] Could not mount IDBFS:', e);
            }
        }

        /* Repeat the cleanup after mounting so a legacy directory loaded
         * from IndexedDB (or left behind by a failed mount) cannot reach the
         * account loader.  This only removes directories, never a save file. */
        if (removeDirectoryTree(dataDir + '/users')) {
            console.log("[Lugaru] Removed legacy 'users' directory.");
        }

        if (!Module['__lugaruIdbLoaded']) {
            Module['__lugaruIdbLoaded'] = true;
        } else {
            Module['__lugaruIdbLoaded'] = true;
        }
    });
#pragma GCC diagnostic pop

    /* Wait (Asyncify) for the asynchronous IndexedDB read, with a safety
     * timeout so a blocked/denied IndexedDB cannot freeze the boot. */
    int guard = 0;
    while (!EM_ASM_INT({ return Module['__lugaruIdbLoaded'] ? 1 : 0; }) && guard++ < 600) {
        emscripten_sleep(5);
    }
    if (guard >= 600) {
        printf("Warning: timed out waiting for browser storage; using defaults.\n");
    }
#endif
}

#ifdef __EMSCRIPTEN__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdollar-in-identifier-extension"
std::string Folders::loadStringFromStorage(const char* key)
{
    char* out = (char*)EM_ASM_PTR({
        try { return stringToNewUTF8(localStorage.getItem(UTF8ToString($0)) || ""); } catch(e) { return stringToNewUTF8(""); }
    }, key);
    std::string ret(out ? out : "");
    if (out) free(out);
    return ret;
}
void Folders::saveStringToStorage(const char* key, const std::string& value)
{
    EM_ASM({
        try { localStorage.setItem(UTF8ToString($0), UTF8ToString($1)); } catch(e) {}
    }, key, value.c_str());
}
#pragma GCC diagnostic pop
#endif

void Folders::syncPersistentStorage()
{
#ifdef __EMSCRIPTEN__
    // no-op: persistence via localStorage
#endif
}
FILE* Folders::openMandatoryFile(const std::string& filename, const char* mode)
{
    FILE* tfile = fopen(filename.c_str(), mode);
    if (tfile == NULL) {
        throw FileNotFoundException(filename);
    }
    return tfile;
}

bool Folders::file_exists(const std::string& filepath)
{
    FILE* file;
    file = fopen(filepath.c_str(), "rb");
    if (file == NULL) {
        return false;
    } else {
        fclose(file);
        return true;
    }
}
