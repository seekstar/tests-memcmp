#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <immintrin.h>
#include <xxhash.h>

#define SIZE (1ULL << 30)
#define PAGE_SIZE 4096
#define BLOCK_SIZE 4096

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

// Assume that block_a and block_b is 8 byte aligned
int64_t cmp64(const uint64_t *block_a, const uint64_t *block_b) {
	const uint64_t *b_end = block_b + BLOCK_SIZE / sizeof(block_b[0]);
	for (; block_b < b_end; ++block_b, ++block_a) {
		if (*block_a != *block_b) {
			return *block_a - *block_b;
		}
	}
	return 0;
}

static inline int64_t cmp64_wrapper(const char *a, const char *b) {
	return cmp64((const uint64_t *)a, (const uint64_t *)b);
}

// Assume that block_a and block_b is 8 byte aligned
uint64_t cmp64_2(const uint64_t *block_a, const uint64_t *block_b) {
	const uint64_t *b_end = block_b + BLOCK_SIZE / sizeof(block_b[0]);
	for (; block_b < b_end; block_b += 2, block_a += 2) {
		if (block_a[0] != block_b[0])
			return block_a[0] - block_b[0];
		if (block_a[1] != block_b[1])
			return block_a[1] - block_b[1];
	}
	return 0;
}

static inline int64_t cmp64_2_wrapper(const char *a, const char *b) {
	return cmp64_2((const uint64_t *)a, (const uint64_t *)b);
}

// Assume that block_a and block_b is 8 byte aligned
uint64_t cmp64_4(const uint64_t *block_a, const uint64_t *block_b) {
	const uint64_t *b_end = block_b + BLOCK_SIZE / sizeof(block_b[0]);
	for (; block_b < b_end; block_b += 4, block_a += 4) {
		if (block_a[0] != block_b[0])
			return block_a[0] - block_b[0];
		if (block_a[1] != block_b[1])
			return block_a[1] - block_b[1];
		if (block_a[2] != block_b[2])
			return block_a[2] - block_b[2];
		if (block_a[3] != block_b[3])
			return block_a[3] - block_b[3];
	}
	return 0;
}

static inline int64_t cmp64_4_wrapper(const char *a, const char *b) {
	return cmp64_4((const uint64_t *)a, (const uint64_t *)b);
}

// Assume that block_a and block_b is 8 byte aligned
uint64_t cmp64_8(const uint64_t *block_a, const uint64_t *block_b) {
	const uint64_t *b_end = block_b + BLOCK_SIZE / sizeof(block_b[0]);
	for (; block_b < b_end; block_b += 8, block_a += 8) {
		if (block_a[0] != block_b[0])
			return block_a[0] - block_b[0];
		if (block_a[1] != block_b[1])
			return block_a[1] - block_b[1];
		if (block_a[2] != block_b[2])
			return block_a[2] - block_b[2];
		if (block_a[3] != block_b[3])
			return block_a[3] - block_b[3];
		if (block_a[4] != block_b[4])
			return block_a[4] - block_b[4];
		if (block_a[5] != block_b[5])
			return block_a[5] - block_b[5];
		if (block_a[6] != block_b[6])
			return block_a[6] - block_b[6];
		if (block_a[7] != block_b[7])
			return block_a[7] - block_b[7];
	}
	return 0;
}

static inline int64_t cmp64_8_wrapper(const char *a, const char *b) {
	return cmp64_8((const uint64_t *)a, (const uint64_t *)b);
}

// ccnz: https://www.felixcloutier.com/documents/gcc-asm.html
static inline int64_t memcmp_repe_cmpsb(const void *block_a, const void *block_b, size_t len) {
	int64_t diff;
	asm volatile("repe; cmpsb"
		: "=@ccnz" (diff), "+D" (block_a), "+S" (block_b), "+c" (len));
	return diff;
}

static inline int64_t memcmp_repe_cmpsb_wrapper(const char *a, const char *b) {
	return memcmp_repe_cmpsb(a, b, PAGE_SIZE);
}

// Assume that block_a and block_b is 8 byte aligned
static inline int64_t memcmp_repe_cmpsq(const void *block_a, const void *block_b, size_t len) {
	int64_t diff;
	asm volatile("repe; cmpsq"
		: "=@ccnz" (diff), "+D" (block_a), "+S" (block_b), "+c" (len));
	return diff;
}

static inline int64_t memcmp_repe_cmpsq_wrapper(const char *a, const char *b) {
	return memcmp_repe_cmpsq(a, b, PAGE_SIZE / 8);
}

// Assume that block_a and block_b is 8 byte aligned
uint64_t cmp64_8_or(const uint64_t *block_a, const uint64_t *block_b) {
	const uint64_t *b_end = block_b + BLOCK_SIZE / sizeof(block_b[0]);
	for (; block_b < b_end; block_b += 8, block_a += 8) {
		if (block_a[0] != block_b[0] || block_a[1] != block_b[1] || block_a[2] != block_b[2] || block_a[3] != block_b[3] || block_a[4] != block_b[4] || block_a[5] != block_b[5] || block_a[6] != block_b[6] || block_a[7] != block_b[7]) {
			if (block_a[0] != block_b[0])
				return block_a[0] - block_b[0];
			if (block_a[1] != block_b[1])
				return block_a[1] - block_b[1];
			if (block_a[2] != block_b[2])
				return block_a[2] - block_b[2];
			if (block_a[3] != block_b[3])
				return block_a[3] - block_b[3];
			if (block_a[4] != block_b[4])
				return block_a[4] - block_b[4];
			if (block_a[5] != block_b[5])
				return block_a[5] - block_b[5];
			if (block_a[6] != block_b[6])
				return block_a[6] - block_b[6];
			if (block_a[7] != block_b[7])
				return block_a[7] - block_b[7];
		}
	}
	return 0;
}

static inline int64_t cmp64_8_or_wrapper(const char *a, const char *b) {
	return cmp64_8_or((const uint64_t *)a, (const uint64_t *)b);
}

// Assume that block_a and block_b is 8 byte aligned
uint64_t cmp64_4_bitwise_or(const uint64_t *block_a, const uint64_t *block_b) {
	const uint64_t *b_end = block_b + BLOCK_SIZE / sizeof(block_b[0]);
	for (; block_b < b_end; block_b += 4, block_a += 4) {
		if ((block_a[0] - block_b[0]) | (block_a[1] - block_b[1]) | (block_a[2] - block_b[2]) | (block_a[3] - block_b[3])) {
			if (block_a[0] != block_b[0])
				return block_a[0] - block_b[0];
			if (block_a[1] != block_b[1])
				return block_a[1] - block_b[1];
			if (block_a[2] != block_b[2])
				return block_a[2] - block_b[2];
			if (block_a[3] != block_b[3])
				return block_a[3] - block_b[3];
		}
	}
	return 0;
}

static inline int64_t cmp64_4_bitwise_or_wrapper(const char *a, const char *b) {
	return cmp64_4_bitwise_or((const uint64_t *)a, (const uint64_t *)b);
}

// Assume that block_a and block_b is 8 byte aligned
uint64_t cmp64_8_bitwise_or(const uint64_t *block_a, const uint64_t *block_b) {
	const uint64_t *b_end = block_b + BLOCK_SIZE / sizeof(block_b[0]);
	for (; block_b < b_end; block_b += 8, block_a += 8) {
		if ((block_a[0] - block_b[0]) | (block_a[1] - block_b[1]) | (block_a[2] - block_b[2]) | (block_a[3] - block_b[3]) | (block_a[4] - block_b[4]) | (block_a[5] - block_b[5]) | (block_a[6] - block_b[6]) | (block_a[7] - block_b[7])) {
			if (block_a[0] != block_b[0])
				return block_a[0] - block_b[0];
			if (block_a[1] != block_b[1])
				return block_a[1] - block_b[1];
			if (block_a[2] != block_b[2])
				return block_a[2] - block_b[2];
			if (block_a[3] != block_b[3])
				return block_a[3] - block_b[3];
			if (block_a[4] != block_b[4])
				return block_a[4] - block_b[4];
			if (block_a[5] != block_b[5])
				return block_a[5] - block_b[5];
			if (block_a[6] != block_b[6])
				return block_a[6] - block_b[6];
			if (block_a[7] != block_b[7])
				return block_a[7] - block_b[7];
		}
	}
	return 0;
}

static inline int64_t cmp64_8_bitwise_or_wrapper(const char *a, const char *b) {
	return cmp64_8_bitwise_or((const uint64_t *)a, (const uint64_t *)b);
}

int fastcmp(const char *block_a, const char *block_b) {
	int ret;
#ifdef __AVX512BW__
	//#pragma message "fastcmp uses AVX512"
	const __m512i *a = (const __m512i *)block_a;
	const __m512i *b = (const __m512i *)block_b;
	const __m512i *b_end = (const __m512i *)(block_b + BLOCK_SIZE);
	__mmask64 res;
	for (; b < b_end; ++b, ++a) {
		res = _mm512_cmpeq_epi8_mask(_mm512_loadu_si512(a), _mm512_loadu_si512(b));
		if (res != ~(__mmask64)(0)) {
			break;
		}
	}
	if (b == b_end) {
		ret = 0;
	} else {
		int index = __builtin_ctzll(~res);
		ret = *((char *)a + index) - *((char *)b + index);
	}
#elif defined(__AVX2__)
	#ifdef __AVX__
		//#pragma message "fastcmp uses AVX2"
		const __m256i *a = (const __m256i *)block_a;
		const __m256i *b = (const __m256i *)block_b;
		const __m256i *b_end = (const __m256i *)(block_b + BLOCK_SIZE);
		int res;
		for (; b < b_end; ++b, ++a) {
			__m256i tmp;
			tmp = _mm256_cmpeq_epi8(_mm256_loadu_si256(a), _mm256_loadu_si256(b));
			res = _mm256_movemask_epi8(tmp);
			if (res != ~(int)(0)) {
				break;
			}
		}
		if (b == b_end) {
			ret = 0;
		} else {
			int index = __builtin_ctzll(~res);
			ret = *((char *)a + index) - *((char *)b + index);
		}
	#else
		#define RUN_TRIVIAL
	#endif
#else
	#define RUN_TRIVIAL
#endif

#ifdef RUN_TRIVIAL
	#pragma message "Warning: fastcmp uses trivial compare method."
	size_t i;
	for (i = 0; i < BLOCK_SIZE; ++i) {
		if (block_a[i] != block_b[i]) {
			break;
		}
	}
	ret = block_a[i] - block_b[i];
#endif
	return ret;
}

uint64_t cmp64_bitwise_or_1(const uint64_t *a, const uint64_t *b)
{
	size_t i;
	uint64_t ans = 0;
	for (i = 0; i < 4096 / 8; ++i, ++a, ++b) {
		ans |= (*a - *b);
	}
	return ans;
}

static inline int64_t cmp64_bitwise_or_1_wrapper(const char *a, const char *b) {
	return cmp64_bitwise_or_1((const uint64_t *)a, (const uint64_t *)b);
}

uint64_t cmp64_bitwise_or_2(const uint64_t *a, const uint64_t *b)
{
	size_t i;
	uint64_t ans = 0;
	for (i = 0; i < 4096 / 8 / 2; ++i, a += 2, b += 2) {
		ans |= (a[0] - b[0]) | (a[1] - b[1]);
	}
	return ans;
}

static inline int64_t cmp64_bitwise_or_2_wrapper(const char *a, const char *b) {
	return cmp64_bitwise_or_2((const uint64_t *)a, (const uint64_t *)b);
}

uint64_t cmp64_bitwise_or_4(const uint64_t *a, const uint64_t *b)
{
	size_t i;
	uint64_t ans = 0;
	for (i = 0; i < 4096 / 8 / 4; ++i, a += 4, b += 4) {
		ans |= (a[0] - b[0]) | (a[1] - b[1]) |
			(a[2] - b[2]) | (a[3] - b[3]);
	}
	return ans;
}

static inline int64_t cmp64_bitwise_or_4_wrapper(const char *a, const char *b) {
	return cmp64_bitwise_or_4((const uint64_t *)a, (const uint64_t *)b);
}

uint64_t cmp64_bitwise_or_2_2(const uint64_t *a, const uint64_t *b)
{
	uint64_t v1 = 0, v2 = 0;
	const uint64_t *limit = a + 4096 / 8;
	do {
		v1 |= *a - *b;
		++a;
		++b;
		v2 |= *a - *b;
		++a;
		++b;
	} while (a < limit);
	return v1 | v2;
}

static inline int64_t cmp64_bitwise_or_2_2_wrapper(const char *a, const char *b) {
	return cmp64_bitwise_or_2_2((const uint64_t *)a, (const uint64_t *)b);
}

uint64_t cmp64_bitwise_or_4_4(const uint64_t *a, const uint64_t *b)
{
	uint64_t v1 = 0, v2 = 0, v3 = 0, v4 = 0;
	const uint64_t *limit = a + 4096 / 8;
	do {
		v1 |= *a - *b;
		++a;
		++b;
		v2 |= *a - *b;
		++a;
		++b;
		v3 |= *a - *b;
		++a;
		++b;
		v4 |= *a - *b;
		++a;
		++b;
	} while (a < limit);
	return v1 | v2 | v3 | v4;
}

static inline int64_t cmp64_bitwise_or_4_4_wrapper(const char *a, const char *b) {
	return cmp64_bitwise_or_4_4((const uint64_t *)a, (const uint64_t *)b);
}

int64_t cmp64_xxh64(const void *a, const void *b)
{
	return XXH64(a, 4096, 0) - XXH64(b, 4096, 0);
}

// Assume that block_a and block_b is 8 byte aligned
int64_t cmp64_v2_2(const uint64_t *a, const uint64_t *b) {
	const uint64_t *limit = b + BLOCK_SIZE / sizeof(*b);
	do {
		if (unlikely(*a != *b))
			break;
		++a;
		++b;
		if (unlikely(*a != *b))
			break;
		++a;
		++b;
	} while (b != limit);
	if (b == limit)
		return 0;
	else
		return *a - *b;
}

static inline int64_t cmp64_v2_2_wrapper(const char *a, const char *b) {
	return cmp64_v2_2((const uint64_t *)a, (const uint64_t *)b);
}

// Assume that block_a and block_b is 8 byte aligned
int64_t cmp64_v2_4(const uint64_t *a, const uint64_t *b) {
	const uint64_t *limit = b + BLOCK_SIZE / sizeof(*b);
	do {
		if (unlikely(*a != *b))
			break;
		++a;
		++b;
		if (unlikely(*a != *b))
			break;
		++a;
		++b;
		if (unlikely(*a != *b))
			break;
		++a;
		++b;
		if (unlikely(*a != *b))
			break;
		++a;
		++b;
	} while (b != limit);
	if (b == limit)
		return 0;
	else
		return *a - *b;
}

static inline int64_t cmp64_v2_4_wrapper(const char *a, const char *b) {
	return cmp64_v2_4((const uint64_t *)a, (const uint64_t *)b);
}

static inline int64_t cmp64_asm(const void *a, const void *b, uint64_t len) {
	uint64_t tmp;
	asm volatile("\n"
		"1:	movq (%2),%1\n"
		"2:	cmpq %1,(%3)\n"
		"	jnz 3f\n"
		"	leaq 8(%2),%2\n"
		"	leaq 8(%3),%3\n"
		"	subq $8,%0\n"
		"	jnz 1b\n"
		"3:\n"
		: "+r" (len), "=r" (tmp), "+r" (a), "+r" (b)
		:
		: "cc");
	// TODO: "memory" clobber?
	return (int64_t)len;
}

static inline int64_t cmp64_asm_wrapper(const char *a, const char *b) {
	return cmp64_asm(a, b, PAGE_SIZE);
}

#define test_memcmp_page(func, func_name) ({								\
	start = clock();														\
	for (a = aa, b = bb; a != aa + SIZE; a += PAGE_SIZE, b += PAGE_SIZE) {	\
		if (func(a, b))														\
			printf("WTF???\n");												\
	}																		\
	end = clock();															\
	printf("%s: %f ns for each page\n", func_name,							\
			(double)(end - start) / CLOCKS_PER_SEC / (SIZE / PAGE_SIZE) * 1e9);\
})

static inline int memcmp_page(const char *a, const char *b) {
	return memcmp(a, b, PAGE_SIZE);
}

int main() {
	static char aa[SIZE], bb[SIZE];
	char *a, *b;
	clock_t start, end;

	memset(aa, 0x3f, SIZE);
	memset(bb, 0x3f, SIZE);

	test_memcmp_page(memcmp_page, "memcmp");
	test_memcmp_page(cmp64_wrapper, "cmp64");
	test_memcmp_page(cmp64_2_wrapper, "cmp64_2");
	test_memcmp_page(cmp64_4_wrapper, "cmp64_4");
	test_memcmp_page(cmp64_8_wrapper, "cmp64_8");
	test_memcmp_page(memcmp_repe_cmpsb_wrapper, "memcmp_repe_cmpsb");
	test_memcmp_page(memcmp_repe_cmpsq_wrapper, "memcmp_repe_cmpsq");
	test_memcmp_page(cmp64_8_or_wrapper, "cmp64_8_or");
	test_memcmp_page(cmp64_4_bitwise_or_wrapper, "cmp64_4_bitwise_or");
	test_memcmp_page(cmp64_8_bitwise_or_wrapper, "cmp64_8_bitwise_or");
	test_memcmp_page(fastcmp, "fastcmp");
	test_memcmp_page(cmp64_bitwise_or_1_wrapper, "cmp64_bitwise_or_1_wrapper");
	test_memcmp_page(cmp64_bitwise_or_2_wrapper, "cmp64_bitwise_or_2");
	test_memcmp_page(cmp64_bitwise_or_4_wrapper, "cmp64_bitwise_or_4");
	test_memcmp_page(cmp64_bitwise_or_2_2_wrapper, "cmp64_bitwise_or_2_2");
	test_memcmp_page(cmp64_bitwise_or_4_4_wrapper, "cmp64_bitwise_or_4_4");
	test_memcmp_page(cmp64_xxh64, "cmp64_xxh64");
	test_memcmp_page(cmp64_v2_2_wrapper, "cmp64_v2_2");
	test_memcmp_page(cmp64_v2_4_wrapper, "cmp64_v2_4");
	test_memcmp_page(cmp64_asm_wrapper, "cmp64_asm");


	return 0;
}

