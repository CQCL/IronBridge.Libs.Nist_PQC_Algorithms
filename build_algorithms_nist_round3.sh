#!/bin/sh

# +-----------+-------------+------------+-------------+------------+
# |           | NIST_Round2 | CQC_Round2 | NIST_Round3 | CQC_Round3 |
# +-----------+-------------+------------+-------------+------------+
# | kem       | frodo       | frodoKEM   |             |            |
# |           | kyber       | kyber      | kyber       | kyber      |
# |           | mceliece    | mceliece   | mceliece    | mceliece   |
# |           | newhope     | newhope    |             |            |
# |           | ntru        | ntru       | ntru        | ntru       |
# |           | nts         | nts        |             |            |
# |           |             |            | saber       | saber      |
# +-----------+-------------+------------+-------------+------------+
# | signature | dilithium   | dilithium  | dilithium   | dilithium  |
# |           | falcon      | falcon     | falcon      | falcon     |
# |           | picnic      | picnic     |             |            |
# |           | sphincs     | sphincs    |             |            |
# |           |             |            | rainbow     | rainbow    |
# +-----------+-------------+------------+-------------+------------+
# | support   |             | libkeccak  |             | libkeccak  |
# |           |             | cqcrandom  |             | cqcrandom  |
# +-----------+-------------+------------+-------------+------------+

cd sources_nist_round3

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
# JG: In round2 we built only mceliece6960119 and mceliece8192128
#make --no-print-directory -s -C mceliece348864   clean shared && sudo cp mceliece348864/*.so   $LIB_TARGET
#make --no-print-directory -s -C mceliece348864f  clean shared && sudo cp mceliece348864f/*.so  $LIB_TARGET
#make --no-print-directory -s -C mceliece460896   clean shared && sudo cp mceliece460896/*.so   $LIB_TARGET
#make --no-print-directory -s -C mceliece460896f  clean shared && sudo cp mceliece460896f/*.so  $LIB_TARGET
#make --no-print-directory -s -C mceliece6688128  clean shared && sudo cp mceliece6688128/*.so  $LIB_TARGET
#make --no-print-directory -s -C mceliece6688128f clean shared && sudo cp mceliece6688128f/*.so $LIB_TARGET
make --no-print-directory -s -C mceliece6960119  clean shared && sudo cp mceliece6960119/*.so  $LIB_TARGET
#make --no-print-directory -s -C mceliece6960119f clean shared && sudo cp mceliece6960119f/*.so $LIB_TARGET
make --no-print-directory -s -C mceliece8192128  clean shared && sudo cp mceliece8192128/*.so  $LIB_TARGET
#make --no-print-directory -s -C mceliece8192128f clean shared && sudo cp mceliece8192128f/*.so $LIB_TARGET
cd ../../..

###################################
echo "Building NTRU..."
cd ntru/Reference_Implementation/crypto_kem
make --no-print-directory -s -C ntruhps2048509 clean shared && sudo cp ntruhps2048509/*.so $LIB_TARGET
make --no-print-directory -s -C ntruhps2048677 clean shared && sudo cp ntruhps2048677/*.so $LIB_TARGET
make --no-print-directory -s -C ntruhps4096821 clean shared && sudo cp ntruhps4096821/*.so $LIB_TARGET
make --no-print-directory -s -C ntruhrss701    clean shared && sudo cp ntruhrss701/*.so    $LIB_TARGET
cd ../../..

###################################
echo "Building Saber..."
cd saber/Reference_Implementation_KEM
echo "Building Saber... <======================== TODO: Build files for cqc .so files not yet implemented"
#make --no-print-directory -s clean copy_cqc shared && sudo cp SABER_kem/lib/*.so  $LIB_TARGET
cd ../..

cd ..

echo "########## Signatures #############"
cd signature

###################################
echo "Building Dilithium..."
cd dilithium/Reference_Implementation/crypto_sign
# JG: In round2 we built dilithium[1234] and dilithium[1234]-AES. Round 3 has dropped 1 and 4, added 5 and added an -R and and -AES-R variant for each one.
make --no-print-directory -s -C dilithium2       clean shared_cqc && sudo cp dilithium2/*.so       $LIB_TARGET
make --no-print-directory -s -C dilithium2-AES   clean shared_cqc && sudo cp dilithium2-AES  /*.so $LIB_TARGET
make --no-print-directory -s -C dilithium2-AES-R clean shared_cqc && sudo cp dilithium2-AES-R/*.so $LIB_TARGET
make --no-print-directory -s -C dilithium2-R     clean shared_cqc && sudo cp dilithium2-R/*.so     $LIB_TARGET
make --no-print-directory -s -C dilithium3       clean shared_cqc && sudo cp dilithium3/*.so       $LIB_TARGET
make --no-print-directory -s -C dilithium3-AES   clean shared_cqc && sudo cp dilithium3-AES  /*.so $LIB_TARGET
make --no-print-directory -s -C dilithium3-AES-R clean shared_cqc && sudo cp dilithium3-AES-R/*.so $LIB_TARGET
make --no-print-directory -s -C dilithium3-R     clean shared_cqc && sudo cp dilithium3-R/*.so     $LIB_TARGET
make --no-print-directory -s -C dilithium5       clean shared_cqc && sudo cp dilithium5/*.so       $LIB_TARGET
make --no-print-directory -s -C dilithium5-AES   clean shared_cqc && sudo cp dilithium5-AES  /*.so $LIB_TARGET
make --no-print-directory -s -C dilithium5-AES-R clean shared_cqc && sudo cp dilithium5-AES-R/*.so $LIB_TARGET
make --no-print-directory -s -C dilithium5-R     clean shared_cqc && sudo cp dilithium5-R/*.so     $LIB_TARGET
cd ../../..

###################################
echo "Building Falcon..."
cd falcon/Reference_Implementation
# JG: In round2 we built 512,768,1024. Round 3 has dropped 768.
make --no-print-directory -s -C falcon512/falcon512int   clean shared && sudo cp falcon512/falcon512int/*.so  $LIB_TARGET
make --no-print-directory -s -C falcon1024/falcon1024int clean shared && sudo cp falcon1024/falcon1024int/*.so $LIB_TARGET
cd ../..

###################################
echo "Building Rainbow..."
cd rainbow/Reference_Implementation
echo "Building Rainbow... <======================== TODO: Build files for cqc .so files not yet implemented"
#make --no-print-directory -s clean shared && sudo cp *.so $LIB_TARGET
cd ../..

cd ..

echo "############# DONE ################"

###################################
sudo chmod -R 755 ${LIB_TARGET}/*.so

cd ..

echo "All Done"