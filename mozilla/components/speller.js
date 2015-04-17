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
const MAX_SUGGS = 50;

function Shim() {
}

Shim.prototype = {
    shim: null,

    data_loc: "",

    call_abi: ctypes.default_abi,

    fn_shim_init: null,
    fn_shim_terminate: null,
    fn_shim_spell_cstr: null,
    fn_shim_suggest_cstr: null,

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
            lib_name = "shim.so.1";
        }
        else if (abi == "WINNT_x86-msvc" || abi == "WINNT_x86_64-msvc") {
            lib_name = "shim-1.dll";
            this.call_abi = ctypes.winapi_abi;
        }
        else if (abi == "Darwin_x86_64-gcc3" || abi == "Darwin_x86-gcc3" || abi == "Darwin_ppc-gcc3") {
            lib_name = "shim.1.dylib";
        }
        else {
            throw "Unsupported ABI " + abi;
        }

        // Try to locate shim inside extension directory and
        // replace value
        var extension_dir = __LOCATION__.parent.parent;
        var shim_loc = extension_dir.clone();
        shim_loc.append("voikko");
        shim_loc.append(abi);
        shim_loc.append(lib_name);
        if (shim_loc.exists() && shim_loc.isFile()) {
            this.shim = ctypes.open(shim_loc.path);
            aConsoleService.logStringMessage("{NAME}: loaded " + shim_loc.path);
        }
        else {
            this.shim = ctypes.open(lib_name);
            aConsoleService.logStringMessage("{NAME}: loaded system shim");
            aConsoleService.logStringMessage("{NAME}: shim_loc = " + shim_loc.path);
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


        //
        // int shim_init();
        //
        this.fn_shim_init = this.shim.declare(
            "shim_init",
            this.call_abi,
            ctypes.int);

        //
        // void shim_terminate();
        //
        this.fn_shim_terminate = this.shim.declare(
            "shim_terminate",
            this.call_abi,
            ctypes.void_t);

        //
        // int shim_is_valid_word(const char * word);
        //
        this.fn_shim_spell_cstr = this.shim.declare(
            "shim_is_valid_word",
            this.call_abi,
            ctypes.int,
            ctypes.char.ptr);

        //
        // const char ** shim_find_alternatives(const char * word, int suggs);
        //
        this.fn_shim_suggest_cstr = this.shim.declare(
            "shim_find_alternatives",
            this.call_abi,
            ctypes.char.ptr.array(MAX_SUGGS).ptr,
            ctypes.char.ptr,
            ctypes.int);
    },

    finalize : function() {
        this.fn_shim_init = null;
        this.fn_shim_terminate = null;
        this.shim.close();
    }
};

var aConsoleService = Components.classes["@mozilla.org/consoleservice;1"].
    getService(Components.interfaces.nsIConsoleService);


try {
    var shim = new Shim;
    shim.init();
}
catch (err) {
    Components.utils.reportError(err);
    throw err;
}


function ShimHandle(shim) {
}

ShimHandle.prototype = {
    handle: null,
    shim: null,

    open : function(shim) {
        this.shim = shim;
        this.handle = this.shim.fn_shim_init();
    },

    finalize : function() {
        if (handle) {
            this.shim.fn_shim_terminate(this.handle);
            this.handle = null;
        }
    }
};


function Speller() {
    try {
        this.shim_handle = new ShimHandle;
        this.shim_handle.open(shim);
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

    locales: {LOCALES_JSON},
    shim_handle: null,
    mPersonalDictionary: null,
    mCurrentLocale: null,

    QueryInterface: XPCOMUtils.generateQI([mozISpellCheckingEngine, Components.interfaces.nsISupport]),

    get dictionary() {
        return "fi_FI";
    },

    set dictionary(dict) {
        if (dict != "fi_FI") {
            throw "{NAME}: dictionary '" + dict + "' is not supported by this component (but may be supported by others)";
        }
    },

    get providesPersonalDictionary() {
        return false;
    },

    get providesWordUtils() {
        return false;
    },

    get name() {
        return "{NAME}";
    },

    get copyright() {
        return "GPL v3+";
    },

    get personalDictionary() {
        return this.mPersonalDictionary;
    },

    set personalDictionary(mPersonalDictionary) {
        this.mPersonalDictionary = mPersonalDictionary;
    },

    getDictionaryList: function(dicts, count) {
        dicts.value = this.locales;
        count.value = this.locales.length;
    },

    check: function(word) {
        var result = shim.fn_shim_spell_cstr(word);

        if (result == 0 && this.mPersonalDictionary) {
            return this.mPersonalDictionary.check(word, "fi_FI");
        }
		return result != 0;
    },

    suggest: function(word, sugg, count) {
        var tmp = shim.fn_shim_suggest_cstr(word, MAX_SUGGS);

		sugg.value = [];
		count.value = 0;

        if (tmp.isNull()) {
        	return;
        }

		for (var i = 0; i < MAX_SUGGS && !tmp.contents[i].isNull(); ++i) {
			sugg.value[i] = tmp.contents[i].readString();
			++count.value;
		}
    }
}


if (XPCOMUtils.generateNSGetFactory) {
    var NSGetFactory = XPCOMUtils.generateNSGetFactory([Speller]);
}
else {
    var NSGetModule = XPCOMUtils.generateNSGetModule([Speller]);
}
