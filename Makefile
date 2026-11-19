fractal3d: cpu.c
	gcc -O3 cpu.c -o fractal3d -lraylib -lm

clean:
	rm -f fractal3d

.PHONY: clean
