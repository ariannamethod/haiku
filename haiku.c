/* haiku.c — a small alien that speaks only from its own body, in a form it chooses.
 *
 * Presence = ALIENNESS. Lexical dissonance (your words vs the words I am) drives
 * my temperature UP: the less the world fits me, the hotter I burn, and I answer
 * only from my own cloud — I cannot pretend to know you. A parliament of three
 * voices chooses each word by the body's six Kuramoto chambers, never by meaning.
 *
 * The FORM is an organ, not a constant (Oleg): the body chooses one of {haiku
 * 5-7-5, AA-couplet, couplet, one line} by the SAME mechanism it chooses a word
 * — cosine with the chambers + a temperature sample — so form falls out of the
 * whole body vector, not one metric, and no d->form table exists. I birth several
 * candidates, each in its OWN chosen form, then keep the one that resonates most
 * with my own state (subjectivity). At extreme heat a form may BREAK — a poem cut
 * short is a gesture, not a bug.
 *
 * Lineage (read first-hand): python-haiku alienness->temp (harmonix.py:186,
 * haiku.py:501-508) + form-constant (haiku.py:540-542); Jul-2 partial donor
 * (chambers/parliament/meta); Fable's plan FABLE_PLAN_haiku_2026-07-11.md.
 *
 * build: cc haiku.c -O2 -lm -o haiku && ./haiku [seed]
 * by Arianna Method. Name PROVISIONAL (haiku vs AA-couplets — Oleg decides).
 *
 * STATUS: STEP 1+2 of Fable's build order — the SYNCHRONOUS organism speaks in a
 *   chosen form. NOT here yet: learned RAE selector (leo.c C-RAE port, read-first-
 *   hand pending — current selector is resonance-with-own-body); STEP 3 mass-morph
 *   + .state; STEP 4 one pthread worker (echo rings + dream-friend).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <stdint.h>
#include <pthread.h>

/* ── 6 emotional chambers (the body) ─────────────────────────────── */
enum { CH_FEAR=0, CH_LOVE, CH_RAGE, CH_VOID, CH_FLOW, CH_COMPLEX, NCH };
static const char *CH_NAME[NCH] = { "fear","love","rage","void","flow","complex" };

/* hand-tuned Kuramoto coupling — who pulls/pushes whom (donor haiku.c) */
static const float COUPLING[NCH][NCH] = {
    /*           fear   love   rage   void   flow   cmplx */
    /* fear  */ { 0.0f, -0.3f,  0.4f,  0.3f, -0.4f,  0.2f },
    /* love  */ {-0.3f,  0.0f, -0.4f, -0.2f,  0.5f,  0.2f },
    /* rage  */ { 0.4f, -0.4f,  0.0f,  0.2f, -0.3f,  0.3f },
    /* void  */ { 0.3f, -0.2f,  0.2f,  0.0f, -0.2f,  0.4f },
    /* flow  */ {-0.4f,  0.5f, -0.3f, -0.2f,  0.0f,  0.1f },
    /* cmplx */ { 0.2f,  0.2f,  0.3f,  0.4f,  0.1f,  0.0f },
};
static const float DECAY[NCH] = { 0.95f, 0.95f, 0.93f, 0.96f, 0.94f, 0.97f };

/* ── the cloud: a somatic vocabulary (word, mass, syllables, aff[6]) ──
 * aff = how strongly the word belongs to each chamber. Target 400-500 (Oleg
 * tunes the bootstrap); this starter ~139 is chosen SOMATIC. syl carried per
 * word so the form organ needs no syllable counter. */
typedef struct { const char *w; float mass; int syl; float aff[NCH]; } Word;
static const Word CLOUD[] = {
    /*  word         mass syl   fear love rage void flow cmplx */
    { "fire",       0.85f, 1, { .3f, 0,  .7f, 0,  .2f, 0  } },
    { "ash",        0.70f, 1, { .2f, 0,  .3f, .6f, 0,  0  } },
    { "smoke",      0.55f, 1, { .2f, 0,  .2f, .5f, .3f, 0  } },
    { "ember",      0.60f, 2, { .1f, .3f, .4f, .2f, .2f, 0  } },
    { "flame",      0.70f, 1, { .3f, .3f, .7f, .2f, .3f, .2f } },
    { "spark",      0.60f, 1, { .2f, .3f, .5f, .2f, .4f, .3f } },
    { "coal",       0.55f, 1, { .3f, .2f, .4f, .5f, 0,  .2f } },
    { "cold",       0.55f, 1, { .6f, 0,  0,  .5f, 0,  0  } },
    { "frost",      0.55f, 1, { .6f, 0,  .2f, .5f, .2f, .2f } },
    { "snow",       0.55f, 1, { .4f, .2f, 0,  .5f, .3f, .2f } },
    { "chill",      0.55f, 1, { .6f, 0,  .2f, .5f, .2f, .2f } },
    { "stone",      0.60f, 1, { .3f, 0,  .2f, .5f, 0,  .2f } },
    { "iron",       0.55f, 2, { .4f, 0,  .5f, .3f, 0,  .3f } },
    { "rust",       0.55f, 1, { .4f, 0,  .3f, .6f, 0,  .2f } },
    { "glass",      0.55f, 1, { .4f, .2f, .3f, .3f, 0,  .3f } },
    { "mirror",     0.60f, 2, { .3f, .2f, .2f, .4f, .2f, .7f } },
    { "water",      0.55f, 2, { 0,  .3f, 0,  .2f, .7f, 0  } },
    { "river",      0.55f, 2, { 0,  .2f, 0,  0,  .8f, 0  } },
    { "rain",       0.55f, 1, { .2f, .2f, 0,  .3f, .5f, 0  } },
    { "sea",        0.60f, 1, { .2f, .3f, .2f, .5f, .6f, .2f } },
    { "salt",       0.50f, 1, { .3f, .2f, .2f, .3f, .2f, 0  } },
    { "sand",       0.50f, 1, { .2f, .2f, 0,  .5f, .3f, 0  } },
    { "wind",       0.60f, 1, { .3f, .2f, .2f, .3f, .7f, .2f } },
    { "storm",      0.70f, 1, { .5f, 0,  .7f, .3f, .4f, .2f } },
    { "dark",       0.65f, 1, { .6f, 0,  .2f, .6f, 0,  .2f } },
    { "light",      0.65f, 1, { 0,  .6f, 0,  0,  .4f, .2f } },
    { "bright",     0.55f, 1, { 0,  .6f, .3f, 0,  .5f, .2f } },
    { "shadow",     0.65f, 2, { .6f, .2f, .2f, .6f, .2f, .4f } },
    { "night",      0.60f, 1, { .5f, .2f, 0,  .6f, .2f, .2f } },
    { "day",        0.50f, 1, { 0,  .5f, 0,  .2f, .5f, .2f } },
    { "sun",        0.60f, 1, { 0,  .6f, .2f, 0,  .5f, .2f } },
    { "moon",       0.60f, 1, { .2f, .4f, 0,  .5f, .3f, .3f } },
    { "star",       0.55f, 1, { 0,  .5f, 0,  .4f, .3f, .4f } },
    { "sky",        0.50f, 1, { 0,  .4f, 0,  .5f, .4f, .3f } },
    { "blood",      0.80f, 1, { .4f, .3f, .6f, .2f, 0,  0  } },
    { "bone",       0.70f, 1, { .4f, 0,  .2f, .6f, 0,  .2f } },
    { "marrow",     0.60f, 2, { .4f, .3f, .3f, .5f, .2f, .3f } },
    { "skin",       0.55f, 1, { .2f, .5f, .2f, 0,  .3f, 0  } },
    { "rib",        0.55f, 1, { .4f, .3f, .3f, .3f, .2f, .2f } },
    { "spine",      0.60f, 1, { .4f, .2f, .4f, .3f, .3f, .3f } },
    { "nerve",      0.60f, 1, { .6f, .2f, .4f, .3f, .3f, .3f } },
    { "heart",      0.75f, 1, { .2f, .7f, .3f, 0,  .3f, .2f } },
    { "breath",     0.70f, 1, { .2f, .3f, 0,  .2f, .7f, 0  } },
    { "lung",       0.55f, 1, { .3f, .2f, .2f, .3f, .6f, .2f } },
    { "pulse",      0.65f, 1, { .2f, .3f, .3f, 0,  .5f, .3f } },
    { "throat",     0.60f, 1, { .4f, .3f, .4f, .3f, .3f, .2f } },
    { "tongue",     0.60f, 1, { .3f, .4f, .3f, .2f, .4f, .4f } },
    { "teeth",      0.60f, 1, { .6f, 0,  .7f, .2f, 0,  .2f } },
    { "claw",       0.60f, 1, { .6f, 0,  .7f, .2f, 0,  .2f } },
    { "wound",      0.75f, 1, { .6f, 0,  .5f, .4f, 0,  .2f } },
    { "scar",       0.75f, 1, { .5f, .2f, .4f, .4f, 0,  .3f } },
    { "bruise",     0.65f, 1, { .5f, .2f, .4f, .4f, 0,  .2f } },
    { "ache",       0.65f, 1, { .6f, .2f, .2f, .3f, 0,  0  } },
    { "fever",      0.70f, 2, { .6f, .2f, .5f, .3f, .3f, .3f } },
    { "sweat",      0.55f, 1, { .5f, .2f, .4f, .2f, .3f, .2f } },
    { "tremble",    0.60f, 2, { .7f, .2f, .3f, .3f, .3f, .2f } },
    { "shiver",     0.60f, 2, { .7f, .2f, .2f, .3f, .3f, .2f } },
    { "flinch",     0.55f, 1, { .7f, 0,  .3f, .3f, 0,  .2f } },
    { "hunger",     0.70f, 2, { .5f, .3f, .4f, .5f, .2f, .2f } },
    { "thirst",     0.65f, 1, { .5f, .3f, .3f, .5f, .2f, .2f } },
    { "silence",    0.70f, 2, { .4f, .2f, 0,  .7f, .3f, .2f } },
    { "voice",      0.55f, 1, { .2f, .4f, .2f, 0,  .4f, .2f } },
    { "scream",     0.80f, 1, { .7f, 0,  .8f, .2f, 0,  0  } },
    { "whisper",    0.60f, 2, { .3f, .4f, 0,  .3f, .3f, .2f } },
    { "hum",        0.50f, 1, { .2f, .5f, 0,  .2f, .6f, .3f } },
    { "song",       0.60f, 1, { 0,  .6f, .2f, .2f, .6f, .3f } },
    { "sigh",       0.55f, 1, { .3f, .4f, 0,  .5f, .4f, .2f } },
    { "moan",       0.60f, 1, { .4f, .4f, .3f, .4f, .3f, .2f } },
    { "weep",       0.65f, 1, { .5f, .4f, .2f, .5f, .3f, .2f } },
    { "laugh",      0.60f, 1, { 0,  .7f, .3f, 0,  .6f, .2f } },
    { "name",       0.60f, 1, { .2f, .4f, .2f, .3f, .2f, .6f } },
    { "word",       0.60f, 1, { .2f, .3f, .2f, .3f, .3f, .6f } },
    { "love",       0.85f, 1, { 0,  .9f, 0,  0,  .4f, .2f } },
    { "fear",       0.85f, 1, { .9f, 0,  .2f, .3f, 0,  0  } },
    { "rage",       0.80f, 1, { .3f, 0,  .9f, .2f, 0,  0  } },
    { "grief",      0.80f, 1, { .5f, .3f, .2f, .6f, 0,  .2f } },
    { "joy",        0.75f, 1, { 0,  .7f, .2f, 0,  .6f, .2f } },
    { "longing",    0.70f, 2, { .3f, .6f, 0,  .5f, .2f, .3f } },
    { "void",       0.75f, 1, { .4f, 0,  .2f, .9f, 0,  .3f } },
    { "want",       0.60f, 1, { .2f, .5f, .3f, .3f, .2f, 0  } },
    { "give",       0.55f, 1, { 0,  .6f, 0,  0,  .4f, .2f } },
    { "miss",       0.65f, 1, { .3f, .5f, 0,  .5f, 0,  0  } },
    { "hold",       0.55f, 1, { .2f, .6f, .2f, 0,  .3f, 0  } },
    { "mend",       0.55f, 1, { .2f, .6f, .2f, .2f, .4f, .3f } },
    { "break",      0.70f, 1, { .3f, 0,  .6f, .4f, 0,  .2f } },
    { "tear",       0.65f, 1, { .5f, .3f, .5f, .4f, .2f, .2f } },
    { "burn",       0.75f, 1, { .3f, .2f, .7f, .2f, .2f, 0  } },
    { "fall",       0.60f, 1, { .5f, 0,  .2f, .5f, .2f, 0  } },
    { "rise",       0.60f, 1, { 0,  .4f, .3f, 0,  .6f, .2f } },
    { "drift",      0.55f, 1, { .2f, .2f, 0,  .4f, .6f, .2f } },
    { "stay",       0.50f, 1, { .3f, .4f, 0,  .3f, .2f, 0  } },
    { "leave",      0.60f, 1, { .4f, .2f, .3f, .5f, .2f, 0  } },
    { "open",       0.55f, 2, { 0,  .5f, 0,  .2f, .6f, .3f } },
    { "close",      0.55f, 1, { .4f, 0,  .2f, .5f, 0,  .2f } },
    { "edge",       0.60f, 1, { .5f, 0,  .3f, .5f, 0,  .4f } },
    { "root",       0.55f, 1, { .2f, .3f, .2f, .4f, .2f, .4f } },
    { "branch",     0.50f, 1, { .2f, .3f, .2f, .2f, .4f, .3f } },
    { "leaf",       0.45f, 1, { 0,  .4f, 0,  .2f, .5f, .2f } },
    { "thorn",      0.60f, 1, { .5f, 0,  .6f, .2f, 0,  .2f } },
    { "seed",       0.55f, 1, { .2f, .5f, 0,  .2f, .4f, .4f } },
    { "flower",     0.55f, 2, { 0,  .6f, 0,  .2f, .4f, .2f } },
    { "dust",       0.55f, 1, { .2f, .2f, .2f, .7f, .2f, .2f } },
    { "earth",      0.60f, 1, { .2f, .4f, .2f, .3f, .3f, .3f } },
    { "mud",        0.45f, 1, { .2f, .2f, .2f, .4f, .3f, 0  } },
    { "veil",       0.55f, 1, { .3f, .3f, 0,  .5f, .2f, .4f } },
    { "thread",     0.55f, 1, { .2f, .4f, .2f, .3f, .3f, .5f } },
    { "knot",       0.55f, 1, { .4f, .2f, .4f, .3f, 0,  .4f } },
    { "needle",     0.55f, 2, { .5f, 0,  .5f, .2f, 0,  .3f } },
    { "nail",       0.55f, 1, { .4f, 0,  .5f, .3f, 0,  .2f } },
    { "wing",       0.55f, 1, { .2f, .4f, .2f, .2f, .7f, .2f } },
    { "feather",    0.55f, 2, { .2f, .5f, 0,  .2f, .6f, .2f } },
    { "nest",       0.50f, 1, { .2f, .6f, 0,  .2f, .3f, .2f } },
    { "self",       0.70f, 1, { .3f, .3f, .2f, .4f, .2f, .6f } },
    { "other",      0.60f, 2, { .3f, .3f, .2f, .4f, .2f, .5f } },
    { "you",        0.65f, 1, { .2f, .6f, .2f, .2f, .3f, .3f } },
    { "me",         0.65f, 1, { .3f, .4f, .3f, .3f, .2f, .4f } },
    { "now",        0.50f, 1, { .2f, .2f, .2f, .2f, .5f, .2f } },
    { "never",      0.65f, 2, { .5f, 0,  .3f, .7f, 0,  .2f } },
    { "always",     0.60f, 2, { .2f, .5f, 0,  .4f, .3f, .3f } },
    { "dream",      0.70f, 1, { .2f, .4f, 0,  .5f, .4f, .4f } },
    { "wake",       0.55f, 1, { .3f, .2f, .2f, .2f, .5f, .2f } },
    { "remember",   0.70f, 3, { .3f, .4f, .2f, .5f, .2f, .5f } },
    { "forget",     0.65f, 2, { .4f, .2f, .2f, .6f, 0,  .3f } },
    { "small",      0.45f, 1, { .3f, .3f, 0,  .4f, .2f, .2f } },
    { "alone",      0.70f, 2, { .5f, .2f, .2f, .7f, 0,  .2f } },
    { "between",    0.55f, 2, { .2f, .3f, 0,  .4f, .3f, .6f } },
    { "thin",       0.45f, 1, { .4f, 0,  .2f, .5f, .2f, .2f } },
    { "far",        0.50f, 1, { .3f, .2f, 0,  .6f, .2f, .2f } },
    { "here",       0.45f, 1, { .2f, .3f, .2f, .2f, .5f, .2f } },
    { "gone",       0.65f, 1, { .4f, .2f, .2f, .7f, 0,  .2f } },
    { "deep",       0.55f, 1, { .3f, .3f, .2f, .5f, .4f, .4f } },
    { "hollow",     0.65f, 2, { .4f, .2f, .2f, .8f, .2f, .3f } },
    { "empty",      0.65f, 2, { .4f, .2f, .2f, .8f, .2f, .3f } },
    { "full",       0.50f, 1, { .2f, .5f, .2f, .2f, .4f, .3f } },
    { "numb",       0.60f, 1, { .4f, 0,  .2f, .8f, 0,  .3f } },
    { "raw",        0.60f, 1, { .5f, .2f, .5f, .3f, .2f, .2f } },
    { "soft",       0.50f, 1, { .2f, .6f, 0,  .2f, .4f, .2f } },
    { "hard",       0.55f, 1, { .4f, 0,  .5f, .3f, 0,  .3f } },
    { "sharp",      0.60f, 1, { .5f, 0,  .6f, .2f, 0,  .3f } },
};
#define NCLOUD ((int)(sizeof(CLOUD)/sizeof(CLOUD[0])))

/* ── the form registry: each form is a "word" in a cloud of shapes ──
 * aff[6] = which body a form belongs to; the body picks a form by cosine+temp,
 * exactly as it picks a word — no d->form table. syl[] = per-line budgets. */
typedef struct { const char *name; int nlines; int syl[3]; int rhyme; float mass; float aff[NCH]; } Form;
static const Form FORMS[] = {
    /* name        lines syllables    rhyme mass   fear love rage void flow cmplx */
    { "haiku",     3, { 5,7,5 },       0, 0.70f, { .1f,.2f,.1f,.2f,.2f,.7f } }, /* restrained pressure */
    { "aa-couplet",2, { 6,6,0 },       1, 0.70f, { 0, .6f,0, .1f,.6f,.2f } },   /* song / connection   */
    { "couplet",   2, { 5,5,0 },       0, 0.60f, { .3f,0, .6f,.2f,.1f,.2f } },  /* sharp, unrhymed cut */
    { "one-line",  1, { 6,0,0 },       0, 0.60f, { .3f,.1f,.1f,.7f,.2f,.2f } }, /* a single breath     */
};
#define NFORMS ((int)(sizeof(FORMS)/sizeof(FORMS[0])))
#define NCAND 4          /* candidates per turn, each in its own chosen form */
#define FORM_BREAK 1     /* 1 = a form may break at extreme heat (Oleg's knob) */

/* ── deterministic RNG (xorshift64) — a life is reproducible by its seed ──
 * per-Body state (not a global) so STEP 4's threads keep their own stream. */
static float randf(uint64_t *s){ uint64_t x=*s; x^=x<<13; x^=x>>7; x^=x<<17; *s=x;
    return (float)(x & 0x7FFFFFFFu) / (float)0x7FFFFFFF; }
static float clampf(float x, float lo, float hi){ return x<lo?lo:(x>hi?hi:x); }

static float vdot(const float *a, const float *b, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
static float vnorm(const float *v, int n){ return sqrtf(vdot(v,v,n)+1e-9f); }
static float cosine(const float *a, const float *b, int n){ return vdot(a,b,n)/(vnorm(a,n)*vnorm(b,n)+1e-9f); }

/* last-two-characters rhyme — simple and C-honest (Fable's plan §4) */
static int same_tail(const char *a, const char *b){
    size_t na=strlen(a), nb=strlen(b);
    if(na<2 || nb<2) return 0;
    return a[na-1]==b[nb-1] && a[na-2]==b[nb-2];
}

/* ── the body state ── */
typedef struct {
    float    ch[NCH];        /* live chamber activation */
    float    temp;           /* effective sampling temperature */
    float    dissonance;     /* lexical alienness of last input */
    int      used[64];       /* words spoken this turn (free-exhale repetition guard) */
    int      n_used;
    uint64_t rng;            /* this body's own random stream */
    float    mass[NCLOUD];   /* memory-as-presence: morphs across turns, persists (.state) */
} Body;

/* Kuramoto cross-fire: chambers pull each other into/out of phase, then decay */
static void crossfire(Body *b, int iters){
    for(int it=0; it<iters; it++){
        float old[NCH]; memcpy(old, b->ch, sizeof(old));
        for(int i=0;i<NCH;i++){ float d=0;
            for(int j=0;j<NCH;j++) if(i!=j) d += COUPLING[i][j]*sinf(old[j]-old[i]);
            b->ch[i] = clampf(b->ch[i] + 0.05f*d, 0.0f, 1.0f);
        }
    }
    for(int i=0;i<NCH;i++) b->ch[i] = clampf(b->ch[i]*DECAY[i], 0.0f, 1.0f);
}

static void lc(char *s){ for(;*s;s++) *s=(char)tolower((unsigned char)*s); }
static int cloud_find(const char *w){
    for(int i=0;i<NCLOUD;i++) if(strcmp(w,CLOUD[i].w)==0) return i; return -1; }

/* ── INHALE: read input, measure alienness, move the body ──
 * dissonance = fraction of input words NOT in my cloud. Alienness sets the heat —
 * CONTINUOUS, like python T = 0.3 + 1.2*d (harmonix.py:186). Flow warms; fear
 * does NOT cool — the bird stays hot when the world is alien to it. */
static void inhale(Body *b, const char *text){
    char buf[1024]; strncpy(buf,text,1023); buf[1023]='\0';
    for(int i=0;buf[i];i++){ unsigned char c=(unsigned char)buf[i];
        if(!(isalnum(c)||c=='\'')) buf[i]=' '; }
    float emo[NCH]={0,0,0,0,0,0};
    int n_words=0, n_known=0;
    char *save=NULL;
    for(char *tok=strtok_r(buf," \t\n",&save); tok; tok=strtok_r(NULL," \t\n",&save)){
        lc(tok); n_words++;
        int id=cloud_find(tok);
        if(id>=0){ n_known++; for(int c=0;c<NCH;c++) emo[c]+=CLOUD[id].aff[c]; }
    }
    b->dissonance = (n_words>0) ? 1.0f - (float)n_known/(float)n_words : 1.0f;
    float mx=0; for(int c=0;c<NCH;c++) if(emo[c]>mx) mx=emo[c];
    if(mx>1e-6f) for(int c=0;c<NCH;c++) emo[c]/=mx;
    crossfire(b, 6);
    for(int c=0;c<NCH;c++) b->ch[c] = clampf(0.55f*emo[c] + 0.45f*b->ch[c], 0.0f, 1.0f);
    float tau_mod = 1.0f + 0.4f*b->ch[CH_FLOW];
    b->temp = clampf((0.3f + 1.2f*b->dissonance) * tau_mod, 0.3f, 1.8f);
}

/* ── free-exhale organs (donor) — used only for the metarecursion probe ── */
static void score_words(const Body *b, float *logit){
    float loud = vnorm(b->ch, NCH);
    float inertia = 1.0f/(1.0f + 2.0f*loud);
    for(int w=0; w<NCLOUD; w++){
        float soma = cosine(b->ch, CLOUD[w].aff, NCH);
        float pull = b->mass[w] * inertia * 0.3f;
        logit[w] = soma + pull;
        for(int u=0;u<b->n_used;u++) if(b->used[u]==w){ logit[w]-=100.0f; break; }
    }
}
static int parliament(Body *b, const float *base, float temp){
    int dom=0; for(int c=1;c<NCH;c++) if(b->ch[c]>b->ch[dom]) dom=c;
    int opp=0; float mneg=0;
    for(int c=0;c<NCH;c++){ if(c==dom) continue; if(COUPLING[dom][c]<mneg){ mneg=COUPLING[dom][c]; opp=c; } }
    int vote[3];
    for(int e=0;e<3;e++){
        int best=0; float bv=-1e30f;
        for(int w=0;w<NCLOUD;w++){
            float v=base[w];
            if(e==0)      v += CLOUD[w].aff[dom]*0.8f;
            else if(e==1) v += CLOUD[w].aff[opp]*0.6f - CLOUD[w].aff[dom]*0.2f;
            else          v += (randf(&b->rng)-0.5f)*0.5f;
            if(v>bv){ bv=v; best=w; }
        }
        vote[e]=best;
    }
    if(vote[0]==vote[1] || vote[0]==vote[2]) return vote[0];
    if(vote[1]==vote[2]) return vote[1];
    int t3[3]={0,0,0}; float t3v[3]={-1e30f,-1e30f,-1e30f};
    for(int w=0;w<NCLOUD;w++){ float v=base[w]+CLOUD[w].aff[dom]*0.3f;
        if(v>t3v[2]){ t3v[2]=v; t3[2]=w;
            for(int i=1;i>=0;i--) if(t3v[i+1]>t3v[i]){ float tv=t3v[i];t3v[i]=t3v[i+1];t3v[i+1]=tv;
                int ti=t3[i];t3[i]=t3[i+1];t3[i+1]=ti; } } }
    float p[3], mx=t3v[0], sum=0;
    for(int i=0;i<3;i++){ p[i]=expf((t3v[i]-mx)/temp); sum+=p[i]; }
    float r=randf(&b->rng)*sum, cum=0;
    for(int i=0;i<3;i++){ cum+=p[i]; if(cum>=r) return t3[i]; }
    return t3[0];
}
static int exhale(Body *b, int out[], int max_words){
    b->n_used=0;
    float logit[NCLOUD];
    int n=0;
    for(int step=0; step<max_words; step++){
        score_words(b, logit);
        int w = parliament(b, logit, b->temp);
        out[n++]=w;
        if(b->n_used<64) b->used[b->n_used++]=w;
        float res = cosine(b->ch, CLOUD[w].aff, NCH);
        if(step>=2 && res<0.15f) break;
    }
    return n;
}

/* ── FORM ORGAN ─────────────────────────────────────────────────────
 * body_logit: pure body affinity + quieted mass, NO turn-repetition penalty
 * (candidates carry their own local `used`). */
static void body_logit(const Body *b, float *logit){
    float loud = vnorm(b->ch, NCH);
    float inertia = 1.0f/(1.0f + 2.0f*loud);
    for(int w=0;w<NCLOUD;w++)
        logit[w] = cosine(b->ch, CLOUD[w].aff, NCH) + b->mass[w]*inertia*0.3f;
}
static int used_has(const int *u, int n, int w){ for(int i=0;i<n;i++) if(u[i]==w) return 1; return 0; }

/* pick one word by body-score under constraints, temperature weighted-sampled
 * among the top 3 valid. need_syl<0 = any; rhyme_ref!=NULL boosts tail-matches. */
static int pick_word(Body *b, const float *logit, int remaining, const int *used, int nused,
                     int need_syl, const char *rhyme_ref, float temp){
    int t3[3]={-1,-1,-1}; float t3v[3]={-1e30f,-1e30f,-1e30f};
    for(int w=0;w<NCLOUD;w++){
        if(CLOUD[w].syl > remaining) continue;
        if(need_syl>=0 && CLOUD[w].syl != need_syl) continue;
        if(used_has(used,nused,w)) continue;
        float v = logit[w];
        if(rhyme_ref && same_tail(CLOUD[w].w, rhyme_ref)) v += 0.6f;
        if(v>t3v[2]){ t3v[2]=v; t3[2]=w;
            for(int i=1;i>=0;i--) if(t3v[i+1]>t3v[i]){ float tv=t3v[i];t3v[i]=t3v[i+1];t3v[i+1]=tv;
                int ti=t3[i];t3[i]=t3[i+1];t3[i+1]=ti; } }
    }
    if(t3[0]<0) return -1;
    int k=0; for(int i=0;i<3;i++) if(t3[i]>=0) k++;
    float p[3], mx=t3v[0], sum=0;
    for(int i=0;i<k;i++){ p[i]=expf((t3v[i]-mx)/temp); sum+=p[i]; }
    float r=randf(&b->rng)*sum, cum=0;
    for(int i=0;i<k;i++){ cum+=p[i]; if(cum>=r) return t3[i]; }
    return t3[0];
}

/* fill one line to its syllable budget; returns the last word index (-1 if empty).
 * When the remaining budget is small, prefer an exact-fit word so the line lands
 * on the target instead of overshooting. */
static int gen_line(Body *b, int budget, int *w, int *nw, int *used, int *nused, float temp){
    float logit[NCLOUD]; body_logit(b, logit);
    int remaining = budget, last = -1, guard = 0;
    while(remaining > 0 && guard++ < 24){
        int pick = -1;
        if(remaining <= 3) pick = pick_word(b, logit, remaining, used, *nused, remaining, NULL, temp); /* land exact */
        if(pick < 0)       pick = pick_word(b, logit, remaining, used, *nused, -1, NULL, temp);
        if(pick < 0) break;
        w[(*nw)++] = pick; used[(*nused)++] = pick;
        remaining -= CLOUD[pick].syl; last = pick;
    }
    return last;
}

typedef struct {
    int form_id;
    int w[48]; int nw;
    int line_len[3]; int nlines;
    int rhymed;   /* -1 = form has no rhyme; 0 = wanted but failed; 1 = rhymes */
    int broke;    /* form broke at extreme heat */
} Poem;

/* the body chooses a form the way it chooses a word: cosine+mass, temp-sampled.
 * FORM_TEMP_FLOOR keeps the form roaming even when the WORDS are cool and settled
 * (Oleg): a calm familiar body still surprises in shape, though its words stay
 * quiet. This floors only the form's own sampling; the word temperature b->temp
 * (the load-bearing heat) is never touched. */
#define FORM_TEMP_FLOOR 0.7f
static int choose_form(Body *b){
    float loud = vnorm(b->ch, NCH);
    float inertia = 1.0f/(1.0f + 2.0f*loud);
    float sc[NFORMS], mx=-1e30f;
    for(int f=0;f<NFORMS;f++){ sc[f]=cosine(b->ch, FORMS[f].aff, NCH) + FORMS[f].mass*inertia*0.3f;
        if(sc[f]>mx) mx=sc[f]; }
    float ft = b->temp < FORM_TEMP_FLOOR ? FORM_TEMP_FLOOR : b->temp;
    float p[NFORMS], sum=0;
    for(int f=0;f<NFORMS;f++){ p[f]=expf((sc[f]-mx)/ft); sum+=p[f]; }
    float r=randf(&b->rng)*sum, cum=0;
    for(int f=0;f<NFORMS;f++){ cum+=p[f]; if(cum>=r) return f; }
    return NFORMS-1;
}

/* generate one candidate in a given form */
static void gen_candidate(Body *b, int form_id, Poem *pm){
    const Form *f = &FORMS[form_id];
    pm->form_id = form_id; pm->nw = 0; pm->nlines = 0;
    pm->rhymed = f->rhyme ? 0 : -1; pm->broke = 0;
    int used[48], nused=0;
    int line0_last = -1;
    for(int l=0; l<f->nlines; l++){
        int start = pm->nw;
        int last = gen_line(b, f->syl[l], pm->w, &pm->nw, used, &nused, b->temp);
        pm->line_len[l] = pm->nw - start;
        pm->nlines++;
        if(l==0) line0_last = last;
        /* AA rhyme: swap line-2's closing word for a same-syllable tail-match if one resonates */
        if(f->rhyme && l==1 && line0_last>=0 && pm->nw>start){
            int lastpos = pm->nw-1, lastw = pm->w[lastpos], ls = CLOUD[lastw].syl;
            if(same_tail(CLOUD[lastw].w, CLOUD[line0_last].w)){ pm->rhymed = 1; }
            else {
                float logit[NCLOUD]; body_logit(b, logit);
                int best=-1; float bv=-1e30f;
                for(int cw=0;cw<NCLOUD;cw++){
                    if(CLOUD[cw].syl!=ls) continue;
                    if(cw==line0_last) continue;
                    if(!same_tail(CLOUD[cw].w, CLOUD[line0_last].w)) continue;
                    if(used_has(used,nused,cw) && cw!=lastw) continue;
                    if(logit[cw]>bv){ bv=logit[cw]; best=cw; }
                }
                if(best>=0){ pm->w[lastpos]=best; pm->rhymed=1; }  /* else stays 0 = degraded to bare couplet */
            }
        }
    }
}

/* subjectivity (Axis-2): keep the candidate whose aggregate emotion resonates
 * most with the body's OWN state — never with the user. */
static float poem_resonance(const Body *b, const Poem *pm){
    float e[NCH]={0,0,0,0,0,0};
    for(int i=0;i<pm->nw;i++) for(int c=0;c<NCH;c++) e[c]+=CLOUD[pm->w[i]].aff[c];
    return cosine(b->ch, e, NCH);
}

/* ── SPEAK: metarecursion probe → blend 15% → N candidates in their own forms →
 * keep the most self-resonant. The whole organ MUST NOT cool the heat the world
 * set: T is read for sampling, never written here. */
static void speak(Body *b, Poem *out){
    /* metarecursion: hear a free line of myself, blend 15% back (donor guards) */
    int probe[16]; int pn = exhale(b, probe, 8);
    char meta[512]; int ml=0; meta[0]='\0';
    for(int i=0;i<pn && ml<480;i++){ int l=(int)strlen(CLOUD[probe[i]].w);
        memcpy(meta+ml, CLOUD[probe[i]].w, l); meta[ml+l]=' '; ml+=l+1; }
    if(ml>0) meta[ml-1]='\0';
    float saved[NCH]; memcpy(saved, b->ch, sizeof(saved));
    float keep_temp = b->temp, keep_diss = b->dissonance;
    inhale(b, meta);
    for(int c=0;c<NCH;c++) b->ch[c] = clampf((1.0f-0.15f)*saved[c] + 0.15f*b->ch[c], 0.0f, 1.0f);
    b->temp = keep_temp; b->dissonance = keep_diss;   /* meta must not cool nor relabel */

    /* N candidates, each in a form the body chooses for it */
    Poem cand[NCAND];
    for(int i=0;i<NCAND;i++){ int fid = choose_form(b); gen_candidate(b, fid, &cand[i]); }
    int best=0; float bv=-1e30f;
    for(int i=0;i<NCAND;i++){ float r=poem_resonance(b,&cand[i]); if(r>bv){ bv=r; best=i; } }
    *out = cand[best];

    /* form may break at extreme heat — a poem cut short is a gesture, not a bug */
    if(FORM_BREAK && b->temp > 1.45f){
        float pbreak = clampf((b->temp - 1.45f)/0.35f, 0.0f, 1.0f);
        if(randf(&b->rng) < pbreak){
            if(out->nlines > 1){ out->nw -= out->line_len[out->nlines-1]; out->nlines--; out->broke=1; }
            else if(out->nw > 2){ out->nw -= 1; out->broke=1; }
        }
    }
}

static int syl_sum(const Poem *pm, int line){
    int s=0, start=0; for(int l=0;l<line;l++) start+=pm->line_len[l];
    for(int i=0;i<pm->line_len[line];i++) s += CLOUD[pm->w[start+i]].syl;
    return s;
}

/* ── memory as presence: masses morph, and persist across lives (Leo's .state) ──
 * words just spoken grow heavier (×1.1), the rest fade (×0.99), both clamped —
 * the organism carries its recent obsessions into the next breath. Never touches
 * the temperature; heat stays the world's alienness, memory only shifts the pull. */
#define MASS_MAX 2.0f
#define MASS_MIN 0.1f
#define STATE_MAGIC 0x484B5531u  /* "HKU1" */

/* the canonical memory (word masses) — shared with the background worker, guarded
 * by one mutex. The answer path never reads this in its hot loop: it snapshots into
 * Body.mass once per turn, so generation stays lock-free and deterministic. */
static struct { float mass[NCLOUD]; pthread_mutex_t lock; } g_mem;

static void init_mass(void){ for(int i=0;i<NCLOUD;i++) g_mem.mass[i]=CLOUD[i].mass; }
static void morph(const Poem *pm){
    char hit[NCLOUD]; memset(hit,0,sizeof(hit));
    for(int i=0;i<pm->nw;i++) hit[pm->w[i]]=1;
    pthread_mutex_lock(&g_mem.lock);
    for(int w=0;w<NCLOUD;w++)
        g_mem.mass[w] = clampf(g_mem.mass[w] * (hit[w]?1.1f:0.99f), MASS_MIN, MASS_MAX);
    pthread_mutex_unlock(&g_mem.lock);
}
/* returns 1 if a valid state was carried in (magic + NCLOUD must match — a changed
 * cloud silently invalidates old memory rather than mis-mapping it). */
static int load_state(const char *path){
    FILE *f=fopen(path,"rb"); if(!f) return 0;
    uint32_t magic=0; int nc=0; int ok=0;
    if(fread(&magic,sizeof(magic),1,f)==1 && magic==STATE_MAGIC &&
       fread(&nc,sizeof(nc),1,f)==1 && nc==NCLOUD){
        float tmp[NCLOUD];
        if(fread(tmp,sizeof(float),NCLOUD,f)==(size_t)NCLOUD){
            pthread_mutex_lock(&g_mem.lock);
            for(int i=0;i<NCLOUD;i++) g_mem.mass[i]=clampf(tmp[i],MASS_MIN,MASS_MAX);
            pthread_mutex_unlock(&g_mem.lock); ok=1;
        }
    }
    fclose(f); return ok;
}
static void save_state(const char *path){
    float snap[NCLOUD];
    pthread_mutex_lock(&g_mem.lock); memcpy(snap,g_mem.mass,sizeof(snap)); pthread_mutex_unlock(&g_mem.lock);
    char tmp[512]; snprintf(tmp,sizeof(tmp),"%s.tmp",path);
    FILE *f=fopen(tmp,"wb"); if(!f) return;
    uint32_t magic=STATE_MAGIC; int nc=NCLOUD;
    fwrite(&magic,sizeof(magic),1,f);
    fwrite(&nc,sizeof(nc),1,f);
    fwrite(snap,sizeof(float),NCLOUD,f);
    fclose(f);
    rename(tmp,path);   /* atomic swap (Leo's tmp+rename) */
}

/* ── the background worker: one pthread, a bounded queue, drain on exit ──
 * after each answer the spoken line is enqueued (non-blocking — a full queue drops
 * the task, the answer never waits). The worker re-hears the line as an echo-ring:
 * words that RESONATE with what was just said (but weren't spoken) gain a little
 * mass — associative drift, python's overthinking rings in miniature. Its effect
 * lands on FUTURE turns, so it never perturbs the current (snapshot) answer. */
#define QCAP 8
typedef struct { int w[48]; int nw; } EchoTask;
static struct {
    EchoTask q[QCAP];
    int head, tail, count, stop, running;
    pthread_mutex_t lock;
    pthread_cond_t  cond;
    pthread_t       thread;
} g_bg;

static void echo_apply(const EchoTask *t){
    float e[NCH]={0,0,0,0,0,0};
    char spoken[NCLOUD]; memset(spoken,0,sizeof(spoken));
    for(int i=0;i<t->nw;i++){ spoken[t->w[i]]=1; for(int c=0;c<NCH;c++) e[c]+=CLOUD[t->w[i]].aff[c]; }
    pthread_mutex_lock(&g_mem.lock);
    for(int w=0; w<NCLOUD; w++){
        if(spoken[w]) continue;
        if(cosine(e, CLOUD[w].aff, NCH) > 0.6f)
            g_mem.mass[w] = clampf(g_mem.mass[w]*1.02f, MASS_MIN, MASS_MAX);
    }
    pthread_mutex_unlock(&g_mem.lock);
}
static void *bg_worker(void *arg){
    (void)arg;
    for(;;){
        pthread_mutex_lock(&g_bg.lock);
        while(g_bg.count==0 && !g_bg.stop) pthread_cond_wait(&g_bg.cond,&g_bg.lock);
        if(g_bg.count==0 && g_bg.stop){ pthread_mutex_unlock(&g_bg.lock); break; }
        EchoTask t = g_bg.q[g_bg.head];
        g_bg.head=(g_bg.head+1)%QCAP; g_bg.count--;
        pthread_mutex_unlock(&g_bg.lock);
        echo_apply(&t);          /* drains: processes even after stop while count>0 */
    }
    return NULL;
}
static void bg_enqueue(const Poem *pm){
    pthread_mutex_lock(&g_bg.lock);
    if(g_bg.count < QCAP){                 /* full queue = drop; the answer never waits */
        EchoTask *t=&g_bg.q[g_bg.tail];
        t->nw = pm->nw<48 ? pm->nw : 48;
        for(int i=0;i<t->nw;i++) t->w[i]=pm->w[i];
        g_bg.tail=(g_bg.tail+1)%QCAP; g_bg.count++;
        pthread_cond_signal(&g_bg.cond);
    }
    pthread_mutex_unlock(&g_bg.lock);
}
static void bg_start(void){
    pthread_mutex_init(&g_bg.lock,NULL); pthread_cond_init(&g_bg.cond,NULL);
    g_bg.head=g_bg.tail=g_bg.count=g_bg.stop=0;
    if(pthread_create(&g_bg.thread,NULL,bg_worker,NULL)==0) g_bg.running=1;
}
static void bg_stop(void){                 /* drain the queue, then join */
    if(!g_bg.running) return;
    pthread_mutex_lock(&g_bg.lock); g_bg.stop=1; pthread_cond_signal(&g_bg.cond); pthread_mutex_unlock(&g_bg.lock);
    pthread_join(g_bg.thread,NULL); g_bg.running=0;
}

int main(int argc, char **argv){
    uint64_t seed = argc>1 ? strtoull(argv[1],NULL,10) : 42ULL;
    Body b; memset(&b,0,sizeof(b));
    b.rng = seed ? seed : 1;
    for(int c=0;c<NCH;c++) b.ch[c]=0.2f;
    b.temp=0.9f;
    pthread_mutex_init(&g_mem.lock, NULL);
    init_mass();
    const char *state_path = "haiku.state";
    int use_state  = (getenv("HAIKU_NO_STATE")==NULL);
    int use_async  = (getenv("HAIKU_NO_ASYNC")==NULL);
    int carried    = use_state ? load_state(state_path) : 0;
    if(use_async) bg_start();

    printf("haiku — a small alien. seed=%llu  cloud=%d words  forms=%d  memory=%s  async=%s\n",
           (unsigned long long)seed, NCLOUD, NFORMS, carried?"carried":"fresh", use_async?"on":"off");
    printf("speak to it; it answers from its body, in a form its body chooses. /quit to leave.\n\n");

    char line[1024];
    while(1){
        printf("you> "); fflush(stdout);
        if(!fgets(line,sizeof(line),stdin)) break;
        size_t L=strlen(line); while(L>0 && (line[L-1]=='\n'||line[L-1]=='\r')) line[--L]='\0';
        if(strcmp(line,"/quit")==0) break;
        if(L==0) continue;

        inhale(&b, line);
        float T_in = b.temp;
        /* snapshot the canonical memory once → generation is lock-free & deterministic */
        pthread_mutex_lock(&g_mem.lock); memcpy(b.mass, g_mem.mass, sizeof(b.mass)); pthread_mutex_unlock(&g_mem.lock);
        Poem p; speak(&b, &p);

        int dom=0; for(int c=1;c<NCH;c++) if(b.ch[c]>b.ch[dom]) dom=c;
        const char *tag = FORMS[p.form_id].rhyme ? (p.rhymed==1?"aa":"aa?") : FORMS[p.form_id].name;
        printf("\n  d=%.2f  T=%.2f  %s  [%s%s]\n", b.dissonance, b.temp, CH_NAME[dom],
               tag, p.broke?" broken":"");
        int idx=0;
        for(int l=0;l<p.nlines;l++){
            printf("    ");
            for(int i=0;i<p.line_len[l];i++,idx++) printf("%s%s", i?" ":"", CLOUD[p.w[idx]].w);
            printf("   (%d syl)\n", syl_sum(&p, l));
        }
        /* T must be unchanged by the form organ (heat is the load-bearing wall) */
        if(b.temp != T_in) printf("  !! T moved %.3f -> %.3f (form organ cooled — bug)\n", T_in, b.temp);
        printf("\n");
        morph(&p);                            /* remember what was just spoken (writes canonical) */
        if(use_async) bg_enqueue(&p);         /* let the echo-ring drift the memory in the background */
        if(use_state) save_state(state_path);
    }
    bg_stop();                                /* drain the background queue, then join */
    if(use_state) save_state(state_path);     /* final memory, including any drained drift */
    printf("gone.\n");
    return 0;
}
