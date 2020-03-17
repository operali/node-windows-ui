const os = require('os');
const assert = require('assert');

if (os.platform() !== 'win32') {
    assert(false, "dialog is only for windows now");
    return;
}

const nativeMethods = require('./build/Release/index.node')

const dialog = {
    async info(title, msg) {
        return new Promise((res) => {
            nativeMethods.messagebox('CONFIRM', title, msg, (val) => {
                res(val);
            });
        })
    },

    async okCancel(title, msg) {
        return new Promise((res) => {
            nativeMethods.messagebox('OKCANCEL', title, msg, (isOk) => {
                res(isOk);
            });
        })
    },
    async openfile(initPath, filter) {
        return new Promise((res) => {
            nativeMethods.selectfile(true, initPath, filter, (retPath) => {
                res(retPath);
            });
        })
    },
    async savefile(initPath, filter) {
        return new Promise((res) => {
            nativeMethods.selectfile(false, initPath, filter, (retPath) => {
                res(retPath);
            });
        })
    },
    async selectdir(initPath, desc) {
        return new Promise((res) => {
            nativeMethods.selectdir(desc, initPath, (retPath) => {
                res(retPath);
            });
        })
    }
}

module.exports = { dialog }