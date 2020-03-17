
import * as winUI from './windows-ui/index';

const test = async () => {
    console.log(await winUI.dialog.info('title', 'hello world, 你好世界'));
    console.log(await winUI.dialog.okCancel('title', 'hello world, 请选择'));
    console.log(await winUI.dialog.openfile('C:/Program Files (x86)', '*.*'));
    console.log(await winUI.dialog.savefile('C:/Program Files (x86)', '*.*'));
    console.log(await winUI.dialog.selectdir("请选择目录", 'C:/Program Files (x86)'));
}
test();