# Differences between the Empire at War and Forces of Corruption engines

The Forces of Corruption ("FoC") engine supports additional features on top of the base game ("EaW"), such as the corruption mechanic, which is expressed as additional game object behaviors. However, there are some cases where the two engines have conflicting implementations for elements used in both games. All of these are listed in this document.

## Additional features

### User Interface

#### Main Menu

Unlike EaW, FoC has a "play demo" on the main menu, at the same position as the "single player" button. On demo builds, the single-player button is hidden. On non-demo builds, the demo button is hidden. OpenEaW always tries to hide the demo button (if it exists).

## Conflicting features

### User Interface

#### Combobox
A combobox consists of a text field indicating the selected value and a dropdown button to open a listbox.
Read from `GUIDialogs.xml`, EaW styles the combobox with `<Small_Frame_*>`, whereas FoC uses `<Combo_Box_Left_Cap>` and `<Combo_Box_Text_Box>` to style the left edge and interior of the combobox widget, respectively.

OpenEAW resolves this conflict by checking if `<Combo_Box_Left_Cap>` or `<Combo_Box_Text_Box>` is present (it can be set to "None" to have no texture). If so, it uses FoC behavior, otherwise it falls back to EaW behavior.
