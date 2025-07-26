# Example-imp

Two variants, six samples, imputed unphased calls.

## File

### Header

| 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
| - | - | - | - | - | - | - | - | - | - | - | - |
|6C |1B |10 |02 |00 |00 |00 |06 |00 |00 |00 |40|
|M  |M  |S  |V  |V  |V  |V  |S  |S  |S  |S  |C |

* M, All PGEN and PGEN-index files start with the two magic bytes 0x6c 0x1b
* S, storage mode byte (see docs; 0x10 is the standard PLINK 2 format, with variable-width variant records).
* V, Number of variants as a little-endian uint32 (2); call this M.
* S, Number of samples as a little-endian uint32 (6); call this N.
* C, Control header (`header_ctrl`) if S not 0x20 (see docs); bits 0-3: Indicates vrtype and variant record length storage widths.

Array of 8-byte fpos (0-based offsets) values for the first variant in each vblock (B) as little-endian uint64 (0x17 -> 23). Number of blocks is B = ceil(M/2^16). This subsection is not present for the fixed-width modes.

| C | D | E | F | 10| 11| 12| 13|
| - | - | - | - | - | - | - | - |
|17 |00 |00 |00 |00 |00 |00 |00 |

What are these?!?

| 14| 15| 16|
| - | - | - |
|00 |02 |02 |

Sequence of header blocks, each containing information about kPglVblockSize variants (except the last may be shorter)?

### Variant Blocks

For the variable-width storage modes (0x10, 0x11, 0x20, 0x21), the PGEN file is divided into “variant blocks” of 2^16 variants (the last block may contain fewer variants).

| 17| 18| 19| 1A|
| - | - | - | - |
|CA |05 |63 |08 |

Variant 1, Sample 1: 1/1:0.0000,0.0039,0.9961
Variant 1, Sample 2: 1/1:0.0000,0.0157,0.9843

If difflist_len is zero, that's it. Otherwise, the difflist is organized into 64-element groups (the last group will usually be smaller).

Note that with 20k samples, a difflist is space-saving even with a MAF of 5%:

```text
    ~1/400 hom alt + ~38/400 het = (~39/400) * 20k
                                 = ~1950 sample IDs.
```

that's 31 groups, requiring about 2 + 62 + 30 + 488 + 1919 = 2501 bytes (can be slightly higher since a few ID deltas may be larger than 127); uncompressed storage requires 5000 bytes.


https://github.com/chrchang/plink-ng/blob/9e2ebe5d5197f0e84f2a98f1a8c4b164675a4809/2.0/include/pgenlib_read.cc
https://github.com/chrchang/plink-ng/blob/9e2ebe5d5197f0e84f2a98f1a8c4b164675a4809/2.0/include/pgenlib_read.cc#L928
https://github.com/chrchang/plink-ng/blob/9e2ebe5d5197f0e84f2a98f1a8c4b164675a4809/2.0/pgenlibr/src/pgenlibr.cpp#L63
plink2::PreinitPgfi