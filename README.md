# statusblocks - Clickable status bar for dwm

Statusblocks is very heavily based on [ashish-yadav11/dwmblocks](https://github.com/ashish-yadav11/dwmblocks), the reason why I decided to make this rewrite is because dwmblocks ran slow and with a lot of bugs on my computer. Statusblocks has yet to give me any troubles.

## Setup

- Update config.h with your own config
- Compile using `make`
- Install to /usr/local/bin using `sudo make install`

## Block scripts

When a block is clicked, its corresponding command is ran but this time the first argument will be the button used to click it. If theres no first argument the block must return the text to be rendered.

To update a block, you must send SIGNAL + RTMIN + 10 to statusblocks i.e:

```bash
# Block blocks[] = {
#	/* CMD              INTERVAL    SIGNAL */
#	{ "sb-prices",		120,		1},
#	{ "sb-forecast",    3600,		2},
#	{ "sb-datetime",    0.25,		4},
#	{ "sb-mem",         4,			6},
#	{ "sb-cpu",         4,			7}
# };

# We want to update blocks[1] or sb-forecast,
# SIGNAL is 2
# RTMIN is usually 34
# 2 + 10 + 34 = 46

kill -46 $(pidof statusblocks)
# block 1 will be updated in the next refresh
```

## dwm patch

This program works exclusively with the [statuscmd](https://dwm.suckless.org/patches/statuscmd/) patch. Though the program will still work without it.

## LICENSE

This program uses the [GNU GPLv3 License](https://www.gnu.org/licenses/gpl-3.0.en.html)