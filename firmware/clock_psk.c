#include "clock_psk.h"
#include <stddef.h>
#include <string.h>

static uint32_t rol(uint32_t v, unsigned n)
{
    return (v << n) | (v >> (32u - n));
}

static void sha1_block(uint32_t state[5], const uint8_t block[64])
{
    uint32_t w[16], a, b, c, d, e, f, k, t;
    unsigned i;
    for (i = 0; i < 16; ++i) {
        const uint8_t *p = block + i * 4u;
        w[i] = ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
               ((uint32_t)p[2] << 8) | p[3];
    }
    a = state[0]; b = state[1]; c = state[2]; d = state[3]; e = state[4];
    for (i = 0; i < 80; ++i) {
        if (i >= 16) w[i & 15u] = rol(w[(i - 3u) & 15u] ^
            w[(i - 8u) & 15u] ^ w[(i - 14u) & 15u] ^ w[i & 15u], 1);
        if (i < 20) { f = (b & c) | (~b & d); k = 0x5a827999u; }
        else if (i < 40) { f = b ^ c ^ d; k = 0x6ed9eba1u; }
        else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8f1bbcdcu; }
        else { f = b ^ c ^ d; k = 0xca62c1d6u; }
        t = rol(a, 5) + f + e + k + w[i & 15u];
        e = d; d = c; c = rol(b, 30); b = a; a = t;
    }
    state[0] += a; state[1] += b; state[2] += c;
    state[3] += d; state[4] += e;
}

static void sha1_initial(uint32_t s[5])
{
    s[0] = 0x67452301u; s[1] = 0xefcdab89u; s[2] = 0x98badcfeu;
    s[3] = 0x10325476u; s[4] = 0xc3d2e1f0u;
}

/* All messages after the precomputed HMAC block fit in one SHA1 block. */
static void sha1_tail(const uint32_t base[5], const uint8_t *message,
                       size_t length, uint8_t digest[20])
{
    uint8_t block[64] = {0};
    uint32_t s[5], bits = (uint32_t)(64u + length) * 8u;
    unsigned i;
    memcpy(s, base, sizeof(s));
    memcpy(block, message, length);
    block[length] = 0x80;
    block[60] = (uint8_t)(bits >> 24); block[61] = (uint8_t)(bits >> 16);
    block[62] = (uint8_t)(bits >> 8); block[63] = (uint8_t)bits;
    sha1_block(s, block);
    for (i = 0; i < 20; ++i)
        digest[i] = (uint8_t)(s[i / 4u] >> (24u - (i % 4u) * 8u));
}

static void hmac(const uint32_t inner[5], const uint32_t outer[5],
                  const uint8_t *message, size_t length, uint8_t out[20])
{
    uint8_t digest[20];
    sha1_tail(inner, message, length, digest);
    sha1_tail(outer, digest, sizeof(digest), out);
}

int clock_psk_derive(const char *ssid, const char *password, uint8_t psk[32],
                     void (*yield_cpu)(void))
{
    uint8_t pad[64], salt[36], u[20], accum[20];
    uint32_t inner[5], outer[5];
    size_t ssid_len, password_len;
    unsigned block, round, i;
    if (!ssid || !password || !psk) return 0;
    for (ssid_len = 0; ssid_len <= 32u && ssid[ssid_len]; ++ssid_len) {}
    for (password_len = 0; password_len <= 63u && password[password_len]; ++password_len) {}
    if (!ssid_len || ssid_len > 32u || password_len < 8u || password_len > 63u) return 0;
    memset(pad, 0x36, sizeof(pad));
    for (i = 0; i < password_len; ++i) pad[i] ^= (uint8_t)password[i];
    sha1_initial(inner); sha1_block(inner, pad);
    for (i = 0; i < sizeof(pad); ++i) pad[i] ^= 0x36u ^ 0x5cu;
    sha1_initial(outer); sha1_block(outer, pad);
    memcpy(salt, ssid, ssid_len);
    memset(salt + ssid_len, 0, 4);
    for (block = 1; block <= 2; ++block) {
        salt[ssid_len + 3u] = (uint8_t)block;
        hmac(inner, outer, salt, ssid_len + 4u, u);
        memcpy(accum, u, sizeof(u));
        for (round = 1; round < 4096; ++round) {
            hmac(inner, outer, u, sizeof(u), u);
            for (i = 0; i < sizeof(u); ++i) accum[i] ^= u[i];
            if (yield_cpu && (round & 63u) == 0u) yield_cpu();
        }
        memcpy(psk + (block - 1u) * 20u, accum, block == 1u ? 20u : 12u);
    }
    memset(inner, 0, sizeof(inner)); memset(outer, 0, sizeof(outer));
    memset(pad, 0, sizeof(pad)); memset(u, 0, sizeof(u));
    return 1;
}
