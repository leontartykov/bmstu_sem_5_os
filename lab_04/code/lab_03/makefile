APPDIR := ./src/
LIBDIR := ./inc/
OUTDIR := ./out/

CC := gcc
CFLAGS := -std=c99 -Wall -Werror -pedantic -c

all: main.out

main.out: $(OUTDIR)main.o $(OUTDIR)table.o $(OUTDIR)spline.o -lm
		$(CC) -o $@ $^

$(OUTDIR)main.o: $(APPDIR)main.c $(LIBDIR)defines.h $(LIBDIR)struct.h $(LIBDIR)table.h
		$(CC) $(CFLAGS) $< -o $@

$(OUTDIR)table.o: $(APPDIR)table.c $(LIBDIR)table.h $(LIBDIR)defines.h $(LIBDIR)struct.h
		$(CC) $(CFLAGS) $< -o $@
		
$(OUTDIR)spline.o: $(APPDIR)spline.c $(LIBDIR)spline.h $(LIBDIR)defines.h $(LIBDIR)struct.h $(LIBDIR)table.h
		$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
		rm -rf $(OUTDIR)*.o *.out
