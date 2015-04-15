
#include <stddef.h>
#include <string.h>

#include "spritz.h"

#define N 256

#if defined(_MSC_VER)
# define ALIGNED(S) __declspec(align(S))
#elif defined(__GNUC__)
# define ALIGNED(S) __attribute__((aligned(S)))
#else
# define ALIGNED(S)
#endif

ALIGNED(64) typedef struct State_ {
    unsigned char s[N];
    unsigned char a;
    unsigned char i;
    unsigned char j;
    unsigned char k;
    unsigned char w;
    unsigned char z;
} State;

#define LOW(B)  ((B) & 0xf)
#define HIGH(B) ((B) >> 4)

State __xdata state;

#define memzero() initialize_state()

static void
initialize_state(void)
{
    uint8_t v = 0;

    do {
        state.s[v] = v;
		v++;
    } while (v != 0);
    state.a = 0;
    state.i = 0;
    state.j = 0;
    state.k = 0;
    state.z = 0;
    state.w = 1;
}

static void
update(void)
{
    unsigned char t;
    unsigned char y;

    state.i += state.w;
    y = state.j + state.s[state.i];
    state.j = state.k + state.s[y];
    state.k = state.i + state.k + state.s[state.j];
    t = state.s[state.i];
    state.s[state.i] = state.s[state.j];
    state.s[state.j] = t;
}

static unsigned char
output(void)
{
    const unsigned char y1 = state.z + state.k;
    const unsigned char x1 = state.i + state.s[y1];
    const unsigned char y2 = state.j + state.s[x1];

    state.z = state.s[y2];

    return state.z;
}

static void
crush(void)
{
    unsigned char v;
    unsigned char x1;
    unsigned char x2;
    unsigned char y;

    for (v = 0; v < N / 2; v++) {
        y = (N - 1) - v;
        x1 = state.s[v];
        x2 = state.s[y];
        if (x1 > x2) {
            state.s[v] = x2;
            state.s[y] = x1;
        } else {
            state.s[v] = x1;
            state.s[y] = x2;
        }
    }
}

static void
whip(void)
{
    const unsigned int r = N * 2;
    unsigned int       v;

    for (v = 0; v < r; v++) {
        update();
    }
    state.w += 2;
}

static void
shuffle(void)
{
    whip();
    crush();
    whip();
    crush();
    whip();
    state.a = 0;
}

static void shuffle_if_needed(void)
{
	if (state.a)
		shuffle();
}

static void
absorb_stop(void)
{
    if (state.a == N / 2) {
        shuffle();
    }
    state.a++;
}

static void
absorb_nibble(const unsigned char x)
{
    unsigned char t;
    unsigned char y;

    if (state.a == N / 2) {
        shuffle();
    }
    y = N / 2 + x;
    t = state.s[state.a];
    state.s[state.a] = state.s[y];
    state.s[y] = t;
    state.a++;
}

static void
absorb_byte(const unsigned char b)
{
    absorb_nibble(LOW(b));
    absorb_nibble(HIGH(b));
}

static void
absorb(const unsigned char __xdata *msg, uint8_t length)
{
    uint8_t v;

    for (v = 0; v < length; v++) {
        absorb_byte(msg[v]);
    }
}

static unsigned char
drip(void)
{
    update();

    return output();
}

static void
key_setup(const unsigned char __xdata *key, uint8_t keylen)
{
    initialize_state();
    absorb(key, keylen);
}

void
spritz_encrypt(unsigned char __xdata *msg, uint8_t msglen,
               const unsigned char __xdata *nonce, uint8_t noncelen,
               const unsigned char __xdata *key, uint8_t keylen)
{
    uint8_t v;

    key_setup(key, keylen);
    absorb_stop();
    absorb(nonce, noncelen);
	shuffle_if_needed();
    for (v = 0; v < msglen; v++) {
        msg[v] = msg[v] + drip();
    }
    memzero();
}

void
spritz_decrypt(unsigned char __xdata *c, uint8_t clen,
               const unsigned char __xdata *nonce, uint8_t noncelen,
               const unsigned char __xdata *key, uint8_t keylen)
{
    uint8_t v;

    key_setup(key, keylen);
    absorb_stop();
    absorb(nonce, noncelen);
	shuffle_if_needed();
    for (v = 0; v < clen; v++) {
        c[v] = c[v] - drip();
    }
    memzero();
}

void
spritz_auth(unsigned char __xdata *out, uint8_t outlen,
            const unsigned char __xdata *msg, uint8_t msglen,
            const unsigned char __xdata *key, uint8_t keylen)
{

    key_setup(key, keylen);
    absorb_stop();
    absorb(msg, msglen);
    absorb_stop();
    absorb(&outlen, 1U);
	shuffle_if_needed();
    for (; outlen > 0; outlen--, out++) {
        *out = drip();
    }

    memzero();
}
