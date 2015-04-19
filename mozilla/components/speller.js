/* ***** BEGIN LICENSE BLOCK *****
 *
 *   This file is part of {NAME}.
 *
 *   {NAME} is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   {NAME} is distributed in the hope that it will be useful,
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
const CLASS_ID = Components.ID("{UUID1}");
const CLASS_NAME = "Spell checker for {LOCALES}";
const CONTRACT_ID = "@tinodidriksen.com/speller;1";
const MAX_SUGGS = 50;

function Shim() {
}

Shim.prototype = {
    shim: null,
    fn_shim_init: null,
    fn_shim_terminate: null,
    fn_shim_spell_cstr: null,
    fn_shim_suggest_cstr: null,

    init : function() {
        var runtime = Components.classes["@mozilla.org/xre/app-info;1"]
            .getService(Components.interfaces.nsIXULRuntime);

        var lib_name;

        // Detect shared library name to load
        if (runtime.OS === "Linux") {
            lib_name = "shim.so";
        }
        else if (runtime.OS === "WINNT") {
            lib_name = "shim32.dll";
        	if (runtime.XPCOMABI.indexOf('x86_64') != -1) {
        		lib_name = "shim64.dll";
        	}
        }
        else if (runtime.OS === "Darwin") {
            lib_name = "shim.dylib";
        }
        else {
            throw "Unsupported OS " + runtime.OS;
        }

        // Try to locate shim inside extension directory and replace value
        var extension_dir = __LOCATION__.parent.parent;
        var shim_loc = extension_dir.clone();
        shim_loc.append("native");
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

        // int shim_init();
        this.fn_shim_init = this.shim.declare(
            "shim_init",
            ctypes.default_abi,
            ctypes.int);

        // void shim_terminate();
        this.fn_shim_terminate = this.shim.declare(
            "shim_terminate",
            ctypes.default_abi,
            ctypes.void_t);

        // int shim_is_valid_word(const char * word);
        this.fn_shim_spell_cstr = this.shim.declare(
            "shim_is_valid_word",
            ctypes.default_abi,
            ctypes.int,
            ctypes.char.ptr);

        // const char ** shim_find_alternatives(const char * word, int suggs);
        this.fn_shim_suggest_cstr = this.shim.declare(
            "shim_find_alternatives",
            ctypes.default_abi,
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
    current_loc: null,
    shim_handle: null,
    mPersonalDictionary: null,

    QueryInterface: XPCOMUtils.generateQI([mozISpellCheckingEngine, Components.interfaces.nsISupport]),

    get dictionary() {
        return this.current_loc;
    },

    set dictionary(dict) {
    	this.current_loc = null;
    	for (var i=0 ; i<this.locales.length ; ++i) {
    		if (dict === this.locales[0]) {
    			this.current_loc = dict;
    			return;
    		}
    	}
    	throw dict+" is not supported by {NAME}\n";
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

        if (result == 0 && this.mPersonalDictionary && this.current_loc) {
            return this.mPersonalDictionary.check(word, this.current_loc);
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
