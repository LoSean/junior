all:testrgb prob1 testhis prob2 testnoise prob3

testrgb:rgb.cpp
	g++ -D PRINT rgb.cpp -o $@

prob1:rgb.cpp
	g++ rgb.cpp -o $@

testhis:histogram.cpp
	g++ -D PRINT histogram.cpp -o $@

prob2:histogram.cpp
	g++ histogram.cpp -o $@

testnoise:noise.cpp
	g++ -D PRINT noise.cpp -o $@

prob3:noise.cpp
	g++ noise.cpp -o $@

clean:
	rm test
