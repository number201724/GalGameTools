Tools included:
parsepck.exe - Dumps .pck archive
createpck.exe - Creates .pck archive (no compression as of right now
because it's too slow)
ssdump.exe - Dumps the text from a .ss script
ssinsert.exe - Inserts the text lazily but also should have 100% 
compatibility
decryptdat.exe - Decrypts Gameexe.dat
encryptdat.exe - Encrypts Gameexe.dat
SiglusEngine-wordwrap.exe - Hacked engine that supports wordwrapping

.bat files have been included to make this process easy:
unpackscene.bat - Dumps Scene.pck (must be in the same folder as
unpackscene.bat and parsepck.exe) to the Scene folder then dumps all
of the scripts using ssdump.exe
packscene.bat - Inserts all of the scripts then repacks Scene.pck
decryptdat.bat - Decrypts Gameexe.dat
encryptdat.bat - Encrypts Gameexe.dat

Gameexe.dat contains things like the window title and character names.
The script inserter supports comments using "//". A comment can be on
its own line or it can be appended at the end of a line.
The bytecode disassembler and assembler were not included because of 
some problems with it. It's not really important anyway due to the way
the .ss files are structured and the way the engine handles overflowed
text.
Wordwrapping is built into the EXE. It's fairly simple code and it
shouldn't cause any problems that I can think of. Be warned that the
engine doesn't seem to support variable width text, so stick with one
of the fixed width fonts like MS Gothic (or ‚l‚rƒSƒVƒbƒN for some people)
and everything should be fine.
The engine uses unicode for text, so I use unicode in the scripts. Do
not change this or you will fuck it up. All scripts on insertion need
to be unicode .txt files. Also because it uses unicode in the engine,
it supports languages that usually require more work (like Russian)
without any additional hacking.

If a serious translator comes along and wants to start up a project,
I can be found on IRC: Nagato on irc.rizon.net (can be found in #Ammy,
almost always idling on Rizon so /msg would also be fine).









Welcome to my botnet.