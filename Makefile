INCLDIR = ./include
CC = gcc
CFLAGS = -O2
CFLAGS += -I$(INCLDIR)

OBJDIR = obj

_DEPS = mop.h threadmanager.h simplelogger.h
DEPS = $(patsubst %, $(INCLDIR)/%,$(_DEPS))

_OBJS = main.o mop.o threadmanager.o simplelogger.o
OBJS = $(patsubst %,$(OBJDIR)/%,$(_OBJS))

$(OBJDIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

app: $(OBJS)
	gcc -o $@ $^ $(CFLAGS)

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o ~ core $(INCLDIR)/~