# IronBridge.Libs.Nist_PQC_Algorithms
Nist Post-Quantum Cryptography (PQC) Algorithms used by RestServerCpp


The libs/crypto folder of this repo contains an unaltered copy (except for the
makefiles) of the Reference Implementation of each PQC algorithm.

### Original Sources

https://csrc.nist.gov/projects/post-quantum-cryptography/

* https://csrc.nist.gov/projects/post-quantum-cryptography/round-2-submissions
* https://csrc.nist.gov/projects/post-quantum-cryptography/round-3-submissions


### Build NIST Crypto Algorithms

To build all algorithms 
```
./build-algorithms.sh
```

Alternatively, to build a specific release...
```
./build_algorithms_nist_round2.sh
./build_algorithms_nist_round3.sh
```

The libraries are installed into **/usr/local/share/ibc/libs**.

The library names have been adjusted to indicate their contents...

* Suffix: "_NR2_STDRNG" = Nist Round 2 with classic/standard rng
* Suffix: "_NR2_CQCRNG" = Nist Round 2 with CQC rng
* Suffix: "_NR3_STDRNG" = Nist Round 3 with classic/standard rng
* Suffix: "_NR3_CQCRNG" = Nist Round 3 with CQC rng


In the ResServerCpp sources, the set of libraries that is used by changing the value of _whichAlgorithms variable.

See ./IronBridge.Software.RestServerCpp/server/sources/Ironbridge/Api/Core/RandomProducer.h

```
    enum
    {
        USE_ALGORITHMS_NISTROUND2_STDRNG = 21, // "_NR2_STDRNG" - Nist Round 2 with classic/standard rng
        USE_ALGORITHMS_NISTROUND2_CQCRNG = 22, // "_NR2_CQCRNG" - Nist Round 2 with CQC rng
        USE_ALGORITHMS_NISTROUND3_STDRNG = 31, // "_NR3_STDRNG" - Nist Round 3 with classic/standard rng
        USE_ALGORITHMS_NISTROUND3_CQCRNG = 32  // "_NR3_CQCRNG" - Nist Round 3 with CQC rng
    } __whichAlgorithms = USE_ALGORITHMS_NISTROUND3_CQCRNG;
```

### Build Results

For example...

```
jgilmore@msasa:[~/dev/github/CQCL/IronBridge.Libs.Nist_PQC_Algorithms]$ find /usr/local/share/ibc/libs -iname "*so"
/usr/local/share/ibc/libs/libdilithium-1_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libdilithium-1aes_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libdilithium-2_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libdilithium-2aes_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libdilithium-3_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libdilithium-3aes_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libdilithium-4_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libdilithium-4aes_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libfalcon-1024_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libfalcon-512_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libfalcon-768_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libfrodokem-1344_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libfrodokem-640_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libfrodokem-976_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libkyber-1024_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libkyber-102490s_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libkyber-512_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libkyber-51290s_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libkyber-768_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libkyber-76890s_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libnewhope-1024cca_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libnewhope-1024cpa_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libnewhope-512cca_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libnewhope-512cpa_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libntru-hps2048509_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libntru-hps2048677_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libntru-hps4096821_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libntru-hrss701_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libnts-1264_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libnts-13136_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libnts-1380_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libpicnic-l1fs_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libpicnic-l1ur_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libpicnic-l3fs_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libpicnic-l3ur_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libpicnic-l5fs_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libpicnic-l5ur_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libsphincs-haraka-128f-robust_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libsphincs-haraka-128s-robust_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libsphincs-haraka-192f-robust_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libsphincs-haraka-192s-robust_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libsphincs-haraka-256f-robust_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libsphincs-haraka-256s-robust_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libsphincs-sha256-128f-robust_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libsphincs-sha256-128s-robust_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libsphincs-sha256-192f-robust_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libsphincs-sha256-192s-robust_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libsphincs-sha256-256f-robust_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libsphincs-sha256-256s-robust_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libsphincs-shake256-128f-robust_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libsphincs-shake256-128s-robust_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libsphincs-shake256-192f-robust_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libsphincs-shake256-192s-robust_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libsphincs-shake256-256f-robust_NR2_CQCRNG.so
/usr/local/share/ibc/libs/libsphincs-shake256-256s-robust_NR2_CQCRNG.so

/usr/local/share/ibc/libs/libdilithium2_NR3_CQCRNG.so
/usr/local/share/ibc/libs/libdilithium2-AES-R_NR3_CQCRNG.so
/usr/local/share/ibc/libs/libdilithium2-R_NR3_CQCRNG.so
/usr/local/share/ibc/libs/libdilithium3_NR3_CQCRNG.so
/usr/local/share/ibc/libs/libdilithium3-AES-R_NR3_CQCRNG.so
/usr/local/share/ibc/libs/libdilithium3-R_NR3_CQCRNG.so
/usr/local/share/ibc/libs/libdilithium5_NR3_CQCRNG.so
/usr/local/share/ibc/libs/libdilithium5-AES-R_NR3_CQCRNG.so
/usr/local/share/ibc/libs/libdilithium5-R_NR3_CQCRNG.so
/usr/local/share/ibc/libs/libkyber-1024_NR3_CQCRNG.so
/usr/local/share/ibc/libs/libkyber-102490s_NR3_CQCRNG.so
/usr/local/share/ibc/libs/libkyber-512_NR3_CQCRNG.so
/usr/local/share/ibc/libs/libkyber-51290s_NR3_CQCRNG.so
/usr/local/share/ibc/libs/libkyber-768_NR3_CQCRNG.so
/usr/local/share/ibc/libs/libkyber-76890s_NR3_CQCRNG.so
/usr/local/share/ibc/libs/libntru-hps2048509_NR3_CQCRNG.so
/usr/local/share/ibc/libs/libntru-hps2048509_NR3_STDRNG.so
/usr/local/share/ibc/libs/libntru-hps2048677_NR3_CQCRNG.so
/usr/local/share/ibc/libs/libntru-hps2048677_NR3_STDRNG.so
/usr/local/share/ibc/libs/libntru-hps4096821_NR3_CQCRNG.so
/usr/local/share/ibc/libs/libntru-hps4096821_NR3_STDRNG.so
/usr/local/share/ibc/libs/libntru-hrss701_NR3_CQCRNG.so
/usr/local/share/ibc/libs/libntru-hrss701_NR3_STDRNG.so

```