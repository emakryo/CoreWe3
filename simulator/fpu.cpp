#include <cmath>
#include "fpu.h"

typedef union{
	uint32_t r;
	float f;
}FU;

uint32_t FPU::fadd(uint32_t x, uint32_t y){
	FU xfu, yfu, zfu;
	xfu.r = x; yfu.r = y;
	zfu.f = xfu.f + yfu.f;
	return zfu.r;
}

uint32_t FPU::fsub(uint32_t x, uint32_t y){
	FU xfu, yfu, zfu;
	xfu.r = x; yfu.r = y;
	zfu.f = xfu.f - yfu.f;
	return zfu.r;
}

uint32_t FPU::fmul(uint32_t x, uint32_t y){
	FU xfu, yfu, zfu;
	xfu.r = x; yfu.r = y;
	zfu.f = xfu.f * yfu.f;
	return zfu.r;
}

uint32_t FPU::fdiv(uint32_t x, uint32_t y){
	FU xfu, yfu, zfu;
	xfu.r = x; yfu.r = y;
	zfu.f = xfu.f / yfu.f;
	return zfu.r;
}

int FPU::fcmp(uint32_t x, uint32_t y){
	FU xfu, yfu, zfu;
	xfu.r = x; yfu.r = y;
	zfu.f = xfu.f - yfu.f;
	if (zfu.f < 0) return -1;
	else if (zfu.f > 0) return 1;
	else return 0;
}

uint32_t FPU::fsqrt(uint32_t x){
	FU xfu, yfu;
	yfu.f = std::sqrt(xfu.f);
	return yfu.r;
}

uint32_t FPU::fabs(uint32_t x){
	FU xfu, yfu;
	yfu.f = std::abs(xfu.f);
	return yfu.r;
}

