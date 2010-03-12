readPNG <- function(source)
  .Call("read_png", if (is.raw(source)) source else path.expand(source), PACKAGE="png")
