CFLAGS=-g -std=c++11

all: plotgad

example: src/line_chart_example.cc
	g++ $(CFLAGS) -I include $^ -o $@

plotgad: src/gad.cc
	g++ $(CFLAGS) -I include $^ -o $@
