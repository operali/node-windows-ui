
declare var dialog: {
    // message box for infomation
    info(title: string, msg: string): Promise<boolean>,
    // message box for choose OK or cancel
    okCancel(title: string, msg: string): Promise<boolean>,
    // open file dialog
    openfile(initPath: string, filter: string): Promise<string>,
    // save file dialog
    savefile(initPath: string, filter: string): Promise<string>,
    // select directory file dialog
    selectdir(initPath: string, desc: string): Promise<string>
}

export { dialog }