#include "Menu.h"
#include "PieMenu.h"
#ifdef USE_WMB_FSS
#    include "FileMenu.h"
#endif
#include "System.h"

void noop(void* arg) {}

const int buttonRadius = 30;

static const char* menu_help_text[] = { "FluidDial",
                                        "Touch icon for scene",
                                        "Touch center for help",
                                        "Flick left to go back",
                                        "Authors: @bdring,@Mitch",
                                        "Bradley,@bDuthieDev,",
                                        "@Design8Studio",
                                        NULL };

// PieMenu axisMenu("Axes", buttonRadius);

class LB : public RoundButton {
public:
    LB(const char* text, callback_t callback, color_t base_color) :
        RoundButton(text, callback, buttonRadius, base_color, GREEN, BLUE, WHITE) {}
    LB(const char* text, Scene* scene, color_t base_color) : RoundButton(text, scene, buttonRadius, base_color, GREEN, BLUE, WHITE) {}
};

constexpr int LIGHTYELLOW = 0xFFF0;
class IB : public ImageButton {
public:
    IB(const char* text, callback_t callback, const char* filename) : ImageButton(text, callback, filename, buttonRadius, WHITE) {}
    IB(const char* text, Scene* scene, const char* filename) : ImageButton(text, scene, filename, buttonRadius, WHITE) {}
};

extern Scene homingScene;
//extern Scene joggingScene;
//extern Scene joggingScene2;
extern Scene multiJogScene;
#ifndef NO_PROBE_MENU
extern Scene probingScene;
#endif
#ifndef NO_TOOLCHANGE_MENU
extern Scene toolchangeScene;
#endif
extern Scene statusScene;
extern Scene macroMenu;

#ifdef USE_WMB_FSS
extern Scene wmbFileSelectScene;
#else
extern Scene fileSelectScene;
#endif

Scene& jogScene = multiJogScene;

extern Scene controlScene;
extern Scene aboutScene;

IB statusButton("Status", &statusScene, "statustp.png");
IB homingButton("Homing", &homingScene, "hometp.png");
IB jogButton("Jog", &jogScene, "jogtp.png");
#ifndef NO_PROBE_MENU
IB probeButton("Probe", &probingScene, "probetp.png");
#endif
#ifndef NO_TOOLCHANGE_MENU
IB toolchangeButton("Tools", &toolchangeScene, "toolchangetp.png");
#endif

#ifdef USE_WMB_FSS
IB filesButton("Files", &wmbFileSelectScene, "filestp.png");
#else
IB filesButton("Files", &fileSelectScene, "filestp.png");
#endif

IB controlButton("Macros", &macroMenu, "macrostp.png");
IB setupButton("About", &aboutScene, "abouttp.png");

class MenuScene : public PieMenu {
public:
    MenuScene() : PieMenu("Main", buttonRadius, menu_help_text) {}
    void disableIcons() {
        statusButton.disable();
        homingButton.disable();
        jogButton.disable();
#ifndef NO_PROBE_MENU
        probeButton.disable();
#endif
#ifndef NO_TOOLCHANGE_MENU
        toolchangeButton.disable();
#endif
        filesButton.disable();
        controlButton.disable();
        setupButton.enable();
    }
    void enableIcons() {
        statusButton.enable();
        homingButton.enable();
        jogButton.enable();
#ifndef NO_PROBE_MENU
        probeButton.enable();
#endif
#ifndef NO_TOOLCHANGE_MENU
        toolchangeButton.enable();
#endif
        filesButton.enable();
        controlButton.enable();
        setupButton.enable();
    }
    void onEntry(void* arg) {
        PieMenu::onEntry(arg);
        if (state == Disconnected) {
            disableIcons();
        } else {
            enableIcons();
        }
    }
    void onStateChange(state_t old_state) override {
        if (state != Disconnected) {
            enableIcons();
            if (old_state == Disconnected) {
#ifdef AUTO_JOG_SCENE
                if (state == Idle) {
                    push_scene(&jogScene);
                    return;
                }
#endif
#ifdef AUTO_HOMING_SCENE
                if (state == Alarm && lastAlarm == 14) {  // Unknown or Unhomed
                    push_scene(&homingScene, (void*)"auto");
                    return;
                }
#endif
            }
        }
        reDisplay();
    }
} menuScene;

Scene* initMenus() {
    menuScene.addItem(&statusButton);
    menuScene.addItem(&homingButton);
    menuScene.addItem(&jogButton);
#ifndef NO_PROBE_MENU
    menuScene.addItem(&probeButton);
#endif
#ifndef NO_TOOLCHANGE_MENU
    menuScene.addItem(&toolchangeButton);
#endif
    menuScene.addItem(&filesButton);
    menuScene.addItem(&controlButton);
    menuScene.addItem(&setupButton);

    return &menuScene;
}
