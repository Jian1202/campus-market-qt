# 链接后部署到实际 exe 目录，保留已有配置。
isEmpty(CAMPUS_SOURCE_ROOT): CAMPUS_SOURCE_ROOT = $$PWD/..
win32 {
 !win32-g++: error(Requires the bundled Qt 6.11.1 MinGW 64-bit kit)
 isEmpty(DESTDIR) {
  CONFIG(debug, debug|release): DESTDIR = $$OUT_PWD/debug
  else: DESTDIR = $$OUT_PWD/release
 }
 CAMPUS_DEPLOY_COMMAND = $$shell_quote($$shell_path($$(SystemRoot)/System32/WindowsPowerShell/v1.0/powershell.exe)) -NoProfile -ExecutionPolicy Bypass -File $$shell_quote($$shell_path($$CAMPUS_SOURCE_ROOT/tools/deploy_runtime.ps1)) -SourceRoot $$shell_quote($$shell_path($$CAMPUS_SOURCE_ROOT)) -OutputDir $$shell_quote($$shell_path($$DESTDIR)) -QtVersion $$QT_VERSION -Architecture $$QT_ARCH
 runtime_deploy.target = deploy_runtime
 runtime_deploy.commands = $$CAMPUS_DEPLOY_COMMAND
 # MinGW make 的目标规则用正斜杠；不能引用含反斜杠的 DESTDIR_TARGET。
 runtime_deploy.depends = $$relative_path($$DESTDIR/$${TARGET}.exe, $$OUT_PWD)
 runtime_deploy.CONFIG += phony
 all.depends += deploy_runtime
 QMAKE_EXTRA_TARGETS += runtime_deploy all
}
