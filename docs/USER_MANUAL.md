# Neon Oubliette User Manual

This manual tracks the current player-facing controls and feature behavior. Any feature add that changes controls, HUD behavior, gameplay verbs, or player-visible systems must update this file in the same change.

## Controls

### Movement And Targeting

| Input | Action |
| --- | --- |
| W/A/S/D | Move on foot. Drive a non-transit vehicle when boarded as the driver. |
| Arrow keys | Move the scan focus target while a scan tool is equipped or a scan panel is active. |
| Space | Use the currently equipped item or scan tool. |
| Escape | Close the active scan panel and clear any pending market trade preview. |

Movement updates the player's facing direction. Arrow keys steer scan focus when a carried scan tool is equipped.

### Equipment And Hotkeys

| Input | Action |
| --- | --- |
| 0 | Equip nothing and clear scan focus. |
| 1 | Equip/use food. |
| 2 | Equip/use water. |
| 3 | Equip/use medical. |
| 4 | Equip surface scan. |
| 5 | Equip biology audit. |
| 6 | Equip cognitive profile. |
| 7 | Equip financial forensics. |
| 8 | Equip structural analysis. |
| Ctrl + 1-9 | Bind the current equipped slot to that number. |

Food, water, and medical hotkeys quick-use a matching discrete inventory item first. If no matching discrete item is carried, they fall back to the survival inventory counter.

Scan tool hotkeys equip the tool but do not fire it. Press Space after equipping a scan tool to open the matching scan panel.

### Inventory

| Input | Action |
| --- | --- |
| E | Interact with nearby world objects, including pickup targets. |
| [ | Select the previous discrete inventory slot. |
| ] | Select the next discrete inventory slot. |
| U | Use the selected discrete inventory item. Scan tools equip; consumables are consumed. |
| G | Drop the selected discrete inventory item. |

The inventory can carry consumables and scan tools. Discrete inventory slots preserve item flags such as legal status, value, uniqueness, and market category when those details are available.

If the player is near a boardable vehicle, E enters it after item pickup checks. Press E again to exit. Transit vehicles can only be boarded or exited while stopped and are automated while riding.

### Scan Tools

| Input | Action |
| --- | --- |
| I | Equip surface scan. |
| Shift + I | Equip biology audit. |
| C | Equip cognitive profile. |
| F | Equip financial forensics. |
| T | Equip structural analysis. |
| Space | Use the equipped scan tool. |

Scan tools require the matching carried inventory tool. If the player does not carry the tool, use is denied and no scan panel opens.

Scan targets prefer entities in the player's facing direction, fall back to the nearest eligible target, and can be manually cycled with the arrow keys while a scan tool is equipped or a scan panel is active. The selected target remains locked while it is alive and within the active tool range.

### Layers, Markets, And Debug Inputs

| Input | Action |
| --- | --- |
| O | Cycle the layer overlay mode. |
| B | Preview buying the next supported market item. |
| N | Preview selling the next carried market item. |
| Y | Confirm the pending market buy or sell transaction. |
| = or keypad + | Zoom the camera in. |
| - or keypad - | Zoom the camera out. |
| Shift + F | Toggle the current flood event state. |

Market trade previews show the pending item, price, wallet/stock context, and denial feedback when a transaction is unavailable.

## Player-Facing Features

### Survival Inventory

The player tracks food, water, and medical supplies through HUD counters. Picking up those item types stores them as discrete inventory entries and keeps the survival counters compatible with quick-use controls.

Food and water restore basic survival needs. Medical items prioritize active injuries before restoring general health.

### Discrete Inventory

The player has a bounded inventory of individual item entries. Items can be selected, inspected through HUD text, used, dropped, bought from markets, or sold to markets when eligible.

Consumables are removed when used. Scan tools remain in the inventory and equip the matching tool slot when used.

### Equipped Tools

The current equipped slot can be empty, a consumable, or a scan tool. Numeric hotkeys select configured equipment, and the bindings for keys 1 through 9 can be changed at runtime with Ctrl plus the desired number.

Scan tools expose different inspection panels and ranges:

| Tool | Default Hotkey | Role |
| --- | --- | --- |
| Surface Scan | 4 | General entity and item inspection. |
| Biology Audit | 5 | Biological state, health, injury, and pathogen context. |
| Cognitive Profile | 6 | Cognitive and memory-related context. |
| Financial Forensics | 7 | Economic, market, value, and legality context. |
| Structural Analysis | 8 | Building, pressure, power, and structural context. |

### Visual Feedback And Alerts

Common actions emit immediate feedback cues for pickup, consume, scan, warning, and denied actions. The HUD also renders recent simulation alerts without blocking scan panels, survival counters, inventory text, or layer overlays.

### Layer Overlays

Layer overlays let the player inspect simulation layers from the world view. The overlay mode cycles with `O`.

### District Markets

Markets expose stock, price, and transaction behavior. Player buy/sell interactions use nearby market entities, available wallet balance, market stock, inventory space, and eligible carried items.

## Manual Maintenance

When adding a feature, update this manual in the same commit if the feature changes any player-facing behavior, including:

- Controls or hotkeys.
- HUD text, panels, overlays, or feedback.
- Inventory, equipment, scan, trade, survival, or interaction behavior.
- Debug controls that developers use while playing the build.

The repository includes a pre-commit documentation hook. See `docs/MANUAL_UPDATE_HOOK.md` for installation and override details.
