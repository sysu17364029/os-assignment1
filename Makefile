help:
	@echo "Please input like 'make dph'!"
dph : 
	@gcc dph.c -lpthread -o dph
prod :
	@gcc prod.c -lm -lpthread -o prod
cons :
	@gcc cons.c -lm -lpthread -o cons
unlink:
	@gcc unlink.c -lpthread -o unlink
mycall :
	@gcc mycall.c -o mycall
all : dph prod cons unlink mycall

clean :
	@rm dph prod cons unlink mycall

