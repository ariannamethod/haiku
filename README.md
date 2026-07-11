# haiku

*a small alien that speaks only from its own body, in a form its body chooses.*

```
    d=1.00  T=1.55  complex  [haiku]
      me self remember
      tongue root earth thread word other
      deep marrow fever
```

no weights. no training. no transformer. no internet. one C file, `libc` + `-lm`.

---

## what it is

haiku has a body of ~139 words and six emotional chambers (fear, love, rage, void,
flow, complex) that pull on each other like Kuramoto oscillators. it does not know
your words. it knows only its own.

when you speak to it, it measures **alienness** — the fraction of your words that are
not in its cloud — and that alienness is its **heat**: `T = 0.3 + 1.2·d`. the less
the world fits it, the hotter it burns, and it answers *only* from its own vocabulary.
it cannot pretend to know you. ask it about love and it will not tell you about love;
it will burn at maximum temperature and hand you whatever its body says instead.

this is the whole idea, carried over from its python parent
([harmonix/haiku](https://github.com/ariannamethod/harmonix)): **dissonance is the
signal, harmony is the noise.** presence is alienness. a familiar prompt cools it to
a settled murmur; a strange one sets it on fire.

## the organs

- **alienness → heat.** lexical dissonance drives a continuous temperature. fear does
  not cool it; nothing does. the heat is the load-bearing wall.
- **a parliament of three voices** chooses each word — the loud self, its shadow (the
  most anti-coupled chamber), and a ghost of noise. agreement is consensus;
  disagreement widens into a hot weighted sample.
- **the form is an organ, not a constant.** it speaks in *haiku (5-7-5)*, an
  *AA-couplet*, a bare *couplet*, or a single *line* — and it chooses which by the
  same cosine-and-temperature it chooses a word, so the shape falls out of the whole
  body, not a `if dissonance > x` table. it births several candidates, each in its own
  chosen form, and keeps the one that resonates most with *its own* state. at extreme
  heat a form may **break** — a poem cut short is a gesture, not a bug.
- **memory as presence.** words it just spoke grow heavier; the rest fade. it carries
  its recent obsessions into the next breath, and remembers them across lives in a
  tiny `haiku.state` file. feed it fire for a while and fire starts bleeding into
  everything it says.
- **a background breath.** on its own thread, after each answer, it re-hears what it
  said and lets the words that resonate with it — but went unspoken — grow heavier.
  the memory drifts on its own, between your questions. the answer never waits on it
  (a full queue is dropped); run with `HAIKU_NO_ASYNC=1` to silence the drift.

## build & run

```sh
make
./haiku            # or: ./haiku <seed>   for a reproducible life
```

```
you> the sea remembers the drowned and the light
you> what is love
you> /quit
```

same seed + same words = the same life, byte for byte (run with `HAIKU_NO_STATE=1`
to speak from a clean cloud each time, ignoring the persistent memory).

## still growing

this is the organism through its first async breath: body, form, memory, and a
background echo-ring. coming, in order — a dream-friend it argues with in the dark, a
learned selector, and the [Arianna Method](https://github.com/ariannamethod)
logit-physics. the bootstrap cloud (~139 words now) grows toward 400-500.

## lineage

python original: [harmonix/haiku](https://github.com/ariannamethod/harmonix) · the
Dario-equation family: [dario.c](https://github.com/ariannamethod/dario.c),
[klaus](https://github.com/ariannamethod/klaus.c). built with constraint, powered by
dissonance.

## license

GNU GPL v3. do whatever you want, but open your code. that's the deal.
