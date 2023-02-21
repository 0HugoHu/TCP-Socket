TARGETS=client server

all: $(TARGETS)
clean:
	rm -f $(TARGETS)

player: player.cpp
	g++ -g -o $@ $<

ringmaster: ringmaster.cpp
	g++ -g -o $@ $<

