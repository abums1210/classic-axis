#include "InputHandler.h"
#include "Configs.h"
#include "Utility.h"
#include "CamNew.h"
#include "CPad.h"

static const std::string controlKeysStrings[62] = {
    "ESC", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9",
    "F10", "F11", "F12", "INS", "DEL", "HOME", "END", "PGUP", "PGDN", "UP",
    "DOWN", "LEFT", "RIGHT", "DIVIDE", "TIMES", "PLUS", "MINUS", "PADDEL", "PADEND", "PADDOWN",
    "PADPGDN", "PADLEFT", "PAD5", "NUMLOCK", "PADRIGHT", "PADHOME", "PADUP", "PADPGUP", "PADINS", "PADENTER",
    "SCROLL", "PAUSE", "BACKSP", "TAB", "CAPSLK", "ENTER", "LSHIFT", "RSHIFT", "SHIFT", "LCTRL",
    "RCTRL", "LALT", "RALT", "LWIN", "RWIN", "APPS", "NULL"
};

void InputHandler::InitGInput() {
    GInput_Load(&pXboxPad);
}

bool InputHandler::HasPadInHands() {
    return (pXboxPad && pXboxPad->HasPadInHands());
}

int32_t InputHandler::StringToKey(const std::string& str) {
    int32_t key;
    int i;

    if (str.empty())
        return 0;

    key = static_cast<unsigned char>(str[0]);
    if (str[0] && str.length() > 1) {
        if (str == "SPACE")
            return rsSPACE;

        for (i = 0; i < 62; i++) {
            if (str == controlKeysStrings[i]) {
                key = i + rsESC;
                break;
            }
        }
    }

    return key;
}

bool InputHandler::GetKeyDown(int32_t key, bool old) {
    int32_t i;
    CPad* pad;
    CKeyboardState* keyState;

    pad = CPad::GetPad(0);
    keyState = old ? &pad->OldKeyState : &pad->NewKeyState;

    if (key < 255) {
        if (keyState->standardKeys[key])
            return true;
    }

    for (i = 0; i < 12; i++) {
        if (i + rsF1 == key) {
            if (keyState->FKeys[i])
                return true;
        }
    }

    switch (key) {
        case rsESC:     return keyState->esc != 0;
        case rsINS:     return keyState->insert != 0;
        case rsDEL:     return keyState->del != 0;
        case rsHOME:    return keyState->home != 0;
        case rsEND:     return keyState->end != 0;
        case rsPGUP:    return keyState->pgup != 0;
        case rsPGDN:    return keyState->pgdn != 0;
        case rsUP:      return keyState->up != 0;
        case rsDOWN:    return keyState->down != 0;
        case rsLEFT:    return keyState->left != 0;
        case rsRIGHT:   return keyState->right != 0;
        case rsSCROLL:  return keyState->scroll != 0;
        case rsPAUSE:   return keyState->pause != 0;
        case rsNUMLOCK: return keyState->numlock != 0;
        case rsDIVIDE:  return keyState->div != 0;
        case rsTIMES:   return keyState->mul != 0;
        case rsMINUS:   return keyState->sub != 0;
        case rsPLUS:    return keyState->add != 0;
        case rsPADENTER:return keyState->enter != 0;
        case rsPADDEL:  return keyState->decimal != 0;
        case rsPADEND:  return keyState->num1 != 0;
        case rsPADDOWN: return keyState->num2 != 0;
        case rsPADPGDN: return keyState->num3 != 0;
        case rsPADLEFT: return keyState->num4 != 0;
        case rsPAD5:    return keyState->num5 != 0;
        case rsPADRIGHT:return keyState->num6 != 0;
        case rsPADHOME: return keyState->num7 != 0;
        case rsPADUP:   return keyState->num8 != 0;
        case rsPADPGUP: return keyState->num9 != 0;
        case rsPADINS:  return keyState->num0 != 0;
        case rsBACKSP:  return keyState->back != 0;
        case rsTAB:     return keyState->tab != 0;
        case rsCAPSLK:  return keyState->capslock != 0;
        case rsENTER:   return keyState->extenter != 0;
        case rsLSHIFT:  return keyState->lshift != 0;
        case rsSHIFT:   return keyState->shift != 0;
        case rsRSHIFT:  return keyState->rshift != 0;
        case rsLCTRL:   return keyState->lctrl != 0;
        case rsRCTRL:   return keyState->rctrl != 0;
        case rsLALT:    return keyState->lmenu != 0;
        case rsRALT:    return keyState->rmenu != 0;
        case rsLWIN:    return keyState->lwin != 0;
        case rsRWIN:    return keyState->rwin != 0;
        case rsAPPS:    return keyState->apps != 0;
        default:        break;
    }

    return false;
}

bool InputHandler::WalkKeyDown() {
    if (CPad::GetPad(0)->DisablePlayerControls)
        return false;

    return GetKeyDown(StringToKey(configs.walkKey));
}

#ifdef GTA3
bool InputHandler::DuckKeyDown() {
    CPad* pad;

    pad = CPad::GetPad(0);
    if (pad->DisablePlayerControls)
        return false;

    if (HasPadInHands())
        return pad->NewState.ShockButtonL && !pad->OldState.ShockButtonL;

    return GetKeyDown(StringToKey(configs.crouchKey)) && !GetKeyDown(StringToKey(configs.crouchKey), true);
}
#endif
