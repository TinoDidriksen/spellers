/* ***** BEGIN LICENSE BLOCK *****
 *
 *   This file is part of mozvoikko2.
 *
 *   mozvoikko2 is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   mozvoikko is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 * ***** END OF LICENSE BLOCK ***** */
Components.utils.import("resource://gre/modules/XPCOMUtils.jsm");
Components.utils.import("resource://gre/modules/ctypes.jsm");

const mozISpellCheckingEngine = Components.interfaces.mozISpellCheckingEngine;
const CLASS_ID = Components.ID("{GUID1}");
const CLASS_NAME = "Spell checker for {LOCALES}";
const CONTRACT_ID = "@tinodidriksen.com/speller;1";

function Shim() {
}

Shim.prototype = {
    libvoikko: null,

    data_loc: "",

    call_abi: ctypes.default_abi,

    fn_voikko_init: null,
    fn_voikko_terminate: null,
    fn_voikko_spell_cstr: null,
    fn_voikko_suggest_cstr: null,
    fn_voikko_free_cstr_array: null,
    fn_voikko_set_boolean_option: null,

    init : function() {
        var runtime = Components.classes["@mozilla.org/xre/app-info;1"]
            .getService(Components.interfaces.nsIXULRuntime);

        var abi = runtime.OS + "_" + runtime.XPCOMABI;

        var lib_name;
        this.data_loc = "";
        //==================================================================
        // Detect shared library name to load
        //==================================================================
        if (abi == "Linux_x86-gcc3" || abi == "Linux_x86_64-gcc3") {
            lib_name = "libvoikko.so.1";
        }
        else if (abi == "WINNT_x86-msvc" || abi == "WINNT_x86_64-msvc") {
            lib_name = "libvoikko-1.dll";
            this.call_abi = ctypes.winapi_abi;
        }
        else if (abi == "Darwin_x86_64-gcc3" || abi == "Darwin_x86-gcc3" || abi == "Darwin_ppc-gcc3") {
            lib_name = "libvoikko.1.dylib";
        }
        else {
            throw "Unsupported ABI " + abi;
        }

        // Try to locate libvoikko inside extension directory and
        // replace value
        var extension_dir = __LOCATION__.parent.parent;
        var libvoikko_loc = extension_dir.clone();
        libvoikko_loc.append("voikko");
        libvoikko_loc.append(abi);
        libvoikko_loc.append(lib_name);
        if (libvoikko_loc.exists() && libvoikko_loc.isFile()) {
            this.libvoikko = ctypes.open(libvoikko_loc.path);
            aConsoleService.logStringMessage("{NAME}: loaded " + libvoikko_loc.path);
        }
        else {
            this.libvoikko = ctypes.open(lib_name);
            aConsoleService.logStringMessage("{NAME}: loaded system libvoikko");
            aConsoleService.logStringMessage("{NAME}: libvoikko_loc = " + libvoikko_loc.path);
        }

        var data_loc = extension_dir.clone();
        data_loc.append("voikko");
        var data_loc_test = data_loc.clone();
        data_loc_test.append("2");
        data_loc_test.append("mor-standard");
        data_loc_test.append("voikko-fi_FI.pro");
        if (data_loc_test.exists() && data_loc_test.isFile()) {
            this.data_loc = data_loc.path;
            aConsoleService.logStringMessage("{NAME}: Found suomi-malaga data at " + this.data_loc);
        }

        /* Detect used libvoikko version and output a message to Javascript console. */
        var fn_voikkoGetVersion = this.libvoikko.declare(
            "voikkoGetVersion",
            this.call_abi,
            ctypes.char.ptr);

        var version_str = fn_voikkoGetVersion().readString();
        aConsoleService.logStringMessage("{NAME}: Using libvoikko version " + version_str);


        //
        // struct VoikkoHandle * voikkoInit(const char ** error, const char * langcode,
        //                                  const char * path);
        //
        this.fn_voikko_init = this.libvoikko.declare(
            "voikkoInit",
            this.call_abi,
            ctypes.voidptr_t, // Return value: we are not interested what is inside
            ctypes.char.ptr.ptr,
            ctypes.char.ptr,
            ctypes.char.ptr);

        //
        // void voikkoTerminate(struct VoikkoHandle * handle);
        //
        this.fn_voikko_terminate = this.libvoikko.declare(
            "voikkoTerminate",
            this.call_abi,
            ctypes.void_t,
            ctypes.voidptr_t);

        //
        // int voikkoSpellCstr(struct VoikkoHandle * handle, const char * word);
        //
        this.fn_voikko_spell_cstr = this.libvoikko.declare(
            "voikkoSpellCstr",
            this.call_abi,
            ctypes.int,
            ctypes.voidptr_t,
            ctypes.char.ptr);

        //
        // char ** voikkoSuggestCstr(struct VoikkoHandle * handle, const char * word);
        //
        this.fn_voikko_suggest_cstr = this.libvoikko.declare(
            "voikkoSuggestCstr",
            this.call_abi,
            ctypes.char.ptr.array(50).ptr,
            ctypes.voidptr_t,
            ctypes.char.ptr);

        //
        // void voikkoFreeCstrArray(char ** cstrArray);
        //
        this.fn_voikko_free_cstr_array = this.libvoikko.declare(
            "voikkoFreeCstrArray",
            this.call_abi,
            ctypes.void_t,
            ctypes.char.ptr.array(50).ptr);

        //
        // int voikkoSetBooleanOption(struct VoikkoHandle * handle, int option, int value);
        //
        this.fn_voikko_set_boolean_option = this.libvoikko.declare(
            "voikkoSetBooleanOption",
            this.call_abi,
            ctypes.int,
            ctypes.voidptr_t,
            ctypes.int,
            ctypes.int);
    },

    finalize : function() {
        this.fn_voikko_init = null;
        this.fn_voikko_terminate = null;
        this.libvoikko.close();
    }
};

var aConsoleService = Components.classes["@mozilla.org/consoleservice;1"].
    getService(Components.interfaces.nsIConsoleService);


try {
    var libvoikko = new Shim;
    libvoikko.init();
}
catch (err) {
    Components.utils.reportError(err);
    throw err;
}

function VoikkoHandle(libvoikko) {
}


VoikkoHandle.prototype = {
    handle: null,
    libvoikko: null,
    lang_code: null,

    open : function(libvoikko, lang_code) {
        var message = ctypes.char.ptr(null);
        var message_ptr = message.address();
        var data_loc = null;

        if (libvoikko.data_loc != "") {
            data_loc = libvoikko.data_loc;
        }

        this.libvoikko = libvoikko;

        this.handle = this.libvoikko.fn_voikko_init(
            message_ptr,
            "fi_FI",
            data_loc);

        this.libvoikko.fn_voikko_set_boolean_option(this.handle, VOIKKO_OPT_IGNORE_DOT, 1);
        this.libvoikko.fn_voikko_set_boolean_option(this.handle, VOIKKO_OPT_IGNORE_NUMBERS, 1);
        this.libvoikko.fn_voikko_set_boolean_option(this.handle, VOIKKO_OPT_IGNORE_UPPERCASE, 1);
        this.libvoikko.fn_voikko_set_boolean_option(this.handle, VOIKKO_OPT_ACCEPT_MISSING_HYPHENS, 1);
    },

    finalize : function() {
        if (handle) {
            this.libvoikko.fn_voikko_terminate(this.handle);
            this.handle = null;
        }
    }
};


function Speller() {
    try {
        this.voikko_handle = new VoikkoHandle;
        this.voikko_handle.open(libvoikko);
    }
    catch (err) {
        Components.utils.reportError(err);
        throw err;
    }
}

Speller.prototype = {
    classDescription: CLASS_NAME,
    classID: CLASS_ID,
    contractID: CONTRACT_ID,

    data_loc : null,
    voikko_handle : null,
    mPersonalDictionary : null,

    QueryInterface: XPCOMUtils.generateQI([mozISpellCheckingEngine, Components.interfaces.nsISupport]),

    get dictionary() {
        return "fi_FI";
    },

    set dictionary(dict) {
        if (dict != "fi_FI") {
            throw "mozvoikko2: dictionary '" + dict + "' is not supported by this component (but may be supported by others)";
        }
    },

    get providesPersonalDictionary() {
        return false;
    },

    get providesWordUtils() {
        return false;
    },

    get name() {
        return "MozVoikko 2";
    },

    get copyright() {
        return "GPL v2";
    },

    get personalDictionary() {
        return this.mPersonalDictionary;
    },

    set personalDictionary(mPersonalDictionary) {
        this.mPersonalDictionary = mPersonalDictionary;
    },

    getDictionaryList : function(dicts, count) {
        dicts.value = ["fi_FI"];
        count.value = 1;
    },

    check : function(word) {
        if (word.length < 2) {
            return true;
        }
        var result = libvoikko.fn_voikko_spell_cstr(this.voikko_handle.handle, word);

        if (result == 0 && this.mPersonalDictionary) {
            return this.mPersonalDictionary.check(word, "fi_FI");
        }
        else {
            return result != 0;
        }
    },

    suggest : function(word, sugg, count) {
        var i;
        var tmp = libvoikko.fn_voikko_suggest_cstr(this.voikko_handle.handle, word);

        count.value = 0;

        if (!tmp.isNull()) {
            sugg.value = [];
            count.value = 0;

            for (i = 0; i < 50 && !tmp.contents[i].isNull(); i++) {
                sugg.value[i] = tmp.contents[i].readString();
                count.value ++;
            }
        }

        libvoikko.fn_voikko_free_cstr_array(tmp);
    }
}


if (XPCOMUtils.generateNSGetFactory)
    var NSGetFactory = XPCOMUtils.generateNSGetFactory([Speller]);
else
    var NSGetModule = XPCOMUtils.generateNSGetModule([Speller]);
