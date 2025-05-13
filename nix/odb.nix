{ pkgs }: pkgs.stdenv.mkDerivation {
  name = "odb";
  src = ../.;
  nativeBuildInputs = with pkgs.buildPackages; [
    flex
    bison
    cmake
    makeWrapper
  ];
  buildInputs = with pkgs; [
    llvm
    lld
    lief
    gtest
  ];
  cmakeFlags = [
    "-DCMAKE_BUILD_TYPE=Release"
    #"-DODB_TESTS=OFF"
  ];
  makeTarget = "all";
  enableParallelBuilding = true;
  doCheck = false;
  checkTarget = "odb-tests";
  postFixup = ''
    wrapProgram "$out/x86_64/linux/bin/odb-tests" \
      --prefix LD_LIBRARY_PATH : "$out/x86_64/linux/lib" \
      --prefix LD_LIBRARY_PATH : "${pkgs.llvm}/lib" \
    '';
}

