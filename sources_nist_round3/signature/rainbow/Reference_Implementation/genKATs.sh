
run_test()
{
  make clean; make PROJ_DIR=$1
  echo $1
  echo "KAT generating..."
  ./PQCgenKAT_sign
  echo "copy files"
  cp *.req $1
  cp *.rsp $1
  echo "remove files"
  rm *.req *.rsp
  make clean
  return 0
}


### Main script start ###

echo " ================ generate KATs ================"

run_test Ia_Classic
run_test Ia_Circumzenithal
run_test Ia_Compressed
run_test IIIc_Classic
run_test IIIc_Circumzenithal
run_test IIIc_Compressed
run_test Vc_Classic
run_test Vc_Circumzenithal
run_test Vc_Compressed
