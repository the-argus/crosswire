{
  linkFarm,
  emscripten,
  ...
}:
linkFarm "emsdk" [
  {
    path = "${emscripten}/share/emscripten/cache/sysroot/include";
    name = "include";
  }
  {
    path = "${emscripten}/share/emscripten/cache/sysroot/bin";
    name = "bin";
  }
  {
    path = "${emscripten}/bin";
    name = "bin";
  }
  {
    path = "${emscripten}/share/emscripten/cache";
    name = "cache";
  }
]
