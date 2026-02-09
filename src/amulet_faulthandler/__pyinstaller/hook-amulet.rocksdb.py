from PyInstaller.utils.hooks import collect_data_files, collect_submodules

hiddenimports = collect_submodules("amulet_faulthandler")
datas = collect_data_files("amulet_faulthandler")
