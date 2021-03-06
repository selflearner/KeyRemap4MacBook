#include "KeyCode.hpp"
#include "Config.hpp"

namespace org_pqrs_KeyRemap4MacBook {
#include "../../../src/bridge/output/include.kext.keycode.cpp"

  // ------------------------------------------------------------
  bool
  EventType::isKeyDownOrModifierDown(KeyCode key, Flags flags) const
  {
    if (*this == EventType::DOWN) return true;
    if (*this == EventType::MODIFY) {
      return flags.isOn(key.getModifierFlag());
    }
    return false;
  }

  bool
  KeyCode::FNKeyHack::remap(KeyCode& key, Flags flags, EventType eventType, bool& active, KeyCode fromKeyCode, KeyCode toKeyCode)
  {
    if (key != fromKeyCode) return false;

    bool isKeyDown = eventType.isKeyDownOrModifierDown(key, flags);
    if (isKeyDown) {
      if (! flags.isOn(ModifierFlag::FN)) return false;
      active = true;
    } else {
      if (! active) return false;
      active = false;
    }

    key = toKeyCode;

    return true;
  }

  namespace {
    KeyCode::FNKeyHack fnkeyhack[] = {
      KeyCode::FNKeyHack(KeyCode::KEYPAD_0, KeyCode::M),
      KeyCode::FNKeyHack(KeyCode::KEYPAD_1, KeyCode::J),
      KeyCode::FNKeyHack(KeyCode::KEYPAD_2, KeyCode::K),
      KeyCode::FNKeyHack(KeyCode::KEYPAD_3, KeyCode::L),
      KeyCode::FNKeyHack(KeyCode::KEYPAD_4, KeyCode::U),
      KeyCode::FNKeyHack(KeyCode::KEYPAD_5, KeyCode::I),
      KeyCode::FNKeyHack(KeyCode::KEYPAD_6, KeyCode::O),
      KeyCode::FNKeyHack(KeyCode::KEYPAD_7, KeyCode::KEY_7),
      KeyCode::FNKeyHack(KeyCode::KEYPAD_8, KeyCode::KEY_8),
      KeyCode::FNKeyHack(KeyCode::KEYPAD_9, KeyCode::KEY_9),
      KeyCode::FNKeyHack(KeyCode::KEYPAD_CLEAR, KeyCode::KEY_6),
      KeyCode::FNKeyHack(KeyCode::KEYPAD_PLUS, KeyCode::SLASH),
      KeyCode::FNKeyHack(KeyCode::KEYPAD_MINUS, KeyCode::SEMICOLON),
      KeyCode::FNKeyHack(KeyCode::KEYPAD_MULTIPLY, KeyCode::P),
      KeyCode::FNKeyHack(KeyCode::KEYPAD_SLASH, KeyCode::KEY_0),
      KeyCode::FNKeyHack(KeyCode::KEYPAD_EQUAL, KeyCode::MINUS),
      KeyCode::FNKeyHack(KeyCode::KEYPAD_DOT, KeyCode::DOT),
      KeyCode::FNKeyHack(KeyCode::PAGEUP, KeyCode::CURSOR_UP),
      KeyCode::FNKeyHack(KeyCode::PAGEDOWN, KeyCode::CURSOR_DOWN),
      KeyCode::FNKeyHack(KeyCode::HOME, KeyCode::CURSOR_LEFT),
      KeyCode::FNKeyHack(KeyCode::END, KeyCode::CURSOR_RIGHT),
      KeyCode::FNKeyHack(KeyCode::ENTER, KeyCode::RETURN),
      KeyCode::FNKeyHack(KeyCode::FORWARD_DELETE, KeyCode::DELETE),
    };
  }

  void
  KeyCode::normalizeKey(KeyCode& key, Flags& flags, EventType eventType, KeyboardType keyboardType)
  {
    // We can drop CURSOR and KEYPAD flags, because we'll set these flags at reverseNormalizeKey.
    flags.stripCURSOR().stripKEYPAD();

    if (keyboardType == KeyboardType::POWERBOOK ||
        keyboardType == KeyboardType::POWERBOOK_G4 ||
        keyboardType == KeyboardType::POWERBOOK_G4_TI) {
      if (key == KeyCode::ENTER_POWERBOOK) { key = KeyCode::ENTER; }
    }

    if (! Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_general_disable_numpad_hack)) {
      for (unsigned int i = 0; i < sizeof(fnkeyhack) / sizeof(fnkeyhack[0]); ++i) {
        if (fnkeyhack[i].normalize(key, flags, eventType)) break;
      }
    }
  }

  void
  KeyCode::reverseNormalizeKey(KeyCode& key, Flags& flags, EventType eventType, KeyboardType keyboardType)
  {
    if (! Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_general_disable_numpad_hack)) {
      for (unsigned int i = 0; i < sizeof(fnkeyhack) / sizeof(fnkeyhack[0]); ++i) {
        if (fnkeyhack[i].reverse(key, flags, eventType)) break;
      }
    }

    if (keyboardType == KeyboardType::POWERBOOK ||
        keyboardType == KeyboardType::POWERBOOK_G4 ||
        keyboardType == KeyboardType::POWERBOOK_G4_TI) {
      if (key == KeyCode::ENTER) { key = KeyCode::ENTER_POWERBOOK; }
    }

    // ------------------------------------------------------------
    // Don't add ModifierFlag::FN automatically for F-keys, PageUp/PageDown/Home/End and Forward Delete.
    //
    // PageUp/PageDown/Home/End and Forward Delete are entered by fn+arrow, fn+delete normally,
    // And, from Cocoa Application, F-keys and PageUp,... keys have Fn modifier
    // even if Fn key is not pressed actually.
    // So, it's natural adding ModifierFlag::FN to these keys.
    // However, there is a reason we must not add ModifierFlag::FN to there keys.
    //
    // Mission Control may have "fn" as shortcut key.
    // If we add ModifierFlag::FN here,
    // "XXX to PageUp" launches Mission Control because Mission Control recognizes fn key was pressed.
    //
    // It's not intended behavior from users.
    // Therefore, we don't add ModifierFlag::FN for these keys.

    // ------------------------------------------------------------
    // set ModifierFlag::KEYPAD, ModifierFlag::CURSOR
    flags.stripCURSOR().stripKEYPAD();

    // Note: KEYPAD_CLEAR, KEYPAD_COMMA have no ModifierFlag::KEYPAD bit.
    if (key == KeyCode::KEYPAD_0 || key == KeyCode::KEYPAD_1 || key == KeyCode::KEYPAD_2 ||
        key == KeyCode::KEYPAD_3 || key == KeyCode::KEYPAD_4 || key == KeyCode::KEYPAD_5 ||
        key == KeyCode::KEYPAD_6 || key == KeyCode::KEYPAD_7 || key == KeyCode::KEYPAD_8 ||
        key == KeyCode::KEYPAD_9 ||
        key == KeyCode::KEYPAD_DOT ||
        key == KeyCode::KEYPAD_MULTIPLY ||
        key == KeyCode::KEYPAD_PLUS ||
        key == KeyCode::KEYPAD_SLASH ||
        key == KeyCode::KEYPAD_MINUS ||
        key == KeyCode::KEYPAD_EQUAL) {
      flags.add(ModifierFlag::KEYPAD);
    }

    if (key == KeyCode::CURSOR_UP ||
        key == KeyCode::CURSOR_DOWN ||
        key == KeyCode::CURSOR_LEFT ||
        key == KeyCode::CURSOR_RIGHT) {
      flags.add(ModifierFlag::CURSOR);
    }
  }

  KeyCode
  ModifierFlag::getKeyCode(void) const {
    if (*this == ModifierFlag::CAPSLOCK) return KeyCode::CAPSLOCK;
    if (*this == ModifierFlag::SHIFT_L) return KeyCode::SHIFT_L;
    if (*this == ModifierFlag::SHIFT_R) return KeyCode::SHIFT_R;
    if (*this == ModifierFlag::CONTROL_L) return KeyCode::CONTROL_L;
    if (*this == ModifierFlag::CONTROL_R) return KeyCode::CONTROL_R;
    if (*this == ModifierFlag::OPTION_L) return KeyCode::OPTION_L;
    if (*this == ModifierFlag::OPTION_R) return KeyCode::OPTION_R;
    if (*this == ModifierFlag::COMMAND_L) return KeyCode::COMMAND_L;
    if (*this == ModifierFlag::COMMAND_R) return KeyCode::COMMAND_R;
    if (*this == ModifierFlag::FN) return KeyCode::FN;
    if (*this == ModifierFlag::EXTRA1) return KeyCode::VK_MODIFIER_EXTRA1;
    if (*this == ModifierFlag::EXTRA2) return KeyCode::VK_MODIFIER_EXTRA2;
    if (*this == ModifierFlag::EXTRA3) return KeyCode::VK_MODIFIER_EXTRA3;
    if (*this == ModifierFlag::EXTRA4) return KeyCode::VK_MODIFIER_EXTRA4;
    if (*this == ModifierFlag::EXTRA5) return KeyCode::VK_MODIFIER_EXTRA5;

    return KeyCode::VK_NONE;
  }

  ModifierFlag
  KeyCode::getModifierFlag(void) const
  {
    if (*this == KeyCode::CAPSLOCK) return ModifierFlag::CAPSLOCK;
    if (*this == KeyCode::SHIFT_L) return ModifierFlag::SHIFT_L;
    if (*this == KeyCode::SHIFT_R) return ModifierFlag::SHIFT_R;
    if (*this == KeyCode::CONTROL_L) return ModifierFlag::CONTROL_L;
    if (*this == KeyCode::CONTROL_R) return ModifierFlag::CONTROL_R;
    if (*this == KeyCode::OPTION_L) return ModifierFlag::OPTION_L;
    if (*this == KeyCode::OPTION_R) return ModifierFlag::OPTION_R;
    if (*this == KeyCode::COMMAND_L) return ModifierFlag::COMMAND_L;
    if (*this == KeyCode::COMMAND_R) return ModifierFlag::COMMAND_R;
    if (*this == KeyCode::FN) return ModifierFlag::FN;
    if (*this == KeyCode::VK_MODIFIER_EXTRA1) return ModifierFlag::EXTRA1;
    if (*this == KeyCode::VK_MODIFIER_EXTRA2) return ModifierFlag::EXTRA2;
    if (*this == KeyCode::VK_MODIFIER_EXTRA3) return ModifierFlag::EXTRA3;
    if (*this == KeyCode::VK_MODIFIER_EXTRA4) return ModifierFlag::EXTRA4;
    if (*this == KeyCode::VK_MODIFIER_EXTRA5) return ModifierFlag::EXTRA5;

    return ModifierFlag::NONE;
  }

  bool
  ConsumerKeyCode::isRepeatable(void) const
  {
    if (*this == ConsumerKeyCode::BRIGHTNESS_DOWN)    { return true; }
    if (*this == ConsumerKeyCode::BRIGHTNESS_UP)      { return true; }
    if (*this == ConsumerKeyCode::KEYBOARDLIGHT_LOW)  { return true; }
    if (*this == ConsumerKeyCode::KEYBOARDLIGHT_HIGH) { return true; }
    if (*this == ConsumerKeyCode::MUSIC_PREV)         { return true; }
    if (*this == ConsumerKeyCode::MUSIC_NEXT)         { return true; }
    if (*this == ConsumerKeyCode::VOLUME_DOWN)        { return true; }
    if (*this == ConsumerKeyCode::VOLUME_UP)          { return true; }

    return false;
  }
}
