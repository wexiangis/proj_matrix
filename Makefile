
CROSS_COMPILE=

target:
	$(CROSS_COMPILE)gcc -O3 -o out main.c view.c bmp.c gbk2312.c _3d_matrix.c -lm

clean:
	@rm -rf m 
