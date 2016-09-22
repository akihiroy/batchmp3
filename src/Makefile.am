INCLUDES = -I../lame-3.99.5/include

bindir = ../
bin_PROGRAMS = batchmp3

batchmp3_SOURCES = \
	Encoder.cpp \
	filesystem.cpp \
	main.cpp \
	util.cpp \
	WavReader.cpp

batchmp3_LDADD = ../lame-3.99.5/libmp3lame/libmp3lame.la
