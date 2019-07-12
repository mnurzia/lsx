- [ ] Implement -d
- [ ] Implement -k
- [ ] Implement -q
- [ ] Add verbose file size names ('bytes', etc)
- [ ] Make -C the default listing mode
- [ ] Add tree option with configurable level
- [ ] Change/Add certain long options to a catch-all long-only option:
    - --sort-size -> --sort=size
    - --listing-long -> --listing=long
    - --size-si -> --size=si
- [ ] Add regex/glob option(s) like fnmatch. .lsignore?
- [ ] Add time style option a la exa (--time-style)
- [ ] Possible switch to a better argument parser. We'll see.
- [ ] Add more sort fields (inode, extension)
- [ ] Add facility to import old LS_COLORS/LSCOLORS environment variable and
      export as new JSON format
- [ ] Add facility to darken every other line when in 24bit mode like a notebook
- [ ] Add facility to create a custom output format sorta like $PROMPT or strftime
- [ ] Add facility to test/specify whether 24bit mode is required possibly using terminfo data (not a library though, maybe just escape codes)