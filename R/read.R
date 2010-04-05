readPNG <- function(source, native=FALSE)
  .Call("read_png", if (is.raw(source)) source else path.expand(source), native, PACKAGE="png")
