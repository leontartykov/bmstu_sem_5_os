APPDIR := ./src/
OUTDIR := ./out/
HEADDIR := ./inc/

CC := gcc
FLAGS := -std=c99 -Wall -Werror -pedantic -c

main_os.out: $(OUTDIR)main.o $(OUTDIR)read.o $(OUTDIR)io.o $(OUTDIR)interpolation.o
	$(CC) -o $@ $^
	
$(OUTDIR)main.o: $(APPDIR)main.c $(HEADDIR)defines.h $(HEADDIR)read.h $(HEADDIR)struct.h $(HEADDIR)io.h $(HEADDIR)interpolation.h
	$(CC) $(FLAGS) $< -o $@
	
$(OUTDIR)read.o: $(APPDIR)read.c $(HEADDIR)defines.h $(HEADDIR)read.h $(HEADDIR)struct.h
	$(CC) $(FLAGS) $< -o $@

$(OUTDIR)io.o: $(APPDIR)io.c $(HEADDIR)io.h $(HEADDIR)struct.h
	$(CC) $(FLAGS) $< -o $@
	
$(OUTDIR)interpolation.o: $(APPDIR)interpolation.c $(HEADDIR)interpolation.h $(HEADDIR)struct.h $(HEADDIR)defines.h
	$(CC) $(FLAGS) $< -o $@
	
.PHONY: clean
clean:
	rm -rf $(OUTDIR)*.o *.out
