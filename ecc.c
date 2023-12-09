#include <gmp.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>

typedef struct{

	mpz_t x;
	mpz_t y;
} point_t;

typedef struct{

	point_t generator;
	mpz_t a;
	mpz_t b;
	mpz_t p;
	mpz_t cofactor;
} curve_t;

/*modular arifmetics*/
// a*x = b (mod p)
// (a, p) = a*u + m*v = 1, p - primary
// x = (u*b) (mod p)
void
solution(mpz_t a, mpz_t b, mpz_t p, mpz_t lymda){

	/*extended Euclidean elgoritm*/
    mpz_t s, old_s, t, old_t, quotient, temp, r, old_r;

    mpz_init(s); mpz_init(old_s); mpz_init(t); mpz_init(old_t);
    mpz_init(quotient); mpz_init(temp); mpz_init(r); mpz_init(old_r);

    mpz_set_si(s, 0); mpz_set_si(old_s, 1); mpz_set_si(t, 1); mpz_set_si(old_t, 0);
    mpz_set_si(quotient, 0); mpz_set_si(temp, 0); mpz_set(r, p); mpz_set(old_r, a);

	while(mpz_cmp_si(r, 0)){

        mpz_fdiv_q(quotient, old_r, r);

        mpz_set(temp, old_r);

		mpz_set(old_r, r);

        mpz_mul(r, quotient, r);
        mpz_sub(r, temp, r);

        mpz_set(temp, old_s);

        mpz_set(old_s, s);

        mpz_mul(s, quotient, s);
        mpz_sub(s, temp, s);

        mpz_set(temp, old_t);

        mpz_set(old_t, t);

        mpz_mul(t, quotient, t);
        mpz_sub(t, temp, t);
	}

	mpz_mul(temp, old_s, b);
    mpz_mod(lymda, temp, p);

    mpz_clear(s); mpz_clear(old_s); mpz_clear(t); mpz_clear(old_t);
    mpz_clear(r); mpz_clear(old_r); mpz_clear(quotient); mpz_clear(temp);
}

/*functions that work with points on curve*/
// y^2 = x^3 + ax + b entry  in Wieierstrass form
// 4a^3 + 27b^2 != 0 additional condition for smooth elliptic curve
int
is_on_curve(point_t *point, mpz_t a, mpz_t b, mpz_t p){

	if(point == NULL)
		return 1;

    mpz_t x, y;

    mpz_init(x); mpz_init(x);

    mpz_set(x, point->x); mpz_set(y, point->y);

    mpz_mul(y, y, y);
    mpz_mul(x, x, x);
    mpz_mul(x, x, x);
    mpz_sub(y, y, x);
    mpz_sub(y, y, a);
    mpz_sub(y, y, b);
    mpz_fdiv_q(y, y, p);

    return mpz_cmp_si(y, 0);
}

// lymda = 3x1^2 + a / 2y1 (mod p)
// x = lymda^2 - 2x1 (mod p)
// y = lymda(x1 - x) - y1 (mod p)
point_t
*point_doubling(point_t *point, mpz_t a, mpz_t p){

    mpz_t x, y, lymda, old_x, old_y, old_lymda;

    mpz_init(x); mpz_init(y); mpz_init(lymda);
    mpz_init(old_x); mpz_init(old_y); mpz_init(old_lymda);

    mpz_set(old_x, point->x); mpz_set(old_y, point->y);

    mpz_mul(x, old_x, old_x);
    mpz_mul_si(x, x, 3);
    mpz_add(x, x, a);
    mpz_mul_si(y, old_y, 2);
    solution(y, x, p, old_lymda);

    mpz_mul(lymda, old_lymda, old_lymda);
    mpz_mul_si(x, old_x, 2);
    mpz_sub(x, lymda, x);
    mpz_fdiv_q(point->x, x, p);

    mpz_sub(x, old_x, x);
    mpz_mul(lymda, old_lymda, x);
    mpz_sub(y, lymda, old_y);
    mpz_fdiv_q(point->y, y, p);

    mpz_clear(x); mpz_clear(y); mpz_clear(lymda);
    mpz_clear(old_x); mpz_clear(old_y); mpz_clear(old_lymda);

    return point;
}

// lymda = y1-y2 / x1-x2 (mod p)
// x = lymda^2 - x1 - x2 (mod p)
// y = lymda(x1 - x3) - y1 (mod p)
point_t
*point_addition(point_t *point1, point_t *point2, mpz_t a, mpz_t b, mpz_t p){

	if(!is_on_curve(point1, a, b, p)){}
	if(!is_on_curve(point2, a, b, p)){}

	if(point1 == NULL)
		return point2;

	if(point2 == NULL)
		return point1;

	if(mpz_cmp(point1->x, point2->x) == 0 && mpz_cmp(point1->y, point2->y) != 0)
        return NULL;

    if(mpz_cmp(point1->x, point2->x) != 0){

        mpz_t x, y,x1, x2, y1, y2, lymda, old_lymda;

        mpz_init(x1); mpz_init(x2); mpz_init(y1);
        mpz_init(y2); mpz_init(lymda); mpz_init(old_lymda);
        mpz_init(x); mpz_init(y);

        mpz_set(x1, point1->x); mpz_set(x2, point2->x);
        mpz_set(y1, point1->y); mpz_set(y2, point2->y);

        mpz_sub(x, x2, x1);
        mpz_sub(y, y2, y1);
        solution(x, y, p, old_lymda);

        mpz_mul(lymda, old_lymda, old_lymda);
        mpz_sub(x, lymda, x1);
        mpz_sub(x, x, x2);
        mpz_fdiv_q(point1->x, x, p);

        mpz_sub(x, x1, point1->x);
        mpz_mul(lymda, old_lymda, x);
        mpz_sub(y, lymda, y1);
        mpz_fdiv_q(point1->y, y, p);

        return point1;
	}

	return point_doubling(point1, a, p);
}

// return -point
point_t
*point_neg(point_t *point, mpz_t p){

    mpz_mul_si(point->y, point->y, -1);
    mpz_fdiv_q(point->y, point->y, p);

    return point;
}

// gen public key
// P = G^K (mod p)
// time: log_2(k)
point_t
*scalar_multiply(point_t *point, mpz_t k, mpz_t a, mpz_t b, mpz_t p){

//	if(!is_on_curve(point, a, b, p))
//		return NULL;
    printf("1\n");
	if(point == NULL)
		return NULL;

	if(mpz_cmp_si(k, 0) < 0){
		mpz_mul_si(k, k, -1);
        return scalar_multiply(point_neg(point, p), k, a, b, p);
    }

	point_t *addend = point, *result = NULL;

    while(mpz_cmp_si(k, 0)){
        if(mpz_odd_p(k))
            if(result != NULL){
                result = point_addition(result, addend, a, b, p);
            } else {
                result = (point_t*)malloc(sizeof(point_t));
                mpz_init(result->x); mpz_init(result->y);
                mpz_set(result->x, addend->x);
                mpz_set(result->y, addend->y);
            }

        addend = point_doubling(addend, a, p);
        mpz_divexact_ui(k, k, 2);
    }

//	if(!is_on_curve(result, a, b, p))
//		return NULL;

	return result;
}

int
main(){

/*    mpz_t a, b, p, lymda;

    mpz_init(a); mpz_init(b); mpz_init(p); mpz_init(lymda);

    mpz_set_si(a, 24); mpz_set_si(b, 675); mpz_set_si(p, 17);

    solution(a, b, p, lymda);
    gmp_printf("lymda: %Zd\n", lymda);     */

    curve_t curve;
    mpz_t secure_key;
    mpz_init(curve.generator.x); mpz_init(curve.generator.x);
    mpz_init(curve.a); mpz_init(curve.b); mpz_init(curve.p); mpz_init(curve.cofactor);
    mpz_init(secure_key);

    mpz_set_si(curve.generator.x, 15); mpz_set_si(curve.generator.y, 13);
    mpz_set_si(curve.a, 0); mpz_set_si(curve.b, 7), mpz_set_si(curve.p, 17); mpz_set_si(secure_key, 6);

    point_t *public_key = scalar_multiply(&curve.generator, secure_key, curve.a, curve.b, curve.p);
    gmp_printf("public_key: (%Zd ; %Zd)\n", public_key->x, public_key->y);

    return 0;
}
