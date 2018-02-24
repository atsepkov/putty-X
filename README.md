My modifications to Alex's putty-X are primarily in various key related areas (and hard-coded); an incomplete list of changes follows:

+ F10 modifier keys are supported with hard-coded keycodes ^[[21~ for F10 (some sort of rxvt? standard) ^[[34~ for Shift+F10 (also "standard"), ^[[44~ for Ctrl+F10 (definitely not standard), and ^[[54~ for Ctrl+Shift+F10 (not this either). The Alt variants have an extra ^[. The reason for all this disproportionate attention to F10 is because I typically bind the Caps Lock key to F10 and then use it to trigger awesome integrated scripts for manipulating tmux and vim.
+ The right Alt key code for replicating AltGr has been gutted with surgical strikes so that it should function similarly to the left Alt. Because I am an ignorant American.
+ Backspace's behavior selection between ^? and ^H is done with the Ctrl key, instead of Shift. Whether backspace defaults to ^? or not is configurable in the options.

Those are the only differences between this branch and atsepkov's. There is one change I made originally which addresses the tmux scrolling-up issue, which has been pulled in.

I include win32 builds corresponding to their commit hashes.

I also include a folder which is not related for building putty which contains 
font files for use on windows (tested on Windows 7) with putty to get 
chevron-style delimiter characters for powerline. If you don't know what that 
is, it's not very important. 
