# SUMMARY

Workaround for KDE/GNOME with broken `Alt+Shift` for layout switching.

Use this daemon if you see one of these problems:
1. `Alt+Shift` is an impossible combination for switching llayout.
2. `Shift` is ignored in some cases.
    - Press `Alt`
    - Press `Shift`
    - Release ONLY `Alt`
    - Try to type text. Typed text is lowercase, `Shift` is ignored.
3. Text is not typed in some cases.
    - Press `Alt`
    - Press `Shift`
    - Release ONLY `Alt`
    - Try to type text. Nothing happens, input blocked and window lost focus.

# KDE
1. Build and install:
    ```sh
    sudo apt install g++ gcc cmake libxi-dev libx11-dev fakeroot
    git clone https://github.com/Azq2/alt-shift-kbd-layout-switcher
    cd alt-shift-kbd-layout-switcher/kde/
    fakeroot debian/rules binary
    dpkg -i ../kde-alt-shift-kbd-layout-switcher_1.0_amd64.deb
    ```
2. Delete any shortcuts for layout switching.
3. Relogin current KDE session. 
4. Try `Alt+Shift`, now it works.

# GNOME
1. Build and install:
    ```sh
    sudo apt install g++ gcc cmake libxi-dev libx11-dev fakeroot
    git clone https://github.com/Azq2/alt-shift-kbd-layout-switcher
    cd alt-shift-kbd-layout-switcher/gnome/
    fakeroot debian/rules binary
    dpkg -i ../gnome-alt-shift-kbd-layout-switcher_1.0_amd64.deb
    ```
2. Delete any shortcuts for layout switching.
3. Relogin GNOME session. 
4. Try `Alt+Shift`, now it works.
