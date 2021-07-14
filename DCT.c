#include<stdio.h>
#include<math.h>
#include <stdlib.h>
#include <time.h>
# define org_row  512 
# define org_col  512
# define sizebyte 1
# define in_file "pic.img"
# define scale 10
# define pi 3.1415926535897
# define stripe_gap 4 //세로 줄무늬 영상과, 가로 줄무늬 영상의 주기


//====option================
# define out_file_D "jpeg_상하반전_양자화_DCT_8x8.img"
# define out_file "jpeg_상하반전_양자화_복원_8x8.img"
# define N 16//DCT 주파수 개수 (2,4,8,16.32.64.128.256)
# define image_chioce 1 //(1 = lena 영상 입력    2 = 가로 줄무늬 영상 입력    3 = 세로 줄무늬 영상 입력)
# define sample  0 //양자화 하기위한 나눗셈 (1, 10, 100)
# define LQ_num 1 //복호화 계수(1,2,3 ...)
# define num 0//(1 = 일부 계수만을 이용한 복호화 사용 ,  2 = 양자화 사용, 3 = jpeg 양자화 사용, 4 = jpeg 계수 상하반전 양자화 사용, 그 외 = DCT 만 수행)
# define use_psnr 0//(0 : psnr 측정 x   , 1 : psnr 측정 o )  

//파일 읽기
unsigned char* readFile(char* s, int size_row, int size_col) {
	//FILE* input_img = fopen(in_file, "rb");
	FILE* input_img;
	fopen_s(&input_img, s, "rb");
	unsigned char* org_img = NULL;
	org_img = (unsigned char*)malloc(sizeof(unsigned char)*(size_row*size_col));
	if (input_img == NULL) {
		puts("input 파일 오픈 실패 ");
		return NULL;
	}
	else {
		fseek(input_img, 0, SEEK_SET);
		fread((void*)org_img, sizebyte, size_row * size_col, input_img);
	}
	return org_img;
	fclose(input_img);
	free(org_img);
}

//unsigned char 형 파일 쓰기
unsigned char* WriteFile_U(unsigned char* out_img, char* s, int size_row, int size_col) {
	//FILE* output_img = fopen(out_file, "wb");
	FILE* output_img;
	fopen_s(&output_img, s, "wb");
	if (output_img == NULL) {
		puts("output 파일 오픈 실패");
		return NULL;
	}
	else {
		fseek(output_img, 0, SEEK_SET);
		fwrite((void*)out_img, sizebyte, size_row*size_col, output_img);
	}
	fclose(output_img);
	return out_img;

}
//DCT : 원본 영상에서 가로 DCT 를 한 결과를 double 형으로 출력 (8x8 블록의 forward DCT)
double* DCT(unsigned char* org_img) {
	int i, j; //신호 공간 좌표값
	int m, n; //block안의 좌표값;
	int u, v; //가로, 세로 주파수
	double* DCT_img = NULL;
	double* DST_img = NULL;
	double* DCT_out = NULL;
	float a_sum[N*N] = {0};
	float a_avg[N*N] = { 0 };
	float a = 7;
	float b = 0;
	DCT_img = (double*)malloc(sizeof(double)*(org_row*org_col));
	DST_img = (double*)malloc(sizeof(double)*(org_row*org_col));
	DCT_out = (double*)malloc(sizeof(double)*(org_row*org_col));
	for (i = 0; i < org_row; i = i + N) {
		for (j = 0; j < org_col; j = j + N) {
			for (v = 0; v < N; v++) {
				for (u = 0; u < N; u++) {
					double temp_c = 0.0;
					double temp_s = 0.0;
					double C_u = 1 / sqrt((double)N / 2.0);
					double C_v = 1 / sqrt((double)N / 2.0);
					if (u == 0)
						C_u = 1 / sqrt((double)N);
					if (v == 0)
						C_v = 1 / sqrt((double)N);
					for (m = 0; m < N; m++) {//f(n,m)
						for (n = 0; n < N; n++) {
							temp_c += cos(((2 * n + 1)*(double)u*pi) / (2.0*N)) * cos(((2 * m + 1)*(double)v*pi) / (2.0*N)) * (((*(org_img + (j + m)*org_row + (i + n)) - *(org_img + (j + m)*org_row + (i + n - 1))) + (*(org_img + (j + m)*org_row + (i + n)) - *(org_img + (j + m - 1)*org_row + (i + n))))/2.0);
							//temp_s += sin(((2 * n + 1)*(double)u*pi) / (2.0*N)) * sin(((2 * m + 1)*(double)v*pi) / (2.0*N)) * (*(org_img + (j + m)*org_row + (i + n)));
						}
					}
					b = sqrt(u*u + v*v);
					*(DCT_img + (j + v)*org_row + i + u) = temp_c * (C_u*C_v);//****
					if (u == 0 && v == 0) {
						*(DCT_img + (j + v)*org_row + i + u) += 1700;
					}
					//*(DST_img + (j + v)*org_row + i + u) = temp_s * (C_u*C_v);//****
					//*(DCT_img + (j + v)*org_row + i + u) = /*(float)((u + v)*(u + v)) / (float)((u + v)*(u + v) + a * a)**/(*(DCT_img + (j + v)*org_row + i + u));
					//*(DST_img + (j + v)*org_row + i + u) = /*(float)((b)*a) / (float)((b)*(b) + a * a)**/(*(DST_img + (j + v)*org_row + i + u));
					*(DCT_out + (j + v)*org_row + i + u) =   *(DCT_img + (j + v)*org_row + i + u);
					a_sum[v*N + u] += *(DCT_out + (j + v)*org_row + i + u);
				}
			}
			
		}
	}
	for (i = 0; i < N*N; i++) {
		a_avg[i] = a_sum[i] / (org_col*org_row / (N*N));
		printf("DCT %d 번째 평균: %f\n", i, a_avg[i]);
	}
	free(DCT_img);
	free(DST_img);
	return DCT_out;
	
}

//IDCT : double 형의 DCT 영상을 다시 원본영상에 가깝게 unsinged char형으로 복원한다.
unsigned char* IDCT(double* DCT_img) {
	int i, j;
	int m, n; //block안의 좌표값;
	int u, v; //가로, 세로 주파수
	double* IDCT_double = NULL;
	unsigned char* IDCT_img = NULL;

	IDCT_double = (double*)malloc(sizeof(double)*(org_row*org_col));
	IDCT_img = (unsigned char*)malloc(sizeof(unsigned char)*(org_row*org_col));

	//unsigned char 로 형변환 하기 전 0.5 더해주기.
	for (i = 0; i < org_row; i = i + N) {
		for (j = 0; j < org_col; j = j + N) {
			for (n = 0; n < N; n++) {
				for (m = 0; m < N; m++) {
					double temp = 0.0;
					for (u = 0; u < N; u++) {//F(u,v)
						for (v = 0; v < N; v++) {
							double C_u = 1 / sqrt((double)N / 2.0);
							double C_v = 1 / sqrt((double)N / 2.0);
							if (u == 0)
								C_u = 1 / sqrt((double)N);
							if (v == 0)
								C_v = 1 / sqrt((double)N);
							temp += (C_u*C_v)*cos(((2 * n + 1)*(double)u*pi) / (2.0*N)) * cos(((2 * m + 1)*(double)v*pi) / (2.0*N)) * (*(DCT_img + (j + v)*org_row + i + u)); //***
							
						}
					}
					*(IDCT_img + (j + m)*org_row + i + n) = (unsigned char)(temp + 0.5);//****
				}
			}
		}
	}
	//uchar 형으로 형변환해주기

	return IDCT_img;
	free(IDCT_double);
	free(IDCT_img);
}


unsigned char cut_off(unsigned char pix_val) {
	int data;
	if (pix_val > 255) {
		data = 255;
	}
	else if (pix_val < 0) {
		data = 0;
	}
	else {
		data = pix_val;
	}


	return data;
}

unsigned char* subtraction_image(unsigned char* buff_img, unsigned char* org_img) {
	int i, j;
	unsigned char* subtraction_img = NULL;
	subtraction_img = (unsigned char*)malloc(sizeof(unsigned char)*(org_row*org_col));

	for (i = 0; i < org_row; i++) {
		for (j = 0; j < org_col; j++) {
			*(subtraction_img + j * org_row + i) = cut_off(*(org_img + j * org_row + i) - *(buff_img + j * org_row + i));

		}
	}

	return subtraction_img;
}

unsigned char* sum_image(unsigned char* buff_img, unsigned char* org_img) {
	int i, j;
	unsigned char* sum_img = NULL;
	sum_img = (unsigned char*)malloc(sizeof(unsigned char)*(org_row*org_col));

	for (i = 0; i < org_row; i++) {
		for (j = 0; j < org_col; j++) {
			*(sum_img + j * org_row + i) = cut_off(*(org_img + j * org_row + i) + *(buff_img + j * org_row + i));

		}
	}

	return sum_img;
}



//미완성
unsigned char* add_detail(unsigned char* org_img) {
	int i, j;
	unsigned char* detail_img = NULL;
	unsigned char* HP_img = NULL;
	detail_img = (unsigned char*)malloc(sizeof(unsigned char)*(org_row*org_col));
	HP_img = (unsigned char*)malloc(sizeof(unsigned char)*(org_row*org_col));


	for (i = 0; i < org_row; i++) {
		for (j = 0; j < org_col; j++) {
			
		}
	}


	HP_img = sum_image(org_img, detail_img);
	free(detail_img);

	return HP_img;
}







//main 함수
int main(void) {
	int size_row = org_row;
	int size_col = org_col;
	unsigned char* buff_img = NULL;
	unsigned char* org_img = NULL;
	double* buff_img_D = NULL;
	clock_t before, after;
	double Time;
	org_img = (unsigned char*)malloc(sizeof(unsigned char)*(size_row*size_col));
	buff_img = (unsigned char*)malloc(sizeof(unsigned char)*(size_row*size_col));
	buff_img_D = (double*)malloc(sizeof(double)*(size_row*size_col));
	//org_img = readFile(in_file, org_row, org_col);

	buff_img = readFile((char*)in_file, org_row, org_col); //1. lena 영상 불러오기
	org_img = buff_img;
	buff_img_D = DCT(buff_img); //DCT

	//WriteFile_D(buff_img_D); //DCT 한 영상(일부 계수 복호화 or 양자화  추가)을 곧바로 출력
	buff_img = IDCT(buff_img_D); //IDCT
	buff_img = sum_image(buff_img, org_img);
	WriteFile_U(buff_img, (char*)out_file, org_row, org_col); //복원 영상 출력

	free(org_img);
	free(buff_img);
	free(buff_img_D);
	return 0;
}