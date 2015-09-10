arcv3pack.exe:arcv3pack.obj writestream.obj
	link /subsystem:console /debug /INCREMENTAL:NO arcv3pack.obj writestream.obj kernel32.lib user32.lib

arcv3pack.obj:arcv3pack.c
	cl /c /O2 arcv3pack.c
writestream.obj:writestream.c
	cl /c /O2 writestream.c
	
