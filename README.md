# node-windows-ui
node module for windows native UI utilities

# install 
```sh
> yarn add windows-ui
```

# usage
you may try `npm start`
```js
const winUI = require('windows-ui')

const test = async () => {
    // messagebox info
    console.log(await winUI.dialog.info('title', 'hello world, 你好世界'));
    // messagebox ok & cancel
    console.log(await winUI.dialog.okCancel('title', 'hello world, 请选择'));
    // open file dialog
    console.log(await winUI.dialog.openfile('C:/Program Files (x86)', '*.exe'));
    // save as file dialog
    console.log(await winUI.dialog.savefile('C:/Program Files (x86)', '*.*'));
    // select a directory
    console.log(await winUI.dialog.selectdir("请选择目录", 'C:/Program Files (x86)'));
}
test();
```



