#!/bin/sh

cd sources_nist_round2

echo "############ KECCAK ###############"
echo "Installing Keccak..."
cd support/libkeccak
make --no-print-directory -s
sudo make --no-print-directory -s install
cd ../..


###################################
echo "Installing algorithms libraries..."

# Directory that will contain all of our crypto libraries (.so)
LIB_TARGET=/usr/local/share/ibc/libs
sudo mkdir -p $LIB_TARGET

echo "############# KEM #################"
cd kem

###################################
echo "Building Frodo..."
cd frodoKEM/Reference_Implementation
make --no-print-directory -s -C FrodoKEM-640  clean shared && sudo cp FrodoKEM-640/*.so   $LIB_TARGET
make --no-print-directory -s -C FrodoKEM-976  clean shared && sudo cp FrodoKEM-976/*.so   $LIB_TARGET
make --no-print-directory -s -C FrodoKEM-1344 clean shared && sudo cp FrodoKEM-1344/*.so $LIB_TARGET
cd ../..

###################################
echo "Building Kyber..."
cd kyber/Reference_Implementation/crypto_kem
make --no-print-directory -s -C kyber512      clean shared && sudo cp kyber512/*.so      $LIB_TARGET
make --no-print-directory -s -C kyber512-90s  clean shared && sudo cp kyber512-90s/*.so  $LIB_TARGET
make --no-print-directory -s -C kyber768      clean shared && sudo cp kyber768/*.so      $LIB_TARGET
make --no-print-directory -s -C kyber768-90s  clean shared && sudo cp kyber768-90s/*.so  $LIB_TARGET
make --no-print-directory -s -C kyber1024     clean shared && sudo cp kyber1024/*.so     $LIB_TARGET
make --no-print-directory -s -C kyber1024-90s clean shared && sudo cp kyber1024-90s/*.so $LIB_TARGET
cd ../../..

###################################
echo "Building McEliece..."
cd mceliece/Reference_Implementation/kem
echo "Building McEliece... <======================== TODO: Both McEliece builds are failing. Need fixing. Skipping."
#make --no-print-directory -s -C mceliece6960119 clean shared && sudo cp mceliece6960119/*.so $LIB_TARGET
#make --no-print-directory -s -C mceliece8192128 clean shared && sudo cp mceliece8192128/*.so $LIB_TARGET
cd ../../..

###################################
echo "Building NewHope..."
cd newhope/Reference_Implementation
make --no-print-directory -s -C newhope512cca  clean shared && sudo cp newhope512cca/*.so  $LIB_TARGET
make --no-print-directory -s -C newhope512cpa  clean shared && sudo cp newhope512cpa/*.so  $LIB_TARGET
make --no-print-directory -s -C newhope1024cca clean shared && sudo cp newhope1024cca/*.so $LIB_TARGET
make --no-print-directory -s -C newhope1024cpa clean shared && sudo cp newhope1024cpa/*.so $LIB_TARGET
cd ../..

###################################
echo "Building NTRU..."
cd ntru/Reference_Implementation/crypto_kem
make --no-print-directory -s -C ntruhps2048509 clean shared && sudo cp ntruhps2048509/*.so $LIB_TARGET
make --no-print-directory -s -C ntruhps2048677 clean shared && sudo cp ntruhps2048677/*.so $LIB_TARGET
make --no-print-directory -s -C ntruhps4096821 clean shared && sudo cp ntruhps4096821/*.so $LIB_TARGET
make --no-print-directory -s -C ntruhrss701    clean shared && sudo cp ntruhrss701/*.so    $LIB_TARGET
cd ../../..

###################################
echo "Building NTS..."
cd nts/Reference_Implementation
make --no-print-directory -s -C nts_kem_12_64  clean copy_cqc shared && sudo cp nts_kem_12_64/lib/*.so  $LIB_TARGET
make --no-print-directory -s -C nts_kem_13_80  clean copy_cqc shared && sudo cp nts_kem_13_80/lib/*.so  $LIB_TARGET
make --no-print-directory -s -C nts_kem_13_136 clean copy_cqc shared && sudo cp nts_kem_13_136/lib/*.so $LIB_TARGET
cd ../..

cd ..

echo "########## Signatures #############"
cd signature

###################################
echo "Building Dilithium..."
cd dilithium/Reference_Implementation/crypto_sign
make --no-print-directory -s -C dilithium1     clean shared && sudo cp dilithium1/*.so     $LIB_TARGET
make --no-print-directory -s -C dilithium1-AES clean shared && sudo cp dilithium1-AES/*.so $LIB_TARGET
make --no-print-directory -s -C dilithium2     clean shared && sudo cp dilithium2/*.so     $LIB_TARGET
make --no-print-directory -s -C dilithium2-AES clean shared && sudo cp dilithium2-AES/*.so $LIB_TARGET
make --no-print-directory -s -C dilithium3     clean shared && sudo cp dilithium3/*.so     $LIB_TARGET
make --no-print-directory -s -C dilithium3-AES clean shared && sudo cp dilithium3-AES/*.so $LIB_TARGET
make --no-print-directory -s -C dilithium4     clean shared && sudo cp dilithium4/*.so     $LIB_TARGET
make --no-print-directory -s -C dilithium4-AES clean shared && sudo cp dilithium4-AES/*.so $LIB_TARGET
cd ../../..

###################################
echo "Building Sphincs..."
cd sphincs/Reference_Implementation/crypto_sign
make --no-print-directory -s -C sphincs-haraka-128f-robust   clean shared && sudo cp sphincs-haraka-128f-robust/*.so   $LIB_TARGET
make --no-print-directory -s -C sphincs-haraka-128s-robust   clean shared && sudo cp sphincs-haraka-128s-robust/*.so   $LIB_TARGET
make --no-print-directory -s -C sphincs-haraka-192f-robust   clean shared && sudo cp sphincs-haraka-192f-robust/*.so   $LIB_TARGET
make --no-print-directory -s -C sphincs-haraka-192s-robust   clean shared && sudo cp sphincs-haraka-192s-robust/*.so   $LIB_TARGET
make --no-print-directory -s -C sphincs-haraka-256f-robust   clean shared && sudo cp sphincs-haraka-256f-robust/*.so   $LIB_TARGET
make --no-print-directory -s -C sphincs-haraka-256s-robust   clean shared && sudo cp sphincs-haraka-256s-robust/*.so   $LIB_TARGET
make --no-print-directory -s -C sphincs-sha256-128f-robust   clean shared && sudo cp sphincs-sha256-128f-robust/*.so   $LIB_TARGET
make --no-print-directory -s -C sphincs-sha256-128s-robust   clean shared && sudo cp sphincs-sha256-128s-robust/*.so   $LIB_TARGET
make --no-print-directory -s -C sphincs-sha256-192f-robust   clean shared && sudo cp sphincs-sha256-192f-robust/*.so   $LIB_TARGET
make --no-print-directory -s -C sphincs-sha256-192s-robust   clean shared && sudo cp sphincs-sha256-192s-robust/*.so   $LIB_TARGET
make --no-print-directory -s -C sphincs-sha256-256f-robust   clean shared && sudo cp sphincs-sha256-256f-robust/*.so   $LIB_TARGET
make --no-print-directory -s -C sphincs-sha256-256s-robust   clean shared && sudo cp sphincs-sha256-256s-robust/*.so   $LIB_TARGET
make --no-print-directory -s -C sphincs-shake256-128f-robust clean shared && sudo cp sphincs-shake256-128f-robust/*.so $LIB_TARGET
make --no-print-directory -s -C sphincs-shake256-128s-robust clean shared && sudo cp sphincs-shake256-128s-robust/*.so $LIB_TARGET
make --no-print-directory -s -C sphincs-shake256-192f-robust clean shared && sudo cp sphincs-shake256-192f-robust/*.so $LIB_TARGET
make --no-print-directory -s -C sphincs-shake256-192s-robust clean shared && sudo cp sphincs-shake256-192s-robust/*.so $LIB_TARGET
make --no-print-directory -s -C sphincs-shake256-256f-robust clean shared && sudo cp sphincs-shake256-256f-robust/*.so $LIB_TARGET
make --no-print-directory -s -C sphincs-shake256-256s-robust clean shared && sudo cp sphincs-shake256-256s-robust/*.so $LIB_TARGET
cd ../../..

# While the following two work fine on MacOS/Clang, they need a bit of work to build on Ubuntu.
# Updated in next build.

###################################
echo "Building Falcon..."
cd falcon/Reference_Implementation
make --no-print-directory -s -C falcon512  clean shared && sudo cp falcon512/*.so  $LIB_TARGET
make --no-print-directory -s -C falcon768  clean shared && sudo cp falcon768/*.so  $LIB_TARGET
make --no-print-directory -s -C falcon1024 clean shared && sudo cp falcon1024/*.so $LIB_TARGET
cd ../..

###################################
echo "Building Picnic..."
cd picnic/Reference_Implementation
make --no-print-directory -s -C picnicl1fs clean shared && sudo cp picnicl1fs/*.so $LIB_TARGET
make --no-print-directory -s -C picnicl1ur clean shared && sudo cp picnicl1ur/*.so $LIB_TARGET
make --no-print-directory -s -C picnicl3fs clean shared && sudo cp picnicl3fs/*.so $LIB_TARGET
make --no-print-directory -s -C picnicl3ur clean shared && sudo cp picnicl3ur/*.so $LIB_TARGET
make --no-print-directory -s -C picnicl5fs clean shared && sudo cp picnicl5fs/*.so $LIB_TARGET
make --no-print-directory -s -C picnicl5ur clean shared && sudo cp picnicl5ur/*.so $LIB_TARGET
cd ../..

cd ..

echo "############# DONE ################"

###################################
sudo chmod -R 755 ${LIB_TARGET}/*.so

cd ..

echo "All Done"