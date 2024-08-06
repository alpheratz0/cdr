OBJS = src/cdr_main.o \
       src/cdr_match.o \
       src/cdr_ref.o \
       src/cdr_render.o \
       src/intset.o \
       src/strutil.o \
       data/cdr_data.o
CFLAGS += -Wall -Isrc/
LDLIBS += -lreadline

cdr: $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(OBJS) $(LDLIBS)

src/cdr_main.o: src/cdr_main.c src/cdr_config.h src/cdr_data.h src/cdr_match.h src/cdr_ref.h src/cdr_render.h src/strutil.h

src/cdr_match.o: src/cdr_match.h src/cdr_match.c src/cdr_config.h src/cdr_data.h src/cdr_ref.h

src/cdr_ref.o: src/cdr_ref.h src/cdr_ref.c src/intset.h src/cdr_data.h

src/cdr_render.o: src/cdr_render.h src/cdr_render.c src/cdr_config.h src/cdr_data.h src/cdr_match.h src/cdr_ref.h

src/insetset.o: src/intset.h src/insetset.c

src/strutil.o: src/strutil.h src/strutil.c

data/cdr_data.o: src/cdr_data.h data/cdr_data.c

data/cdr_data.c: data/cdr.tsv data/generate.awk src/cdr_data.h
	awk -f data/generate.awk $< > $@

.PHONY: clean
clean:
	rm -rf $(OBJS) cdr
