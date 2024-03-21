# WTF?
Non-blocking Alt-Shift input source switcher for Gnome/KDE.

# SUMMARY

Workaround for bug: https://www.linux.org.ru/forum/general/14022907

In Ubuntu (since >= 17.10) come in strange bug with Alt+Shift hotkey for change keyboard layout:

1. Press ALt

2. Press Shift

3. Release ONLY Alt (expected - continue text typing using uppercase)

4. Try to type text - but nothing happens, input blocked and window unfocused. WTF?

Yes, i tryed using setxkbmap:
```
setxkbmap -model pc104 -layout ru,us -option grp:lalt_toggle -verbose
```

And it not working:

1. Press Alt

2. Press Shift

3. Release ONLY Alt (expected - continue text typing using uppercase)

4. Try to type text. Typed text in lowercase, ignoring Shift. 

:( 

But I remembered that I was a programmer and using Linux... i wrote own daemon for switching keyboard layout using my favorite ALt+Shift combo!

# INSTALL
1. Build and install:
```
sudo apt install g++ gcc cmake libxi-dev libx11-dev fakeroot
git clone https://github.com/Azq2/gnome-alt-shift-kbd-layout-switcher
cd gnome
# cd kde
fakeroot debian/rules binary
dpkg -i ../gnome-alt-shift-kbd-layout-switcher_1.0_amd64.deb
# dpkg -i ../kde-alt-shift-kbd-layout-switcher_1.0_amd64.deb
```
2. Delete any shortcuts for layout switching.
3. Relogin current gnome session. 
4. Try Alt+Shift and relax. 
