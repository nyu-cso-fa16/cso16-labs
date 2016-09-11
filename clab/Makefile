BUILD  := build
STATIC := static

SOURCES := part1.c part2.c part3.c part4.c part5.c part6.c 

SOBJS :=	$(STATIC)/part1_harness.o \
		$(STATIC)/part2_harness.o \
		$(STATIC)/part3_harness.o \
		$(STATIC)/part4_harness.o \
		$(STATIC)/part5_harness.o \
		$(STATIC)/part6_harness.o

CC     := gcc
CFLAGS := -std=c99 

OBJS :=	$(BUILD)/part1 \
	$(BUILD)/part2 \
	$(BUILD)/part3 \
	$(BUILD)/part4 \
	$(BUILD)/part5 \
	$(BUILD)/part6


all: $(OBJS)
	@:

real_all: $(OBJS)

$(BUILD)/part%: part%.c $(SOBJS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -g -c part$*.c -o $(BUILD)/part$*.o
	$(CC) $(CFLAGS) $(STATIC)/part$*_harness.o $(BUILD)/part$*.o -lm -o $(BUILD)/part$*

clean-logs: always
	rm -f *.out

clean: always clean-logs
	rm -rf $(BUILD)

grade:
	@echo $(MAKE) clean
	@$(MAKE) -s --no-print-directory clean
	@./grade-lab

TARBALL  := lab1.tar.gz

# Most of the next two rules ripped from MIT JOS.
handin-from-git:
	@if ! git diff-files --quiet || ! git diff-index --quiet --cached HEAD; then \
		git status; \
		echo; \
		echo "You have uncomitted changes.  Please commit or stash them."; \
		false; \
	fi
	@if test -n "`git ls-files -o --exclude-standard`"; then \
		git status; \
		read -p "Untracked files will not be handed in.  Continue? [y/N] " r; \
		test "$$r" = y; \
	fi
	tar -cf - `git ls-files` | gzip > $(TARBALL)

handin: $(SOURCES)
	tar czvf $(TARBALL) $(SOURCES)

.PHONY: all always
