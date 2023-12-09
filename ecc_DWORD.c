#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#define modulo(a,p) a >= 0? a: a + p

#pragma pack(push, 1)
typedef struct{

	int x;
	int y;
} point_t;

typedef struct{

	point_t generator;
	int a;
	int b;
	int p;
	int cofactor;
} curve_t;
#pragma pack(pop)

/*modular arifmetics*/
// a*x = b (mod p)
// (a, p) = a*u + m*v = 1, p - primary
// x = (u*b) (mod p)
int
solution(int a, int b, int p){

	//extended Euclidean elgoritm
	int s = 0, old_s = 1, t = 1, old_t = 0, quotient = 0, temp = 0, r = p, old_r = a;
	while(r){
		quotient = old_r / r;
		temp = old_r;
		old_r = r;
		r = temp - quotient * r;
		temp = old_s;
		old_s = s;
		s = temp - quotient * s;
		temp = old_t;
		old_t = t;
		t = temp - quotient * t;
	}

	return modulo((old_s * b) % p, p);
}

/*functions that work with points on curve*/
// y^2 = x^3 + ax + b entry  in Wieierstrass form
// 4a^3 + 27b^2 != 0 additional condition for smooth elliptic curve
int
is_on_curve(point_t *point, int a, int b, int p){

	if(point == NULL)
		return 1;

	int x = point->x, y = point->y;
	return modulo((y*y - x*x*x - a - b) % p, p) == 0;
}

// lymda = 3x1^2 + a / 2y1 (mod p)
// x = lymda^2 - 2x1 (mod p)
// y = lymda(x1 - x) - y1 (mod p)
point_t
*point_doubling(point_t *point, int a, int p){

	int x = point->x, y = point->y;
	int lymda = solution(2*y, 3*x*x + a, p);
	point->x = modulo((lymda*lymda - 2*x) % p, p);
	point->y = modulo((lymda*(x - point->x) - y) % p, p);

	return point;
}

// lymda = y1-y2 / x1-x2 (mod p)
// x = lymda^2 - x1 - x2 (mod p)
// y = lymda(x1 - x3) - y1 (mod p)
point_t
*point_addition(point_t *point1, point_t *point2, int a, int b, int p){

	if(!is_on_curve(point1, a, b, p)){}
	if(!is_on_curve(point2, a, b, p)){}

	if(point1 == NULL)
		return point2;

	if(point2 == NULL)
		return point1;

	if(point1->x == point2->x && point1->y != point2->y)
        return NULL;

    if(point1->x != point2->x){
		int x1 = point1->x, x2 = point2->x, y1 = point1->y, y2 = point2->y;
		int lymda = solution(x2 - x1, y2 - y1, p);
		point1->x = modulo((lymda*lymda - x1 - x2) % p, p);
		point1->y = modulo((lymda*(x1 - point1->x) - y1) % p, p);
        return point1;
	}

	return point_doubling(point1, a, p);
}

// return -point
point_t
*point_neg(point_t *point, int p){

	point->y = modulo(-point->y % p, p);

	return point;
}

// gen public key
// P = G^K (mod p)
// time: log_2(k)
point_t
*scalar_multiply(point_t *point, int k, int a, int b, int p){

//	if(!is_on_curve(point, a, b, p))
//		return NULL;

	if(point == NULL)
		return NULL;

	if(k < 0)
		return scalar_multiply(point_neg(point, p), -k, a, b, p);

	point_t *addend = point, *result = NULL;

	for(int i = k; i != 0; i >>= 1){
		if(i & 1)
			if(result != NULL){
				result = point_addition(result, addend, a, b, p);
            } else {
                result = (point_t*)malloc(sizeof(point_t));
                result->x = addend->x;
				result->y = addend->y;
			}

		addend = point_doubling(addend, a, p);
	}

//	if(!is_on_curve(result, a, b, p))
//		return NULL;

	return result;
}

int
main(int argc, char *argv[], char **env){

	curve_t curve = {
		.generator = {
			.x = 15,
			.y = 13
		},
		.a = 0,
		.b = 7,
		.p = 17,
		.cofactor = 1
	};
	int secure_key = atoi(argv[1]);
	point_t *public_key = scalar_multiply(&curve.generator, secure_key, curve.a, curve.b, curve.p);
	printf("public key is: (%d ; %d)\n", public_key->x, public_key->y);

	return 0;
}
