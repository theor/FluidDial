// MSVC type-decorated symbol aliases for global scene/menu objects.
//
// On GCC/Clang, global variable symbols are NOT mangled with their type, so
// "extern Scene menuScene" and "MenuScene menuScene" resolve to the same symbol.
// On MSVC they DO include the type, causing LNK2001 when the declared type in a
// TU differs from the defined type.  /ALTERNATENAME maps the base-class-typed
// reference to the correctly-typed definition.
//
// Format: /ALTERNATENAME:<declared-mangled>=<defined-mangled>

#ifdef _MSC_VER
// menuScene: declared as Scene, defined as MenuScene
#pragma comment(linker, "/ALTERNATENAME:?menuScene@@3VScene@@A=?menuScene@@3VMenuScene@@A")
// statusScene: declared as Scene, defined as StatusScene
#pragma comment(linker, "/ALTERNATENAME:?statusScene@@3VScene@@A=?statusScene@@3VStatusScene@@A")
// macroMenu: declared as Menu (FileParser.cpp) or Scene (MenuScene.cpp), defined as MacroMenu
#pragma comment(linker, "/ALTERNATENAME:?macroMenu@@3VMenu@@A=?macroMenu@@3VMacroMenu@@A")
#pragma comment(linker, "/ALTERNATENAME:?macroMenu@@3VScene@@A=?macroMenu@@3VMacroMenu@@A")
// aboutScene: declared as Scene, defined as AboutScene
#pragma comment(linker, "/ALTERNATENAME:?aboutScene@@3VScene@@A=?aboutScene@@3VAboutScene@@A")
// filePreviewScene: declared as Scene, defined as FilePreviewScene
#pragma comment(linker, "/ALTERNATENAME:?filePreviewScene@@3VScene@@A=?filePreviewScene@@3VFilePreviewScene@@A")
// fileSelectScene: declared as Scene, defined as FileSelectScene
#pragma comment(linker, "/ALTERNATENAME:?fileSelectScene@@3VScene@@A=?fileSelectScene@@3VFileSelectScene@@A")
// homingScene: declared as Scene, defined as HomingScene
#pragma comment(linker, "/ALTERNATENAME:?homingScene@@3VScene@@A=?homingScene@@3VHomingScene@@A")
// multiJogScene: declared as Scene, defined as MultiJogScene
#pragma comment(linker, "/ALTERNATENAME:?multiJogScene@@3VScene@@A=?multiJogScene@@3VMultiJogScene@@A")
// toolchangeScene: declared as Scene, defined as ToolChangeScene
#pragma comment(linker, "/ALTERNATENAME:?toolchangeScene@@3VScene@@A=?toolchangeScene@@3VToolChangeScene@@A")
// probingScene: declared as Scene, defined as ProbingScene
#pragma comment(linker, "/ALTERNATENAME:?probingScene@@3VScene@@A=?probingScene@@3VProbingScene@@A")
// helpScene: declared as Scene, defined as HelpScene
#pragma comment(linker, "/ALTERNATENAME:?helpScene@@3VScene@@A=?helpScene@@3VHelpScene@@A")
#endif
